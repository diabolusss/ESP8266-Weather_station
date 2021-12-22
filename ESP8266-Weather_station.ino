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
 
 ICACHE_RAM_ATTR void btnPressedISR() {
    Serial.println("BTN pressed!!!");
    triggerDisplayWakeup = true;
  }

void setup() {
  pinMode(OLED_WAKEUP_BTN, INPUT);
  
  // Set button pin as interrupt, to wakeup display (power on)
  //  CHANGE: to trigger the interrupt whenever the pin changes value – for example from HIGH to LOW or LOW to HIGH;
  //  FALLING: for when the pin goes from HIGH to LOW;
  //  RISING: to trigger when the pin goes from LOW to HIGH.
  attachInterrupt(digitalPinToInterrupt(OLED_WAKEUP_BTN), btnPressedISR, RISING);

  // Serial port for debugging purposes
  Serial.begin(115200);

  Wire.begin(SDA_PIN, SDC_PIN);

  delay(1000); //timeout to make serial come up

  //This begins the CCS811 sensor and prints error status of .beginWithStatus()
  CCS811Core::CCS811_Status_e returnCode = MCU811b.beginWithStatus();
    Serial.print("CCS811 begin exited with: ");
    Serial.println(MCU811b.statusString(returnCode));

  {
    hdc1080.begin(HDC1080_I2C_ADDR);
      Serial.print("Manufacturer ID=0x");
      Serial.println(hdc1080.readManufacturerId(), HEX); // 0x5449 ID of Texas Instruments
      Serial.print("Device ID=0x");
      Serial.println(hdc1080.readDeviceId(), HEX); // 0x1050 ID of the device
    hdc1080.setResolution(HDC1080_TEMP_RESOLUTION_BITS, HDC1080_HUMID_RESOLUTION_BITS);
    //WRONG!!!
    //14:51:24.915 -> Measurement Resolution: T=0 (0=14 bit, 1=11 bit) RH=1 (00=14 bit, 01=11 bit, 10=8 bit)
    HDC1080_Registers reg = hdc1080.readRegister();
      Serial.print("Measurement Resolution: T=");
      Serial.print(reg.TemperatureMeasurementResolution, BIN);
      Serial.print(" (0=14 bit, 1=11 bit)");
    
      Serial.print(" RH=");
      Serial.print(reg.HumidityMeasurementResolution, BIN);
      Serial.println(" (00=14 bit, 01=11 bit, 10=8 bit)");
  }

  {
    Wire.beginTransmission(Atom_ADDR);
    //initialize Atmosphere sensor
    if (!bmp.begin()) {
      Serial.println("Could not find BMP180 or BMP085 sensor at 0x77");
    }else{
      Serial.println("Found BMP180 or BMP085 sensor at 0x77");
    }
    Wire.endTransmission();
  }

  //initialize light sensor
  Wire.beginTransmission(Light_ADDR);
  Wire.write(0b00000001);
  Wire.endTransmission();

  //dht.begin();

  // initialize display
  display.init();
  //display.flipScreenVertically();
  
  displayBlank(&display);

  drawBootWelcome(&display, "Weather Station");

  connectWifi();
  
  ui.setTargetFPS(30);
  ui.setTimePerFrame(10*1000); // Setup frame display time to 10 sec
  
  //Hack until disableIndicator works:
  //Set an empty symbol
  ui.setActiveSymbol(emptySymbol);
  ui.setInactiveSymbol(emptySymbol);
  
  ui.disableIndicator();

  // You can change the transition that is used
  // SLIDE_LEFT, SLIDE_RIGHT, SLIDE_TOP, SLIDE_DOWN
  ui.setFrameAnimation(SLIDE_LEFT);
  ui.setFrames(frames, numberOfFrames);
  ui.setOverlays(overlays, numberOfOverlays);
  
  // You can change this to
  // TOP, LEFT, BOTTOM, RIGHT
  ui.setIndicatorPosition(BOTTOM);
  // Defines where the first frame is located in the bar.
  ui.setIndicatorDirection(LEFT_RIGHT);
  
  
  // Inital UI takes care of initalising the display too.
  ui.init();
  Serial.println("");
  
  while (!client.connect(host, httpPort)) {
    Serial.println("Connection to thingspeak Failed");
  }
  
  // don't wait for network, observe time changing
  // when NTP timestamp is received
  configTime(MYTZ, "pool.ntp.org");

  updateServiceData(&display);

  //attach INT_flags or maybe attach event handlers directly? (attaching directly triggers board crash
  forecastUpdateTicker.attach(FORECAST_UPDATE_INTERVAL_SECS,      setReadyForWeatherServiceUpdate); //hours
  weatherUpdateTicker.attach(CURR_WEATHER_UPDATE_INTERVAL_SECS,   setReadyForCurrentWeatherUpdate); //minutes
  sensorsDataUploadTicker.attach(TS_WRITE_UPDATE_INTERVAL_SEC,    setReadyForSensorDataUpload);
  sensorsReadTicker.attach(5,                                     setReadyForSensorsRead);
}

void loop() {  
  //Read Temperature Humidity every 5 seconds
  if(readyForSensorsRead){
    readTemperatureHumidity();
    readAtmosphere();
    if(humi > 0 && temp > 0){ //todo -1 for not initialized + thingspeak don't update such values
      MCU811b.setEnvironmentalData(humi, temp); //compensate environment
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
  
  if(readyForWeatherServiceUpdate && ui.getUiState()->frameState == FIXED) {
    updateServiceData(&display);
  }

  int remainingTimeBudget = ui.update();

  if (remainingTimeBudget > 0) {
    // You can do some work here
    // Don't do stuff if you are below your
    // time budget.
    delay(remainingTimeBudget);
  }
}

//todo use interrupt to avoid calling dataAvailable
void readCCS811b(){
   //Check to see if data is ready with .dataAvailable()
  if (MCU811b.dataAvailable())  {
    //If so, have the sensor read and calculate the results.
    //Get them later
    MCU811b.readAlgorithmResults();

    eCO2 = MCU811b.getCO2();
    etVOC = MCU811b.getTVOC();
    
    Serial.print("CO2[");
    //Returns calculated CO2 reading
    Serial.print(eCO2);
    Serial.print("] tVOC[");
    //Returns calculated TVOC reading
    Serial.print(etVOC);
    Serial.print("]");
    Serial.println();
    
  } else if (MCU811b.checkForStatusError())  {
    printSensorError();
  }
}

//printSensorError gets, clears, then prints the errors
//saved within the error register.
void printSensorError()
{
  uint8_t error = MCU811b.getErrorRegister();

  if (error == 0xFF) //comm error
  {
    Serial.println("Failed to get ERROR_ID register.");
  }
  else
  {
    Serial.print("Error: ");
    if (error & 1 << 5)
      Serial.print("HeaterSupply");
    if (error & 1 << 4)
      Serial.print("HeaterFault");
    if (error & 1 << 3)
      Serial.print("MaxResistance");
    if (error & 1 << 2)
      Serial.print("MeasModeInvalid");
    if (error & 1 << 1)
      Serial.print("ReadRegInvalid");
    if (error & 1 << 0)
      Serial.print("MsgInvalid");
    Serial.println();
  }
}

//read temperature humidity data
void readTemperatureHumidity(){
  humi = hdc1080.readHumidity();
  Serial.print("hdc1080 T=");
  Serial.print(hdc1080.readTemperature());
  Serial.print("C, RH=");
  Serial.print(humi);
  Serial.println("%");
}

void readLight(){
  Wire.beginTransmission(Light_ADDR);
    Wire.write(BH1750FVI_RESET_RDATA); //Asynchronous reset/Reset Data register value
  Wire.endTransmission();
 
  Wire.beginTransmission(Light_ADDR);
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
  
  Wire.requestFrom(Light_ADDR, 2); // 2byte every time
  for (tempLight = 0; Wire.available() >= 1; ) {
    char c = Wire.read();
    tempLight = (tempLight << 8) + (c & 0xFF);
  }
  
  tempLight = LUX_PER_COUNT(tempLight); 
  
  Serial.print("light: ");
  Serial.println(tempLight);
}


void readAtmosphere(){
  atmPressure = bmp.readPressure();
  Serial.print("Pressure = ");
  Serial.print(atmPressure);
  Serial.println(" Pascal");

  float tempAtm = bmp.readTemperature();
  temp = tempAtm;
  Serial.print("Temperature = ");
  Serial.print(tempAtm);
  Serial.println(" *C");
        
  // Calculate altitude assuming 'standard' barometric
  // pressure of 1013.25 millibar = 101325 Pascal
  float tempAlt = bmp.readAltitude();
  atmAlt = tempAlt; 
  Serial.print("Altitude = ");
  Serial.print(tempAlt);
  Serial.println(" meters");

  Serial.print("Pressure at sealevel (calculated) = ");
  Serial.print(bmp.readSealevelPressure());
  Serial.println(" Pa");

  // you can get a more precise measurement of altitude
  // if you know the current sea level pressure which will
  // vary with weather and such. If it is 1015 millibars
  // that is equal to 101500 Pascals.
    Serial.print("Real altitude = ");
    Serial.print(bmp.readAltitude(101500));
    Serial.println(" meters");
}

//upload temperature humidity data to thinkspeak.com
void uploadTemperatureHumidity(){
   if(!client.connect(host, httpPort)){
    Serial.println("connection failed");
    return;
  }
  // Three values(field1 field2 field3 field4) have been set in thingspeak.com 
  client.print(String("GET ") + "/update?api_key="+api_key
    +"&field1="+temp
    +"&field2="+humi 
    +"&field3="+tempLight
    +"&field4="+atmPressure
    +"&field5="+atmAlt
    +"&field6="+eCO2
    +"&field7="+etVOC
    +" HTTP/1.1\r\n" +"Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
    
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  
  readyForSensorDataUpload = false; //reset
}

  void updateCurrentWeather() {
    currentWeatherClient.setMetric(IS_METRIC);
    currentWeatherClient.setLanguage(OPEN_WEATHER_MAP_LANGUAGE);
    currentWeatherClient.updateCurrent(&currentWeather, OPEN_WEATHER_MAP_APP_ID, OPEN_WEATHER_MAP_LOCATION);

    delay(100); //do we actually need that here?
   Serial.println("------------------------------------");
  
    // "lon": 8.54, float lon;
    Serial.printf("lon: %f\n", currentWeather.lon);
    // "lat": 47.37 float lat;
    Serial.printf("lat: %f\n", currentWeather.lat);
    // "id": 521, weatherId weatherId;
    Serial.printf("weatherId: %d\n", currentWeather.weatherId);
    // "main": "Rain", String main;
    Serial.printf("main: %s\n", currentWeather.main.c_str());
    // "description": "shower rain", String description;
    Serial.printf("description: %s\n", currentWeather.description.c_str());
    // "icon": "09d" String icon; String iconMeteoCon;
    Serial.printf("icon: %s\n", currentWeather.icon.c_str());
    Serial.printf("iconMeteoCon: %s\n", currentWeather.iconMeteoCon.c_str());
    // "temp": 290.56, float temp;
    Serial.printf("temp: %f\n", currentWeather.temp);
    // "pressure": 1013, uint16_t pressure;
    Serial.printf("pressure: %d\n", currentWeather.pressure);
    // "humidity": 87, uint8_t humidity;
    Serial.printf("humidity: %d\n", currentWeather.humidity);
    // "temp_min": 289.15, float tempMin;
    Serial.printf("tempMin: %f\n", currentWeather.tempMin);
    // "temp_max": 292.15 float tempMax;
    Serial.printf("tempMax: %f\n", currentWeather.tempMax);
    // "wind": {"speed": 1.5}, float windSpeed;
    Serial.printf("windSpeed: %f\n", currentWeather.windSpeed);
    // "wind": {"deg": 1.5}, float windDeg;
    Serial.printf("windDeg: %f\n", currentWeather.windDeg);
    // "clouds": {"all": 90}, uint8_t clouds;
    Serial.printf("clouds: %d\n", currentWeather.clouds);
    // "dt": 1527015000, uint64_t observationTime;
    time_t time = currentWeather.observationTime;
    Serial.printf("observationTime: %d, full date: %s", currentWeather.observationTime, ctime(&time));
    // "country": "CH", String country;
    Serial.printf("country: %s\n", currentWeather.country.c_str());
    // "sunrise": 1526960448, uint32_t sunrise;
    time = currentWeather.sunrise;
    Serial.printf("sunrise: %d, full date: %s", currentWeather.sunrise, ctime(&time));
    // "sunset": 1527015901 uint32_t sunset;
    time = currentWeather.sunset;
    Serial.printf("sunset: %d, full date: %s", currentWeather.sunset, ctime(&time));
  
    // "name": "Zurich", String cityName;
    Serial.printf("cityName: %s\n", currentWeather.cityName.c_str());
    Serial.println();
    Serial.println("---------------------------------------------------/\n");
    
    readyForCurrentWeatherUpdate = false;
  }

  void updateServiceData(OLEDDisplay *display) {
    display->flipScreenVertically();
    drawProgress(display, 20, "Updating current weather...");
      updateCurrentWeather();
    
    drawProgress(display, 50, "Updating forecasts...");
      forecastClient.setMetric(IS_METRIC);
      forecastClient.setLanguage(OPEN_WEATHER_MAP_LANGUAGE);
      
      uint8_t allowedHours[] = {12};
      forecastClient.setAllowedHours(allowedHours, sizeof(allowedHours));
      forecastClient.updateForecasts(forecasts, OPEN_WEATHER_MAP_APP_ID, OPEN_WEATHER_MAP_LOCATION, MAX_FORECASTS);
  
    //drawProgress(display, 90, "Updating thingspeak...");
      //thingspeak.getLastChannelItem(THINGSPEAK_CHANNEL_ID, THINGSPEAK_API_READ_KEY);
      
    readyForWeatherServiceUpdate = false;
    
    drawProgress(display, 100, "Done...");
    delay(1000);
  }
  
  
  
  

  
