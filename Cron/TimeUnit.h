#ifndef TIMEUNIT_H
#define TIMEUNIT_H

#include <set>
#include <vector>
#include <functional>

namespace Cron
{

class TimeUnit
{
public:
    typedef typename std::set<int>  PossibleValues;

    enum TimeUnitType
    {
        Year = 0,
        Mon,
        Day,
        Hour,
        Min,
        TypeCount
    };

    TimeUnit(const PossibleValues& range = {});
	virtual ~TimeUnit();
	
	int operator++(int);
	int operator++();
    int stepNext();

    int currentValue();
    void setCurrentValue(int value);
    void attach(TimeUnit* parent, TimeUnit* child);
    void setPosibValues(const PossibleValues& range);
    void setPosibValues(PossibleValues&& range);
    void resetToBeginning();
    bool isValidValue() const;

    static bool applyActionRecursivelyFromRoot(TimeUnit* pUnit, std::function<bool (TimeUnit*)> c);
    static bool applyActionRecursivelyFromLeaf(TimeUnit* pUnit, std::function<bool (TimeUnit*)> c);
    static void specifyTime(TimeUnit* pClockWise, const std::vector<int>& cTime);

protected:
    // With day in month, we have to consider the mday and wday
    virtual PossibleValues& calculatePosibRange() {  return _possibValues; }

	bool rangeAny() const;

    TimeUnit*       _pParentUnit;
    TimeUnit*       _pChildUnit;
    PossibleValues	_possibValues;
    int 			_value;

    friend class Clock;
	friend class MDay;
};

class MDay : public TimeUnit
{
public:
    void setWDayRange(const PossibleValues& range);
    void setMDayRange(const PossibleValues& range);
    virtual PossibleValues& calculatePosibRange() override;

private:
    PossibleValues _wDayRange;
    PossibleValues _mDayRange;
};

}

#endif // TIMEUNIT_H
