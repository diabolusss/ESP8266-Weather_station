#ifndef __SETTINGS_H_
#define __SETTINGS_H_

#define LV //to identify region
#define _I2C_OLED //define to load proper lib
#define OLED_WAKEUP_BTN D1 
#define ABSOLUTE_ZERO_TEMP_C -273 //-273.15

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

  //sensor libs
    #include "DHT.h"
    #include <Adafruit_BMP085.h>

    #ifdef _SPI_OLED
      #include "SSD1306Spi.h"
    #elif defined _I2C_OLED
      #include "SSD1306Wire.h"
    #endif

  //other peripheral libs
    #include "Wire.h"
    #include <ESPWiFi.h>
    #include <ESP8266WiFi.h>
    #include "OLEDDisplayUi.h"
     
  //service libs
    #include <ESPHTTPClient.h>
    #include <JsonListener.h>
    #include "OpenWeatherMapCurrent.h"
    #include "OpenWeatherMapForecast.h"

  //system libs
    #include <time.h>                       // time() ctime()
    #include <sys/time.h>                   // struct timeval
    
    #include <Ticker.h> //Works like a "thread", where a secondary function will run when necessary. 
                    // The library use no interupts of the hardware timers and works with the micros() / millis() function
                    // if repeats is 0 the ticker runs in endless mode. 
    #include "WeatherStationFonts.h"
    #include "WeatherStationImages.h"


/***************************
 * Time Settings
 **************************/
  #define STYLE_24HR
  //#include <simpleDSTadjust.h>
  #include <TZ.h>
  #ifdef LV
    #define MYTZ TZ_Europe_Riga
    #define NTP_SERVERS "0.lv.pool.ntp.org", "1.lv.pool.ntp.org", "2.lv.pool.ntp.org"//, "3.lv.pool.ntp.org"
    #define UTC_OFFSET +2
    
    //struct dstRule StartRule = {"EEST", Last, Sun, Mar, 28, 3600}; // Eastern European Summer Time = UTC/GMT +1 hours
    //struct dstRule EndRule = {"EET", Last, Sun, Oct, 31, 0};       // Eastern European Time = UTC/GMT +2 hour
  #endif

  // Setup simpleDSTadjust Library rules
  //simpleDSTadjust dstAdjusted(StartRule, EndRule);
  
  /*#define TZ              2       // (utc+) TZ in hours
  #define DST_MN          60      // use 60mn for summer time in some countries
  #define TZ_MN           ((TZ)*60)
  #define TZ_SEC          ((TZ)*3600)
  #define DST_SEC         ((DST_MN)*60)*/

/***************************
 * WIFI Settings
 **************************/
  //include credentials
  #include "secret.h" //WIFI_SSID, WIFI_PWD
  
  WiFiClient client;

/***************************
 * Begin CCS811 Settings
 **************************/
 /*
  * save baseline to eprom(flash): https://pastebin.com/KGigYPr6
  * VOCs detected 
  *   Alcohols, Aldehydes, Ketones, Organic Acids, Amines, Aliphatic and Aromatic Hydrocarbons
  * Cross sensitivity 
  *   Humidity and Hydrogen
  * 
  * eCO2
  *   The equivalent CO2 (eCO2) output range for CCS811 is from
  *   400ppm up to 32768ppm.
  * eTVOC
  *   The equivalent Total Volatile Organic Compound (eTVOC)
  *   output range for CCS811 is from 0ppb up to 29206ppb.
  *   
  * CO2
  *   250-400ppm  Normal background concentration in outdoor ambient air
  *   400-1,000ppm  Concentrations typical of occupied indoor spaces with good air exchange
  *   1,000-2,000ppm  Complaints of drowsiness and poor air.
  *   2,000-5,000 ppm Headaches, sleepiness and stagnant, stale, stuffy air. Poor concentration, loss of attention, 
  *                     increased heart rate and slight nausea may also be present.
  *   5,000 Workplace exposure limit (as 8-hour TWA) in most jurisdictions.
  *   >40,000 ppm Exposure may lead to serious oxygen deprivation resulting in permanent brain damage, coma, even death.
  * 
  * CO
  *   9 ppm CO Max prolonged exposure (ASHRAE standard)
  *   35 ppm  CO Max exposure for 8 hour work day (OSHA)
  *   800 ppm CO Death within 2 to 3 hours
  *   12,800 ppm  CO Death within 1 to 3 minutes
  * 
  * - ppb (v) is parts per billion by volume (i.e., volume of gaseous pollutant per 10^9 volumes of ambient air).
  * - µg/m3 is micrograms of gaseous pollutant per cubic meter of ambient air.
  * 
  * The conversion assumes an ambient pressure of 1 atmosphere (1 atm = 1.01325 bar) and a temperature of 25 degrees Celsius.
  *   Sulphur dioxide (SO2)   1 ppb = 2.62 µg/m3 = 64.07 g/mol
  *   Nitrogen dioxide (NO2)  1 ppb = 1.88 µg/m3 = 46.01 g/mol
  *   Nitric oxide (NO)       1 ppb = 1.25 µg/m3 = 30.01 g/mol
  *   Ozone (O3)              1 ppb = 2.00 µg/m3 = 48 g/mol
  *   Carbon monoxide (CO)    1 ppb = 1.15 µg/m3 = 28.01 g/mol
  *   Benzene                 1 ppb = 3.19 µg/m3
  *   Ammonia (NH3)           1 ppb = 0.70 µg/m3 = 17.03 g/mol
  *   
  * The general equation is µg/m^3 = M * (ppb)*(12.187) / (273.15 + °C), where 
  *    M is the molecular weight of the gaseous pollutant. 
  * 
  * Concentration (µg/m3) = M x concentration (ppb) ÷ 24.45
  * Concentration (ppb) = 24.45 x concentration (µg/m3) ÷ M
  *   The number 24.45 in the equation is the volume (litres) of a mole (gram molecular weight) of a gas when the temperature is at 25°C and the pressure is at 1 atmosphere (1 atm = 1.01325 bar). 
  *  
  * In view of the fact that there are few controlled human exposure studies and the results are not confirmed, 
  *   and that the results of epidemiological studies are inconsistent, it is today not possible to conclude that 
  *   sensory irritation is associated with the sum of mass concentrations of VOCs at the low exposure levels 
  *   typically encountered in non-industrial indoor air. Therefore, although the likelihood of sensory effects 
  *   will increase with increasing TVOC concentration, at present no precise guidance can be given on which levels of TVOC 
  *   are of concern from a health and comfort point of view, and the magnitude of protection margins needed cannot be estimated. 
  *   Nevertheless, the general need for improved source control to diminish the pollution load on the indoor environments 
  *   from health, comfort, energy efficiency and sustainability points of view leads to the recommendation that VOC levels 
  *   in indoor air should be kept As Low As Reasonably - Achievable (ALARA).
  *   
  * ... air leakage in most homes, and in non-residential buildings too, will generally remove the chemicals faster 
  *   than the researchers reported for the plants tested by NASA. 
  *   The most effective household plants reportedly included aloe vera, English ivy, 
  *   and Boston fern for removing chemicals and biological compounds.
  *   https://cdn.sparkfun.com/assets/learn_tutorials/1/4/3/ECA_Report19.pdf
  */
  #include "SparkFunCCS811.h" //Click here to get the library: http://librarymanager/All#SparkFun_CCS811
  
  //#define CCS811_ADDR 0x5B //2nd I2C Address (HIGH)
  #define CCS811_ADDR 0x5A //1st I2C Address (LOW)

  CCS811 MCU811b(CCS811_ADDR);

  /*
   * The CCS811 supports compensation for relative humidity and ambient temperature. 
   * The ENV_DATA registers can be updated with the temperature and humidity (TH) values on each cycle.
   * The data sheet requirement that current temp value plus 25 is written
   *   to the ENV_DATA’s temperature register.
   *   NB If the application supports temperature or humidity compensation, but not both, the data sheet’s
   *      default value must be written to the register corresponding to the unsupported environment
   *      parameter. The user must not write zero to the unsupported temperature or humidity field of the ENV_DATA register.
   */
/***************************
 * Begin HDC1080 Settings
 **************************/
 /*
  * NB sensor is unable to compensate for the heat produced by the CCS811 
  *     This can cause the displayed temperature to be up to 15° higher than the actual ambient temperature. 
  *     To compensate for this take the difference between an initial temperature reading (with a cold sensor) 
  *     and a final temperature reading (after letting the sensor warm up). 
  *     Then subtract this value from `BMEtempC` to ensure the proper calibration is taking place.
  *     
  * The HDC1080 has two modes of operation: 
  *   - sleep mode  
  *   - measurement mode. 
  *  After power up, the HDC1080 is in sleep mode. In this mode, the HDC1080 waits for I2C input including commands 
  *   to configure the conversion times, read the status of the battery, trigger a measurement, and read measurements. 
  *  Once it receives a command to trigger a measurement, the HDC1080 moves from sleep mode to measurement mode. 
  *  After completing the measurement the HDC1080 returns to sleep mode.
  */
  #include "ClosedCube_HDC1080.h"
  #define HDC1080_I2C_ADDR 0x40 
  #define HDC1080_TEMP_RESOLUTION_BITS  HDC1080_RESOLUTION_11BIT  //conversion times:
                                                                  //  11bit   3.65ms
                                                                  //  14bit   6.35ms
  #define HDC1080_HUMID_RESOLUTION_BITS HDC1080_RESOLUTION_8BIT
                                                                  //  8bit    2.5ms
                                                                  //  11bit   3.85ms
                                                                  //  14bit   6.5ms

  ClosedCube_HDC1080 hdc1080;
 
/***************************
 * Begin Atmosphere and Light Sensor Settings
 **************************/
  #define BH1750FVI_I2C_ADDR 0b0100011 // BH1750FVI: 
                                      // Slave Address is 2 types, it is determined by ADDR Terminal
                                      // ADDR = ‘H’ ( ADDR ≧ 0.7VCC ) → “1011100“ -> 0x5c
                                      // ADDR = 'L' ( ADDR ≦ 0.3VCC ) → “0100011“ -> 0x23

      
  //#define MTREG_DEFAULT 69 //changable range of MTreg.
                             //                         Min.          Typ.        Max.
                             //                   
                             //changeable       bin    0001_1111      0100_0101   1111_1110
                             //range of MTreg   dec    31             69          254
                             //sensitivity:           (default*0.45)  (default)   (default*3.68)

  #define LUX_PER_COUNT(X) ( (X) * 0.83 ) //H-reslution mode : Illuminance per 1 count ( lx / count ) = 1 / 1.2 *( 69 / X )
                                         //H-reslution mode2 : Illuminance per 1 count ( lx / count ) = 1 / 1.2 *( 69 / X ) / 2

  #define BH1750FVI_POWERON                 0b00000001
  
  #define BH1750FVI_RESET_RDATA             0b00000111  // remove previous measurement result.
                                                        // Asynchronous reset:  It is necessary on power supply sequence
                                                        // Reset command is not acceptable in Power Down mode.       
  
  //HResMode is suitable for darkness ( less than 10 lx )
  //#define BH1750FVI_HRES_MODE_CONTINUOUS    0b00010000 //Start measurement at 1lx resolution.
  #define BH1750FVI_HRES_MODE_ONCE          0b00100000 //Automatically set to Power Down mode after measurement.
                                                       //Measurement Time is typically 120ms
                                                       
  //#define BH1750FVI_HRES_MODE2_CONTINUOUS   0b00010001 //Start measurement at 0.5lx resolution.
  //#define BH1750FVI_HRES_MODE2_ONCE         0b00100001 //Automatically set to Power Down mode after measurement.
                                                       //Measurement Time is typically 120ms
                                                       
  //#define BH1750FVI_LRES_MODE_CONTINUOUS    0b00010011 //Start measurement at 4lx resolution.
  //#define BH1750FVI_LRES_MODE_ONCE          0b00100011 //Automatically set to Power Down mode after measurement.
                                                       //Measurement Time is typically 16ms

  
  Adafruit_BMP085 bmp; //has own temperature sensor
  #define BMP180_I2C_ADDR 0b1110111  // BMP180: address:0x77
                                    // The LSB of the device address distinguishes between read (1) and write (0) operation,
                                    //  corresponding to address 0xEF (read) and 0xEE (write).
                                    // read : 0b1110111[1]
                                    // write: 0b1110111[0]

  /***************************
   * Begin Oled display Settings
   **************************/
    // Display Settings
    #ifdef _I2C_OLED
      const int I2C_DISPLAY_ADDRESS = 0x3c; //it's harcoded, thus if not in datasheet must be guessed
  
      //Wire lib for ESP8266 and ESP32 allows you to specify the SDA (and SCL) pins of the I2C bus. 
      #if defined(ESP8266)
        //https://github.com/esp8266/Arduino/blob/master/variants/nodemcu/pins_arduino.h  
        const int SDA_PIN = D3; //0
        const int SDC_PIN = D4; //2
      #else
        //const int SDA_PIN = GPIO5;
        //const int SDC_PIN = GPIO4 
        
        const int SDA_PIN = GPIO0;
        const int SDC_PIN = GPIO2 
      #endif
      
      // Initialize the oled display for address 0x3c
      SSD1306Wire     display(I2C_DISPLAY_ADDRESS, SDA_PIN, SDC_PIN);
    #endif
    
    // Initialize the oled display UI
    OLEDDisplayUi   ui( &display );

    //bool triggerDisplayWakeup = true; //control display state via btn interrupt
    
/***************************
 * Begin thingspeak.com settings
 ***************************/
  //@see secret.h for credentials THINGSPEAK_API_WRITE, THINGSPEAK_API_READ
  #define THINGSPEAK_SERVER "api.thingspeak.com"
  #define THINGSPEAK_PORT 80              
  
  #define TS_WRITE_UPDATE_INTERVAL_SEC 60 //sec
                               //max Message update interval limit for home licence 1 upd/sec   
                               //total messages per year 33 million (~90k/day)

/***************************
 * Begin OpenWeatherMap settings
 ***************************/
  const boolean IS_METRIC = true;
  
  // @see secret.h for credentials
  String OPEN_WEATHER_MAP_LOCATION = "Riga,LV";

  // Pick a language:
  String OPEN_WEATHER_MAP_LANGUAGE = "en";
  const uint8_t MAX_FORECASTS = 3;

  // Adjust according to your language
  const String WDAY_NAMES[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
  const String MONTH_NAMES[] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};

  OpenWeatherMapCurrentData currentWeather;
  OpenWeatherMapCurrent currentWeatherClient;
  
  OpenWeatherMapForecastData forecasts[MAX_FORECASTS];
  OpenWeatherMapForecast forecastClient;
  
  /***************************
   * Begin ticker settings
   **************************/
    //service update interval (forecast, time, etc)
    #define FORECAST_UPDATE_INTERVAL_SECS 12*60*60 // Update every 12 hours
    #define CURR_WEATHER_UPDATE_INTERVAL_SECS 20*60 // Update every 20 minutes
    #define DISPLAY_SLEEP_INTERVAL_SECS 5*60 //put display into sleep after 5 min
    
    Ticker forecastUpdateTicker;
    Ticker weatherUpdateTicker;
    Ticker sensorsReadTicker;
    Ticker sensorsDataUploadTicker;
    Ticker displaySleepTicker;
    
    // read remote service data; flag changed in the ticker function every UPDATE_INTERVAL_SECS
    bool readyForWeatherServiceUpdate = false;
    bool readyForCurrentWeatherUpdate = false;
    
    // read sensors; flag changed in the ticker function every 5 sec
    bool readyForSensorsRead = false;
  
    // post local sensor data to remote services; 
    bool readyForSensorDataUpload = false;
   
/***************************
 * End ticker Settings
 **************************/
  time_t now;

  int tempLight = 0;
  int atmPressure = 0;
  int atmAlt = 0;         // calculate altitude, assuming 'standard' barometric
                          // pressure of 1013.25 millibar = 101325 Pascal 

  float temp = ABSOLUTE_ZERO_TEMP_C; //temperature
  uint8_t humidity = 0; //humidity
 // long readTime = 0; 
  //long uploadTime = 0; 
  int eCO2 = 0;
  int eTVOC = 0;
  
/******************************
 * Basic init
 *****************************/
  //String lastUpdate = "--";
  //long timeSinceLastWUpdate = 0;

/******************************
 * Sensor Prototypes
 *****************************/
    void uploadTemperatureHumidity();
    
    void readLight();
    void readAtmosphere();
    void readTemperatureHumidity();
    void readCCS811b();
    
 /******************************
 * End of Sensor Prototypes
 *****************************/
    void connectWifi();
    int8_t getWifiQuality();
    
/******************************
 * Timer(ticker) flag update Prototypes
 *****************************/
    void setReadyForWeatherServiceUpdate();
    void setReadyForCurrentWeatherUpdate();
    void setReadyForSensorsRead();
    void setReadyForSensorDataUpload();
    
#endif /* define __SETTINGS_H_*/
