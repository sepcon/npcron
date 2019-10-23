#ifndef CLOCK_H
#define CLOCK_H

#include <npcron/chrono.h>
#include <npcron/TimeUnit.h>
#include <vector>

namespace Cron
{

enum TimePointAnchor
{
    FromNow,
    FromSpecificPoint
};

class Clock
{
public:
    using sysclock = std::chrono::system_clock;
    Clock();
    Clock(const Clock& rhs);
    Clock(Clock&& rhs);
    Clock& operator=(const Clock& rhs);
    Clock& operator=(Clock&& rhs);
    ~Clock();
    bool isValid() const;
    void syncWithLocalTime();
	void syncWithSpecialTime(const tm & tmTime);
    sysclock::time_point getNext(TimePointAnchor anchor = FromNow);
    sysclock::time_point getBack(TimePointAnchor anchor = FromNow);
    std::string getNextCTime(TimePointAnchor anchor = FromNow);
    std::string getBackCTime(TimePointAnchor anchor = FromNow);
    void specifyUnitsRange(const std::vector<TimeUnit::PossibleValues> &ranges);

private:
    sysclock::time_point doOneStep(int (TimeUnit::*step)(), TimePointAnchor anchor);
    void cloneFrom(const Clock& rhs);
    void moveFrom(Clock&& rhs);
    TimeUnit*   _pYear;
    TimeUnit*   _pMon;
    TimeUnit*   _pDay;
    TimeUnit*   _pHour;
    TimeUnit*   _pMin;

    friend class Parser;
};

}
#endif // CLOCK_H
