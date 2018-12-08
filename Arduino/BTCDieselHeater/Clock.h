#ifndef __BTC_TIMERS_H__
#define __BTC_TIMERS_H__

#include "BTCDateTime.h"

/*const BTCDateTime& getCurrentTime();
void setCurrentTime(const DateTime& newDateTime);

void initClock();
void checkClock();*/

class CClock {

  RTC_DS3231& _rtc;
  unsigned long _nextRTCfetch;
  BTCDateTime _currentTime;

  void _checkTimers();
  void _checkTimer(int timer, const DateTime& now);
public:
  CClock(RTC_DS3231& rtc);
  void begin();
  const BTCDateTime& update();
  const BTCDateTime& get() const;
  void set(const DateTime& newTime);
};

extern CClock Clock;

#endif // __BTC_TIMERS_H__
