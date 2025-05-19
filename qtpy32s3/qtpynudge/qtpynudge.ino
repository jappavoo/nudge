// Date and time functions using a DS3231 RTC connected via I2C and Wire lib
#include "RTClib.h"
#include "Adafruit_DRV2605.h"
#include <Adafruit_NeoPixel.h>

// CONFIG FEATURES OF THE CODE
//#define NOSERIAL
#define LOW_POWER
#define USE_LED

#ifdef LOW_POWER
#define NOSERIAL
#endif

#ifndef NOSERIAL
#define USE_LED
#define SPRN(str) Serial.print(str)
#define SPRNL(str) Serial.println(str)
#define SPRNF(fmt,...) Serial.printf(fmt, ##__VA_ARGS__)
#else
#define SPRN(str)
#define SPRNL(str)
#define SPRNF(fmt,...)
#endif

// Serial 
bool       serialReady = false;
const int  serialBaud  = 115200;

// led
bool              ledReady      = false;

// NeoPixel
#define NUMPIXELS        1
Adafruit_NeoPixel pixels(NUMPIXELS, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);
bool              neopixelReady = false;

// Haptic 
Adafruit_DRV2605 drv;
bool             hapticReady      = false;
uint8_t          hapticBuzzEffect = 47;
uint32_t         periodicBuzzSec  = 600; // every 10 minutes

// RTC
RTC_DS3231 rtc;
bool rtcReady             = false;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// BFF Battery Voltage on Pin A2
bool            batteryReady   = false;
const    int8_t batteryPin     = A2;
float           batteryVoltage = 0.0;

// Touch Pads on A3, SDA and SCL
bool             touchReady     = false;        
const    int8_t  touchPins[]    = { A3, SDA, SCL };
const    int8_t  touchWakeupPin  = 0; 
int              touchThreshold = 1400;

#ifdef LOW_POWER
RTC_DATA_ATTR int      bootCount    = 0;
RTC_DATA_ATTR uint32_t lastBuzzTime = 0;
#else
int      bootCount        = 0;
uint32_t lastBuzzTime     = 0;
#endif

uint32_t loopPeriodMS     = 30000;

inline void ledPwrOn()
{
    pinMode(LED_BUILTIN, OUTPUT);
    ledReady = true;
}

inline void ledOn()
{
  digitalWrite(LED_BUILTIN, HIGH);
}

inline void ledOff()
{
  digitalWrite(LED_BUILTIN, LOW);
}

#ifndef USE_LED
inline void neoPixelPwrOn()
{
  pinMode(NEOPIXEL_POWER, OUTPUT);
  digitalWrite(NEOPIXEL_POWER, HIGH);
  
  pixels.begin();
  pixels.setBrightness(50);
  neopixelReady = true;
}
#endif
inline void neoPixelPwrOff()
{
  digitalWrite(NEOPIXEL_POWER, LOW);
  neopixelReady = false;
}

#ifndef USE_LED
inline void 
neoPixelOn(int32_t color)
{
    if (!neopixelReady) neoPixelPwrOn();
    pixels.fill(color);
    pixels.show();
}

inline void
neoPixelOff()
{
  pixels.fill(0x000000);
  pixels.show();
}
#endif

#ifdef USE_LED
#else
#endif
inline void 
flash(int num, int speed=250, int32_t color=0xFF0000) 
{ 
  for (int i=0; i<num; i++) {
#ifdef USE_LED
    ledOn();
#else
    neoPixelOn(color);
#endif
    delay(speed);
#ifdef USE_LED
    ledOff();  
#else                   
    neoPixelOff();
#endif
    if (i<(num-1)) delay(speed);
  }
}

inline void 
errFlash(int num, int speed=250, int32_t color=0xFF0000)
{
#ifdef USE_LED
   if (!ledReady) ledPwrOn();
#else
   if (!neopixelReady) neoPixelPwrOn();
#endif
   flash(num, speed, color);
}

inline void
setupRTC()
{

  if (! rtc.begin(&Wire1)) {
    SPRNL("Couldn't find RTC");
#ifndef NOSERIAL    
    Serial.flush();
#endif
    while (1) errFlash(2);
  }

  if (rtc.lostPower()) {
    SPRNL("RTC lost power, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    errFlash(3, 500, 0xffff00);
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

  // When time needs to be re-set on a previously configured device, the
  // following line sets the RTC to the date & time this sketch was compiled
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // This line sets the RTC with an explicit date & time, for example to set
  // January 21, 2014 at 3am you would call:
  // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  rtcReady = true;
}

inline void
setupTouch()
{
  touchSleepWakeUpEnable(touchPins[touchWakeupPin], touchThreshold);
  touchReady = true;
}

inline void
setupSerial()
{
#ifndef NOSERIAL
  Serial.begin(serialBaud);
  while (!Serial); // wait for serial port to connect. Needed for native USB
  serialReady = true;
#endif
}

#ifndef USE_LED
inline void
setupNeoPixel()
{
   neoPixelPwrOn();
}
#else
inline void
setupLed()
{
  ledPwrOn();
}
#endif

inline void
setupHaptic(uint8_t effect)
{
  SPRNL("setupHaptic"); 
  if (! drv.begin(&Wire1)) {
    SPRNL("Could not find DRV2605");
    while (1) errFlash(3);
  }
  drv.selectLibrary(1);
  // I2C trigger by sending 'go' command 
  // default, internal trigger when sending GO command
  drv.setMode(DRV2605_MODE_INTTRIG);
  drv.setWaveform(0, effect);  // play effect
  drv.setWaveform(1, 0);       // end waveform
  hapticReady = true;
  SPRNL("setupHaptic Done");
}

inline void
setupBattery()
{
   pinMode(batteryPin, INPUT);
   batteryReady = true;
}

inline void
getBatteryVoltage()
{
  if (!batteryReady) setupBattery();
  float v = analogRead(batteryPin);
  // 16.87 is a fudge factor I measured using usb charger doctor.
  batteryVoltage=(v/65535) * 3.3 * 2.0 * 16.87;
}

inline void
batteryLoopAction(DateTime &now)
{
  getBatteryVoltage();
  SPRN("VBat: "); SPRNL(batteryVoltage);
}

inline void 
printTime(DateTime &now)
{
#ifndef NOSERIAL
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" (");
    Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    Serial.print(") ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();

    Serial.print(" since midnight 1/1/1970 = ");
    Serial.print(now.unixtime());
    Serial.print("s = ");
    Serial.print(now.unixtime() / 86400L);
    Serial.println("d");

    // calculate a date which is 7 days, 12 hours, 30 minutes, 6 seconds into the future
    DateTime future (now + TimeSpan(7,12,30,6));

    Serial.print(" now + 7d + 12h + 30m + 6s: ");
    Serial.print(future.year(), DEC);
    Serial.print('/');
    Serial.print(future.month(), DEC);
    Serial.print('/');
    Serial.print(future.day(), DEC);
    Serial.print(' ');
    Serial.print(future.hour(), DEC);
    Serial.print(':');
    Serial.print(future.minute(), DEC);
    Serial.print(':');
    Serial.print(future.second(), DEC);
    Serial.println();

    Serial.print("Temperature: ");
    Serial.print(rtc.getTemperature());
    Serial.println(" C");
#endif
}

inline void
rtcLoopAction(DateTime &now)
{
  if (!rtcReady) setupRTC();
  now = rtc.now();
  printTime(now);
}

inline void 
hapticPlay(bool wait=false)
{
  drv.go();
  if (wait) {
    while (drv.readRegister8(DRV2605_REG_GO) == 1) {
      delay(100);
    }
  }
}
inline void
hapticLoopAction(DateTime &now)
{
  if (!hapticReady) setupHaptic(hapticBuzzEffect);
  uint32_t ut = now.unixtime();
  if (periodicBuzzSec && ((ut - lastBuzzTime)> periodicBuzzSec)) {
    hapticPlay(false);   
    lastBuzzTime = ut;
  } 
}
void touchLoopAction(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : 
      SPRNL("Wakeup caused by external signal using RTC_IO");
      break;
    case ESP_SLEEP_WAKEUP_EXT1 : 
      SPRNL("Wakeup caused by external signal using RTC_CNTL");
      break;
    case ESP_SLEEP_WAKEUP_TIMER : 
      SPRNL("Wakeup caused by timer");  
      break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : 
      SPRNL("Wakeup caused by touchpad"); 
#ifdef USE_LED
      ledOn();
#else 
      neoPixelOn(0x00ff00);
#endif
      hapticPlay(true);  // Buzz and wait 
#ifdef USE_LED
      ledOff();
#else
      neoPixelOff();
#endif
      break;
    case ESP_SLEEP_WAKEUP_ULP : 
      SPRNL("Wakeup caused by ULP program"); 
      break;
    default : 
      SPRNF("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); 
      break;
  }
}

inline void 
gotoSleep()
{
#ifndef NOSERIAL
  delay(20000);
#endif  
  neoPixelPwrOff();
  if (!touchReady) setupTouch();
  esp_sleep_enable_touchpad_wakeup();
  esp_sleep_enable_timer_wakeup(loopPeriodMS*1000);
  esp_deep_sleep_start();
}

void ESPCoreVersion()
{
   SPRNL("ESP-IDF version: "+ String(esp_get_idf_version()));
}
// Core Dispatch routines

void 
setup() 
{
#ifdef USE_LED
  setupLed();
#else
  setupNeoPixel();
#endif
  if (bootCount==0) { 
     flash(2,10, 0x00000ff);
  }
  setupSerial();
#ifndef NOSERIAL  
  SPRN("QTPY Nudge: ");
  ESPCoreVersion();
#endif
  
  //setupTouch();
  //SPRNL("Touch Done");
  setupHaptic(hapticBuzzEffect);
  SPRNL("Haptic Done.");
  setupRTC();
  SPRNL("RTC Done.");
  setupBattery();
  SPRNL("Battery Done");
  if (bootCount == 0) {
    flash(3,250, 0x00ff00);
  }
#ifdef LOW_POWER
  bootCount++;
  loop();
  gotoSleep();
#endif
  SPRNL("Setup Done.");
}

void 
loop () {
  SPRNL("Loop");
  DateTime now;
  touchLoopAction();
  rtcLoopAction(now);
  batteryLoopAction(now);
  hapticLoopAction(now);
#ifndef LOW_POWER  
  delay(loopPeriodMS);
#endif
}
