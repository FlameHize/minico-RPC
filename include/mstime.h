#pragma once
#include <stdint.h>
#include <time.h>

struct timespec;

namespace minico
{

	const char days[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

	class Time
	{
	public:
		Time(int64_t msSinceEpoch) : _timeVal(msSinceEpoch) {}

		Time(const Time& time) { _timeVal = time._timeVal; }

		Time(const Time&& time) { _timeVal = time._timeVal; }

		Time& operator=(const Time& time)
		{
			_timeVal = time._timeVal;
			return *this;
		}

		~Time() {}

		static Time now();

		static time_t nowSec();

		static void toLocalTime(time_t second, long timezone, struct tm* tm_time);

		struct timespec timeIntervalFromNow();

		int64_t getTimeVal() { return _timeVal; }

	private:
		int64_t _timeVal;
	};

	inline bool operator < (Time lhs, Time rhs)
	{
		return lhs.getTimeVal() < rhs.getTimeVal();
	}

	inline bool operator <= (Time lhs, Time rhs)
	{
		return lhs.getTimeVal() <= rhs.getTimeVal();
	}

	inline bool operator > (Time lhs, Time rhs)
	{
		return lhs.getTimeVal() > rhs.getTimeVal();
	}

	inline bool operator >= (Time lhs, Time rhs)
	{
		return lhs.getTimeVal() >= rhs.getTimeVal();
	}

	inline bool operator == (Time lhs, Time rhs)
	{
		return lhs.getTimeVal() == rhs.getTimeVal();
	}

}
