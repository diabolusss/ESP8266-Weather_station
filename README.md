# ESP8266-Weather_station
esp8266, cjmcu8118+hdc1080, bmp180, ccs811, gy30, ssd1306 oled

 Indoor air quality & weather station with display.
 https://thingspeak.com/channels/1579072
 
# Features:
  - status bar - show datetime and outdoor/indoor temperature;
  - indoor details frame: indoor sensors readings;
  - current weather frame: weather icon, weather short description and temperature;
  - weather forecast frame: forecast for 3 days - day, icon and temperature;
  - frame autochange;
  - display sleep timeout;
  - display on/off by button click;
  - read sensors every second, upload data to thingspeak once in a minute;

# todo
  - check bmp180 temperature reading sanity (+2 *C reading difference compared to real thermometer)
  - check ccs811 firmware version and upgrade if needed
  - save/restore ccs811 baseline to/from EEPROM. (save onClick & by timer - how often?)
  - lower sensor reading frequency based on inactivity time
  - add forecast day/night temperatures
  - add more details for current weather data
  
# Custom libraries and changes 
 * PinButtonEventISR
   - static library to use with button interrupts. Handles debounced clicks (single, double) and press (short, long) events.
   
 * SparkFun_CCS811_Arduino_Library
   - read firmware, bootloader, application versions ([ported from](https://github.com/maarten-pennings/CCS811))
   
 * esp8266-oled-ssd1306
  #211229
   - added resetState method to reset timers and move to the first frame
   - dont draw abything if display is not awake
  #211225
   - added isAwake display status flag to avoid rendering when display is off
 
 * ClosedCube_HDC1080  
  #211225
   - added begin method that allows to override default resolution parameters

