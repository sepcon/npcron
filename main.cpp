#define _CRT_SECURE_NO_WARNINGS

#include "Cron/Clock.h"
#include "Cron/TimeUtil.h"
#include "Cron/Parser.h"
#include <iostream>
#include <string>
#include <algorithm>

using namespace std;

void printTime(std::chrono::system_clock::time_point tp)
{
    auto timeT = Cron::Clock::sysclock::to_time_t(tp);
    std::cout << std::ctime(&timeT) << std::endl;
}

void printWeekDay(int year, int mon, int day)
{
    std::tm tmTime = Cron::TimeUtil::createTimeInfo(year - 1900, mon, day);
    auto timet = std::mktime(&tmTime);
    auto ptmTime = std::localtime(&timet);
    if(ptmTime)
    {
        std::cout << "week day = " << ptmTime->tm_wday << std::endl;
    }
}

int main()
{
    try
    {
        Cron::Clock scheduler = Cron::Parser("5 4 * * *").createClock();
        for(int i = 0; i < 70; ++i)
        {
            printTime(scheduler.getNext());
        }
    }
    catch(Cron::BadSyntaxException& e)
    {
        std::cout << e << std::endl;
    }
    return 0;
}
