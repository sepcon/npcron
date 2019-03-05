#ifndef TIMEUTIL_H
#define TIMEUTIL_H

#include "chrono.h"

namespace Cron {


class TimeUtil
{
public:
    static unsigned int dayEndOfMonth(unsigned int month, int year);
	static std::tm* localTime();
    static int getWeekDayOf(int year, int mon, int day);
    static bool isLeapYear(int year);
    static std::tm createTimeInfo(int year, int month, int mDay = 0, int hour = 0, int min = 0, int sec = 0);
};

}
#endif // TIMEUTIL_H
