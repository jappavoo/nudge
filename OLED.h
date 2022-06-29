#ifndef __OLED_H__
#define __OLED_H__

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define CLEAR_DISPLAY_AFTER_MS 5000

class OLED : public  Adafruit_SSD1306 {    
  bool setupDone_;
  unsigned long clearDisplay_;
  unsigned long clearTime_;
  bool sleeping_;
public:
  OLED(int resetPin, unsigned long clearTime=CLEAR_DISPLAY_AFTER_MS) : 
  Adafruit_SSD1306(128, 32, &Wire, resetPin), setupDone_(false), 
  clearDisplay_(0), clearTime_(clearTime), sleeping_(false) {}
  void setup() {
     VPRINTLN("OLED::setup START");
     if (setupDone_ == false) {
        // SSD1306_SWITCHCAPVCC generate display voltage from 3.3V internally
        // Address 0x3C for 128x32
        // reset using reset pin = true
        // call wire begin periphBegin = true  -- not sure we need this
        begin(SSD1306_SWITCHCAPVCC, 0x3C, true, true); 
        // text display tests
        setTextSize(1);
        setTextColor(SSD1306_WHITE,SSD1306_BLACK);
        clearDisplay();
        OLED::sleep();
        sleeping_ = true; 
     } else {
      VPRINTLN("  setup already done.");
     }
     VPRINTLN("OLED::setup END");
  }
  
  void update(bool clear=true) {
    if (sleeping_) { wake(); sleeping_ = false; }
    display();
    if (clear) {
       clearDisplay_ = millis() + CLEAR_DISPLAY_AFTER_MS;
    } else {   
       clearDisplay_ = 0;
    }
  }

  void loopAction() {
    if (clearDisplay_) {
      if (millis() > clearDisplay_) {
          clearDisplay();
          OLED::sleep();
          sleeping_ = true;
          clearDisplay_ = 0;
      }
    }
  }
  
  void sleep() {
    ssd1306_command(SSD1306_DISPLAYOFF);
  }
  
  void wake() {
    ssd1306_command(SSD1306_DISPLAYON);
  }
  
  void msg(const char *msg) {
    setTextSize(1);
    setCursor(0,0);
    VPRINT(msg);
    Adafruit_SSD1306::print(msg);
  }
    
};
#endif
