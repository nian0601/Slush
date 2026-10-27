#pragma once

#include "Core/Time.h"
#include <FW_StaticArray.h>
#include <FW_GrowingArray.h>
#include <FW_String.h>

namespace Slush
{
	// Main-thread only. Timestamps are microseconds, like Time::TimeUnit.
	class Profiler
	{
	public:
		static const int MaxMarkers = 256;
		static const int HistorySize = 600;
		struct Sample
		{
			unsigned long long myCalls = 0;
			double myInclusiveMs = 0;
			double mySelfMs = 0;
		};
		struct Marker
		{
			FW_String myName;
			unsigned long long myCalls = 0;
			unsigned long long myMaxCallsPerFrame = 0;
			double myInclusiveMs = 0;
			double myMaxCallMs = 0;
			double myFrameInclusiveMs = 0;
			double myFrameSelfMs = 0;
			double myMaxFrameInclusiveMs = 0;
			double myMaxFrameSelfMs = 0;
		};
		struct Frame
		{
			unsigned long long myIndex = 0;
			double myMs = 0;
			FW_StaticArray<Sample, MaxMarkers> myMarkers;
		};
		struct SpikeMarker
		{
			int myIndex = -1;
			Sample mySample;
		};
		struct Spike
		{
			unsigned long long myFrameIndex = 0;
			double myEngineSeconds = 0;
			double myFrameMs = 0;
			double myRollingMs = 0;
			FW_String myGameState;
			FW_StaticArray<SpikeMarker, 5> myTop;
		};
		enum class Ranking { WorstSingleCall, HeaviestPerFrameAvg, WorstFrameContribution };

		Profiler();
		static Profiler& GetInstance();
		int RegisterMarker(const char* aName);
		void BeginFrame(Time::TimeUnit aTime);
		void EndFrame(Time::TimeUnit aTime);
		void BeginMarker(int anIndex, Time::TimeUnit aTime);
		void EndMarker(int anIndex, Time::TimeUnit aTime);
		void SetGameStateName(const char* aName) { myGameState = aName; }
		const FW_GrowingArray<Marker>& GetMarkers() const { return myMarkers; }
		const FW_GrowingArray<Frame>& GetHistory() const { return myHistory; }
		const FW_GrowingArray<Spike>& GetSpikes() const { return mySpikes; }
		unsigned long long GetFrameCount() const { return myFrameCount; }
		double GetDurationMs() const { return myDurationMs; }
		double GetMinMs() const { return myMinMs; }
		double GetMaxMs() const { return myMaxMs; }
		double GetAverageMs() const { return myFrameCount ? myDurationMs / myFrameCount : 0; }
		double GetPercentile(double aPercentile) const;
		FW_GrowingArray<int> GetRanking(Ranking aRanking, int aCount) const;

	private:
		struct StackEntry
		{
			int myIndex = -1;
			Time::TimeUnit myStart = 0;
			Time::TimeUnit myChildren = 0;
		};
		FW_GrowingArray<Marker> myMarkers;
		FW_GrowingArray<Frame> myHistory;
		FW_GrowingArray<Spike> mySpikes;
		FW_StaticArray<Sample, MaxMarkers> myPending;
		FW_StaticArray<StackEntry, MaxMarkers> myStack;
		FW_StaticArray<unsigned long long, 10001> myHistogram;
		int myStackCount = 0;
		unsigned long long myFrameCount = 0;
		Time::TimeUnit myFrameStart = 0;
		bool myInFrame = false;
		double myDurationMs = 0;
		double myMinMs = 0;
		double myMaxMs = 0;
		FW_String myGameState = "startup";
	};

	class ProfileScope
	{
	public:
		explicit ProfileScope(int anIndex) : myIndex(anIndex)
		{
			Profiler::GetInstance().BeginMarker(myIndex, Time::GetCurrentExactTime());
		}
		~ProfileScope() { Profiler::GetInstance().EndMarker(myIndex, Time::GetCurrentExactTime()); }
		ProfileScope(const ProfileScope&) = delete;
		ProfileScope& operator=(const ProfileScope&) = delete;
	private:
		int myIndex;
	};
}

#define SLUSH_PROFILE_JOIN_IMPL(aLeft, aRight) aLeft##aRight
#define SLUSH_PROFILE_JOIN(aLeft, aRight) SLUSH_PROFILE_JOIN_IMPL(aLeft, aRight)
#define SLUSH_PROFILE_SCOPE(aName) \
	static const int SLUSH_PROFILE_JOIN(profileIndex, __LINE__) = Slush::Profiler::GetInstance().RegisterMarker(aName); \
	Slush::ProfileScope SLUSH_PROFILE_JOIN(profileScope, __LINE__)(SLUSH_PROFILE_JOIN(profileIndex, __LINE__))
#define SLUSH_PROFILE_FUNCTION() SLUSH_PROFILE_SCOPE(__FUNCTION__)
