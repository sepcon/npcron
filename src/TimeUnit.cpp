#include "TimeUnit.h"

#include <npcron/TimeUtil.h>

#include <algorithm>

namespace npcron {

namespace {
namespace internal {
TimeUnit::PossibleValues getMDayOf(const TimeUnit::PossibleValues &weekDays,
                                   int month, int year) {
  unsigned int maxPermonth = util::dayEndOfMonth(month, year);
  int startWDay = util::getWeekDayOf(year, month, 1);
  int deviate = startWDay - 1;
  TimeUnit::PossibleValues allWDaysOfThisMonth;
  for (unsigned int i = 1; i <= maxPermonth; ++i) {
    int wday = (deviate + static_cast<int>(i)) % 7;
    if (weekDays.find(wday) != weekDays.end()) {
      allWDaysOfThisMonth.insert(static_cast<int>(i));
    }
  }
  return allWDaysOfThisMonth;
}

TimeUnit::PossibleValues getPossibMdays(const TimeUnit::PossibleValues &mDays,
                                        int month, int year) {
  unsigned int maxPerMonth = util::dayEndOfMonth(month, year);
  return TimeUnit::PossibleValues{mDays.begin(),
                                  mDays.upper_bound(maxPerMonth)};
}
}  // namespace internal
}  // namespace

TimeUnit::TimeUnit(const PossibleValues &range)
    : _pParentUnit(nullptr),
      _pChildUnit(nullptr),
      _possibValues(range),
      _value(0) {}

int TimeUnit::operator++(int) {
  int currentValue = _value;
  stepNext();
  return currentValue;
}

int TimeUnit::operator++() { return stepNext(); }

bool TimeUnit::rangeAny() const { return _possibValues.empty(); }

void TimeUnit::attach(TimeUnit *parent, TimeUnit *child) {
  this->_pParentUnit = parent;
  this->_pChildUnit = child;
}

int TimeUnit::currentValue() { return _value; }

void TimeUnit::setCurrentValue(int value) { this->_value = value; }

void TimeUnit::setPosibValues(const TimeUnit::PossibleValues &range) {
  _possibValues = range;
}

void TimeUnit::setPosibValues(TimeUnit::PossibleValues &&range) {
  _possibValues = std::move(range);
}

void TimeUnit::resetToBeginning() {
  calculatePosibRange();
  if (!_possibValues.empty()) {
    _value = *_possibValues.begin();
    if (_pChildUnit) {
      _pChildUnit->resetToBeginning();
    }
  } else if (_pParentUnit) {
    _pParentUnit->stepNext();
  }
}

void TimeUnit::resetToLast() {
  calculatePosibRange();
  if (!_possibValues.empty()) {
    _value = *_possibValues.rbegin();
    if (_pChildUnit) {
      _pChildUnit->resetToLast();
    }
  } else if (_pParentUnit) {
    _pParentUnit->stepBack();
  }
}

bool TimeUnit::isValidValue() const {
  if (rangeAny()) {
    return true;
  } else {
    return _possibValues.find(_value) != _possibValues.end();
  }
}

int TimeUnit::stepBack() {
  if (rangeAny()) {
    _value--;
    if (_pChildUnit) {
      _pChildUnit->resetToLast();
    }
  } else {
    auto currentPos = _possibValues.lower_bound(_value);
    if (currentPos != _possibValues.begin()) {
      _value = *(--currentPos);
      if (_pChildUnit) {
        _pChildUnit->resetToLast();
      }
    } else  // parent unit must step next
    {
      if (_pParentUnit) {
        _pParentUnit->stepBack();
      } else if (rangeAny())  // means root unit
      {
        _value--;
      }
    }
  }
  return _value;
}
int TimeUnit::stepNext() {
  if (rangeAny()) {
    _value++;
    if (_pChildUnit) {
      _pChildUnit->resetToBeginning();
    }
  } else {
    auto nextVal = _possibValues.upper_bound(_value);
    if (nextVal != _possibValues.end()) {
      _value = *nextVal;
      if (_pChildUnit) {
        _pChildUnit->resetToBeginning();
      }
    } else  // parent unit must step next
    {
      if (_pParentUnit) {
        _pParentUnit->stepNext();
      } else  // means root unit
      {
        _value++;
      }
    }
  }
  return _value;
}

TimeUnit::~TimeUnit() {}

RecursiveAction TimeUnit::applyActionRecursivelyFromRoot(
    TimeUnit *pUnit, std::function<RecursiveAction(TimeUnit *)> actionOn) {
  if (pUnit) {
    if (actionOn(pUnit) == RecursiveAction::STOP) {
      return RecursiveAction::STOP;
    } else {
      return applyActionRecursivelyFromRoot(pUnit->_pChildUnit, actionOn);
    }
  } else {
    return RecursiveAction::NONSTOP;
  }
}

RecursiveAction TimeUnit::applyActionRecursivelyFromLeaf(
    TimeUnit *pUnit, std::function<RecursiveAction(TimeUnit *)> actionOn) {
  if (pUnit) {
    if (applyActionRecursivelyFromLeaf(pUnit->_pChildUnit, actionOn) ==
        RecursiveAction::STOP) {
      return RecursiveAction::STOP;
    } else {
      return actionOn(pUnit);
    }
  } else {
    return RecursiveAction::NONSTOP;
  }
}

void TimeUnit::specifyTime(TimeUnit *pClockWise,
                           const std::vector<int> &cTime) {
  for (const auto &val : cTime) {
    if (pClockWise) {
      pClockWise->_value = val;
      pClockWise = pClockWise->_pChildUnit;
    } else {
      break;
    }
  }
}

void MDay::setWDayRange(const TimeUnit::PossibleValues &range) {
  _possibWDays = range;
}

void MDay::setMDayRange(const TimeUnit::PossibleValues &range) {
  _possibMDays = range;
}

TimeUnit::PossibleValues &MDay::calculatePosibRange() {
  auto pMonth = _pParentUnit;
  auto pYear = pMonth->_pParentUnit;
  _possibValues.clear();
  if (_possibMDays.size() == 31) {
    if (_possibWDays.size() == 7) {
      _possibValues = internal::getPossibMdays(
          _possibMDays, pMonth->currentValue(), pYear->currentValue());
    } else {
      _possibValues = internal::getMDayOf(_possibWDays, pMonth->currentValue(),
                                          pYear->currentValue());
    }
  } else if (_possibWDays.size() == 7) {
    _possibValues = internal::getPossibMdays(
        _possibMDays, pMonth->currentValue(), pYear->currentValue());
  } else {
    _possibValues = internal::getPossibMdays(
        _possibMDays, pMonth->currentValue(), pYear->currentValue());
    auto mdaysOfWdays = internal::getMDayOf(
        _possibWDays, pMonth->currentValue(), pYear->currentValue());
    _possibValues.insert(mdaysOfWdays.begin(), mdaysOfWdays.end());
  }
  return _possibValues;
}

}  // namespace npcron
