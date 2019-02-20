#include "TimeUtil.h"

namespace Cron {

std::tm* TimeUtil::localTime()
{
	std::time_t ttNow;
	std::time(&ttNow);

	return std::localtime(&ttNow);
}

int TimeUtil::getWeekDayOf(int year, int mon, int day)
{
    std::tm tmTime = Cron::TimeUtil::createTimeInfo(year, mon, day);
    auto timet = std::mktime(&tmTime);
    auto ptmTime = std::localtime(&timet);
    return ptmTime->tm_wday;
}

bool TimeUtil::isLeapYear(int year)
{
    return year % 4 == 0;
}

tm TimeUtil::createTimeInfo(int year, int month, int mDay, int hour, int min, int sec)
{
    return std::tm {
        sec,        // int tm_sec;
        min,        // int tm_min;
        hour,       // int tm_hour;
        mDay,       // int tm_mday;
        month,      // int tm_mon;
        year,       // int tm_year;
        0,          // int tm_wday;
        0,          // int tm_yday;
        0,          // int tm_isdst;
    };
}

}
