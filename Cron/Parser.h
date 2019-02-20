#pragma once

#include <string>
#include <set>
#include "Clock.h"
#include "CronValidator.h"

#define MIGHT_THROW(exception, ...)

namespace Cron
{
	class Parser
	{
    public:
        Parser(const std::string& expression = "");
        void parse(const std::string& expression);
        Cron::Clock createClock();
	private:
        TimeUnit::PossibleValues _cronFieldValues[FieldType::cfFieldCount];
	};
	
}
