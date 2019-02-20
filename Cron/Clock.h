#ifndef CLOCK_H
#define CLOCK_H

#include "chrono.h"
#include <vector>
#include "TimeUnit.h"

namespace Cron
{

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
    void syncWithLocalTime();
	void syncWithSpecialTime(const std::tm* tmTime);
    sysclock::time_point getNext();
    void specifyUnitsRange(const std::vector<TimeUnit::PossibleValues> &ranges);

private:
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
