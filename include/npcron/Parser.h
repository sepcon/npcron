#pragma once

#include <npcron/Clock.h>
#include <npcron/CronValidator.h>
#include <string>
#include <set>

#define MIGHT_THROW(exception, ...)

namespace Cron
{

using ImpossibleValueException = std::string;

class Parser
{
public:
    Parser(const std::string& expression = "");
    void parse(const std::string& expression);
    Cron::Clock createClock();
private:
    void reset();
    TimeUnit::PossibleValues _cronFieldValues[FieldType::cfFieldCount];
};

}
