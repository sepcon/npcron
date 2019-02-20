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

void replace(std::string& word, const std::string& with, const std::string& by)
{
	size_t pos = 0;
	std::vector<size_t> foundPoses;
	do
	{
		size_t foundPos = word.find(with, pos);
		if (foundPos != std::string::npos)
		{
			foundPoses.push_back(foundPos);
			pos = foundPos + with.size();
		}
		else
		{
			break;
		}
	} while (pos < word.size());
	word.reserve(word.size() + foundPoses.size() * by.size());
    for (int i = static_cast<int>(foundPoses.size()) - 1; i >= 0; --i)
	{
		word.replace(foundPoses[i], with.size(), by);
	}
}

#include <regex>

int main()
{
    try
    {
        Cron::Parser parser("0 0 19 5 6,0");
        Cron::Clock clock = parser.createClock();
        for(int i = 0; i < 10; ++i)
        {
            printTime(clock.getNext());
        }
    }
    catch(Cron::BadSyntaxException& e)
    {
        std::cout << e << std::endl;
    }
    return 0;
}
