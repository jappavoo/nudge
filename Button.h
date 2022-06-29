#ifndef __BUTTON_H__
#define __BUTTON_H__

#include <Bounce2.h>

//Debouncing Time in Milliseconds
#define DEBOUNCE_MS 5 

class Button {
  int pin_;
  int dms_;
  Bounce2::Button button_;
public:
  Button(int pin, int dms=DEBOUNCE_MS) :  
  pin_(pin), dms_(dms) {}
  void setup() {
    VPRINTLN("Button::setup: START");
    button_.attach(pin_, INPUT_PULLUP);
    button_.interval(dms_);
    button_.setPressedState(LOW); 
    VPRINTLN("Button::setup: END");
  }
   void loopAction() {
    button_.update();
   }
  bool pressed() { return button_.pressed(); }
};

#endif
