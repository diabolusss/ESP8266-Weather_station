#ifndef __BUTTON_EVENT_HANDLER_
#define __BUTTON_EVENT_HANDLER_

#include <Arduino.h>

#ifndef ICACHE_RAM_ATTR
  #define ICACHE_RAM_ATTR
#endif

#ifdef BUTTON_NORMALLY_CLOSED
  #define _LOW HIGH
  #define _HIGH LOW
#else
  #define _LOW LOW
  #define _HIGH HIGH
#endif

#define BTN_DEBOUNCE_DELAY_MS 		  50
#define BTN_CLICK_DELAY_MS 		  250
#define BTN_DOUBLE_CLICK_DELAY_MS 	  350
#define BTN_HOLD_DELAY_MS 		  1000
#define BTN_LONG_PRESS_DELAY_MS 	  3000

//#define DEBUG

#ifdef DEBUG
  #define PRINTLN(s)    Serial.println(s)
  #define PRINT(s)      Serial.print(s)
  #define PRINTF(s,m)   Serial.printf(s,m)
  #define PRINTLN2(s,m) Serial.println(s,m)
  #define PRINT2(s,m)   Serial.print(s,m)
#else
  #define PRINTLN(s)    
  #define PRINT(s)     
  #define PRINTF(s,m) 
  #define PRINTLN2(s,m)
  #define PRINT2(s,m)   
#endif

#define EVENT_RESET_DELAY_MS 1000 //if event wasn't read by this time, then erase it

class ButtonEventHandler{
	//ButtonEventHandler();
	
  public:
    /**
     * Note:
     * - SingleClick is only emitted when it's definitely not going to be a double or long click (any type of press)
     * - Click is emitted when either SingleClick or DoubleClick happens 
     *
     * Several example patterns and resulting states.
     * Delays aren't at scale. State transitions when input
     * changes are based on that input, transitions when
     * input didn't change are due to timeouts.
     * Starting state is NONE.
     * 
     * Rising edge can be identified as event, too (but is not yet), i.e. start of press event
     * previous state was LOW && now is HIGH && delay > BTN_DEBOUNCE_DELAY_MS
     * _____/---___________________
     *      |  |   
     *      |  |   
     *      |  |   
     *      |  |   
     *      |  |   
     *      |  Debounced PRESS - uncaptured, but handled
     *      |
     *      Physical press on switch
     *
     * Clean single click (can be identified as LONG_CLICK if needed, CLICK is not emitted):
     * delay > BTN_DOUBLE_CLICK_DELAY_MS && delay < BTN_HOLD_DELAY_MS
     * _____/------\___________________
     *        |   |   ||      NONE <- event read or EVENT_RESET_DELAY_MS has passed
     *        |   |   ||      
     *        |   |   |(ClickIdle) - uncaptured
     *        |   |   (ClickUp) - uncaptured
     *        |   SINGLE_CLICK <- (tH < BTN_HOLD_DELAY_MS && tH > BTN_DOUBLE_CLICK_DELAY_MS)
     *        Debounced PRESS - uncaptured 
     *        
     * (Single) click:
     * delay > BTN_DEBOUNCE_DELAY_MS && delay < BTN_CLICK_DELAY_MS && (no click before BTN_DOUBLE_CLICK_DELAY_MS)
     * _____/-------\____#####___________
     *        |   |   ||      |   NONE <- event read or EVENT_RESET_DELAY_MS has passed
     *        |   |   ||      SingleClick <- idle for BTN_DOUBLE_CLICK_DELAY_MS time
     *        |   |   |(ClickIdle) - uncaptured
     *        |   |   (ClickUp) - uncaptured
     *        |   CLICK <- before BTN_CLICK_DELAY_MS
     *        Debounced PRESS
     *
     * Clean double click:
     * delay > BTN_CLICK_DELAY_MS && delay < BTN_DOUBLE_CLICK_DELAY_MS
     * _____/------\____/------\_________________
     *        |   |   ||    |     NONE <- event read or EVENT_RESET_DELAY_MS has passed
     *        |   |   ||    |  
     *        |   |   ||    DBL_CLICK <- second press before BTN_DOUBLE_CLICK_DELAY_MS has passed
     *        |   |   |(ClickIdle) - uncaptured
     *        |   |   (ClickUp) - uncaptured
     *        |   CLICK <- before BTN_CLICK_DELAY_MS
     *        Debounced PRESS
     *        
     * Clean hold:
     * delay >= BTN_HOLD_DELAY_MS && delay < BTN_LONG_PRESS_DELAY_MS && !CLICK
     * _____/--------------------\_____________________
     *        |                   |   ||         NONE <- event read or EVENT_RESET_DELAY_MS has passed
     *        |                   |   ||      
     *        |                   |   ||    
     *        |                   |   |(ClickIdle) - uncaptured
     *        |                   |   (ClickUp) - uncaptured
     *        |                   HOLD <- only if it's not any tipe of click and BTN_HOLD_DELAY_MS is passed, 
     *        |                             but not yet long press
     *        Debounced PRESS 
     *        
     * Clean long hold:
     * delay >= BTN_LONG_PRESS_DELAY_MS && !CLICK
     * _____/--------------------------------\_____________________
     *        |                               |   ||         NONE <- event read or EVENT_RESET_DELAY_MS has passed
     *        |                               |   ||      
     *        |                               |   ||    
     *        |                               |   |(ClickIdle) - uncaptured
     *        |                               |   (ClickUp) - uncaptured
     *        |                               LONG_HOLD <- only if it's not any tipe of click 
     *        |                                               and BTN_LONG_PRESS_DELAY_MS is passed 
     *        |                            
     *        Debounced PRESS
     *        
     */
    typedef enum {
      NONE,
      CLICK,
      SNGL_CLICK,
      DBL_CLICK,
      HOLD,
      LONG_HOLD//,
      //...
    } Event;
    
    /*ButtonEventHandler(void){}
    
    ButtonEventHandler(const uint8_t _pin){
    	ButtonEventHandler::pin = _pin;
    }*/

    static void onButtonChangeISR();
    
    static void setPin(uint8_t _pin){
    	ButtonEventHandler::pin = _pin;
    }
    
    static void resetEvent(){ //event must be reset after processing
      ButtonEventHandler::event = NONE;
    } 
    
    static Event getEvent(){  //get event for handling
      return ButtonEventHandler::event;
    }

    static bool hasEvent(){
      //if event wasn't reset before, then do it now
      if(isFinal() && (millis() - risingTs) > EVENT_RESET_DELAY_MS){
        event = NONE; 
        PRINTLN("reset event state after t/o");
        return false;
      }
      return event != NONE;
    }

    /**
     * True when any kind of click is detected (single, double).
     * A Click is detected before Double, but
     * earlier than a Single click.
     */
    static bool isClick() {
      return event == CLICK || event == DBL_CLICK || event == SNGL_CLICK;
    }

    /**
     * True when a Single click is detected, i.e. it will not trigger before
     * e.g. a Double click.
     */
    static bool isSingleClick() {
      if(event == CLICK){ //wait long enough to give time for second click
        if((millis() - risingTs) > BTN_DOUBLE_CLICK_DELAY_MS){
          event = NONE;
          //event = SNGL_CLICK;
          return true; //is single click, so skip setting event as it's already obsolete
        }
        
      }else{
        bool is = event == SNGL_CLICK; 
        if(is) {event = NONE;}
        return is;
      }
      
      return false;
    }

    /**
     * True when a Double click is detected.
     */
    static bool isDoubleClick() {
      bool is = event == DBL_CLICK; 
      if(is) {event = NONE;}
      return is;
    }

    /**
     * True when a Hold is detected.
     */
    static bool isPress() {
      bool is = event == HOLD;
      if(is) {event = NONE;}
      return is;
    }

    /**
     * True when a Long Hold is detected.
     */
    static bool isLongPress() {
      bool is = event == LONG_HOLD;
      if(is){event = NONE; }
      return is;
    }

    static bool isFinal(){
      return event == LONG_HOLD || event == HOLD || event == DBL_CLICK || event == SNGL_CLICK;
    }

    /**
     * True once the button is released after Click, Long click or Double click.
     *
     * Note: there is no release event after a Single click, because that is a
     * 'synthetic' event that happens after a normal click.
     */
    //bool isReleased() {
      //return _new && (_state == StateClickUp || _state == StateOtherUp);
    //}
    
  protected:
  	static uint8_t pin;
      static volatile Event event;   //generated event type
      static volatile uint8_t state; //captured button state (noise filtered)
      
      static volatile uint32_t risingTs; //timestamp to capture rising edge
      static volatile uint32_t fallingTs; //ts to capture falling edge
    
};

#endif /*define __BUTTON_EVENT_HANDLER_*/
