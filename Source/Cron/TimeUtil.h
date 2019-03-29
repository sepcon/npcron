#ifndef TIMEUTIL_H
#define TIMEUTIL_H

#include "chrono.h"
#include <string>

namespace Cron {


class TimeUtil
{
public:
    static unsigned int dayEndOfMonth(unsigned int month, int year);
	static tm localTime();
    static int getWeekDayOf(int year, int mon, int day);
    static bool isLeapYear(int year);
    static std::tm createTimeInfo(int year, int month, int mDay = 0, int hour = 0, int min = 0, int sec = 0);
    static std::string toCTime(std::time_t tmt);
    template<class Theclock>
    static std::string toCTime(typename Theclock::time_point tp);
};

template<class Theclock>
std::string TimeUtil::toCTime(typename Theclock::time_point tp)
{
    auto tmt = Theclock::to_time_t(tp);
    return TimeUtil::toCTime(tmt);
}

}
#endif // TIMEUTIL_H
