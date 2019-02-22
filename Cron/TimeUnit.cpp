#include "TimeUnit.h"
#include "TimeUtil.h"
#include <algorithm>

namespace Cron
{

TimeUnit::TimeUnit(const PossibleValues& range) :
    _pParentUnit(nullptr),
    _pChildUnit(nullptr),
    _possibValues(range),
    _value(0)
{
}

int TimeUnit::operator++(int)
{
	int currentValue = _value;
	stepNext();
	return currentValue;
}

int TimeUnit::operator++()
{
    return stepNext();
}


bool TimeUnit::rangeAny() const
{
    return _possibValues.empty();
}

void TimeUnit::attach(TimeUnit *parent, TimeUnit *child)
{
    this->_pParentUnit = parent;
    this->_pChildUnit = child;
}

int TimeUnit::currentValue()
{
    return _value;
}

void TimeUnit::setCurrentValue(int value)
{
    this->_value = value;
}

void TimeUnit::setPosibValues(const TimeUnit::PossibleValues &range)
{
    _possibValues = range;
}

void TimeUnit::setPosibValues(TimeUnit::PossibleValues &&range)
{
    _possibValues = std::move(range);
}

void TimeUnit::resetToBeginning()
{
    if(rangeAny())
    {
        return;
    }
    else
    {
        _value = *_possibValues.begin();
        if(_pChildUnit)
        {
            _pChildUnit->resetToBeginning();
        }
    }
}

void TimeUnit::resetToLast()
{
    if(rangeAny())
    {
        return;
    }
    else
    {
        _value = *_possibValues.rbegin();
        if(_pChildUnit)
        {
            _pChildUnit->resetToLast();
        }
    }
}

bool TimeUnit::isValidValue() const
{
    if(rangeAny())
    {
        return true;
    }
    else
    {
        return _possibValues.find(_value) != _possibValues.end();
    }
}

int TimeUnit::stepBack()
{
    if(rangeAny())
    {
        _value--;
        if(_pChildUnit)
        {
            _pChildUnit->resetToLast();
        }
    }
    else
    {
        auto currentPos = _possibValues.lower_bound(_value);
        if( currentPos != _possibValues.begin())
        {
            _value = *(--currentPos);
            if(_pChildUnit)
            {
                _pChildUnit->resetToLast();
            }
        }
        else  //parent unit must step next
        {
            if(_pParentUnit)
            {
                _pParentUnit->stepBack();
            }
            else if(rangeAny())//means root unit
            {
                _value--;
            }
        }
    }
    return _value;
}
int TimeUnit::stepNext()
{
    if(rangeAny())
    {
        _value++;
        if(_pChildUnit)
        {
            _pChildUnit->resetToBeginning();
        }
    }
    else
    {
        auto nextVal = _possibValues.upper_bound(_value);
        if( nextVal != _possibValues.end())
        {
            _value = *nextVal;
            if(_pChildUnit)
            {
                _pChildUnit->resetToBeginning();
            }
        }
        else  //parent unit must step next
        {
            if(_pParentUnit)
            {
                _pParentUnit->stepNext();
            }
            else //means root unit
            {
                _value ++;
            }
        }
    }
    return _value;
}

TimeUnit::~TimeUnit()
{

}

bool TimeUnit::applyActionRecursivelyFromRoot(TimeUnit *pUnit, std::function<bool (TimeUnit *)> c)
{
    if(pUnit)
    {
        if(c(pUnit))
        {
            return true;
        }
        else
        {
            return applyActionRecursivelyFromRoot(pUnit->_pChildUnit, c);
        }
    }
    else
    {
        return false;
    }
}

bool TimeUnit::applyActionRecursivelyFromLeaf(TimeUnit *pUnit, std::function<bool (TimeUnit *)> c)
{
    if(pUnit)
    {
        if(applyActionRecursivelyFromLeaf(pUnit->_pChildUnit, c))
        {
            return true;
        }
        else
        {
            return c(pUnit);
        }
    }
    else
    {
        return false;
    }
}

void TimeUnit::specifyTime(TimeUnit *pClockWise, const std::vector<int> &cTime)
{
    for(const auto& val : cTime)
    {
        if (pClockWise)
        {
            pClockWise->_value = val;
            pClockWise = pClockWise->_pChildUnit;
        }
        else
        {
            break;
        }
    }
}

void MDay::setWDayRange(const TimeUnit::PossibleValues &range)
{
    _wDayRange = range;
}

void MDay::setMDayRange(const TimeUnit::PossibleValues &range)
{
    _mDayRange = range;
}

TimeUnit::PossibleValues &MDay::calculatePosibRange()
{
    int maxPermonth = 31;
    _possibValues = _mDayRange;
    if (_pParentUnit)
    {
        TimeUnit* pMonth = _pParentUnit;
        TimeUnit* pYear = _pParentUnit->_pParentUnit;
        if (1 == pMonth->currentValue()) //Febuary
        {
            if (pYear && TimeUtil::isLeapYear(pYear->currentValue()))
            {
                maxPermonth = 29;
            }
            else
            {
                maxPermonth = 28;
            }
        }
        else if (_pParentUnit->currentValue() % 2 == 1)
        {
            maxPermonth = 30;
        }
        // calculate the week day of the first mday in month:
        // remember that mday always starts from 1. 1-31
        if (pYear)
        {
            int startWDay = TimeUtil::getWeekDayOf(pYear->currentValue(), pMonth->currentValue(), 1);
            int deviate = startWDay - 1;
            PossibleValues allWDaysOfThisMonth;
            for (int i = 1; i <= maxPermonth; ++i)
            {
                int wday = (deviate + i) % 7;
                if (_wDayRange.find(wday) != _wDayRange.end())
                {
                    _possibValues.insert(i);
                }
            }
        }
    }
    return _possibValues;
}


}
