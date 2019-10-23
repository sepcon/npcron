#include <npcron/Clock.h>
#include <npcron/TimeUtil.h>


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
        TimeUnit::applyActionRecursivelyFromLeaf(*ppClockWise, [](TimeUnit* pUnit) { delete pUnit; return RecursiveAction::NONSTOP; } );
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

bool Clock::isValid() const
{
    return _pMon && !_pMon->calculatePosibRange().empty();
}

void Clock::syncWithLocalTime()
{
	syncWithSpecialTime(TimeUtil::localTime());
}

void Clock::syncWithSpecialTime(const std::tm& tmTime)
{
    if (_pYear)
	{
        TimeUnit::specifyTime(_pYear, std::vector<int>{tmTime.tm_year, tmTime.tm_mon, tmTime.tm_mday, tmTime.tm_hour, tmTime.tm_min});
        TimeUnit::applyActionRecursivelyFromRoot(_pYear, [](TimeUnit* pUnit) { pUnit->calculatePosibRange(); return RecursiveAction::NONSTOP;});
	}
}

std::chrono::system_clock::time_point Clock::getNext(TimePointAnchor anchor)
{
    return doOneStep(&TimeUnit::stepNext, anchor);
}

std::chrono::system_clock::time_point Clock::getBack(TimePointAnchor anchor)
{
    return doOneStep(&TimeUnit::stepBack, anchor);
}

std::string Clock::getNextCTime(TimePointAnchor anchor)
{
    return TimeUtil::toCTime(sysclock::to_time_t(getNext(anchor)));
}

std::string Clock::getBackCTime(TimePointAnchor anchor)
{
    return TimeUtil::toCTime(sysclock::to_time_t(getBack(anchor)));
}

void Clock::specifyUnitsRange(const std::vector<TimeUnit::PossibleValues>& ranges)
{
    size_t idx = TimeUnit::Year;
    auto setPossibValues = [&idx, &ranges](TimeUnit* pUnit) { pUnit->_possibValues = ranges[idx++]; return RecursiveAction::NONSTOP; };
    TimeUnit::applyActionRecursivelyFromRoot(this->_pYear, setPossibValues);
}

std::chrono::system_clock::time_point Clock::doOneStep(int (TimeUnit::*step)(), TimePointAnchor anchor)
{
    if(anchor == FromNow) { syncWithLocalTime(); }
    if(_pYear)
    {
        //year must be current year
        TimeUnit::applyActionRecursivelyFromRoot(_pYear, [&step](TimeUnit* pUnit) {
            pUnit->calculatePosibRange();
            if(!pUnit->isValidValue() || !pUnit->_pChildUnit)
            {
                (pUnit->*step)();
                return RecursiveAction::STOP;
            }
            else //last unit must be forced to move
            {
                return RecursiveAction::NONSTOP;
            }
        });
        std::vector<int> vecTime;
        TimeUnit::applyActionRecursivelyFromRoot(_pYear, [&vecTime](TimeUnit* pUnit) {
            vecTime.push_back(pUnit->currentValue());
            return RecursiveAction::NONSTOP;
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
