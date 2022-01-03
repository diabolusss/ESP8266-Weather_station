#include "settings.h"
  #ifdef _EEPROM_
  /**
   *  Locations that have never been written to have the value of 255.
   */
  uint16_t  eepromGetBaseline(EEPROM_Rotate* _EEPROM){
      if ( !(_EEPROM->read(10) == EEPROM_BASELINE_START_1B && _EEPROM->read(13) == EEPROM_BASELINE_START_2B) )  {
        return 0;
      }
  
      //baseline is packed into a 16 bit word
      return  (((unsigned int)_EEPROM->read(11) << 8) | _EEPROM->read(12));
  }

  /**
   *  The value is written only if differs from the one already saved at the same address.
   *  
   *  NB An EEPROM write takes 3.3 ms to complete. 
   *  NB The EEPROM memory has a specified life of 100,000 write/erase cycles, so 
   *    using update function instead of write() can save cycles if the written data does not change often
   */
  uint16_t eepromStoreBaseline(EEPROM_Rotate* _EEPROM, uint16_t baseline){
    if(baseline == 0) { return 0;}

    _EEPROM->write(10, EEPROM_BASELINE_START_1B);
    _EEPROM->write(13, EEPROM_BASELINE_START_2B);
    _EEPROM->write(11, (baseline >> 8) & 0x00FF);
    _EEPROM->write(12, baseline & 0x00FF);

    bool ok = _EEPROM->commit();

    PRINT("EEPROM >>> save status ");
    PRINTLN(ok ? "OK" : "FAILED");
    
    return baseline;
  }
  #endif  
  
  void setReadyForSensorDataUpload() {
    PRINTLN(">>>> Setting readyForSensorDataUpload to true");
    readyForSensorDataUpload = true;

    if(!ccs811BaselineRestored){//one shot counter
      ++ccs811BaselineRestoreCount;//this method is called every 1 min; thus count at least for 23 to restore ccs baseline
                                   //read datasheet for minimal delay
    }
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
