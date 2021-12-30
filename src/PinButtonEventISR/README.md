# Introduction

Simple, reliable button with multiple types of click detection.

Supports* debounced click, singleClick, doubleClick, press and longPress events.

*Already handles, but doesn't identify PRESS and LONG_CLICK events.
** Every switch event (CLICK and HOLD) starts with PRESS, i.e. is debounced transition from LOW to HIGH.

Provides a generic PinButtonEventISR class the ButtonEventHandler wrapper to simply use an Arduino digital pin as a button with interrupts.

# Notes
 ButtonEventHandler is completely a static class, thus it's impossible to use it for multiple buttons.
 
# Installing the library

Place this lib into your IDE library folder.

# Using in your own code

In the Arduino IDE, click `Sketch` > `Include Library` > `PinButtonEventISR`.

# Using the examples

In the Arduino IDE, click `File` > `Examples` > `PinButtonEventISR` and choose an example.
Connect a switch (or use a piece of wire) between pin 5 and ground (GND).

# Documentation

All classes and public methods are documented in the source code of the library:
* [PinButtonEventISR.h](https://github.com/diabolusss/ESP8266-Weather_station/blob/main/src/PinButtonEventISR/src/PinButtonEventISR.h)
* [ButtonEventHandler.h](https://github.com/diabolusss/ESP8266-Weather_station/blob/main/src/PinButtonEventISR/src/ButtonEventHandler.h)
* [ButtonEventHandler.cpp](https://github.com/diabolusss/ESP8266-Weather_station/blob/main/src/PinButtonEventISR/src/ButtonEventHandler.cpp)

 ```
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
     * Next state could be PRESS.
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
 ```
# Changelog
1.1.0 (2021-12-30):
- fix gap between btn dbl click and hold delay (possible to identify long click - switch released before it can be identified as hold)

1.0.0 (2021-12-29):
- Initial version.

# License

The MIT license.
