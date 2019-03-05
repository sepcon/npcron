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

using namespace std::chrono;

int main()
{
    std::string expression;
    Cron::Parser parser;
    expression = "0 0 30 2 *";
    std::cout << "Your expression is: " << expression << std::endl;
    try
    {
        parser.parse(expression);
        auto clock = parser.createClock();
//        for(int i = 0; i < 5; ++i )
//        {
//            printTime(clock.getBack(false));
//        }
//        clock.syncWithLocalTime();
        for(int i = 0; i < 50; ++i )
        {
            printTime(clock.getNext(false));
        }
    }
    catch(const Cron::BadSyntaxException& s)
    {
        std::cout << s << std::endl;
    }



    return 0;
}
