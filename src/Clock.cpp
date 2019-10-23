#include <npcron/Clock.h>
#include <npcron/Exception.h>
#include <npcron/TimeUtil.h>

#include <sstream>

#include "TimeUnit.h"
#include "Validator.h"

namespace npcron {

using namespace std::chrono;
using std::make_unique;
using std::move;

namespace {
namespace internal {

void connectUnits(TimeUnit& pYear, TimeUnit& pMon, TimeUnit& pDay,
                  TimeUnit& pHour, TimeUnit& pMin) {
  pYear.attach(nullptr, &pMon);
  pMon.attach(&pYear, &pDay);
  pDay.attach(&pMon, &pHour);
  pHour.attach(&pDay, &pMin);
  pMin.attach(&pHour, nullptr);
}

std::string toString(size_t timeField) {
  static const char* fields[] = {"MINUTE", "HOUR", "MDAY", "MONTH", "WDAY"};
  if (timeField <= sizeof(fields) / sizeof(const char*)) {
    return fields[timeField];
  } else {
    return "";
  }
}

bool isValidDayField(const TimeUnit::PossibleValues& possibMDay,
                     const TimeUnit::PossibleValues& possibWDay,
                     const TimeUnit::PossibleValues& possibMonths) {
  TimeUnit year;
  TimeUnit month;
  MDay mday;
  year.attach(nullptr, &month);
  month.attach(&year, &mday);
  mday.attach(&month, nullptr);

  mday.setMDayRange(possibMDay);
  mday.setWDayRange(possibWDay);
  auto hasPossibleValues = [&month, &possibMonths](MDay& day) {
    for (auto i : possibMonths) {
      month.setCurrentValue(i - 1);
      if (!day.calculatePosibRange().empty()) {
        return true;
      }
    }
    return false;
  };

  bool valid = true;
  year.setCurrentValue(2001 - 1900);  // Common year
  if (!hasPossibleValues(mday)) {
    year.setCurrentValue(2020 - 1900);  // leap year
    if (!hasPossibleValues(mday)) {
      valid = false;
    }
  }
  return valid;
}
void collectValues(TimeUnit::PossibleValues& possibValues,
                   const ExpInfo& info) {
  switch (info.type) {
    case ExpType::SingleValue:
      possibValues.insert(info.step);
      break;
    case ExpType::Range:
    case ExpType::AnyValue:
    case ExpType::StepFrom:
    case ExpType::StepWholeRange:
    case ExpType::StepFromTo: {
      int start = info.startValue;
      do {
        possibValues.insert(start);
        start += info.step;
      } while (start <= info.endValue);
    } break;
    default:
      break;
  }
}

}  // namespace internal
}  // namespace

using Expression = std::string;
using TimePoint = Clock::TimePoint;

struct ClockDetails {
  TimeUnit year;
  TimeUnit mon;
  MDay day;
  TimeUnit hour;
  TimeUnit min;
  ClockDetails() { internal::connectUnits(year, mon, day, hour, min); }

  ClockDetails(const ClockDetails& rhs)
      : year(rhs.year),
        mon(rhs.mon),
        day(rhs.day),
        hour(rhs.hour),
        min(rhs.min) {
    internal::connectUnits(year, mon, day, hour, min);
  }

  ClockDetails(ClockDetails&& rhs)
      : year(move(rhs.year)),
        mon(move(rhs.mon)),
        day(move(rhs.day)),
        hour(move(rhs.hour)),
        min(move(rhs.min)) {
    internal::connectUnits(year, mon, day, hour, min);
  }

  ClockDetails& operator=(const ClockDetails& rhs) {
    if (&rhs != this) {
      year = rhs.year;
      mon = rhs.mon;
      day = rhs.day;
      hour = rhs.hour;
      min = rhs.min;
      internal::connectUnits(year, mon, day, hour, min);
    }
    return *this;
  }

  ClockDetails& operator=(ClockDetails&& rhs) {
    if (&rhs != this) {
      year = move(rhs.year);
      mon = move(rhs.mon);
      day = move(rhs.day);
      hour = move(rhs.hour);
      min = move(rhs.min);
      internal::connectUnits(year, mon, day, hour, min);
    }
    return *this;
  }

  TimePoint doOneStep(int (TimeUnit::*step)()) {
    // year must be current year
    TimeUnit::applyActionRecursivelyFromRoot(&year, [&step](TimeUnit* pUnit) {
      pUnit->calculatePosibRange();
      if (!pUnit->isValidValue() || !pUnit->_pChildUnit) {
        (pUnit->*step)();
        return RecursiveAction::STOP;
      } else  // last unit must be forced to move
      {
        return RecursiveAction::NONSTOP;
      }
    });
    std::vector<int> vecTime;
    TimeUnit::applyActionRecursivelyFromRoot(
        &year, [&vecTime](TimeUnit* pUnit) {
          vecTime.push_back(pUnit->currentValue());
          return RecursiveAction::NONSTOP;
        });
    std::tm tmTime = util::createTimeInfo(
        vecTime[TimeUnit::Year], vecTime[TimeUnit::Mon], vecTime[TimeUnit::Day],
        vecTime[TimeUnit::Hour], vecTime[TimeUnit::Min]);
    return system_clock::from_time_t(std::mktime(&tmTime));
  }
  void syncWithLocalTime(TimePoint tp = system_clock::now()) {
    std::tm tmlocal;
    util::timet2local(system_clock::to_time_t(tp), tmlocal);
    syncWithLocalTime(tmlocal);
  }

  void syncWithLocalTime(const std::tm& tmTime) {
    TimeUnit::specifyTime(
        &year, std::vector<int>{tmTime.tm_year, tmTime.tm_mon, tmTime.tm_mday,
                                tmTime.tm_hour, tmTime.tm_min});
    TimeUnit::applyActionRecursivelyFromRoot(&year, [](TimeUnit* pUnit) {
      pUnit->calculatePosibRange();
      return RecursiveAction::NONSTOP;
    });
  }

  bool valid() { return !mon.calculatePosibRange().empty(); }
};

struct Parser {
  Parser(const Expression& expression) {
    if (!expression.empty()) {
      parse(expression);
    }
  }

  void reset() {
    for (size_t i = 0; i < FieldType::cfFieldCount; ++i) {
      fieldValues[i].clear();
    }
  }

  void parse(const Expression& expression) {
    std::regex matchedRegex;
    std::istringstream extractor(expression);
    using IStreamIt = std::istream_iterator<std::string>;
    std::vector<std::string> cronExpressions{IStreamIt{extractor}, IStreamIt{}};
    if (cronExpressions.size() != cfFieldCount) {
      std::ostringstream oss;
      oss << "The number of field is not correct\n Input is "
          << cronExpressions.size() << "\nCorrect is: " << cfFieldCount
          << " fields";
      BadSyntaxException::raise(oss.str());
    } else {
      Validator v;
      for (size_t field = FieldType::cfMin; field < FieldType::cfFieldCount;
           ++field) {
        try {
          v.changeExpression(cronExpressions[field],
                             static_cast<FieldType>(field));
          auto subExpressionInfo = v.subExpressionInfo();
          for (const auto& info : subExpressionInfo) {
            internal::collectValues(fieldValues[field], info);
          }
        } catch (const BadSyntaxException& e) {
          BadSyntaxException::raise("Bad Syntax at field " +
                                    internal::toString(field) + "[ " +
                                    cronExpressions[field] + " ]: " + e.what());
        }
      }

      if (!internal::isValidDayField(fieldValues[FieldType::cfMDay],
                                     fieldValues[FieldType::cfWDay],
                                     fieldValues[FieldType::cfMon])) {
        BadSyntaxException::raise("Don't have possible value for fields DAY");
      }
    }
  }

  Clock createClock() {
    Clock clock;
    clock.d_->min.setPosibValues(fieldValues[FieldType::cfMin]);
    clock.d_->hour.setPosibValues(fieldValues[FieldType::cfHour]);
    static_cast<npcron::MDay&>(clock.d_->day)
        .setMDayRange(fieldValues[FieldType::cfMDay]);
    static_cast<npcron::MDay&>(clock.d_->day)
        .setWDayRange(fieldValues[FieldType::cfWDay]);

    TimeUnit::PossibleValues monthValues;
    for (auto v : fieldValues[FieldType::cfMon]) {
      monthValues.insert(v - 1);
    }
    clock.d_->mon.setPosibValues(monthValues);

    clock.d_->syncWithLocalTime();
    return clock;
  }

  TimeUnit::PossibleValues fieldValues[FieldType::cfFieldCount];
};

Clock::Clock() : d_(new ClockDetails) {}

Clock::Clock(const Clock& rhs) { d_ = new ClockDetails(*(rhs.d_)); }

Clock::Clock(Clock&& rhs) { d_ = new ClockDetails(move(*(rhs.d_))); }

Clock& Clock::operator=(const Clock& rhs) {
  if (this != &rhs) {
    delete d_;
    rhs.d_->operator=(*rhs.d_);
  }
  return *this;
}

Clock& Clock::operator=(Clock&& rhs) {
  if (this != &rhs) {
    delete d_;
    rhs.d_->operator=(move(*rhs.d_));
  }
  return *this;
}

Clock::~Clock() { delete d_; }

bool Clock::valid() const { return d_->valid(); }

void Clock::syncWithLocalTime(const tm& tmTime) {
  d_->syncWithLocalTime(tmTime);
}

void Clock::syncWithLocalTime(Clock::TimePoint tp) {
  d_->syncWithLocalTime(tp);
}

TimePoint Clock::next() { return d_->doOneStep(&TimeUnit::stepNext); }

TimePoint Clock::previous() { return d_->doOneStep(&TimeUnit::stepBack); }

std::string Clock::ctimeNext() {
  return util::toCTime(system_clock::to_time_t(next()));
}

std::string Clock::ctimeBack() {
  return util::toCTime(system_clock::to_time_t(previous()));
}

Clock parse(const CronExpression expr) {
  Parser parser(expr);
  return parser.createClock();
}

}  // namespace npcron
