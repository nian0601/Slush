#pragma once

namespace Slush
{
	class Time
	{
	public:
		typedef unsigned long long TimeUnit;

		static void Init();
		static void Update();

		static float GetDelta();
		static void ClampToFramerate(float aTargetFramerate);

		static TimeUnit GetTime();
		static TimeUnit GetCurrentExactTime();

		static float ConvertTimeUnitToGameTime(TimeUnit aTimeUnit);
		static TimeUnit ConvertGameTimeToTimeUnit(float aGameTime);

	private:
		static TimeUnit myLastTime;
		static TimeUnit myCurrentTime;
		static TimeUnit myFrequency;
	};

	class Timer
	{
	public:
		void Start(float aDuration);

		bool IsStarted() const;
		bool HasExpired() const;

	private:
		Time::TimeUnit myExpireTime = 0;
	};
}