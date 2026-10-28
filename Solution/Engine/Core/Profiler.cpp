#include "stdafx.h"
#include "Core/Profiler.h"
#include <cmath>
#include <share.h>

namespace Slush
{
	Profiler::Profiler()
	{
		myHistogram.Fill(0);
		myHistory.Respace(HistorySize);
		myMarkers.Respace(MaxMarkers);
	}

	Profiler& Profiler::GetInstance()
	{
		static Profiler instance;
		return instance;
	}

	Profiler::~Profiler()
	{
		if (myLiveFile)
			fclose(myLiveFile);
	}

	int Profiler::RegisterMarker(const char* aName)
	{
		for (int i = 0; i < myMarkers.Count(); ++i)
			if (myMarkers[i].myName == aName)
				return i;
		FW_ASSERT(myMarkers.Count() < MaxMarkers, "Profiler marker table is full");
		Marker& marker = myMarkers.Add();
		marker.myName = aName;
		return myMarkers.Count() - 1;
	}

	void Profiler::BeginFrame(Time::TimeUnit aTime)
	{
		FW_ASSERT(!myInFrame && myStackCount == 0, "Unbalanced profiler frame");
		myFrameStart = aTime;
		myInFrame = true;
		// Completed markers outside a frame remain pending and belong to the next
		// frame. Their per-call stats are recorded immediately, even during loading.
	}

	void Profiler::BeginMarker(int anIndex, Time::TimeUnit aTime)
	{
		FW_ASSERT(anIndex >= 0 && anIndex < myMarkers.Count(), "Invalid profiler marker");
		FW_ASSERT(myStackCount < MaxMarkers, "Profiler nesting overflow");
		StackEntry& entry = myStack[myStackCount++];
		entry.myIndex = anIndex;
		entry.myStart = aTime;
		entry.myChildren = 0;
	}

	void Profiler::EndMarker(int anIndex, Time::TimeUnit aTime)
	{
		FW_ASSERT(myStackCount > 0, "Profiler stack underflow");
		const StackEntry& entry = myStack[--myStackCount];
		FW_ASSERT(entry.myIndex == anIndex && aTime >= entry.myStart, "Unbalanced profiler marker");
		const Time::TimeUnit elapsed = aTime - entry.myStart;
		const double inclusive = elapsed / 1000.0;
		const double self = (elapsed - entry.myChildren) / 1000.0;
		if (myStackCount)
			myStack[myStackCount - 1].myChildren += elapsed;
		Marker& marker = myMarkers[anIndex];
		++marker.myCalls;
		marker.myInclusiveMs += inclusive;
		marker.myMaxCallMs = FW_Max(marker.myMaxCallMs, inclusive);
		Sample& sample = myPending[anIndex];
		++sample.myCalls;
		sample.myInclusiveMs += inclusive;
		sample.mySelfMs += self;
	}

	void Profiler::EndFrame(Time::TimeUnit aTime)
	{
		FW_ASSERT(myInFrame && myStackCount == 0 && aTime >= myFrameStart, "Unbalanced profiler frame");
		const double ms = (aTime - myFrameStart) / 1000.0;
		double rolling = 0;
		if (myFrameCount >= 60)
		{
			for (int i = 1; i <= 60; ++i)
				rolling += myHistory[static_cast<int>((myFrameCount - i) % HistorySize)].myMs;
			rolling /= 60;
		}
		const int slot = static_cast<int>(myFrameCount % HistorySize);
		if (myHistory.Count() < HistorySize)
			myHistory.Add();
		Frame& frame = myHistory[slot];
		frame.myIndex = myFrameCount;
		frame.myMs = ms;
		for (int i = 0; i < myMarkers.Count(); ++i)
		{
			const Sample& sample = myPending[i];
			frame.myMarkers[i] = sample;
			Marker& marker = myMarkers[i];
			marker.myFrameInclusiveMs += sample.myInclusiveMs;
			marker.myFrameSelfMs += sample.mySelfMs;
			marker.myMaxFrameInclusiveMs = FW_Max(marker.myMaxFrameInclusiveMs, sample.myInclusiveMs);
			marker.myMaxFrameSelfMs = FW_Max(marker.myMaxFrameSelfMs, sample.mySelfMs);
			marker.myMaxCallsPerFrame = FW_Max(marker.myMaxCallsPerFrame, sample.myCalls);
		}
		myDurationMs += ms;
		if (myFrameCount == 0 || ms < myMinMs)
			myMinMs = ms;
		myMaxMs = FW_Max(myMaxMs, ms);
		const int bucket = ms >= 1000 ? 10000 : static_cast<int>(ms * 10);
		++myHistogram[bucket];
		if (myFrameCount >= 60 && ms > 8 && ms > rolling * 2)
		{
			Spike& spike = mySpikes.Add();
			spike = Spike();
			spike.myFrameIndex = myFrameCount;
			spike.myEngineSeconds = myDurationMs / 1000;
			spike.myFrameMs = ms;
			spike.myRollingMs = rolling;
			spike.myGameState = myGameState;
			for (int i = 0; i < myMarkers.Count(); ++i)
			{
				if (!myPending[i].myCalls)
					continue;
				for (int j = 0; j < 5; ++j)
				{
					if (spike.myTop[j].myIndex < 0 || myPending[i].mySelfMs > spike.myTop[j].mySample.mySelfMs)
					{
						for (int k = 4; k > j; --k)
							spike.myTop[k] = spike.myTop[k - 1];
						spike.myTop[j].myIndex = i;
						spike.myTop[j].mySample = myPending[i];
						break;
					}
				}
			}
		}
		++myFrameCount;
		myPending.Fill(Sample());
		myInFrame = false;
	}

	double Profiler::GetPercentile(double aPercentile) const
	{
		if (!myFrameCount)
			return 0;
		const unsigned long long target = static_cast<unsigned long long>(ceil(aPercentile * myFrameCount / 100));
		unsigned long long count = 0;
		for (int i = 0; i <= 10000; ++i)
		{
			count += myHistogram[i];
			if (count >= target)
				return i == 10000 ? myMaxMs : (i + 1) / 10.0;
		}
		return myMaxMs;
	}

	FW_GrowingArray<int> Profiler::GetRanking(Ranking aRanking, int aCount) const
	{
		FW_GrowingArray<int> result;
		auto score = [&](int anIndex)
		{
			const Marker& marker = myMarkers[anIndex];
			if (aRanking == Ranking::WorstSingleCall)
				return marker.myMaxCallMs;
			if (aRanking == Ranking::HeaviestPerFrameAvg)
				return myFrameCount ? marker.myFrameSelfMs / myFrameCount : 0;
			return marker.myMaxFrameInclusiveMs;
		};
		for (int i = 0; i < myMarkers.Count(); ++i)
		{
			if (!myMarkers[i].myCalls)
				continue;
			int position = 0;
			while (position < result.Count() && score(result[position]) >= score(i))
				++position;
			if (position >= aCount)
				continue;
			result.Add(i);
			for (int j = result.Count() - 1; j > position; --j)
				result[j] = result[j - 1];
			result[position] = i;
			if (result.Count() > aCount)
				result.RemoveLast();
		}
		return result;
	}

	bool Profiler::StartReport(const char* aPath)
	{
		FW_ASSERT(myStackCount == 0 && !myInFrame, "Cannot start a report inside a scope");
		// Keep indices stable for cached macro sites, but discard measurements from
		// startup unit tests so they cannot pollute the game's report.
		for (Marker& marker : myMarkers)
		{
			FW_String name = marker.myName;
			marker = Marker();
			marker.myName = name;
		}
		myFrameCount = 0;
		myDurationMs = 0;
		myMinMs = 0;
		myMaxMs = 0;
		myHistogram.Fill(0);
		myPending.Fill(Sample());
		myHistory.RemoveAll();
		mySpikes.RemoveAll();
		myGameState = "startup";
		if (myLiveFile)
			fclose(myLiveFile);
		myLiveFile = nullptr;
		myReportPath = aPath;
		// StartReport receives the live .txt path; clean shutdown writes its HTML sibling.
		myHtmlReportPath = myReportPath.SubStr(0, myReportPath.Length() - 4) + ".html";
		remove(myHtmlReportPath.GetBuffer()); // Never present a previous run as this run's completed report.
		myWrittenSpikes = 0;
		myLiveFile = _fsopen(aPath, "w", _SH_DENYWR);
		if (!myLiveFile)
			return false;
		fprintf(myLiveFile, "PERF_RESULT: INCOMPLETE\n[Spikes]\n");
		return fflush(myLiveFile) == 0;
	}

	void Profiler::WriteSpike(FILE* aFile, const Spike& aSpike) const
	{
		fprintf(aFile, "frame=%llu time_s=%.3f frame_ms=%.3f rolling_ms=%.3f gamestate=%s",
			aSpike.myFrameIndex, aSpike.myEngineSeconds, aSpike.myFrameMs, aSpike.myRollingMs, aSpike.myGameState.GetBuffer());
		for (const SpikeMarker& top : aSpike.myTop)
		{
			if (top.myIndex >= 0)
				fprintf(aFile, " marker=%s self_ms=%.3f inclusive_ms=%.3f calls=%llu", myMarkers[top.myIndex].myName.GetBuffer(),
					top.mySample.mySelfMs, top.mySample.myInclusiveMs, top.mySample.myCalls);
		}
		fprintf(aFile, "\n");
	}

	void Profiler::AppendNewSpikes()
	{
		if (!myLiveFile)
			return;
		while (myWrittenSpikes < mySpikes.Count())
			WriteSpike(myLiveFile, mySpikes[myWrittenSpikes++]);
		fflush(myLiveFile);
	}

	bool Profiler::FinishReport()
	{
		if (myLiveFile)
			fclose(myLiveFile);
		myLiveFile = nullptr;
		return !myHtmlReportPath.Empty() && WriteReport(myHtmlReportPath.GetBuffer());
	}

	void Profiler::WriteHtmlText(FILE* aFile, const char* aText)
	{
		for (const char* character = aText; *character; ++character)
		{
			switch (*character)
			{
			case '&': fputs("&amp;", aFile); break;
			case '<': fputs("&lt;", aFile); break;
			case '>': fputs("&gt;", aFile); break;
			case '"': fputs("&quot;", aFile); break;
			case '\'': fputs("&#39;", aFile); break;
			default: fputc(*character, aFile); break;
			}
		}
	}

	void Profiler::WriteMarkerHeader(FILE* aFile) const
	{
		fputs("<table><thead><tr><th>Marker</th><th>Calls</th><th>Calls/frame</th><th>Max calls/frame</th>"
			"<th>Avg call (ms)</th><th>Max call (ms)</th><th>Avg frame inclusive (ms)</th>"
			"<th>Avg frame self (ms)</th><th>Max frame inclusive (ms)</th><th>Max frame self (ms)</th>"
			"</tr></thead><tbody>\n", aFile);
	}

	void Profiler::WriteMarker(FILE* aFile, int anIndex) const
	{
		const Marker& marker = myMarkers[anIndex];
		const double frames = myFrameCount ? static_cast<double>(myFrameCount) : 1;
		fputs("<tr><td>", aFile);
		WriteHtmlText(aFile, marker.myName.GetBuffer());
		fprintf(aFile, "</td><td>%llu</td><td>%.3f</td><td>%llu</td><td>%.6f</td><td>%.6f</td><td>%.6f</td><td>%.6f</td><td>%.6f</td><td>%.6f</td></tr>\n", marker.myCalls,
			marker.myCalls / frames, marker.myMaxCallsPerFrame, marker.myCalls ? marker.myInclusiveMs / marker.myCalls : 0,
			marker.myMaxCallMs, marker.myFrameInclusiveMs / frames, marker.myFrameSelfMs / frames,
			marker.myMaxFrameInclusiveMs, marker.myMaxFrameSelfMs);
	}

	bool Profiler::WriteReport(const char* aPath) const
	{
		FILE* file = nullptr;
		fopen_s(&file, aPath, "w");
		if (!file)
			return false;
		fputs("<!doctype html>\n<html lang=\"en\"><head><meta charset=\"utf-8\">"
			"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
			"<title>Slush performance report</title><style>"
			"body{font-family:system-ui,sans-serif;margin:2rem;color:#222;background:#fff}"
			"table{border-collapse:collapse;margin-bottom:2rem;font-variant-numeric:tabular-nums}"
			"th,td{border:1px solid #ccc;padding:.4rem .65rem;text-align:right}"
			"th{background:#e8edf2}td:first-child,th:first-child{text-align:left}"
			"tbody tr:nth-child(even){background:#f5f7f9}pre{white-space:pre-wrap}"
			"</style></head><body><h1>Performance report</h1>\n", file);
		fprintf(file, "<pre>PERF_RESULT: frames=%llu duration=%.3fs avg_ms=%.3f p99_ms=%.3f max_ms=%.3f spikes=%d</pre>\n",
			myFrameCount, myDurationMs / 1000, GetAverageMs(), GetPercentile(99), myMaxMs, mySpikes.Count());
		fprintf(file, "<h2 id=\"Frame\">Frame statistics</h2>\n<table><thead><tr><th>Min (ms)</th><th>Avg (ms)</th><th>Max (ms)</th>"
			"<th>p50 (ms)</th><th>p95 (ms)</th><th>p99 (ms)</th><th>FPS</th></tr></thead><tbody>"
			"<tr><td>%.3f</td><td>%.3f</td><td>%.3f</td><td>%.3f</td><td>%.3f</td><td>%.3f</td><td>%.3f</td></tr></tbody></table>\n",
			myMinMs, GetAverageMs(), myMaxMs, GetPercentile(50), GetPercentile(95), GetPercentile(99), myDurationMs > 0 ? myFrameCount * 1000 / myDurationMs : 0);
		fputs("<h2 id=\"Spikes\">Spikes</h2>\n", file);
		if (mySpikes.IsEmpty())
			fputs("<p>No spikes recorded.</p>\n", file);
		for (const Spike& spike : mySpikes)
		{
			fprintf(file, "<table><thead><tr><th>Frame</th><th>Time (s)</th><th>Frame (ms)</th><th>Rolling avg (ms)</th><th>Gamestate</th>"
				"</tr></thead><tbody><tr><td>%llu</td><td>%.3f</td><td>%.3f</td><td>%.3f</td><td>",
				spike.myFrameIndex, spike.myEngineSeconds, spike.myFrameMs, spike.myRollingMs);
			WriteHtmlText(file, spike.myGameState.GetBuffer());
			fputs("</td></tr></tbody></table>\n<table><thead><tr><th>Top markers by self time</th><th>Self (ms)</th><th>Inclusive (ms)</th><th>Calls</th></tr></thead><tbody>\n", file);
			for (const SpikeMarker& top : spike.myTop)
			{
				if (top.myIndex < 0)
					continue;
				fputs("<tr><td>", file);
				WriteHtmlText(file, myMarkers[top.myIndex].myName.GetBuffer());
				fprintf(file, "</td><td>%.3f</td><td>%.3f</td><td>%llu</td></tr>\n", top.mySample.mySelfMs, top.mySample.myInclusiveMs, top.mySample.myCalls);
			}
			fputs("</tbody></table>\n", file);
		}
		const char* sections[] = { "WorstSingleCall", "HeaviestPerFrameAvg", "WorstFrameContribution" };
		const char* titles[] = { "Worst single call", "Heaviest average per-frame self time", "Worst single-frame contribution" };
		const Ranking rankings[] = { Ranking::WorstSingleCall, Ranking::HeaviestPerFrameAvg, Ranking::WorstFrameContribution };
		for (int i = 0; i < 3; ++i)
		{
			fprintf(file, "<h2 id=\"%s\">%s (top 10)</h2>\n", sections[i], titles[i]);
			WriteMarkerHeader(file);
			for (int index : GetRanking(rankings[i], 10))
				WriteMarker(file, index);
			fputs("</tbody></table>\n", file);
		}
		fputs("<h2 id=\"AllMarkers\">All markers</h2>\n", file);
		WriteMarkerHeader(file);
		for (int i = 0; i < myMarkers.Count(); ++i)
			WriteMarker(file, i);
		fputs("</tbody></table>\n</body></html>\n", file);
		const bool passed = ferror(file) == 0;
		return fclose(file) == 0 && passed;
	}
}
