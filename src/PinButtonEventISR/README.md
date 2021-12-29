# Introduction

Simple, reliable button with multiple types of click detection.

Supports debounced click, singleClick, doubleClick, press and longPress events.

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

# Changelog

1.0.0 (2021-12-29):
- Initial version.

# License

The MIT license.
