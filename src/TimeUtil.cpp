#include <npcron/TimeUtil.h>

#include <bitset>

namespace npcron {

unsigned int util::dayEndOfMonth(unsigned int month, int year) {
  if (month <= 11) {
    unsigned int maxPermonth = 31;
    static const std::bitset<12> MONTH31s{
        "101011010101"};  //[11 .. <- .. 0 ] the months that have 31 days
    if (1 == month)       // Febuary
    {
      if (util::isLeapYear(year)) {
        maxPermonth = 29;
      } else {
        maxPermonth = 28;
      }
    } else if (!MONTH31s.test(month)) {
      maxPermonth = 30;
    }
    return maxPermonth;
  } else {
    return 0;
  }
}

void util::currentLocalTime(std::tm &tmlocal) {
  timet2local(std::time(nullptr), tmlocal);
}

int util::getWeekDayOf(int year, int mon, int day) {
  std::tm tmTime = createTimeInfo(year, mon, day);
  auto tt = ::mktime(&tmTime);
  timet2local(tt, tmTime);
  return tmTime.tm_wday;
}

bool util::isLeapYear(int year) {
  return ((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0);
}

tm util::createTimeInfo(int year, int month, int mDay, int hour, int min,
                        int sec) {
  return std::tm{
      sec,    // int tm_sec;
      min,    // int tm_min;
      hour,   // int tm_hour;
      mDay,   // int tm_mday;
      month,  // int tm_mon;
      year,   // int tm_year;
      0,      // int tm_wday;
      0,      // int tm_yday;
      -1,     // int tm_isdst;
  };
}

std::string util::toCTime(time_t tt) {
  char sCTime[100] = {'\0'};
#ifdef WIN32
  ::ctime_s(sCTime, 100, &tt);
#else
  ctime_r(&tt, sCTime);
#endif
  return sCTime;
}

void util::timet2local(time_t tt, tm &tmlocal) {
#ifdef WIN32
  ::localtime_s(&tmlocal, &tt);
#else
  ::localtime_r(&tt, &tmlocal);
#endif
}

}  // namespace npcron
