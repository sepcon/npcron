#include "Parser.h"
#include "CronValidator.h"
#include <vector>
#include <map>
#include <regex>
#include <sstream>


namespace Cron
{
    static void collectValues(TimeUnit::PossibleValues& possibValues, const ExpInfo& info)
    {
        switch (info.type) {
        case ExpType::SingleValue:
            possibValues.insert(info.step);
            break;
        case ExpType::Range:
        case ExpType::AnyValue:
        case ExpType::StepFrom:
        case ExpType::StepWholeRange:
        case ExpType::StepFromTo:
        {
            int start = info.startValue;
            do
            {
                possibValues.insert(start);
                start += info.step;
            } while(start <= info.endValue);
        }
            break;
        default:
            break;
        }
    }

	Parser::Parser(const std::string & expression) 
	{
		if (!expression.empty())
		{
			parse(expression);
		}
	}

	void Parser::parse(const std::string & expression)
	{		
        std::regex matchedRegex;
        std::istringstream extractor(expression);
        using IStreamIt = std::istream_iterator<std::string>;
        std::vector<std::string> cronExpressions{IStreamIt{extractor}, IStreamIt{}};
		if (cronExpressions.size() != cfFieldCount)
		{
			std::ostringstream oss;
            oss << "The number of field is not correct\n Input is " << cronExpressions.size() << "\nCorrect is: " << cfFieldCount << " fields";
			throw BadSyntaxException(oss.str().c_str());
		}
		else
		{
            Validator v;
            for(size_t field = FieldType::cfMin; field < FieldType::cfFieldCount; ++field)
            {
                try
                {
                    v.changeExpression(cronExpressions[field], static_cast<FieldType>(field));
                    auto subExpressionInfo = v.subExpressionInfo();
                    for(const auto& info : subExpressionInfo)
                    {
                        collectValues(_cronFieldValues[field], info);
                    }
                }
                catch(...)
                {
                    throw;
                }
            }
			
        }
	}


    Clock Parser::createClock()
    {
        Clock clock;
        clock._pMin->setPosibValues(_cronFieldValues[FieldType::cfMin]);
        clock._pHour->setPosibValues(_cronFieldValues[FieldType::cfHour]);
        static_cast<Cron::MDay*>(clock._pDay)->setMDayRange(_cronFieldValues[FieldType::cfMDay]);
        static_cast<Cron::MDay*>(clock._pDay)->setWDayRange(_cronFieldValues[FieldType::cfWDay]);

        TimeUnit::PossibleValues monthValues;
        for(auto v : _cronFieldValues[FieldType::cfMon])
        {
            monthValues.insert(v - 1);
        }
        clock._pMon->setPosibValues(monthValues);

        clock.syncWithLocalTime();
        return clock;
    }



}


