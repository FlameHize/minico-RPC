#include "../include/mstime.h"
#include <sys/time.h>

using namespace minico;

Time Time::now()
{
	struct timeval tv;
	::gettimeofday(&tv, 0);
	int64_t seconds = tv.tv_sec;
	return Time(seconds * 1000 + tv.tv_usec / 1000);
}

time_t Time::nowSec()
{
	struct timeval tv;
	::gettimeofday(&tv, 0);
	return tv.tv_sec;
}

void Time::toLocalTime(time_t second, long timezone, struct tm* tm_time)
{
	uint32_t n32_Pass4year;
	int32_t n32_hpery;

	second = second - timezone;

	if (second < 0)
	{
		second = 0;
	}
	tm_time->tm_sec = (int)(second % 60);
	second /= 60;
	tm_time->tm_min = (int)(second % 60);
	second /= 60;
	n32_Pass4year = ((unsigned int)second / (1461L * 24L));
	tm_time->tm_year = (n32_Pass4year << 2) + 70;
	second %= 1461L * 24L;
	for (;;)
	{
		n32_hpery = 365 * 24;
		if ((tm_time->tm_year & 3) == 0)
		{
			n32_hpery += 24;
		}
		if (second < n32_hpery)
		{
			break;
		}
		tm_time->tm_year++;
		second -= n32_hpery;
	}
	tm_time->tm_hour = (int)(second % 24);
	second /= 24;
	second++;
	if ((tm_time->tm_year & 3) == 0)
	{
		if (second > 60)
		{
			second--;
		}
		else
		{
			if (second == 60)
			{
				tm_time->tm_mon = 1;
				tm_time->tm_mday = 29;
				return;
			}
		}
	}
	for (tm_time->tm_mon = 0; days[tm_time->tm_mon] < second; tm_time->tm_mon++)
	{
		second -= days[tm_time->tm_mon];
	}

	tm_time->tm_mday = (int)(second);

	return;
}

struct timespec Time::timeIntervalFromNow()
{
	struct timespec ts;
	int64_t microseconds = _timeVal - Time::now().getTimeVal();
	if (microseconds < 1)
	{
		ts.tv_sec = 0;
		ts.tv_nsec = 1000;
	}
	else 
	{
		ts.tv_sec = static_cast<time_t>(
			microseconds / 1000);
		ts.tv_nsec = static_cast<long>(
			(microseconds % 1000) * 1000 * 1000);
	}
	return ts;
}