#include "settings.h"
  #ifdef _EEPROM
  uint16_t  eepromGetBaseline(){
      if ( !(EEPROM.read(0) == EEPROM_BASELINE_START_1B && EEPROM.read(1) == EEPROM_BASELINE_START_2B) )  {
        return 0;
      }
  
      //baseline is packed into a 16 bit word
      return  (((unsigned int)EEPROM.read(2) << 8) | EEPROM.read(3));
  }
  
  uint16_t eepromStoreBaseline(uint16_t baseline){
    if(baseline == 0) { return 0;}

    EEPROM.write(0, EEPROM_BASELINE_START_1B);
    EEPROM.write(1, EEPROM_BASELINE_START_2B);
    EEPROM.write(2, (baseline >> 8) & 0x00FF);
    EEPROM.write(3, baseline & 0x00FF);

    return baseline;
  }
  #endif  
  
  void setReadyForSensorDataUpload() {
    PRINTLN(">>>> Setting readyForSensorDataUpload to true");
    readyForSensorDataUpload = true;
  }
  
  void setReadyForWeatherServiceUpdate() {
    PRINTLN(">>>> Setting readyForWeatherServiceUpdate to true");
    readyForWeatherServiceUpdate = true;
  }
  
  void setReadyForCurrentWeatherUpdate() {
    PRINTLN(">>>> Setting readyForUpdate to true");
    readyForCurrentWeatherUpdate = true;
  }
  
  void setReadyForSensorsRead() {
    PRINTLN(">>>> Setting readyForSensorsRead to true");
    readyForSensorsRead = true;
  }

  // converts the dBm to a range between 0 and 100%
  int8_t getWifiQuality() {
    int32_t dbm = WiFi.RSSI();
    if(dbm <= -100) {
        return 0;
    } else if(dbm >= -50) {
        return 100;
    } else {
        return 2 * (dbm + 100);
    }
  }

  void connectWifi(OLEDDisplay *display){
    WiFi.begin(WIFI_SSID, WIFI_PWD);

    int counter = 0;
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      PRINT(".");
      drawWifiConnecting(display, counter);
  
      counter++;
    }
  
    PRINTLN("");
    PRINTLN("WiFi connected!");
    PRINTLN(WiFi.localIP());
    PRINTLN();
  }
