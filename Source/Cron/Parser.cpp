#include "Parser.h"
#include "CronValidator.h"
#include <vector>
#include <map>
#include <regex>
#include <sstream>


namespace Cron
{
namespace Internal
{

std::string toString(size_t timeField)
{
    static const char* fields[] =
    {
        "MINUTE",
        "HOUR",
        "MDAY",
        "MONTH",
        "WDAY"
    };
    if(timeField <= sizeof (fields) / sizeof(const char*))
    {
        return fields[timeField];
    }
    else
    {
        return "";
    }
}

bool isValidDayField(const TimeUnit::PossibleValues& possibMDay, const TimeUnit::PossibleValues& possibWDay, const TimeUnit::PossibleValues& possibMonths)
{
    TimeUnit year;
    TimeUnit month;
    MDay mday;
    year.attach(nullptr, &month);
    month.attach(&year, &mday);
    mday.attach(&month, nullptr);

    mday.setMDayRange(possibMDay);
    mday.setWDayRange(possibWDay);
    auto hasPossibleValues = [&month, &possibMonths](MDay& day)
    {
        for(auto i : possibMonths)
        {
            month.setCurrentValue(i - 1);
            if(!day.calculatePosibRange().empty())
            {
                return true;
            }
        }
        return false;
    };

    bool valid = true;
    year.setCurrentValue(2001 - 1900); //Common year
    if(!hasPossibleValues(mday))
    {
        year.setCurrentValue(2020 - 1900); //leap year
        if(!hasPossibleValues(mday))
        {
            valid = false;
        }
    }
    return valid;
}
}

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
    this->reset();
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
            catch(Cron::BadSyntaxException except)
            {
                throw "Bad Syntax at field " + Internal::toString(field) + "[ " + cronExpressions[field] + " ]: " +  except;
            }
        }

        if(!Internal::isValidDayField(_cronFieldValues[FieldType::cfMDay], _cronFieldValues[FieldType::cfWDay], _cronFieldValues[FieldType::cfMon]))
        {
            throw ImpossibleValueException("Don't have possible value for fields DAY");
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

void Parser::reset()
{
    for(size_t i = 0; i < FieldType::cfFieldCount; ++i)
    {
        _cronFieldValues[i].clear();
    }
}



}


