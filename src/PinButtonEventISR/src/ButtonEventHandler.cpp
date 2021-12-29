#include "ButtonEventHandler.h"

volatile ButtonEventHandler::Event  ButtonEventHandler::event = ButtonEventHandler::Event::NONE; 
volatile uint8_t                    ButtonEventHandler::state = _LOW;
uint8_t                    		ButtonEventHandler::pin = 0;
volatile uint32_t                   ButtonEventHandler::risingTs = 0;
volatile uint32_t                   ButtonEventHandler::fallingTs = 0;

ICACHE_RAM_ATTR void ButtonEventHandler::onButtonChangeISR() {
  uint32_t tH = millis() - ButtonEventHandler::risingTs;
  uint8_t tS = digitalRead(ButtonEventHandler::pin);
  
  PRINT("BTN state ");
  PRINTLN(tS);
  
  //RISING edge (if true the button was just pressed down)
  if (ButtonEventHandler::state == _LOW && tS == _HIGH && tH > BTN_DEBOUNCE_DELAY_MS) {
    //note the time the event was accepted
    ButtonEventHandler::risingTs = millis();
    ButtonEventHandler::state = _HIGH;

    PRINTLN(">>> press happened ");
  }

  tS = digitalRead(ButtonEventHandler::pin);
  PRINT("BTN state ");
  PRINTLN(tS);
  
  //FALLING
  if (state == _HIGH && tS == _LOW) {
    ButtonEventHandler::fallingTs = millis();
    uint32_t tH = ButtonEventHandler::fallingTs - ButtonEventHandler::risingTs;

    if (tH < BTN_CLICK_DELAY_MS && ButtonEventHandler::event == NONE) {
      ButtonEventHandler::event = CLICK;
      ButtonEventHandler::state = _LOW;
      ButtonEventHandler::risingTs = millis();
      
      PRINTLN(">>> click ended ");
      
    }else if (tH < BTN_DOUBLE_CLICK_DELAY_MS && ButtonEventHandler::event == CLICK) {
      ButtonEventHandler::event = DBL_CLICK;
      ButtonEventHandler::state = _LOW;
      ButtonEventHandler::risingTs = millis();

      PRINTLN(">>> double click ended ");
      
    } else if(ButtonEventHandler::event != CLICK && ButtonEventHandler::event != DBL_CLICK){
      if (tH >= BTN_HOLD_DELAY_MS && tH < BTN_LONG_PRESS_DELAY_MS) {
        ButtonEventHandler::event = HOLD;
        ButtonEventHandler::state = _LOW;
        ButtonEventHandler::risingTs = millis();
        
        PRINTLN(">>> shortpress ended ");
        
      } else if (tH >= BTN_LONG_PRESS_DELAY_MS) {
        ButtonEventHandler::event = LONG_HOLD;
        ButtonEventHandler::state = _LOW;
        ButtonEventHandler::risingTs = millis();
  
        PRINTLN(">>> longpress ended ");
      }
      
    }//end of hold events
  }//end of falling edge handle
  
}
