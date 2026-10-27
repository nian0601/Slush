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
		if (myLiveFile)
			fclose(myLiveFile);
		myLiveFile = nullptr;
		myReportPath = aPath;
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
		return !myReportPath.Empty() && WriteReport(myReportPath.GetBuffer());
	}

	void Profiler::WriteMarkerHeader(FILE* aFile) const
	{
		fprintf(aFile, "name calls calls_per_frame max_calls_per_frame avg_call_ms max_call_ms avg_frame_inclusive_ms avg_frame_self_ms max_frame_inclusive_ms max_frame_self_ms\n");
	}

	void Profiler::WriteMarker(FILE* aFile, int anIndex) const
	{
		const Marker& marker = myMarkers[anIndex];
		const double frames = myFrameCount ? static_cast<double>(myFrameCount) : 1;
		fprintf(aFile, "%s %llu %.3f %llu %.6f %.6f %.6f %.6f %.6f %.6f\n", marker.myName.GetBuffer(), marker.myCalls,
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
		fprintf(file, "PERF_RESULT: frames=%llu duration=%.3fs avg_ms=%.3f p99_ms=%.3f max_ms=%.3f spikes=%d\n",
			myFrameCount, myDurationMs / 1000, GetAverageMs(), GetPercentile(99), myMaxMs, mySpikes.Count());
		fprintf(file, "[Frame]\nmin_ms=%.3f avg_ms=%.3f max_ms=%.3f p50_ms=%.3f p95_ms=%.3f p99_ms=%.3f fps=%.3f\n",
			myMinMs, GetAverageMs(), myMaxMs, GetPercentile(50), GetPercentile(95), GetPercentile(99), myDurationMs > 0 ? myFrameCount * 1000 / myDurationMs : 0);
		fprintf(file, "[Spikes]\n");
		for (const Spike& spike : mySpikes)
			WriteSpike(file, spike);
		const char* sections[] = { "[WorstSingleCall]", "[HeaviestPerFrameAvg]", "[WorstFrameContribution]" };
		const Ranking rankings[] = { Ranking::WorstSingleCall, Ranking::HeaviestPerFrameAvg, Ranking::WorstFrameContribution };
		for (int i = 0; i < 3; ++i)
		{
			fprintf(file, "%s\n", sections[i]);
			WriteMarkerHeader(file);
			for (int index : GetRanking(rankings[i], 10))
				WriteMarker(file, index);
		}
		fprintf(file, "[AllMarkers]\n");
		WriteMarkerHeader(file);
		for (int i = 0; i < myMarkers.Count(); ++i)
			WriteMarker(file, i);
		const bool passed = ferror(file) == 0;
		return fclose(file) == 0 && passed;
	}
}
