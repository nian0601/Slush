#include "stdafx.h"
#include "Core/Profiler.h"
#include <cmath>

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
}
