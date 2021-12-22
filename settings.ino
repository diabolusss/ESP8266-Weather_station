#include "settings.h"

  void setReadyForSensorDataUpload() {
    Serial.println(">>>> Setting readyForSensorDataUpload to true");
    readyForSensorDataUpload = true;
  }
  
  void setReadyForWeatherServiceUpdate() {
    Serial.println(">>>> Setting readyForWeatherServiceUpdate to true");
    readyForWeatherServiceUpdate = true;
  }
  
  void setReadyForCurrentWeatherUpdate() {
    Serial.println(">>>> Setting readyForUpdate to true");
    readyForCurrentWeatherUpdate = true;
  }
  
  void setReadyForSensorsRead() {
    Serial.println(">>>> Setting readyForSensorsRead to true");
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

  void connectWifi(){
    WiFi.begin(WIFI_SSID, WIFI_PWD);

    int counter = 0;
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      drawWifiConnecting(&display, counter);
  
      counter++;
    }
  
    Serial.println("");
    Serial.println("WiFi connected!");
    Serial.println(WiFi.localIP());
    Serial.println();
  }
