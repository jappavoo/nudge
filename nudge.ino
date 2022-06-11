/* Example implementation of an alarm using DS3231
 *
 * VCC and GND of RTC should be connected to some power source
 * SDA, SCL of RTC should be connected to SDA, SCL of arduino
 * SQW should be connected to CLOCK_INTERRUPT_PIN
 * CLOCK_INTERRUPT_PIN needs to work with interrupts
 */
#include <Bounce2.h>
#include <RTClib.h>
#include <Adafruit_DRV2605.h>

#define DEFAULT_PERIOD_MINUTES 20

#define MAX_PERIODICS 16
struct PeriodicAlarms {
  TimeSpan when;
  char msg[16];
  uint8_t haptic;
} Periodics[MAX_PERIODICS] = {
  [0] = { DEFAULT_PERIOD_MINUTES * 60, "Periodic", 0 },
  [1] = {0,"",0},
  [2] = {0,"",0},
  [3] = {0,"",0},
  [4] = {0,"",0},
  [5] = {0,"",0},
  [6] = {0,"",0},
  [7] = {0,"",0},
  [8] = {0,"",0},
  [9] = {0,"",0},
  [10] = {0,"",0},
  [11] = {0,"",0},
  [12] = {0,"",0},
  [13] = {0,"",0},
  [14] = {0,"",0},
  [15] = {0,"",0}
};

#define MAX_REMINDERS 32
struct ReminderAlarms {
  TimeSpan when;
  char    msg[16];
  uint8_t haptic;
} Reminders[MAX_REMINDERS] = {
  [0] = { 0, "", 0 },
  [1] = {0,"",0},
  [2] = {0,"",0},
  [3] = {0,"",0},
  [4] = {0,"",0},
  [5] = {0,"",0},
  [6] = {0,"",0},
  [7] = {0,"",0},
  [8] = {0,"",0},
  [9] = {0,"",0},
  [10] = {0,"",0},
  [11] = {0,"",0},
  [12] = {0,"",0},
  [13] = {0,"",0},
  [14] = {0,"",0},
  [15] = {0,"",0}, 
  [16] = { 0, "", 0 },
  [17] = {0,"",0},
  [18] = {0,"",0},
  [19] = {0,"",0},
  [20] = {0,"",0},
  [21] = {0,"",0},
  [22] = {0,"",0},
  [23] = {0,"",0},
  [24] = {0,"",0},
  [25] = {0,"",0},
  [26] = {0,"",0},
  [27] = {0,"",0},
  [28] = {0,"",0},
  [29] = {0,"",0},
  [30] = {0,"",0},
  [31] = {0,"",0}
};

#define BAT_INFO

// Uncommenting this will cause displayed time to be updated
// while time is showing  --- I found this behaviour to be distracting
//#define UPDATE_TIME

//#define SET_RTC_TIME

//#define CONFIG_REMINDER_ALARM
#define CONFIG_PERIODIC_ALARM



// #define VERBOSE

#ifdef VERBOSE
#define VPRINTLN(...) Serial.println(__VA_ARGS__)
#define VPRINT(...) Serial.print(__VA_ARGS__)
#else
#define VPRINTLN(...) 
#define VPRINT(...) 
#endif 
#define BLE_UART
#define BLE_UART_ECHO_MSG
#define OLED_DISPLAY

#ifdef OLED_DISPLAY
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define BUTTON_A 31
#define BUTTON_B 30
#define BUTTON_C 27
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);
Bounce2::Button buttonA = Bounce2::Button();

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
#else 
#define LED_PIN LED_BUILTIN
// SET A VARIABLE TO STORE THE LED STATE
int ledState = LOW;

void toggleLed()
{
     // TOGGLE THE LED STATE : 
    ledState = !ledState; // SET ledState TO THE OPPOSITE OF ledState
    digitalWrite(LED_PIN,ledState); // WRITE THE NEW ledState
}


#endif

#ifdef BLE_UART
#include <bluefruit.h>
#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>

// BLE Service
BLEDfu  bledfu;  // OTA DFU service
BLEDis  bledis;  // device information
BLEUart bleuart; // uart over ble
BLEBas  blebas;  // battery


void startAdv(void)
{
  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();

  // Include bleuart 128-bit uuid
  Bluefruit.Advertising.addService(bleuart);

  // Secondary Scan Response packet (optional)
  // Since there is no room for 'Name' in Advertising packet
  Bluefruit.ScanResponse.addName();
  
  /* Start Advertising
   * - Enable auto advertising if disconnected
   * - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
   * - Timeout for fast mode is 30 seconds
   * - Start(timeout) with timeout = 0 will advertise forever (until connected)
   * 
   * For recommended advertising interval
   * https://developer.apple.com/library/content/qa/qa1931/_index.html   
   */
  Bluefruit.Advertising.restartOnDisconnect(false);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds 
       wakeDisplay();
      displayTime(); 
      displayMsg("BLE_UART: Advertising");
      updateDisplay(false);   
}

#define BLE_MSG_LEN 40
char ble_msg[BLE_MSG_LEN];
int ble_msg_len = 0;

void ble_msg_reset() {
  ble_msg_len = 0;
  ble_msg[0] = 0;
}
void ble_msg_add(char c) {
  VPRINT("BLE_MSG_ADD: ");
  VPRINT(ble_msg_len);
  VPRINT(" : ");
  VPRINTLN(ble_msg);
  if (ble_msg_len >= (BLE_MSG_LEN-1)) ble_msg_len = 0;
  ble_msg[ble_msg_len] = c;
  ble_msg_len++;
  ble_msg[ble_msg_len] = 0;
}

bool ble_setup = false;
bool ble_uart_connected = false;
// callback invoked when central connects
void connect_callback(uint16_t conn_handle)
{
  ble_uart_connected = true;
  ble_msg_reset();
  // Get the reference to current connection
  BLEConnection* connection = Bluefruit.Connection(conn_handle);

  char central_name[32] = { 0 };
  connection->getPeerName(central_name, sizeof(central_name));

  VPRINT("Connected to ");
  VPRINTLN(central_name);
  wakeDisplay();
  displayTime(); 
  displayMsg("BLE_UART: Connected");
  updateDisplay(true);  
}

/**
 * Callback invoked when a connection is dropped
 * @param conn_handle connection where this event happens
 * @param reason is a BLE_HCI_STATUS_CODE which can be found in ble_hci.h
 */
void disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  (void) conn_handle;
  (void) reason;
   ble_uart_connected = false;
  VPRINTLN();
  VPRINT("Disconnected, reason = 0x"); VPRINTLN(reason, HEX);
  wakeDisplay();
  displayTime(); 
  displayMsg("BLE_UART: Connected");
  updateDisplay(true); 
}

void ble_uart_setup()
{
  VPRINTLN("BLE_UART: Setup Started");

  if (!ble_setup) { 
    // Setup the BLE LED to be enabled on CONNECT
    // Note: This is actually the default behavior, but provided
    // here in case you want to control this LED manually via PIN 19
    Bluefruit.autoConnLed(true);

    // Config the peripheral connection with maximum bandwidth 
    // more SRAM required by SoftDevice
    // Note: All config***() function must be called before begin()
    Bluefruit.configPrphBandwidth(BANDWIDTH_MAX);

    Bluefruit.setName("Nudge");
    Bluefruit.begin();
    Bluefruit.setTxPower(4);    // Check bluefruit.h for supported values
    //Bluefruit.setName(getMcuUniqueID()); // useful testing with multiple central connections
    Bluefruit.Periph.setConnectCallback(connect_callback);
    Bluefruit.Periph.setDisconnectCallback(disconnect_callback);

    // To be consistent OTA DFU should be added first if it exists
    bledfu.begin();

    // Configure and Start Device Information Service
    bledis.setManufacturer("Adafruit Industries");
    bledis.setModel("Bluefruit Feather52");
    bledis.begin();

    // Configure and Start BLE Uart Service
    bleuart.begin();

    // Start BLE Battery Service
    blebas.begin();
    blebas.write(100);
    ble_setup = true;
  }
    // Set up and start advertising
  startAdv();

  VPRINTLN("BLE_UART: Please use Adafruit's Bluefruit LE app to connect in UART mode");
}
#endif




#define REMINDER_RTC_ALARM 1

#define PERIODIC_RTC_ALARM 2


#define STRONG_CLICK 17
#define STRONG_BUZZ 14
#define BUTTON_PRESSED_HAPTIC STRONG_CLICK
#define PERIODIC_ALARM_HAPTIC STRONG_BUZZ

// this code will not compile unless you modify 
// RTC_DDS3231 to be:  class RTC_DS3231 : protected RTC_I2C {
//  default is private inheritance and then we don't have acces to 
//  i2c_dev :-(
class MY_RTC_DS3231 : public RTC_DS3231 {
public:
  bool getAlarm1(DateTime &dt, Ds3231Alarm1Mode &alarm_mode);      
  bool getAlarm2(DateTime &dt, Ds3231Alarm2Mode &alarm_mode);                      
};

bool MY_RTC_DS3231::getAlarm1(DateTime &dt, Ds3231Alarm1Mode &alarm_mode)   {

  uint8_t buffer[4];
  buffer[0] = 0x07;   // DS3231 ARLM 1 ADDR
  i2c_dev->write_then_read(buffer, 1, buffer, 4);

  for (int i=0; i<4; i++){ 
     VPRINTLN(buffer[i], HEX);
  }
  return true;
}

bool MY_RTC_DS3231::getAlarm2(DateTime &dt, Ds3231Alarm2Mode &alarm_mode)   {

  uint8_t buffer[3];
  buffer[0] = 0x0b;   // DS3231 ARLM 2 ADDR
  i2c_dev->write_then_read(buffer, 1, buffer, 3);

  for (int i=0; i<3; i++){ 
     VPRINTLN(buffer[i], HEX);
  }
  return true;
}
MY_RTC_DS3231 rtc;
Adafruit_DRV2605 haptic;

// the pin that is connected to SQW
#define CLOCK_INTERRUPT_PIN A2

#define BUTTON_PIN A0
#define DEBOUNCE_MS 5 //Debouncing Time in Milliseconds
Bounce2::Button button = Bounce2::Button();

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
displayMsg(const char *msg) {
  VPRINT(msg);
#ifdef OLED_DISPLAY
  display.setTextSize(1);
  display.print(msg);
#endif
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
  
#ifdef OLED_DISPLAY
  display.setCursor(0,0);
  display.setTextSize(1);
  display.println(datebuf);

  display.setTextSize(2);
  display.println(timebuf);
#endif
}


void setup() {
#ifdef VERBOSE  
    Serial.begin(9600);
#endif

#ifdef OLED_DISPLAY
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3C for 128x32
   // Clear the buffer.
  display.clearDisplay();
  display.display();
  buttonA.attach(BUTTON_B, INPUT_PULLUP);
  buttonA.interval(DEBOUNCE_MS);
  buttonA.setPressedState(LOW); 

  // pinMode(BUTTON_A, INPUT_PULLUP);
  // pinMode(BUTTON_C, INPUT_PULLUP);
 
  // text display tests
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE,SSD1306_BLACK);
#endif

#ifdef LED_PIN
  pinMode(LED_PIN,OUTPUT);
  digitalWrite(LED_PIN,ledState);
  toggleLed();
#endif

  VPRINTLN("setup:start");
  // pinMode(BUTTON_INTERRUPT_PIN, INPUT_PULLUP);  
  button.attach(BUTTON_PIN, INPUT_PULLUP);
  button.interval(DEBOUNCE_MS);
  button.setPressedState(LOW); 
#ifdef BUTTON_INTERRUPT
#ifdef NRF52
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), onButton, CHANGE);
#else 
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), onButton, CHANGE);
#endif
#endif

    // initializing the rtc
    if(!rtc.begin()) {
        VPRINTLN("Couldn't find RTC!");
        Serial.flush();
        while (1) delay(10);
    }

  VPRINTLN("rtc: started");
#ifdef SET_RTC_TIME
       VPRINTLN("RTC SET TIME!");
        // this will adjust to the date and time at compilation
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
#endif

    //we don't need the 32K Pin, so disable it
    rtc.disable32K();

    // Making it so, that the alarm will trigger an interrupt
    pinMode(CLOCK_INTERRUPT_PIN, INPUT_PULLUP);
#ifdef NRF52
    attachInterrupt(digitalPinToInterrupt(CLOCK_INTERRUPT_PIN), onAlarm, FALLING);
#else
    attachInterrupt(digitalPinToInterrupt(CLOCK_INTERRUPT_PIN), onAlarm, FALLING);
#endif
    // set alarm 1, 2 flag to false (so alarm 1, 2 didn't happen so far)
    // if not done, this easily leads to problems, as both register aren't reset on reboot/recompile
    rtc.clearAlarm(REMINDER_RTC_ALARM);
    rtc.clearAlarm(PERIODIC_RTC_ALARM);

    // stop oscillating signals at SQW Pin
    // otherwise setAlarm1 will fail
    rtc.writeSqwPinMode(DS3231_OFF);

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

    displayTime();
    displayMsg("NUDGE v0.1");
    updateDisplay(true);
    
     haptic.begin();
     haptic.selectLibrary(1);
  
  // I2C trigger by sending 'go' command 
  // default, internal trigger when sending GO command
    haptic.setMode(DRV2605_MODE_INTTRIG);  
#ifndef VERBOSE
 // Serial.end();   // Shut this down to limit power
#endif
#ifdef LED_PIN
  toggleLed();
#endif

#ifdef BAT_INFO
 // Get a single ADC sample and throw it away
  readVBAT();
#endif
  
#ifdef NRF52
  sd_power_mode_set(NRF_POWER_MODE_LOWPWR);
  sd_power_dcdc_mode_set(NRF_POWER_DCDC_ENABLE);    // This saves power
#endif

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

unsigned int event = 0;
#ifdef BUTTON_INTERRUPT
#define BUTTON_EVENT (1U << 0 )
#endif

#define RTC_ALARM_EVENT (1U << 1 )

void docmd(char *cmd) {
  switch (cmd[0]) {
    case 'S':
      // MMM DD YYYY - Apr 16 2020
      char *date = &cmd[1];
      date[11] = 0; 
      // hh:mm:ss
      char *stime = &cmd[13];
      stime[8]=0;;
      rtc.adjust(DateTime(date, stime));
      VPRINT("Set: ");
      VPRINT(date);
      VPRINT(" ");
      VPRINTLN(stime);
      break;
   }
}

void loop() {
#ifdef NRF52
  sd_power_mode_set(NRF_POWER_MODE_LOWPWR);
  waitForEvent();
#endif
  button.update();
#ifdef OLED_DISPLAY
  buttonA.update();
#endif  
  if (button.pressed()) {
        VPRINTLN("Button Pressed");
        
#ifdef LED_PIN
        toggleLed();
#endif 
#ifdef OLED_DISPLAY
        wakeDisplay();
        displayTime();
        displayNextPeriodic();
#ifdef BAT_INFO
        float vbat_mv = readVBAT();
       // Convert from raw mv to percentage (based on LIPO chemistry)
        uint8_t vbat_per = mvToPercent(vbat_mv);
        
        displayMsg(" Bat: ");
       // display.print(vbat_mv);
       // display.print("mV (");
        display.print(vbat_per);
        display.print("%");       
#endif        
        updateDisplay(true);
#endif    
        playEffect(BUTTON_PRESSED_HAPTIC);
        delay(500);
#ifdef LED_PIN
        toggleLed();
#endif
  }
#ifdef OLED_DISPLAY
  if (buttonA.pressed()) {
        wakeDisplay();
        displayTime();
        displayMsg("BLE UART: Start");
        ble_uart_setup();
        updateDisplay(true); 
  }
#endif
  if (event) {
#ifdef BUTTON_INTERRUPT
    if (event &  BUTTON_EVENT) {
      event &= ~BUTTON_EVENT;  
    }
#endif
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

#ifdef BLE_UART
  if (ble_uart_connected) {
     while ( bleuart.available())  {
      char ch = bleuart.read();
      if (ch == '\n') {
 #ifdef BLE_UART_ECHO_MSG
         wakeDisplay();
         displayTime(); 
         displayMsg(ble_msg);
         updateDisplay(true);
#endif
         docmd(ble_msg);
         ble_msg_reset();   
      } else {
         ble_msg_add(ch);
      } 
    }
  }
#endif
  
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

void rtos_idle_callback(void)
{
  // Don't call any other FreeRTOS blocking API()
  // Perform background task(s) here
}

void onAlarm() {
  event |= RTC_ALARM_EVENT;
}

#ifdef BUTTON_INTERRUPT
void onButton() {
   event |= BUTTON_EVENT;
}
#endif



uint32_t vbat_pin = PIN_VBAT;             // A7 for feather nRF52832, A6 for nRF52840

#define VBAT_MV_PER_LSB   (0.73242188F)   // 3.0V ADC range and 12-bit ADC resolution = 3000mV/4096

#ifdef NRF52840_XXAA
#define VBAT_DIVIDER      (0.5F)          // 150K + 150K voltage divider on VBAT
#define VBAT_DIVIDER_COMP (2.0F)          // Compensation factor for the VBAT divider
#else
#define VBAT_DIVIDER      (0.71275837F)   // 2M + 0.806M voltage divider on VBAT = (2M / (0.806M + 2M))
#define VBAT_DIVIDER_COMP (1.403F)        // Compensation factor for the VBAT divider
#endif

#define REAL_VBAT_MV_PER_LSB (VBAT_DIVIDER_COMP * VBAT_MV_PER_LSB)


float readVBAT(void) {
  float raw;

  // Set the analog reference to 3.0V (default = 3.6V)
  analogReference(AR_INTERNAL_3_0);

  // Set the resolution to 12-bit (0..4095)
  analogReadResolution(12); // Can be 8, 10, 12 or 14

  // Let the ADC settle
  delay(1);

  // Get the raw 12-bit, 0..3000mV ADC value
  raw = analogRead(vbat_pin);

  // Set the ADC back to the default settings
  analogReference(AR_DEFAULT);
  analogReadResolution(10);

  // Convert the raw value to compensated mv, taking the resistor-
  // divider into account (providing the actual LIPO voltage)
  // ADC range is 0..3000mV and resolution is 12-bit (0..4095)
  return raw * REAL_VBAT_MV_PER_LSB;
}

uint8_t mvToPercent(float mvolts) {
  if(mvolts<3300)
    return 0;

  if(mvolts <3600) {
    mvolts -= 3300;
    return mvolts/30;
  }

  mvolts -= 3600;
  return 10 + (mvolts * 0.15F );  // thats mvolts /6.66666666
}

/*static uint8_t read_i2c_register(uint8_t addr, uint8_t reg) {
    Wire.beginTransmission(addr);
    Wire.write((byte)reg);
    Wire.endTransmission();

    Wire.requestFrom(addr, (byte)1);
    return Wire.read();
}*/
