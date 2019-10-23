#pragma once

#include <npcron/chrono.h>

#include <memory>
#include <string>

namespace npcron {

enum TimePointAnchor { FromNow, FromSpecificPoint };

using CronExpression = std::string;
class Clock {
 public:
  using SystemClock = std::chrono::system_clock;
  using TimePoint = SystemClock::time_point;

  Clock();
  Clock(const Clock& rhs);
  Clock(Clock&& rhs);
  Clock& operator=(const Clock& rhs);
  Clock& operator=(Clock&& rhs);
  ~Clock();
  bool valid() const;
  void syncWithLocalTime(const tm& tmTime);
  void syncWithLocalTime(TimePoint tp = SystemClock::now());
  TimePoint next();
  TimePoint previous();
  std::string ctimeNext();
  std::string ctimeBack();

 private:
  friend struct Parser;
  struct ClockDetails* d_ = nullptr;
};

Clock parse(const CronExpression expr);

}  // namespace npcron
