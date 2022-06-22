#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <RTClib.h>
#include <Adafruit_DRV2605.h>

#define OLED_DISPLAY

#ifdef NEO
#include <Adafruit_NeoPixel.h>
// How many internal neopixels do we have? some boards have more than one!
#define NUMPIXELS        1
Adafruit_NeoPixel pixels(NUMPIXELS, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);
#endif

#define VERBOSE

#ifdef VERBOSE
#define VPRINTLN(...) Serial.println(__VA_ARGS__)
#define VPRINT(...) Serial.print(__VA_ARGS__)
#else
#define VPRINTLN(...) 
#define VPRINT(...) 
#endif 

//#define CONFIG_REMINDER_ALARM
#define CONFIG_PERIODIC_ALARM

#define DEFAULT_PERIOD_MINUTES 1

// the pin that is connected to SQW
#define CLOCK_INTERRUPT_PIN A3

#define REMINDER_RTC_ALARM 1

#define PERIODIC_RTC_ALARM 2


#define STRONG_CLICK 17
#define STRONG_BUZZ 14
#define BUTTON_PRESSED_HAPTIC STRONG_CLICK
#define PERIODIC_ALARM_HAPTIC STRONG_BUZZ

unsigned int event = 0;
#define RTC_ALARM_EVENT (1U << 1 )

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);
RTC_DS3231 rtc;
Adafruit_DRV2605 haptic;
DateTime nextPeriodicAlarm;


void setNextPeriodic() {
        DateTime tnow = rtc.now();
        uint32_t nowSecs = tnow.secondstime();
        TimeSpan secondsToNext((60 * DEFAULT_PERIOD_MINUTES) - (nowSecs % (60 * DEFAULT_PERIOD_MINUTES)));
        nextPeriodicAlarm = tnow + secondsToNext;
        
        if(!rtc.setAlarm2(nextPeriodicAlarm, DS3231_A2_Minute)) {
              VPRINTLN("Error, alarm wasn't set!");
         } else {
            VPRINT("Periodic Alarm in: "); VPRINT(DEFAULT_PERIOD_MINUTES);
            VPRINTLN(" minutes");
        }
}

void 
displayNextPeriodic()
{
  char timebuf[9] = "hh:mm:ss";

  nextPeriodicAlarm.toString(timebuf);
  VPRINT("Next Periodic: ");
  VPRINTLN(timebuf);

#ifdef OLED_DISPLAY
  display.setTextSize(1);
  display.print(timebuf);
#endif
}

#define CLEAR_DISPLAY_AFTER_MS 5000

unsigned long clearDisplay = 0;

void updateDisplay(bool clear)
{
  display.display();
  if (clear) {
     clearDisplay = millis() + CLEAR_DISPLAY_AFTER_MS;
  } else {   
     clearDisplay = 0;
  }
}


void sleepDisplay() {
  display.ssd1306_command(SSD1306_DISPLAYOFF);
}

void wakeDisplay() {
  display.ssd1306_command(SSD1306_DISPLAYON);
}
void 
displayMsg(const char *msg) {
  VPRINT(msg);
  display.setTextSize(1);
  display.print(msg);
}

void
displayTime()
{
  char datebuf[20] = "DDD DD MMM DD/MM/YY";
  char timebuf[9] = "hh:mm:ss";


  DateTime now = rtc.now();
  now.toString(datebuf);
  now.toString(timebuf);

  VPRINTLN(datebuf);
  VPRINTLN(timebuf);
  
  display.setCursor(0,0);
  display.setTextSize(1);
  display.println(datebuf);

  display.setTextSize(2);
  display.println(timebuf);
}

void playEffect(uint8_t effect)
{
#ifdef VERBOSE  
  if (effect == 1) {
    VPRINTLN("11.2 Waveform Library Effects List");
  }

  if (effect == 1) {
    VPRINTLN(F("1 − Strong Click - 100%"));
  }
  if (effect == 2) {
    VPRINTLN(F("2 − Strong Click - 60%"));
  }
  if (effect == 3) {
    VPRINTLN(F("3 − Strong Click - 30%"));
  }
  if (effect == 4) {
    VPRINTLN(F("4 − Sharp Click - 100%"));
  }
  if (effect == 5) {
    VPRINTLN(F("5 − Sharp Click - 60%"));
  }
  if (effect == 6) {
    VPRINTLN(F("6 − Sharp Click - 30%"));
  }
  if (effect == 7) {
    VPRINTLN(F("7 − Soft Bump - 100%"));
  }
  if (effect == 8) {
    VPRINTLN(F("8 − Soft Bump - 60%"));
  }
  if (effect == 9) {
    VPRINTLN(F("9 − Soft Bump - 30%"));
  }
  if (effect == 10) {
    VPRINTLN(F("10 − Double Click - 100%"));
  }
  if (effect == 11) {
    VPRINTLN(F("11 − Double Click - 60%"));
  }
  if (effect == 12) {
    VPRINTLN(F("12 − Triple Click - 100%"));
  }
  if (effect == 13) {
    VPRINTLN(F("13 − Soft Fuzz - 60%"));
  }
  if (effect == 14) {
    VPRINTLN(F("14 − Strong Buzz - 100%"));
  }
  if (effect == 15) {
    VPRINTLN(F("15 − 750 ms Alert 100%"));
  }
  if (effect == 16) {
    VPRINTLN(F("16 − 1000 ms Alert 100%"));
  }
  if (effect == 17) {
    VPRINTLN(F("17 − Strong Click 1 - 100%"));
  }
  if (effect == 18) {
    VPRINTLN(F("18 − Strong Click 2 - 80%"));
  }
  if (effect == 19) {
    VPRINTLN(F("19 − Strong Click 3 - 60%"));
  }
  if (effect == 20) {
    VPRINTLN(F("20 − Strong Click 4 - 30%"));
  }
  if (effect == 21) {
    VPRINTLN(F("21 − Medium Click 1 - 100%"));
  }
  if (effect == 22) {
    VPRINTLN(F("22 − Medium Click 2 - 80%"));
  }
  if (effect == 23) {
    VPRINTLN(F("23 − Medium Click 3 - 60%"));
  }
  if (effect == 24) {
    VPRINTLN(F("24 − Sharp Tick 1 - 100%"));
  }
  if (effect == 25) {
    VPRINTLN(F("25 − Sharp Tick 2 - 80%"));
  }
  if (effect == 26) {
    VPRINTLN(F("26 − Sharp Tick 3 – 60%"));
  }
  if (effect == 27) {
    VPRINTLN(F("27 − Short Double Click Strong 1 – 100%"));
  }
  if (effect == 28) {
    VPRINTLN(F("28 − Short Double Click Strong 2 – 80%"));
  }
  if (effect == 29) {
    VPRINTLN(F("29 − Short Double Click Strong 3 – 60%"));
  }
  if (effect == 30) {
    VPRINTLN(F("30 − Short Double Click Strong 4 – 30%"));
  }
  if (effect == 31) {
    VPRINTLN(F("31 − Short Double Click Medium 1 – 100%"));
  }
  if (effect == 32) {
    VPRINTLN(F("32 − Short Double Click Medium 2 – 80%"));
  }
  if (effect == 33) {
    VPRINTLN(F("33 − Short Double Click Medium 3 – 60%"));
  }
  if (effect == 34) {
    VPRINTLN(F("34 − Short Double Sharp Tick 1 – 100%"));
  }
  if (effect == 35) {
    VPRINTLN(F("35 − Short Double Sharp Tick 2 – 80%"));
  }
  if (effect == 36) {
    VPRINTLN(F("36 − Short Double Sharp Tick 3 – 60%"));
  }
  if (effect == 37) {
    VPRINTLN(F("37 − Long Double Sharp Click Strong 1 – 100%"));
  }
  if (effect == 38) {
    VPRINTLN(F("38 − Long Double Sharp Click Strong 2 – 80%"));
  }
  if (effect == 39) {
    VPRINTLN(F("39 − Long Double Sharp Click Strong 3 – 60%"));
  }
  if (effect == 40) {
    VPRINTLN(F("40 − Long Double Sharp Click Strong 4 – 30%"));
  }
  if (effect == 41) {
    VPRINTLN(F("41 − Long Double Sharp Click Medium 1 – 100%"));
  }
  if (effect == 42) {
    VPRINTLN(F("42 − Long Double Sharp Click Medium 2 – 80%"));
  }
  if (effect == 43) {
    VPRINTLN(F("43 − Long Double Sharp Click Medium 3 – 60%"));
  }
  if (effect == 44) {
    VPRINTLN(F("44 − Long Double Sharp Tick 1 – 100%"));
  }
  if (effect == 45) {
    VPRINTLN(F("45 − Long Double Sharp Tick 2 – 80%"));
  }
  if (effect == 46) {
    VPRINTLN(F("46 − Long Double Sharp Tick 3 – 60%"));
  }
  if (effect == 47) {
    VPRINTLN(F("47 − Buzz 1 – 100%"));
  }
  if (effect == 48) {
    VPRINTLN(F("48 − Buzz 2 – 80%"));
  }
  if (effect == 49) {
    VPRINTLN(F("49 − Buzz 3 – 60%"));
  }
  if (effect == 50) {
    VPRINTLN(F("50 − Buzz 4 – 40%"));
  }
  if (effect == 51) {
    VPRINTLN(F("51 − Buzz 5 – 20%"));
  }
  if (effect == 52) {
    VPRINTLN(F("52 − Pulsing Strong 1 – 100%"));
  }
  if (effect == 53) {
    VPRINTLN(F("53 − Pulsing Strong 2 – 60%"));
  }
  if (effect == 54) {
    VPRINTLN(F("54 − Pulsing Medium 1 – 100%"));
  }
  if (effect == 55) {
    VPRINTLN(F("55 − Pulsing Medium 2 – 60%"));
  }
  if (effect == 56) {
    VPRINTLN(F("56 − Pulsing Sharp 1 – 100%"));
  }
  if (effect == 57) {
    VPRINTLN(F("57 − Pulsing Sharp 2 – 60%"));
  }
  if (effect == 58) {
    VPRINTLN(F("58 − Transition Click 1 – 100%"));
  }
  if (effect == 59) {
    VPRINTLN(F("59 − Transition Click 2 – 80%"));
  }
  if (effect == 60) {
    VPRINTLN(F("60 − Transition Click 3 – 60%"));
  }
  if (effect == 61) {
    VPRINTLN(F("61 − Transition Click 4 – 40%"));
  }
  if (effect == 62) {
    VPRINTLN(F("62 − Transition Click 5 – 20%"));
  }
  if (effect == 63) {
    VPRINTLN(F("63 − Transition Click 6 – 10%"));
  }
  if (effect == 64) {
    VPRINTLN(F("64 − Transition Hum 1 – 100%"));
  }
  if (effect == 65) {
    VPRINTLN(F("65 − Transition Hum 2 – 80%"));
  }
  if (effect == 66) {
    VPRINTLN(F("66 − Transition Hum 3 – 60%"));
  }
  if (effect == 67) {
    VPRINTLN(F("67 − Transition Hum 4 – 40%"));
  }
  if (effect == 68) {
    VPRINTLN(F("68 − Transition Hum 5 – 20%"));
  }
  if (effect == 69) {
    VPRINTLN(F("69 − Transition Hum 6 – 10%"));
  }
  if (effect == 70) {
    VPRINTLN(F("70 − Transition Ramp Down Long Smooth 1 – 100 to 0%"));
  }
  if (effect == 71) {
    VPRINTLN(F("71 − Transition Ramp Down Long Smooth 2 – 100 to 0%"));
  }
  if (effect == 72) {
    VPRINTLN(F("72 − Transition Ramp Down Medium Smooth 1 – 100 to 0%"));
  }
  if (effect == 73) {
    VPRINTLN(F("73 − Transition Ramp Down Medium Smooth 2 – 100 to 0%"));
  }
  if (effect == 74) {
    VPRINTLN(F("74 − Transition Ramp Down Short Smooth 1 – 100 to 0%"));
  }
  if (effect == 75) {
    VPRINTLN(F("75 − Transition Ramp Down Short Smooth 2 – 100 to 0%"));
  }
  if (effect == 76) {
    VPRINTLN(F("76 − Transition Ramp Down Long Sharp 1 – 100 to 0%"));
  }
  if (effect == 77) {
    VPRINTLN(F("77 − Transition Ramp Down Long Sharp 2 – 100 to 0%"));
  }
  if (effect == 78) {
    VPRINTLN(F("78 − Transition Ramp Down Medium Sharp 1 – 100 to 0%"));
  }
  if (effect == 79) {
    VPRINTLN(F("79 − Transition Ramp Down Medium Sharp 2 – 100 to 0%"));
  }
  if (effect == 80) {
    VPRINTLN(F("80 − Transition Ramp Down Short Sharp 1 – 100 to 0%"));
  }
  if (effect == 81) {
    VPRINTLN(F("81 − Transition Ramp Down Short Sharp 2 – 100 to 0%"));
  }
  if (effect == 82) {
    VPRINTLN(F("82 − Transition Ramp Up Long Smooth 1 – 0 to 100%"));
  }
  if (effect == 83) {
    VPRINTLN(F("83 − Transition Ramp Up Long Smooth 2 – 0 to 100%"));
  }
  if (effect == 84) {
    VPRINTLN(F("84 − Transition Ramp Up Medium Smooth 1 – 0 to 100%"));
  }
  if (effect == 85) {
    VPRINTLN(F("85 − Transition Ramp Up Medium Smooth 2 – 0 to 100%"));
  }
  if (effect == 86) {
    VPRINTLN(F("86 − Transition Ramp Up Short Smooth 1 – 0 to 100%"));
  }
  if (effect == 87) {
    VPRINTLN(F("87 − Transition Ramp Up Short Smooth 2 – 0 to 100%"));
  }
  if (effect == 88) {
    VPRINTLN(F("88 − Transition Ramp Up Long Sharp 1 – 0 to 100%"));
  }
  if (effect == 89) {
    VPRINTLN(F("89 − Transition Ramp Up Long Sharp 2 – 0 to 100%"));
  }
  if (effect == 90) {
    VPRINTLN(F("90 − Transition Ramp Up Medium Sharp 1 – 0 to 100%"));
  }
  if (effect == 91) {
    VPRINTLN(F("91 − Transition Ramp Up Medium Sharp 2 – 0 to 100%"));
  }
  if (effect == 92) {
    VPRINTLN(F("92 − Transition Ramp Up Short Sharp 1 – 0 to 100%"));
  }
  if (effect == 93) {
    VPRINTLN(F("93 − Transition Ramp Up Short Sharp 2 – 0 to 100%"));
  }
  if (effect == 94) {
    VPRINTLN(F("94 − Transition Ramp Down Long Smooth 1 – 50 to 0%"));
  }
  if (effect == 95) {
    VPRINTLN(F("95 − Transition Ramp Down Long Smooth 2 – 50 to 0%"));
  }
  if (effect == 96) {
    VPRINTLN(F("96 − Transition Ramp Down Medium Smooth 1 – 50 to 0%"));
  }
  if (effect == 97) {
    VPRINTLN(F("97 − Transition Ramp Down Medium Smooth 2 – 50 to 0%"));
  }
  if (effect == 98) {
    VPRINTLN(F("98 − Transition Ramp Down Short Smooth 1 – 50 to 0%"));
  }
  if (effect == 99) {
    VPRINTLN(F("99 − Transition Ramp Down Short Smooth 2 – 50 to 0%"));
  }
  if (effect == 100) {
    VPRINTLN(F("100 − Transition Ramp Down Long Sharp 1 – 50 to 0%"));
  }
  if (effect == 101) {
    VPRINTLN(F("101 − Transition Ramp Down Long Sharp 2 – 50 to 0%"));
  }
  if (effect == 102) {
    VPRINTLN(F("102 − Transition Ramp Down Medium Sharp 1 – 50 to 0%"));
  }
  if (effect == 103) {
    VPRINTLN(F("103 − Transition Ramp Down Medium Sharp 2 – 50 to 0%"));
  }
  if (effect == 104) {
    VPRINTLN(F("104 − Transition Ramp Down Short Sharp 1 – 50 to 0%"));
  }
  if (effect == 105) {
    VPRINTLN(F("105 − Transition Ramp Down Short Sharp 2 – 50 to 0%"));
  }
  if (effect == 106) {
    VPRINTLN(F("106 − Transition Ramp Up Long Smooth 1 – 0 to 50%"));
  }
  if (effect == 107) {
    VPRINTLN(F("107 − Transition Ramp Up Long Smooth 2 – 0 to 50%"));
  }
  if (effect == 108) {
    VPRINTLN(F("108 − Transition Ramp Up Medium Smooth 1 – 0 to 50%"));
  }
  if (effect == 109) {
    VPRINTLN(F("109 − Transition Ramp Up Medium Smooth 2 – 0 to 50%"));
  }
  if (effect == 110) {
    VPRINTLN(F("110 − Transition Ramp Up Short Smooth 1 – 0 to 50%"));
  }
  if (effect == 111) {
    VPRINTLN(F("111 − Transition Ramp Up Short Smooth 2 – 0 to 50%"));
  }
  if (effect == 112) {
    VPRINTLN(F("112 − Transition Ramp Up Long Sharp 1 – 0 to 50%"));
  }
  if (effect == 113) {
    VPRINTLN(F("113 − Transition Ramp Up Long Sharp 2 – 0 to 50%"));
  }
  if (effect == 114) {
    VPRINTLN(F("114 − Transition Ramp Up Medium Sharp 1 – 0 to 50%"));
  }
  if (effect == 115) {
    VPRINTLN(F("115 − Transition Ramp Up Medium Sharp 2 – 0 to 50%"));
  }
  if (effect == 116) {
    VPRINTLN(F("116 − Transition Ramp Up Short Sharp 1 – 0 to 50%"));
  }
  if (effect == 117) {
    VPRINTLN(F("117 − Transition Ramp Up Short Sharp 2 – 0 to 50%"));
  }
  if (effect == 118) {
    VPRINTLN(F("118 − Long buzz for programmatic stopping – 100%"));
  }
  if (effect == 119) {
    VPRINTLN(F("119 − Smooth Hum 1 (No kick or brake pulse) – 50%"));
  }
  if (effect == 120) {
    VPRINTLN(F("120 − Smooth Hum 2 (No kick or brake pulse) – 40%"));
  }
  if (effect == 121) {
    VPRINTLN(F("121 − Smooth Hum 3 (No kick or brake pulse) – 30%"));
  }
  if (effect == 122) {
    VPRINTLN(F("122 − Smooth Hum 4 (No kick or brake pulse) – 20%"));
  }
  if (effect == 123) {
    VPRINTLN(F("123 − Smooth Hum 5 (No kick or brake pulse) – 10%"));
  }
#endif
  // set the effect to play
  haptic.setWaveform(0, effect);  // play effect 
  haptic.setWaveform(1, 0);       // end waveform

  // play the effect!
  haptic.go();
}

void setup() {
#ifdef NEO  
   pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.setBrightness(20); // not so bright
   // set color to red
  pixels.fill(0x0000FF);
  pixels.show();
#endif
  
#ifdef VERBOSE  
    Serial.begin(115200);
#ifndef ESP8266
  while (!Serial); // wait for serial port to connect. Needed for native USB
#endif
#endif
  // put your setup code here, to run once:
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
 
 

  if (! rtc.begin()) {
#ifdef VERBOSE        
    Serial.println("Couldn't find RTC");
    Serial.flush();
#endif
    while (1) delay(10);
  }

 if (rtc.lostPower()) {
    VPRINTLN("RTC lost power, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

 //we don't need the 32K Pin, so disable it
    rtc.disable32K();
     // stop oscillating signals at SQW Pin
    // otherwise setAlarm1 will fail
    rtc.writeSqwPinMode(DS3231_OFF);
     rtc.clearAlarm(REMINDER_RTC_ALARM);
    rtc.clearAlarm(PERIODIC_RTC_ALARM);
    
        // Making it so, that the alarm will trigger an interrupt
    pinMode(CLOCK_INTERRUPT_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(CLOCK_INTERRUPT_PIN), onAlarm, FALLING);
     // set alarm 1, 2 flag to false (so alarm 1, 2 didn't happen so far)
    // if not done, this easily leads to problems, as both register aren't reset on reboot/recompile
   #ifdef CONFIG_PERIODIC_ALARM
    setNextPeriodic();
    VPRINTLN("rtc: PERIODIC ALARM CONFIGURED");
#else
    rtc.disableAlarm(PERIODIC_RTC_ALARM);
    VPRINTLN("rtc: PERIODIC ALARM NOT CONFIGURED");
#endif

#ifdef CONFIG_REMINDER_ALARM
    VPRINTLN("rtc: REMINDER ALARM NYI!!!");
#else
    rtc.disableAlarm(REMINDER_RTC_ALARM);
    VPRINTLN("rtc: PERIODIC REMINDER NOT CONFIGURED");
#endif

    
     haptic.begin();
     haptic.selectLibrary(1);
      
  // I2C trigger by sending 'go' command 
  // default, internal trigger when sending GO command
    haptic.setMode(DRV2605_MODE_INTTRIG);  
    
  //we don't need the 32K Pin, so disable it
  rtc.disable32K();
  // When time needs to be re-set on a previously configured device, the
  // following line sets the RTC to the date & time this sketch was compiled
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // This line sets the RTC with an explicit date & time, for example to set
  // January 21, 2014 at 3am you would call:
  // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  delay(1000); // let system settle to see if it helps with battery oled startup issues
  
   display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3C for 128x32
   // Clear the buffer.
  display.clearDisplay();
  //display.display();
    // text display tests
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE,SSD1306_BLACK);

  delay(1000); // let system settle to see if it helps with battery oled startup issues
  // displayTime();
  displayMsg("NUDGE v0.2");
  
  display.display();
}

void loop() {

  // put your main code here, to run repeatedly:
  // playEffect(14);
 if (event) {
   if (event & RTC_ALARM_EVENT) {
      event &= ~RTC_ALARM_EVENT;
      if (rtc.alarmFired(PERIODIC_RTC_ALARM)) {
        rtc.clearAlarm(PERIODIC_RTC_ALARM);
        VPRINTLN("Periodic Alarm");
        setNextPeriodic();
#ifdef OLED_DISPLAY
        wakeDisplay();
        displayTime(); 
        displayMsg("Periodic");
        updateDisplay(true);   
#endif
#ifdef LED_PIN
        toggleLed();
#endif
        playEffect(PERIODIC_ALARM_HAPTIC);
        delay(500);
#ifdef LED_PIN
        toggleLed();
#endif
        }   
    }
  }
#ifdef OLED_DISPLAY 
  if (clearDisplay) {
    if (millis() > clearDisplay) {
        display.clearDisplay();
        sleepDisplay();
        clearDisplay = 0;
    }
#ifdef UPDATE_TIME    
    else {
        displayTime();
#ifdef OLED_DISPLAY
        display.display();
#endif
    }
#endif
#endif
  } 
}

void onAlarm() {
  event |= RTC_ALARM_EVENT;
}
