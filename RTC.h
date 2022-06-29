#ifndef __RTC_H__
#define __RTC_H__

#include <RTClib.h>
#include "OLED.h"


//#define CONFIG_REMINDER_ALARM
#define CONFIG_PERIODIC_ALARM

#define REMINDER_RTC_ALARM 1
#define PERIODIC_RTC_ALARM 2
#define DEFAULT_PERIOD_MINUTES 20

class RTC : public RTC_DS3231 {
  int intPin_;
  DateTime nextPeriodicAlarm_;
  void (*onAlarm_)(void);
public:
  RTC(int intPin, void (*onAlarm)() ) : intPin_(intPin), onAlarm_(onAlarm) {}
  bool getAlarm1(DateTime &dt, Ds3231Alarm1Mode &alarm_mode)  {
    uint8_t buffer[4];
    buffer[0] = 0x07;   // DS3231 ARLM 1 ADDR
    i2c_dev->write_then_read(buffer, 1, buffer, 4);
    for (int i=0; i<4; i++){ 
       VPRINTLN(buffer[i], HEX);
    }
    return true;
  }    
  bool getAlarm2(DateTime &dt, Ds3231Alarm2Mode &alarm_mode) {
    uint8_t buffer[3];
    buffer[0] = 0x0b;   // DS3231 ARLM 2 ADDR
    i2c_dev->write_then_read(buffer, 1, buffer, 3);
    for (int i=0; i<3; i++){ 
       VPRINTLN(buffer[i], HEX);
    }
    return true;                
  }
  
  void setNextPeriodic(int minutes=DEFAULT_PERIOD_MINUTES) {
        DateTime tnow = RTC::now();
        uint32_t nowSecs = tnow.secondstime();
        TimeSpan secondsToNext((60 * minutes) - (nowSecs % (60 * minutes)));
        nextPeriodicAlarm_ = tnow + secondsToNext;
        
        if(!RTC::setAlarm2(nextPeriodicAlarm_, DS3231_A2_Minute)) {
              VPRINTLN("Error, alarm wasn't set!");
         } else {
            VPRINT("Periodic Alarm in: "); VPRINT(minutes);
            VPRINTLN(" minutes");
        }
  }
  
  void displayNextPeriodic(OLED &oled) {
    char timebuf[9] = "hh:mm:ss";
    nextPeriodicAlarm_.toString(timebuf);
    VPRINT("Next Periodic: ");
    VPRINTLN(timebuf);
    oled.setTextSize(1);
    oled.print(timebuf);
  }

  void displayTime(OLED &oled)  {
    char datebuf[20] = "DDD DD MMM DD/MM/YY";
    char timebuf[9] = "hh:mm:ss";
    DateTime now = RTC::now();
    now.toString(datebuf);
    now.toString(timebuf);
    VPRINTLN(datebuf);
    VPRINTLN(timebuf);
    oled.setCursor(0,0);
    oled.setTextSize(1);
    oled.println(datebuf);
    oled.setTextSize(2);
    oled.println(timebuf);
  }
  
  void setup() {
       // initializing the rtc
    if(!RTC::begin()) {
        VPRINTLN("Couldn't find RTC!");
        Serial.flush();
        while (1) delay(10);
    }

  VPRINTLN("rtc: started");
#ifdef SET_RTC_TIME
       VPRINTLN("RTC SET TIME!");
        // this will adjust to the date and time at compilation
        RTC::adjust(DateTime(F(__DATE__), F(__TIME__)));
#endif

    //we don't need the 32K Pin, so disable it
    RTC::disable32K();

    // Making it so, that the alarm will trigger an interrupt
    pinMode(intPin_, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(intPin_), onAlarm_, FALLING);

    // set alarm 1, 2 flag to false (so alarm 1, 2 didn't happen so far)
    // if not done, this easily leads to problems, as both register aren't reset on reboot/recompile
    RTC::clearAlarm(REMINDER_RTC_ALARM);
    RTC::clearAlarm(PERIODIC_RTC_ALARM);

    // stop oscillating signals at SQW Pin
    // otherwise setAlarm1 will fail
    RTC::writeSqwPinMode(DS3231_OFF);

#ifdef CONFIG_PERIODIC_ALARM
    setNextPeriodic();
    VPRINTLN("rtc: PERIODIC ALARM CONFIGURED");
#else
    rtc::disableAlarm(PERIODIC_RTC_ALARM);
    VPRINTLN("rtc: PERIODIC ALARM NOT CONFIGURED");
#endif

#ifdef CONFIG_REMINDER_ALARM
    VPRINTLN("rtc: REMINDER ALARM NYI!!!");
#else
    RTC::disableAlarm(REMINDER_RTC_ALARM);
    VPRINTLN("rtc: PERIODIC REMINDER NOT CONFIGURED");
#endif 
  }

  void loopAction() {
  }
};
#endif
