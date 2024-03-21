#include "stdafx.h"
#include "Time.h"

#include <windows.h>

namespace Slush
{
	typedef unsigned long long TimeUnit;
	TimeUnit Time::myLastTime;
	TimeUnit Time::myCurrentTime;
	TimeUnit Time::myFrequency;

	void Time::Init()
	{
		LARGE_INTEGER largeInt;
		QueryPerformanceFrequency(&largeInt);

		myFrequency = largeInt.QuadPart;

		QueryPerformanceCounter(&largeInt);
		myCurrentTime = largeInt.QuadPart * 1000000 / myFrequency;
		myLastTime = myCurrentTime;
	}

	void Time::Update()
	{
		myLastTime = myCurrentTime;
		myCurrentTime = GetCurrentExactTime();
	}

	void Time::ClampToFramerate(float aTargetFramerate)
	{
		TimeUnit targetTimeUnit = ConvertGameTimeToTimeUnit(aTargetFramerate);
		TimeUnit elasped = GetCurrentExactTime() - myLastTime;

		while (elasped < targetTimeUnit)
			elasped = GetCurrentExactTime() - myLastTime;
	}

	float Time::GetDelta()
	{
		return FW_Min(0.1f, ConvertTimeUnitToGameTime(myCurrentTime - myLastTime));
	}

	TimeUnit Time::GetTime()
	{
		return myCurrentTime;
	}

	TimeUnit Time::GetCurrentExactTime()
	{
		LARGE_INTEGER largeInt;
		QueryPerformanceCounter(&largeInt);
		return largeInt.QuadPart * 1000000 / myFrequency;
	}

	float Time::ConvertTimeUnitToGameTime(TimeUnit aTimeUnit)
	{
		return aTimeUnit / 1000000.f;
	}

	TimeUnit Time::ConvertGameTimeToTimeUnit(float aGameTime)
	{
		return static_cast<TimeUnit>(aGameTime * 1000000.f);
	}

	//////////////////////////////////////////////////////////////////////////

	void Timer::Start(float aDuration)
	{
		myExpireTime = Time::GetTime() + Time::ConvertGameTimeToTimeUnit(aDuration);
	}

	bool Timer::IsStarted() const
	{
		return myExpireTime != 0;
	}

	bool Timer::HasExpired() const
	{
		if (myExpireTime == 0)
			return false;

		return myExpireTime < Time::GetTime();
	}

}