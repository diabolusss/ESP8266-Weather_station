//#define DEBUG
#define CCS811_STORE_BASELINE
#define CCS811_RESTORE_BASELINE
#include "settings.h"
#include "UIHelper.h"

/******************************
 * Begin Service Prototypes
 *****************************/
  void updateCurrentWeather();
  void updateServiceData(OLEDDisplay *display);
  
/******************************
 * End of Service Prototypes
 *****************************/
 
void setup() {
  #ifdef DEBUG
    // Serial port for debugging purposes
    Serial.begin(115200);
    delay(10000); //timeout to be sure that serial is ready
  #endif

  #ifdef _EEPROM_
    EEPROMPool.size(EEPROM_SECTOR_POOL_COUNT);
    EEPROMPool.begin(EEPROM_SECTOR_POOL_SIZE);
    //EEPROMPool.offset(EEPROM_SECTOR_POOL_SIZE-4); //2Byte CRC and 1B auto increment wil be stored there
                                                    //INCORRECT
                                                    // default: sector begin 
  #endif

  Wire.begin(SDA_PIN, SDC_PIN);

  {
    //This begins the CCS811 sensor and prints error status of .beginWithStatus()
    bool status = MCU811b.begin();
    #ifdef DEBUG
      PRINT("CCS811 >>> begin status ");
      PRINTLN(status ? "ok" : "failed");
      
      PRINT(" HW version: ");           PRINTLN2(MCU811b.hardware_version(),HEX);
      PRINT(" bootloader  version: ");  PRINTLN2(MCU811b.bootloader_version(),HEX);
      // Check if flashing should be executed
      if( MCU811b.application_version() >= 0x2000 ) { 
        PRINT(" APP version is up to date: 0x");
        PRINTLN2(MCU811b.application_version(),HEX);
      }else{
        PRINTLN(" APP version is old, consider upgrading...");
      }
    #endif
    
    #ifdef _EEPROM_ //just inform about baseline (will be restored after ~20 min)
      PRINT("EEPROM >>> retrieved baseline 0x");
      PRINTLN2(eepromGetBaseline(&EEPROMPool), HEX);
    #endif
    
    // Start measuring
    status = MCU811b.start(CCS811_MODE_1SEC);
    if( !status ){ PRINTLN("CCS811 >>> measure start FAILED");
    } else {PRINTLN("CCS811 >>> measuring every 1 sec");}
  }

  {//prepare hdc1080
    hdc1080.begin(HDC1080_I2C_ADDR, HDC1080_HUMID_RESOLUTION_BITS, HDC1080_TEMP_RESOLUTION_BITS);
      PRINTLN("HDC1080 >>> found");
      PRINT("  Manufacturer ID=0x");
      PRINTLN2(hdc1080.readManufacturerId(), HEX); // 0x5449 ID of Texas Instruments
      PRINT("  Device ID=0x");
      PRINTLN2(hdc1080.readDeviceId(), HEX); // 0x1050 ID of the device
      PRINT("  Device Serial Number=");
      HDC1080_SerialNumber sernum = hdc1080.readSerialNumber();
      char format[12];
      sprintf(format, "%02X-%04X-%04X", sernum.serialFirst, sernum.serialMid, sernum.serialLast);
      PRINTLN(format);
      
    HDC1080_Registers reg = hdc1080.readRegister();
      //by default:
      //  heater disabled
      //  T or RH is acquired in separate calls
      //  T res 14b & H res 14b
      PRINT("  Measurement Resolution: T=");
      PRINT2(reg.TemperatureMeasurementResolution, BIN);
      PRINT(" (0=14 bit, 1=11 bit)");
    
      PRINT("  RH=");
      PRINT2(reg.HumidityMeasurementResolution, BIN);
      PRINTLN(" (00=14 bit, 01=11 bit, 10=8 bit)");

      PRINT("  Software reset bit: ");
      PRINT2(reg.SoftwareReset, BIN);
      PRINTLN(" (0=Normal Operation, 1=Software Reset)");
    
      PRINT("  Heater: ");
      PRINT2(reg.Heater, BIN);
      PRINTLN(" (0=Disabled, 1=Enabled)");
    
      PRINT("  Mode of Acquisition: ");
      PRINT2(reg.ModeOfAcquisition, BIN);
      PRINTLN(" (0=T or RH is acquired, 1=T and RH are acquired in sequence, T first)");
    
      PRINT("  Battery Status: ");
      PRINT2(reg.BatteryStatus, BIN);
      PRINTLN(" (0=Battery voltage > 2.8V, 1=Battery voltage < 2.8V)");
  }//end of prepare HDC1080

  {
    Wire.beginTransmission(BMP180_I2C_ADDR);
    //initialize Atmosphere sensor
    if (!bmp.begin()) {
      PRINTLN("BMP180 >>> Sensor missing");
    }else{
      PRINTLN("BMP180 >>> ready");
    }
    Wire.endTransmission();

    PRINTLN(" Pressure calibration vars");
    PRINT("  AC1=");
      PRINTLN(bmp.getAC1());
    PRINT("  AC2=");
      PRINTLN(bmp.getAC2());
    PRINT("  AC3=");
      PRINTLN(bmp.getAC3());
    PRINT("  AC4=");
      PRINTLN(bmp.getAC4());
    
    PRINT("  B1=");
      PRINTLN(bmp.getB1());
    PRINT("  B2=");
      PRINTLN(bmp.getB2());

    PRINTLN(" Temperature calibration vars");
    PRINT("  AC5=");
      PRINTLN(bmp.getAC5());
    PRINT("  AC6=");
      PRINTLN(bmp.getAC6());
    
    PRINT("  MC=");
      PRINTLN(bmp.getMC());
    PRINT("  MD=");
      PRINTLN(bmp.getMD());
  }

  //initialize light sensor
  Wire.beginTransmission(BH1750FVI_I2C_ADDR);
    Wire.write(BH1750FVI_POWERON);
  Wire.endTransmission();

  {
    ui.setTargetFPS(25); //updateInterval = 1/fps * 1000 = 40 ms
    ui.setTimePerFrame(10*1000); // 10s per frame; ticksPerFrame = time / updateInterval = 250
    
    ui.disableAllIndicators();
  
    // You can change the transition that is used
    // SLIDE_LEFT, SLIDE_RIGHT, SLIDE_TOP, SLIDE_DOWN
    ui.setFrameAnimation(SLIDE_LEFT);
    ui.setFrames(frames, numberOfFrames);
    ui.setOverlays(overlays, numberOfOverlays);
    
    // UI.init takes care of display.init()
    ui.init();

    //directly writes to a register, so call it only after initialization
    display.flipScreenVertically();
    display.setBrightness(32);
    
    PRINTLN("OLED >>> Display & UI ready.");
  }

  drawBootWelcome(&display, "Weather Station");
  connectWifi(&display);
  
  while (!client.connect(THINGSPEAK_SERVER, THINGSPEAK_PORT)) {
    PRINTLN("Connection to thingspeak Failed");
  }
  
  // don't wait for network, observe time changing
  // when NTP timestamp is received
  configTime(MYTZ, "pool.ntp.org");

  updateServiceData(&display);

  //attach INT_flags or maybe attach event handlers directly? (attaching directly triggers board crash
  forecastUpdateTicker.attach(FORECAST_UPDATE_INTERVAL_12H_SECS,      setReadyForWeatherServiceUpdate); //hours
  weatherUpdateTicker.attach(CURR_WEATHER_UPDATE_INTERVAL_20M_SECS,   setReadyForCurrentWeatherUpdate); //minutes
  sensorsDataUploadTicker.attach(TS_WRITE_UPDATE_INTERVAL_1M_SEC,     setReadyForSensorDataUpload);
  sensorsReadTicker.attach(SENSORS_READ_AWAKE_INTERVAL_3S_SECS,       setReadyForSensorsRead); //update twice before frame change

  displaySleepTicker.attach(DISPLAY_SLEEP_INTERVAL_5M_SECS, displayOff, (OLEDDisplay *)&display);
}

void loop() {  
  if(!ccs811BaselineRestored){//one shot baseline restore
    if(ccs811BaselineRestoreCount == 1 || ccs811BaselineRestoreCount > 23){ //restore instantly for smooth graph
                                                                            // then wait at least 20min for factory suggested restore
                                                                            // meanwhile it may change
      restoreCCS811Baseline();
      
      if(ccs811BaselineRestoreCount > 23){
        ccs811BaselineRestored = true;
        ccs811BaselineRestoreCount = 0;
      }
    }
  }
  
  //Read Temperature Humidity every X seconds
  // or just before uploading data to server
  if(readyForSensorsRead || readyForSensorDataUpload){
    readTemperatureHumidity();
    readAtmosphere();
    if(humidity > 0 && temp > ABSOLUTE_ZERO_TEMP_C){ 
      MCU811b.set_envdata_Celsius_percRH(temp, humidity); //compensate CCS811 environment 
                                                     // NB (dont use builtin hdc1080 as CCS811 introduces heat dissipation error)
    }
    readLight();
    readCCS811b();
    
    readyForSensorsRead = false; //reset
  }
  
  //Upload Temperature Humidity every 60 seconds
  if(readyForSensorDataUpload){
    uploadTemperatureHumidity();
  }

  if(readyForCurrentWeatherUpdate) {
    updateCurrentWeather();
  }

  //handle button event
  if(D7Button.hasEvent()){
    if(D7Button.isLongPress()){
      storeCCS811Baseline();
      
    }else if(D7Button.isPress()){
       restoreCCS811Baseline();
       drawBootWelcome(&display, "Baseline restored");

    }else if(D7Button.isClick()){
      if(D7Button.isDoubleClick()){
        PRINTLN("DBL CLICK");
        
      }else if(D7Button.isSingleClick()){
        toggleDisplayState(&display, &ui);
      }
    }
  }//end of handle button event
    
  if(readyForWeatherServiceUpdate && display.isAwake() && ui.getUiState()->frameState == FIXED) {
    updateServiceData(&display);
  }

  if(display.isAwake()){//if display is off, then skip ui handling
    int remainingTimeBudget = ui.update();
  
    if (remainingTimeBudget > 0) {
      // You can do some work here
      // Don't do stuff if you are below your
      // time budget.
      delay(remainingTimeBudget);
    }
  }
}

void readCCS811b(){
  //todo use interrupt to avoid calling dataAvailable
  uint16_t eco2, etvoc, errstat, raw;
  MCU811b.read(&eco2,&etvoc,&errstat,&raw); 

  if( errstat==CCS811_ERRSTAT_OK ) {
    eCO2 = eco2;
    eTVOC = etvoc;
    
    PRINT("CO2[");
    PRINT(eCO2); //calculated CO2 reading
    PRINT("] tVOC[");
    PRINT(eTVOC); //calculated TVOC reading
    PRINT("]");
    PRINTLN();
   } else if( errstat==CCS811_ERRSTAT_OK_NODATA ) {
      PRINTLN("waiting for (new) data");
   } else if( errstat & CCS811_ERRSTAT_I2CFAIL ) { 
      PRINTLN("I2C error");
   } else {
      PRINT( "error: " );
      PRINTLN( MCU811b.errstat_str(errstat) ); 
   }

   MCU811b.get_baseline(&raw);
   ccs811_baseline = raw;
   PRINT("CCS811 >>> current baseline 0x");
   PRINTLN2(raw, HEX);
}

/**
 *  Store CCS811 baseline into eeprom.
 *  To use, wait until the sensor is burned in, warmed up, and in clean air.  Then,  
 *   save the baseline to EEPROM.  
 *  Aftewards, the sensor can be powered up in dirty air and the baseline should be 
 *   restored to the CCS811 to help the sensor stablize faster.
 *   
 *   EEPROM memory usage:
 *  addr: data  
 *  ----------  
 *  0x00: 0xA5  
 *  0x01: 0xnn  
 *  0x02: 0xmm
 *  0x03: 0xB2  
 *  
 *    0xA5B2 is written as an indicator that 0x01 and 0x02 contains a valid number.
 *    0xnnmm is the saved data.
 */
void storeCCS811Baseline(){
  uint16_t baseline;
  bool ok = MCU811b.get_baseline(&baseline);
  if(ok){
    #ifdef _EEPROM_
    ok = (baseline != eepromGetBaseline(&EEPROMPool));
    #endif
  }else{ PRINT("CCS811 >>> Failed to get baseline"); return;}

  if(ok){
      PRINT("CCS811 >>> storing baseline 0x");
      PRINTLN2(baseline, HEX);
      #ifdef CCS811_STORE_BASELINE
        eepromStoreBaseline(&EEPROMPool, baseline);
      #endif
   }else{ PRINT("CCS811 >>> Baseline already exists");}
}

void restoreCCS811Baseline(){
  #ifdef _EEPROM_
    //try to restore baseline
    uint16_t baseline = eepromGetBaseline(&EEPROMPool);
  
    PRINT("EEPROM >>> retrieved baseline 0x");
    PRINTLN2(baseline, HEX);
         
    if(baseline > 0 && baseline != 0xFFFF){
       #ifdef CCS811_RESTORE_BASELINE
       bool status = MCU811b.set_baseline(baseline);
       PRINT("CCS811 >>> Baseline restore ");
       PRINTLN( status ? "OK" : "FAILED");
       #endif
    }else{
       PRINTLN("CCS811 >>> Missing or wrong baseline.");
    }
  #endif
}

//read temperature humidity data
void readTemperatureHumidity(){
  PRINT("Temperature: ");
  PRINT("CCS811HDC = ");
    ccs811hdc1080Temp = hdc1080.readTemperature(); //dont use it, as CCS811 introduces heat dissipation error
    PRINT(ccs811hdc1080Temp);
  PRINT("; BMP factory = ");
    PRINT(bmp.readTemperature());

  PRINT("; custom = ");
    temp = bmp.readCustomCalibratedTemperature(AC5, AC6, MC, MD);
    PRINT(temp);
    
  PRINTLN(" *C");

  humidity = hdc1080.readHumidity(); //The result of the acquisition is always a 14 bit value,
                                     //  while the accuracy is related to the selected conversion time
  PRINT("Humidity=");
  PRINT(humidity);
  PRINTLN("%");
}

void readLight(){
  Wire.beginTransmission(BH1750FVI_I2C_ADDR);
    Wire.write(BH1750FVI_RESET_RDATA); //Asynchronous reset/Reset Data register value
  Wire.endTransmission();
 
  Wire.beginTransmission(BH1750FVI_I2C_ADDR);
    Wire.write(BH1750FVI_HRES_MODE_ONCE); //hres read and power down; why we don't send power on cmd?
                                          // In one time measurement, Statement moves to power down mode 
                                          //   after measurement completion. If updated result is need
                                          //   then please resend measurement instruction.
  Wire.endTransmission();

  #if defined BH1750FVI_LRES_MODE_CONTINUOUS || defined BH1750FVI_LRES_MODE_ONCE
    delay(17);//first reading up to 24ms
  #else
    delay(163);//first reading up to 180ms
  #endif
  
  Wire.requestFrom(BH1750FVI_I2C_ADDR, 2); // 2byte every time
  for (tempLight = 0; Wire.available() >= 1; ) {
    char c = Wire.read();
    tempLight = (tempLight << 8) + (c & 0xFF);
  }
  
  tempLight = LUX_PER_COUNT(tempLight); 
  
  PRINT("light: ");
  PRINTLN(tempLight);
}


void readAtmosphere(){
  atmPressure = bmp.readPressure();
  PRINT("Pressure = ");
  PRINT(atmPressure);
  PRINTLN(" Pascal");

  // Calculate altitude assuming 'standard' barometric
  // pressure of 1013.25 millibar = 101325 Pascal
  float tempAlt = bmp.readAltitude();
  atmAlt = tempAlt; 
  PRINT("Altitude = ");
  PRINT(tempAlt);
  PRINTLN(" meters");

  PRINT("Pressure at sealevel (calculated) = ");
  PRINT(bmp.readSealevelPressure());
  PRINTLN(" Pa");

  // you can get a more precise measurement of altitude
  // if you know the current sea level pressure which will
  // vary with weather and such. If it is 1015 millibars
  // that is equal to 101500 Pascals.
    PRINT("Real altitude = ");
    PRINT(bmp.readAltitude(101500));
    PRINTLN(" meters");
}

//upload temperature humidity data to thinkspeak.com
void uploadTemperatureHumidity(){
   if(!client.connect(THINGSPEAK_SERVER, THINGSPEAK_PORT)){
    PRINTLN("connection failed");
    return;
  }

  String fields = "";
  if(temp > ABSOLUTE_ZERO_TEMP_C){
    fields += String("&field1=")+temp;
  }
  if(humidity > 0){
    fields += String("&field2=")+humidity;
  }
  fields += String("&field3=")+tempLight
    +"&field4="+atmPressure
    +"&field5="+atmAlt
    +"&field6="+eCO2
    +"&field7="+eTVOC
    ;
    
  client.print(String("GET ") + "/update?api_key="+THINGSPEAK_API_WRITE + fields 
    +" HTTP/1.1\r\n" +"Host: " + THINGSPEAK_SERVER 
    + "\r\n" + "Connection: close\r\n\r\n"
  );
    
  while(client.available()){
    String line = client.readStringUntil('\r');
    PRINT(line);
  }

  /*
  fields = String("&field1=")+temp
    + "&field2="+ccs811hdc1080Temp
    //+ "&field3="+eTVOC
    ;

  //extra call for debugging
  //doesn't work, possibly need to wait some time before making second call
  client.print(String("GET ") + "/update?api_key="+THINGSPEAK_API_WRITE2 + fields 
    +" HTTP/1.1\r\n" +"Host: " + THINGSPEAK_SERVER 
    + "\r\n" + "Connection: close\r\n\r\n"
  );
    
  while(client.available()){
    String line = client.readStringUntil('\r');
    PRINT(line);
  }
  */
  readyForSensorDataUpload = false; //reset
}

  void updateCurrentWeather() {
    currentWeatherClient.setMetric(IS_METRIC);
    currentWeatherClient.setLanguage(OPEN_WEATHER_MAP_LANGUAGE);
    currentWeatherClient.updateCurrent(&currentWeather, OPEN_WEATHER_MAP_APP_ID, OPEN_WEATHER_MAP_LOCATION);

    delay(100); //do we actually need that here?
    PRINTLN("------------------------------------");

    PRINTF("country: %s\n", currentWeather.country.c_str());// "country": "CH", String country; 
    PRINTF("cityName: %s\n", currentWeather.cityName.c_str());// "name": "Zurich", String cityName;
    
    PRINTF("lon: %f\n", currentWeather.lon); //float
    PRINTF("lat: %f\n", currentWeather.lat); //float
    PRINTF("weatherId: %d\n", currentWeather.weatherId);
    PRINTF("main: %s\n", currentWeather.main.c_str()); // "main": "Rain", String main;
    PRINTF("description: %s\n", currentWeather.description.c_str());// "description": "shower rain", String description;
    PRINTF("icon: %s\n", currentWeather.icon.c_str());// "icon": "09d" String icon; String iconMeteoCon;
    PRINTF("iconMeteoCon: %s\n", currentWeather.iconMeteoCon.c_str());
    PRINTF("temp: %f\n", currentWeather.temp);// float temp;
    PRINTF("pressure: %d\n", currentWeather.pressure); // "pressure": 1013, uint16_t pressure;
    PRINTF("humidity: %d\n", currentWeather.humidity);// "humidity": 87, uint8_t humidity;
    PRINTF("tempMin: %f\n", currentWeather.tempMin); //float
    PRINTF("tempMax: %f\n", currentWeather.tempMax); //float
    PRINTF("windSpeed: %f\n", currentWeather.windSpeed);// "wind": {"speed": 1.5}, float windSpeed;
    PRINTF("windDeg: %f\n", currentWeather.windDeg);// "wind": {"deg": 1.5}, float windDeg;
    PRINTF("clouds: %d\n", currentWeather.clouds);// "clouds": {"all": 90}, uint8_t clouds;

    time_t time = currentWeather.observationTime;
    PRINTF("observationTime: %s", ctime(&time));
    time = currentWeather.sunrise;
    PRINTF("sunrise: %s", ctime(&time));
    time = currentWeather.sunset;
    PRINTF("sunset: %s", ctime(&time));
  
   PRINTLN();
    PRINTLN("---------------------------------------------------/\n");
    
    readyForCurrentWeatherUpdate = false;
  }

  void updateForecast(){
    //3h forecast api. min/max values are useless
      forecastClient.setMetric(IS_METRIC);
      forecastClient.setLanguage(OPEN_WEATHER_MAP_LANGUAGE);
      
      uint8_t allowedHours[] = {12};
      forecastClient.setAllowedHours(allowedHours, sizeof(allowedHours));
      forecastClient.updateForecasts(forecasts, OPEN_WEATHER_MAP_APP_ID, OPEN_WEATHER_MAP_LOCATION, MAX_FORECASTS);
  }

  void updateServiceData(OLEDDisplay *display) {
    if(display->isAwake()){
      drawProgress(display, 20, "Updating current weather...");
    }
      updateCurrentWeather();

    if(display->isAwake()){
      drawProgress(display, 50, "Updating forecasts...");
    }
      updateForecast();
      
    //drawProgress(display, 90, "Reading thingspeak...");
      //thingspeak.getLastChannelItem(THINGSPEAK_CHANNEL_ID, THINGSPEAK_API_READ);
      
    readyForWeatherServiceUpdate = false;

    if(display->isAwake()){
      drawProgress(display, 100, "Done...");
      delay(1000);
    }
  }

  
