#include <map>
#include <exception>
#include <sstream>
#include <iostream>
#include "CronValidator.h"

namespace Cron
{
const std::string Validator::constFieldValidationRegexStr = "(((((([0-9])([0-9])*-([0-9])([0-9])*)|([0-9])([0-9])*|\\*)(\\/[1-9]+[0-9]*)*),)+(((([0-9])([0-9])*-([0-9])([0-9])*)|([0-9])([0-9])*|\\*)(\\/[1-9]+[0-9]*)*))|(((([0-9])([0-9])*-([0-9])([0-9])*)|([0-9])([0-9])*|\\*)(\\/[1-9]+[0-9]*)*)";
const std::string Validator::constSubFieldValidationRegexStr = "(((([0-9])([0-9])*-([0-9])([0-9])*)|([0-9])([0-9])*|\\*)(\\/[1-9]+[0-9]*)*)";

static const char* gFieldNames[] =
{
    "minute",
    "hour",
    "mday",
    "month",
    "wday"
};

static const std::pair<int, int> gValidFieldRanges[] =
{
    {0, 59}, //minute
    {0, 23}, //hour
    {1, 31}, //mday
    {1, 12}, //month
    {0, 6}, //wday
};

static const std::map<std::string, std::string> gMonthMap =
{
    { "JAN", "1" }, { "FEB", "2" }, { "MAR", "3" }, { "APR", "4" }, { "MAY", "5" }, { "JUN", "6" },
    { "JUL", "7" }, { "AUG", "8" }, { "SEP", "9" }, { "OCT", "10" }, { "NOV", "11" }, { "DEC", "12" }
};

static const std::map<std::string, std::string> gWDayMap =
{
    { "SUN", "0" }, { "MON", "1" }, { "TUE", "2" }, { "WED", "3" }, { "THU", "4" }, { "FRI", "5" }, { "SAT", "6" }
};

static std::string replace(std::string word, const std::string& with, const std::string& by)
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
    for(auto rIt = foundPoses.rbegin(); rIt != foundPoses.rend(); ++rIt)
    {
        word.replace(*rIt, with.size(), by);
    }
    return word;
}

static std::string getValidRangeInStringOf(FieldType field)
{
    return "[" + std::to_string(gValidFieldRanges[field].first)
            + " - " +
            std::to_string(gValidFieldRanges[field].second) + "]";
}

static BadSyntaxException badSyntaxException(FieldType field, const std::string& expression)
{
    return std::string("Invalid field ") + gFieldNames[field] + " with value: " + expression +
            " - Note: Possible values must be in range:" + getValidRangeInStringOf(field);
}

bool isValidExpression(int from, int to, int step, FieldType field)
{
    int min = gValidFieldRanges[field].first;
    int max = gValidFieldRanges[field].second;
    if (from > max || from < min || to > max)
    {
        return false;
    }
    else if(from + step > max)
    {
        return false;
    }
    return true;
}

std::pair<int, int> extractRange(const std::string& expression)
{
    std::pair<int, int> range;
    size_t idxOfMinus = expression.find('-');
    if (idxOfMinus != std::string::npos)
    {
        range.first = std::atoi(expression.substr(0, idxOfMinus).c_str());
        range.second = std::atoi(expression.substr(idxOfMinus + 1).c_str());
    }
    return range;
}

int extractStepValue(const std::string& expression, size_t& idxOfSlash)
{
    idxOfSlash = expression.find('/');
    if(idxOfSlash != std::string::npos && idxOfSlash != expression.size() - 1)
    {
        return std::atoi(expression.substr(idxOfSlash + 1).c_str());
    }
    return -1;
}

static std::string convertToNumberBasedExpression(std::string expression, const std::map<std::string, std::string>& theMap)
{
    std::string result = expression;
    for (auto& it : theMap)
    {
        result = replace(result, it.first, it.second);
    }
    return result;
}

Validator::Validator() : _valid(false)
{
}

Validator::Validator(const std::string & expression, FieldType field):
    _field(field),
    _valid(false)
{
    changeExpression(expression, field);
}

void Validator::changeExpression(const std::string & expression, FieldType field)
{
    _field = field;
    if (field == FieldType::cfMon)
    {
        _expression = convertToNumberBasedExpression(expression, gMonthMap);
    }
    else if (field == FieldType::cfWDay)
    {
        _expression = convertToNumberBasedExpression(expression, gWDayMap);
    }
    else
    {
        _expression = expression;
    }
    _subExprInfo.clear();
    validate();
}

bool Validator::validate()
{
    _valid = std::regex_match(_expression, std::regex(constFieldValidationRegexStr));
    if (_valid)
    {
        std::sregex_iterator itEnd;
        std::regex subExprRegex = std::regex(constSubFieldValidationRegexStr);
        for (auto it = std::sregex_iterator(_expression.begin(), _expression.end(), subExprRegex); it != itEnd; ++it)
        {
            std::string subExpr = (*it).str();
            try
            {
                validateSubExpression(subExpr);
            }
            catch(...)
            {
                throw ;
            }
        }
    }
    else
    {
        throw BadSyntaxException("Invalid systax!");
    }
    return _valid;
}


bool Validator::isValidFiedValue(int value)
{
    return gValidFieldRanges[_field].first <= value && gValidFieldRanges[_field].second >= value;
}

const std::vector<ExpInfo>& Validator::subExpressionInfo() const
{
    return _subExprInfo; //will be empty if itself is invalid
}

bool Validator::isValidRange(int min, int max, int step)
{
    bool valid = false;
    if (min <= max)
    {
        if(gValidFieldRanges[_field].first <= min
                && max <= gValidFieldRanges[_field].second)
        {
            valid = true;
        }
    }
    return valid;
}

bool Validator::validateSubExpression(const std::string & subExpr)
{
    _valid = false;
    auto contains = [](const std::string& str, const char c) -> bool
    {
        return str.find(c) != std::string::npos;
    };

    if (contains(subExpr, '*'))
    {
        if (contains(subExpr, '/'))
        {
            size_t idxOfSlash;
            int step = extractStepValue(subExpr, idxOfSlash);
            if(step > 0 /*&& gValidFieldRanges[_field].first + step <= gValidFieldRanges[_field].second*/)
            {
                _subExprInfo.emplace_back(subExpr, ExpType::StepWholeRange, gValidFieldRanges[_field].first, gValidFieldRanges[_field].second, step);
                _valid = true;
            }
            else
            {
                throw badSyntaxException(_field, subExpr);
            }
        }
        else
        {
            _subExprInfo.emplace_back(subExpr, ExpType::AnyValue, gValidFieldRanges[_field].first, gValidFieldRanges[_field].second);
        }
    }
    else if (contains(subExpr, '/'))
    {
        size_t idxOfSlash;
        int step = extractStepValue(subExpr, idxOfSlash);
        if (contains(subExpr, '-'))
        {
            auto range = extractRange(subExpr.substr(0, idxOfSlash));
            if(isValidRange(range.first, range.second, step))
            {
                _subExprInfo.emplace_back(subExpr, ExpType::StepFromTo, range.first, range.second, step);
            }
            else
            {
                throw badSyntaxException(_field, subExpr);
            }
        }
        else
        {
            int startValue = std::atoi(subExpr.substr(0, idxOfSlash).c_str());
            if(step > 0 && startValue >= gValidFieldRanges[_field].first /*&& startValue + step <= gValidFieldRanges[_field].second*/)
            {
                _subExprInfo.emplace_back(subExpr, ExpType::StepFrom, startValue, gValidFieldRanges[_field].second, step);
            }
            else
            {
                throw badSyntaxException(_field, subExpr);
            }
        }
    }
    else if (contains(subExpr, '-'))
    {
        size_t idxOfMinus = subExpr.find('-');
        if (idxOfMinus != std::string::npos)
        {
            auto range = extractRange(subExpr);
            if(isValidRange(range.first, range.second))
            {
                _valid = true;
                _subExprInfo.emplace_back(subExpr, ExpType::Range, range.first, range.second);
            }
            else
            {
                throw badSyntaxException(_field, subExpr);
            }
        }
    }
    else
    {
        int value = std::atoi(subExpr.c_str());
        if (!isValidFiedValue(value))
        {
            _valid = false;
            throw badSyntaxException(_field, subExpr);
        }
        else
        {
            _valid = true;
            _subExprInfo.emplace_back(subExpr, ExpType::SingleValue, gValidFieldRanges[_field].first, gValidFieldRanges[_field].second, value);
        }
    }
    return _valid;
}

}
