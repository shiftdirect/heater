#include <Arduino.h>
#include "Clock.h"
#include "BTCDateTime.h"
#include "Wire.h"
#include "RTClib.h"
#include "helpers.h"
#include "NVStorage.h"
#include "DebugPort.h"


// create ONE of the RTClib supported real time clock classes
#if RTC_USE_DS3231 == 1
RTC_DS3231 rtc;
#elif RTC_USE_DS1307 == 1
RTC_DS1307 rtc;
#elif RTC_USE_PCF8523 == 1
RTC_PCF8523 rtc;
#else
RTC_Millis rtc;
#endif

CClock Clock(rtc);


void 
CClock::begin()
{
  // announce which sort of RTC is being used
#if RTC_USE_DS3231 == 1
DebugPort.println("Using DS3231 Real Time Clock");
#elif RTC_USE_DS1307 == 1
DebugPort.println("Using DS1307 Real Time Clock");
#elif RTC_USE_PCF8523 == 1
DebugPort.println("Using PCF8523 Real Time Clock");
#else
#define SW_RTC    // enable different begin() call for the millis() based RTC
DebugPort.println("Using millis() based psuedo \"Real Time Clock\"");
#endif

#ifdef SW_RTC
  DateTime zero(2019, 1, 1);   // can be pushed along as seen fit!
  _rtc.begin(zero);
#else
  _rtc.begin();
#endif

  _nextRTCfetch = millis();
}

const BTCDateTime& 
CClock::update()
{
  long deltaT = millis() - _nextRTCfetch;
  if(deltaT >= 0) {
    uint32_t origClock = Wire.getClock();
    Wire.setClock(400000);
    _currentTime = _rtc.now();             // moderate I2C accesses
    Wire.setClock(origClock);
    _nextRTCfetch = millis() + 500;
    _checkTimers();
  }
  return _currentTime;
}

const BTCDateTime& 
CClock::get() const
{
  return _currentTime;
}

void 
CClock::set(const DateTime& newTimeDate)
{
  _rtc.adjust(newTimeDate);
}

void 
CClock::_checkTimers()
{
  _checkTimer(0, _currentTime);   // test timer 1
  _checkTimer(1, _currentTime);   // test timer 2
}

void 
CClock::_checkTimer(int timer, const DateTime& now)
{
  sTimer Info;
  NVstore.getTimerInfo(timer, Info);
  int DOW = now.dayOfTheWeek();
  int timeNow = now.hour() * 60 + now.minute();
  int timeStart = Info.start.hour * 60 + Info.start.min;
  int timeStop = Info.stop.hour * 60 + Info.stop.min;

  // ensure DOW tracks expected start day should timer straddle midnight
  if(timeStop < timeStart) {   // true if stop is next morning
    if(timeNow <= timeStop) {  // current time has passed midnight - enable flag is based upon prior day
      DOW--;
      ROLLLOWERLIMIT(DOW, 0, 6);   // fixup for saturday night!
    }
  }
  // DOW of week is now correct for the day this timer started
  int maskDOW = 0x01 << DOW;

  if(Info.enabled & (maskDOW | 0x80) ) {  // specific day of week, or next day
    
    if(timeNow == timeStart && now.second() < 3) {  // check start, within 2 seconds of the minute rollover
      requestOn();
    }
    
    if(timeNow == timeStop) {            // check stop
      requestOff();
      if(!Info.repeat) {            // cancel timer if non repeating
        if(Info.enabled & 0x80)     // next day start flag set?
          Info.enabled = 0;         // outright cancel
        else {
          Info.enabled &= ~maskDOW; // otherwise clear specific day
        }
        NVstore.setTimerInfo(timer, Info);
        NVstore.save();
      }
    }
  }
}
