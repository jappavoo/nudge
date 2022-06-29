#define VERBOSE
#define QTPY

#define SPLASH F("NUDGE v0.4")

#include "misc.h"
#include "OLED.h"
#include "Button.h"
#include "haptic.h"
#include "RTC.h"

// PIN mappings for non I2C connections

#define BUTTON_PIN A0

#ifdef QTPY

#define OLED_RST_PIN A1
#define RTC_INTERRUPT_PIN A3

#else
// we do not use explicit reset pin on feather board
// the pin that is connected to SQW
#define RTC_INTERRUPT_PIN A2

#endif

// 
#define BUTTON_PRESSED_HAPTIC RTC_STRONG_BUZZ
#define PERIODIC_ALARM_HAPTIC RTC_STRONG_BUZZ

void rtc_alarm(void);

OLED oled(OLED_RST_PIN);
Button but(BUTTON_PIN);
Haptic haptic;
RTC    rtc(RTC_INTERRUPT_PIN, rtc_alarm);
unsigned int event = 0;

#define RTC_ALARM_EVENT (1U << 1 )
void rtc_alarm() {
  event |= RTC_ALARM_EVENT;
}

void setup() {
  misc_setup();
  VPRINTLN("setup: START");

  haptic.setup();
  haptic.playEffect(BUTTON_PRESSED_HAPTIC);
  rtc.setup();
  but.setup();
  
#ifdef QTPY
  // need time for display to settle on QTPY
  delay(1000);
#endif  
  oled.setup();

  rtc.displayTime(oled);
  oled.msg((char *)SPLASH);
  oled.update();
    
  VPRINTLN("setup: END");
}

void loop() {

  but.loopAction();

  if (but.pressed()) {
    VPRINTLN("button.pressed");
    // oled.setup();
    haptic.playEffect(BUTTON_PRESSED_HAPTIC);
    rtc.displayTime(oled);
    oled.msg("NA:");
    rtc.displayNextPeriodic(oled);
    oled.update();
  } 

  if (event) {
    if (event & RTC_ALARM_EVENT) {
      event &= ~RTC_ALARM_EVENT;
      if (rtc.alarmFired(PERIODIC_RTC_ALARM)) {
        rtc.clearAlarm(PERIODIC_RTC_ALARM);
        VPRINTLN("Periodic Alarm");
        rtc.setNextPeriodic();     
      }
    }
  }
  
  oled.loopAction();
}
