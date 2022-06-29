#ifndef __HAPTIC_H__
#define __HAPTIC_H__

#include <Adafruit_DRV2605.h>

#ifdef VERBOSE
const __FlashStringHelper* hestrs[] = {
   F(""),
   F("1 − Strong Click - 100%"),
   F("2 − Strong Click - 60%"),
   F("3 − Strong Click - 30%"),
   F("4 − Sharp Click - 100%"),
   F("5 − Sharp Click - 60%"),
   F("6 − Sharp Click - 30%"),
   F("7 − Soft Bump - 100%"),
   F("8 − Soft Bump - 60%"),
   F("9 − Soft Bump - 30%"),
   F("10 − Double Click - 100%"),
   F("11 − Double Click - 60%"),
   F("12 − Triple Click - 100%"),
   F("13 − Soft Fuzz - 60%"),
   F("14 − Strong Buzz - 100%"),
   F("15 − 750 ms Alert 100%"),
   F("16 − 1000 ms Alert 100%"),
   F("17 − Strong Click 1 - 100%"),
   F("18 − Strong Click 2 - 80%"),
   F("19 − Strong Click 3 - 60%"),
   F("20 − Strong Click 4 - 30%"),
   F("21 − Medium Click 1 - 100%"),
   F("22 − Medium Click 2 - 80%"),
   F("23 − Medium Click 3 - 60%"),
   F("24 − Sharp Tick 1 - 100%"),
   F("25 − Sharp Tick 2 - 80%"),
   F("26 − Sharp Tick 3 – 60%"),
   F("27 − Short Double Click Strong 1 – 100%"),
   F("28 − Short Double Click Strong 2 – 80%"),
   F("29 − Short Double Click Strong 3 – 60%"),
   F("30 − Short Double Click Strong 4 – 30%"),
   F("31 − Short Double Click Medium 1 – 100%"),
   F("32 − Short Double Click Medium 2 – 80%"),
   F("33 − Short Double Click Medium 3 – 60%"),
   F("34 − Short Double Sharp Tick 1 – 100%"),
   F("35 − Short Double Sharp Tick 2 – 80%"),
   F("36 − Short Double Sharp Tick 3 – 60%"),
   F("37 − Long Double Sharp Click Strong 1 – 100%"),
   F("38 − Long Double Sharp Click Strong 2 – 80%"),
   F("39 − Long Double Sharp Click Strong 3 – 60%"),
   F("40 − Long Double Sharp Click Strong 4 – 30%"),
   F("41 − Long Double Sharp Click Medium 1 – 100%"),
   F("42 − Long Double Sharp Click Medium 2 – 80%"),
   F("43 − Long Double Sharp Click Medium 3 – 60%"),
   F("44 − Long Double Sharp Tick 1 – 100%"),
   F("45 − Long Double Sharp Tick 2 – 80%"),
   F("46 − Long Double Sharp Tick 3 – 60%"),
   F("47 − Buzz 1 – 100%"),
   F("48 − Buzz 2 – 80%"),
   F("49 − Buzz 3 – 60%"),
   F("50 − Buzz 4 – 40%"),
   F("51 − Buzz 5 – 20%"),
   F("52 − Pulsing Strong 1 – 100%"),
   F("53 − Pulsing Strong 2 – 60%"),
   F("54 − Pulsing Medium 1 – 100%"),
   F("55 − Pulsing Medium 2 – 60%"),
   F("56 − Pulsing Sharp 1 – 100%"),
   F("57 − Pulsing Sharp 2 – 60%"),
   F("58 − Transition Click 1 – 100%"),
   F("59 − Transition Click 2 – 80%"),
   F("60 − Transition Click 3 – 60%"),
   F("61 − Transition Click 4 – 40%"),
   F("62 − Transition Click 5 – 20%"),
   F("63 − Transition Click 6 – 10%"),
   F("64 − Transition Hum 1 – 100%"),
   F("65 − Transition Hum 2 – 80%"),
   F("66 − Transition Hum 3 – 60%"),
   F("67 − Transition Hum 4 – 40%"),
   F("68 − Transition Hum 5 – 20%"),
   F("69 − Transition Hum 6 – 10%"),
   F("70 − Transition Ramp Down Long Smooth 1 – 100 to 0%"),
   F("71 − Transition Ramp Down Long Smooth 2 – 100 to 0%"),
   F("72 − Transition Ramp Down Medium Smooth 1 – 100 to 0%"),
   F("73 − Transition Ramp Down Medium Smooth 2 – 100 to 0%"),
   F("74 − Transition Ramp Down Short Smooth 1 – 100 to 0%"),
   F("75 − Transition Ramp Down Short Smooth 2 – 100 to 0%"),
   F("76 − Transition Ramp Down Long Sharp 1 – 100 to 0%"),
   F("77 − Transition Ramp Down Long Sharp 2 – 100 to 0%"),
   F("78 − Transition Ramp Down Medium Sharp 1 – 100 to 0%"),
   F("79 − Transition Ramp Down Medium Sharp 2 – 100 to 0%"),
   F("80 − Transition Ramp Down Short Sharp 1 – 100 to 0%"),
   F("81 − Transition Ramp Down Short Sharp 2 – 100 to 0%"),
   F("82 − Transition Ramp Up Long Smooth 1 – 0 to 100%"),
   F("83 − Transition Ramp Up Long Smooth 2 – 0 to 100%"),
   F("84 − Transition Ramp Up Medium Smooth 1 – 0 to 100%"),
   F("85 − Transition Ramp Up Medium Smooth 2 – 0 to 100%"),
   F("86 − Transition Ramp Up Short Smooth 1 – 0 to 100%"),
   F("87 − Transition Ramp Up Short Smooth 2 – 0 to 100%"),
   F("88 − Transition Ramp Up Long Sharp 1 – 0 to 100%"),
   F("89 − Transition Ramp Up Long Sharp 2 – 0 to 100%"),
   F("90 − Transition Ramp Up Medium Sharp 1 – 0 to 100%"),
   F("91 − Transition Ramp Up Medium Sharp 2 – 0 to 100%"),
   F("92 − Transition Ramp Up Short Sharp 1 – 0 to 100%"),
   F("93 − Transition Ramp Up Short Sharp 2 – 0 to 100%"),
   F("94 − Transition Ramp Down Long Smooth 1 – 50 to 0%"),
   F("95 − Transition Ramp Down Long Smooth 2 – 50 to 0%"),
   F("96 − Transition Ramp Down Medium Smooth 1 – 50 to 0%"),
   F("97 − Transition Ramp Down Medium Smooth 2 – 50 to 0%"),
   F("98 − Transition Ramp Down Short Smooth 1 – 50 to 0%"),
   F("99 − Transition Ramp Down Short Smooth 2 – 50 to 0%"),
   F("100 − Transition Ramp Down Long Sharp 1 – 50 to 0%"),
   F("101 − Transition Ramp Down Long Sharp 2 – 50 to 0%"),
   F("102 − Transition Ramp Down Medium Sharp 1 – 50 to 0%"),
   F("103 − Transition Ramp Down Medium Sharp 2 – 50 to 0%"),
   F("104 − Transition Ramp Down Short Sharp 1 – 50 to 0%"),
   F("105 − Transition Ramp Down Short Sharp 2 – 50 to 0%"),
   F("106 − Transition Ramp Up Long Smooth 1 – 0 to 50%"),
   F("107 − Transition Ramp Up Long Smooth 2 – 0 to 50%"),
   F("108 − Transition Ramp Up Medium Smooth 1 – 0 to 50%"),
   F("109 − Transition Ramp Up Medium Smooth 2 – 0 to 50%"),
   F("110 − Transition Ramp Up Short Smooth 1 – 0 to 50%"),
   F("111 − Transition Ramp Up Short Smooth 2 – 0 to 50%"),
   F("112 − Transition Ramp Up Long Sharp 1 – 0 to 50%"),
   F("113 − Transition Ramp Up Long Sharp 2 – 0 to 50%"),
   F("114 − Transition Ramp Up Medium Sharp 1 – 0 to 50%"),
   F("115 − Transition Ramp Up Medium Sharp 2 – 0 to 50%"),
   F("116 − Transition Ramp Up Short Sharp 1 – 0 to 50%"),
   F("117 − Transition Ramp Up Short Sharp 2 – 0 to 50%"),
   F("118 − Long buzz for programmatic stopping – 100%"),
   F("119 − Smooth Hum 1 (No kick or brake pulse) – 50%"),
   F("120 − Smooth Hum 2 (No kick or brake pulse) – 40%"),
   F("121 − Smooth Hum 3 (No kick or brake pulse) – 30%"),
   F("122 − Smooth Hum 4 (No kick or brake pulse) – 20%"),
   F("123 − Smooth Hum 5 (No kick or brake pulse) – 10%")
};
#endif


#define RTC_SHARP_CLICK 4
#define RTC_SOFT_BUMP 7
#define RTC_DOUBLE_CLICK 10
#define RTC_STRONG_BUZZ 14
#define RTC_ALERT 15
#define RTC_STRONG_CLICK 17
#define RTC_STRONG_TICLK 24
#define RTC_PULSE_STRONG_1 52

class Haptic {
  int last_;
  Adafruit_DRV2605 drv_;
public:
  Haptic(): last_(0) {}
  void setup() {
     drv_.begin();
     drv_.selectLibrary(1);
     // I2C trigger by sending 'go' command 
     // default, internal trigger when sending GO command
     drv_.setMode(DRV2605_MODE_INTTRIG);  
  }
  
  void loopAction() {
  }

  void playEffect(uint8_t effect) {
    if (effect != last_) {
      if (effect > 0 && effect < 124) {
       drv_.setWaveform(0, effect);  // play effect 
       drv_.setWaveform(1, 0);       // end waveform
       last_ = effect;
      }
    }
    drv_.go();
  }
};

#endif
