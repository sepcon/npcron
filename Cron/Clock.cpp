#define _CRT_SECURE_NO_WARNINGS
#include "Clock.h"
#include "TimeUtil.h"


namespace Cron
{

namespace Internal
{

    TimeUnit* makeUnits(TimeUnit** pYear,
                                TimeUnit** pMon ,
                                TimeUnit** pDay ,
                                TimeUnit** pHour,
                                TimeUnit** pMin )
    {
        *pYear = new TimeUnit();
        *pMon = new TimeUnit();
        *pDay = new MDay();
        *pHour = new TimeUnit();
        *pMin = new TimeUnit();
        return *pYear;
    }

    void connectUnits(TimeUnit* pYear,
                      TimeUnit* pMon ,
                      TimeUnit* pDay ,
                      TimeUnit* pHour,
                      TimeUnit* pMin )
    {
        pYear->attach(nullptr, pMon);
        pMon->attach(pYear, pDay);
        pDay->attach(pMon, pHour);
        pHour->attach(pDay, pMin);
        pMin->attach(pHour, nullptr);
    }

    void deleteClockWise(TimeUnit** ppClockWise)
    {
        TimeUnit::applyActionRecursivelyFromLeaf(*ppClockWise, [](TimeUnit* pUnit) -> bool { delete pUnit; return false; } );
        *ppClockWise = nullptr;
    }
}

Clock::Clock()
{
    Internal::makeUnits(&_pYear,
                        &_pMon ,
                        &_pDay ,
                        &_pHour,
                        &_pMin);
    Internal::connectUnits(_pYear,
                           _pMon ,
                           _pDay ,
                           _pHour,
                           _pMin);
}

Clock::Clock(const Clock & rhs)
{
    cloneFrom(rhs);
}

Clock::Clock(Clock && rhs)
{
    moveFrom(std::move(rhs));
}

Clock &Clock::operator=(const Clock &rhs)
{
    cloneFrom(rhs);
    return *this;
}

Clock &Clock::operator=(Clock &&rhs)
{
    moveFrom(std::move(rhs));
    return *this;
}

Clock::~Clock()
{
    delete _pYear;
    delete _pMon ;
    delete _pDay ;
    delete _pHour;
    delete _pMin ;
}

void Clock::syncWithLocalTime()
{
	syncWithSpecialTime(TimeUtil::localTime());
}

void Clock::syncWithSpecialTime(const std::tm * tmTime)
{
    if (tmTime && _pYear)
	{
        TimeUnit::specifyTime(_pYear, std::vector<int>{tmTime->tm_year, tmTime->tm_mon, tmTime->tm_mday, tmTime->tm_hour, tmTime->tm_min});
        TimeUnit::applyActionRecursivelyFromRoot(_pYear, [](TimeUnit* pUnit) { pUnit->calculatePosibRange(); return false;});
	}
}

std::chrono::system_clock::time_point Clock::getNext(bool fromNow)
{
    return doOneStep(StepDirection::NEXT, fromNow);
}

std::chrono::system_clock::time_point Clock::getBack(bool fromNow)
{
    return doOneStep(StepDirection::BACK, fromNow);
}

void Clock::specifyUnitsRange(const std::vector<TimeUnit::PossibleValues>& ranges)
{
    size_t idx = TimeUnit::Year;
    auto setPossibValues = [&idx, &ranges](TimeUnit* pUnit) -> bool { pUnit->_possibValues = ranges[idx++]; return false; };
    TimeUnit::applyActionRecursivelyFromRoot(this->_pYear, setPossibValues);
}

std::chrono::system_clock::time_point Clock::doOneStep(Clock::StepDirection direction, bool fromNow)
{
    if(fromNow) syncWithLocalTime();
    int (TimeUnit::*stepFuncion)();
    if(direction == StepDirection::BACK)
    {
        stepFuncion = &TimeUnit::stepBack;
    }
    else if(direction == StepDirection::NEXT)
    {
        stepFuncion = &TimeUnit::stepNext;
    }
    if(_pYear)
    {
        //year must be current year
        TimeUnit::applyActionRecursivelyFromRoot(_pYear, [&stepFuncion](TimeUnit* pUnit) -> bool {
            if(!pUnit->isValidValue() || !pUnit->_pChildUnit)
            {
                (pUnit->*stepFuncion)();
                return true;
            }
            else //last unit must be forced to move
            {
                return false;
            }
        });
        std::vector<int> vecTime;
        TimeUnit::applyActionRecursivelyFromRoot(_pYear, [&vecTime](TimeUnit* pUnit) -> bool {
            vecTime.push_back(pUnit->currentValue());
            return false;
        });
        std::tm tmTime = TimeUtil::createTimeInfo(vecTime[TimeUnit::Year], vecTime[TimeUnit::Mon], vecTime[TimeUnit::Day], vecTime[TimeUnit::Hour], vecTime[TimeUnit::Min]);
        return sysclock::from_time_t(std::mktime(&tmTime));
    }
    else
    {
        return std::chrono::system_clock::time_point();
    }
}

void Clock::cloneFrom(const Clock &rhs)
{
    if(&rhs == this)
    {
        return;
    }
    Internal::makeUnits(&_pYear, &_pMon, &_pDay, &_pHour, &_pMin);
    *_pYear = *rhs._pYear;
    *_pMon =      *rhs._pMon;
    *_pDay =      *rhs._pDay ;
    *_pHour =     *rhs._pHour;
    *_pMin =      *rhs._pMin ;
    Internal::connectUnits(_pYear, _pMon, _pDay, _pHour, _pMin);
}

void Clock::moveFrom(Clock &&rhs)
{
    if(this == &rhs)
    {
        return;
    }
    _pYear = rhs._pYear;
    _pMon = rhs._pMon;
    _pDay = rhs._pDay;
    _pHour = rhs._pHour;
    _pMin = rhs._pMin;
    rhs._pYear =   nullptr;
    rhs._pMon =    nullptr;
    rhs._pDay =    nullptr;
    rhs._pHour =   nullptr;
    rhs._pMin =    nullptr;
}

}
