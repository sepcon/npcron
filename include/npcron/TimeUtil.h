#pragma once

#include <npcron/chrono.h>

#include <string>

namespace npcron {
namespace util {

unsigned int dayEndOfMonth(unsigned int month, int year);
void currentLocalTime(std::tm& tmlocal);
void timet2local(std::time_t tt, std::tm& tmlocal);
int getWeekDayOf(int year, int mon, int day);
bool isLeapYear(int year);
std::tm createTimeInfo(int year, int month, int mDay = 0, int hour = 0,
                       int min = 0, int sec = 0);
std::string toCTime(std::time_t tt);

template <class Theclock>
std::string toCTime(typename Theclock::time_point tp) {
  auto tmt = Theclock::to_time_t(tp);
  return util::toCTime(tmt);
}

};  // namespace util
}  // namespace npcron
