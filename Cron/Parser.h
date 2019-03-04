#pragma once

#include <string>
#include <set>
#include "Clock.h"
#include "CronValidator.h"

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
