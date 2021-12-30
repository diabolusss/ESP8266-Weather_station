#ifndef __PIN_BUTTON_EVENT_ISR_
#define __PIN_BUTTON_EVENT_ISR_

//uncomment if at default state (released) read returns 1 instead of 0
//#define BUTTON_NORMALLY_CLOSED

#include <Arduino.h>
#include "ButtonEventHandler.h"

class PinButtonEventISR{
  public:
    /**
     * Construct a new PinButton using a switch connected between
     * an Arduino pin and ground.
     * The internal pull-up is automatically enabled.
     * 
     * @param pin {int} Arduino pin to use
     */
    PinButtonEventISR(uint8_t _pin){
      ButtonEventHandler::setPin(_pin);
      
      pinMode(_pin, INPUT_PULLUP);
      _attachInterrupt(_pin);
    }
    
    bool hasEvent(){
    	return ButtonEventHandler::hasEvent();
    }
    bool isClick(){
    	return ButtonEventHandler::isClick();
    }
    bool isSingleClick(){
    	return ButtonEventHandler::isSingleClick();
    }
    bool isDoubleClick(){
    	return ButtonEventHandler::isDoubleClick();
    }
    bool isPress(){
    	return ButtonEventHandler::isPress();
    }
    bool isLongPress(){
    	return ButtonEventHandler::isLongPress();
    }

    void _attachInterrupt(uint8_t _pin){
      // Set button pin as interrupt, to wakeup display (power on)
      //  CHANGE: to trigger the interrupt whenever the pin changes value – for example from HIGH to LOW or LOW to HIGH;
      //  FALLING: for when the pin goes from HIGH to LOW;
      //  RISING: to trigger when the pin goes from LOW to HIGH.
      // NB digitalRead in ISR is useless for !CHANGE mode
      attachInterrupt(digitalPinToInterrupt(_pin), ButtonEventHandler::onButtonChangeISR, CHANGE);
    }

};

#endif /*define __PIN_BUTTON_EVENT_ISR_*/
