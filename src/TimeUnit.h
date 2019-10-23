#pragma once

#include <functional>
#include <set>
#include <vector>

namespace npcron {

enum class RecursiveAction { STOP = true, NONSTOP = false };

struct TimeUnit {
  typedef typename std::set<int> PossibleValues;
  enum TimeUnitType { Year = 0, Mon, Day, Hour, Min, TypeCount };

  TimeUnit(const PossibleValues& range = {});
  virtual ~TimeUnit();

  int operator++(int);
  int operator++();
  int stepBack();
  int stepNext();

  int currentValue();
  void setCurrentValue(int value);
  void attach(TimeUnit* parent, TimeUnit* child);
  void setPosibValues(const PossibleValues& range);
  void setPosibValues(PossibleValues&& range);
  void resetToBeginning();
  void resetToLast();
  bool isValidValue() const;

  static RecursiveAction applyActionRecursivelyFromRoot(
      TimeUnit* pUnit, std::function<RecursiveAction(TimeUnit*)> actionOn);
  static RecursiveAction applyActionRecursivelyFromLeaf(
      TimeUnit* pUnit, std::function<RecursiveAction(TimeUnit*)> actionOn);
  static void specifyTime(TimeUnit* pClockWise, const std::vector<int>& cTime);

  // With day in month, we have to consider the mday and wday
  virtual PossibleValues& calculatePosibRange() { return _possibValues; }

  bool rangeAny() const;

  TimeUnit* _pParentUnit;
  TimeUnit* _pChildUnit;
  PossibleValues _possibValues;
  int _value;
};

struct MDay : public TimeUnit {
 public:
  void setWDayRange(const PossibleValues& range);
  void setMDayRange(const PossibleValues& range);
  virtual PossibleValues& calculatePosibRange() override;

  PossibleValues _possibWDays;
  PossibleValues _possibMDays;
};

}  // namespace npcron
