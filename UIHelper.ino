#include "UIHelper.h"

  void toggleDisplayState(OLEDDisplay *display, OLEDDisplayUi *ui){
    if(display->isAwake()){
      ui->disableAutoTransition();
      display->displayOff();
      
      displaySleepTicker.detach();
      
    }else{
      display->displayOn();
      ui->enableAutoTransition();
      
      displaySleepTicker.attach(DISPLAY_SLEEP_INTERVAL_SECS, displayOff, display);
    }
  }

  void displayOff(OLEDDisplay *display){
    if(display->isAwake()){
      ui.disableAutoTransition();
      display->displayOff();
      displaySleepTicker.detach();
    }
  }
  
  void drawWifiConnecting(OLEDDisplay *display, int counter){
      display->clear();
      display->drawString(64, 10, "Connecting to WiFi");
      display->drawXbm(46, 30, 8, 8, counter % 3 == 0 ? activeSymbole : inactiveSymbole);
      display->drawXbm(60, 30, 8, 8, counter % 3 == 1 ? activeSymbole : inactiveSymbole);
      display->drawXbm(74, 30, 8, 8, counter % 3 == 2 ? activeSymbole : inactiveSymbole);
      display->display();
  }

  void displayBlank(OLEDDisplay *display){
    display->clear();
    display->display();
  }

  void drawBootWelcome(OLEDDisplay *display, String title) {
    display->clear();
    display->setFont(ArialMT_Plain_10);
    display->setTextAlignment(TEXT_ALIGN_CENTER);
    display->setContrast(255);
  
    display->drawXbm(-6, 5, WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits);
    display->drawString(88, 18, title);
    display->display();
  }
  
void drawProgress(OLEDDisplay *display, int percentage, String label) {
  if(!display->isAwake()){return;}
  
  display->clear();
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  display->drawString(64, 10, label);
  display->drawProgressBar(2, 28, 124, 10, percentage);
  display->display();
}

/**
 * ALT H2O LUX
 * Pressure
 * TVOC eCO2
 */
void drawIndoor(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  if(!display->isAwake()){return;}
   // PRINTF("drawIndoor %d",display->isAwake());
  
  char FormattedHumidity[10];
  char FormattedAltitude[10];
  
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  //display->drawString(/*64+*/x, 0, "Indoor Sensors" );
  //display->setFont(ArialMT_Plain_16);

  display->drawString(x, correct_y(0), " H2O " + String(humidity) + " %");
  display->drawString(64+x, correct_y(0), "LUX " + String(tempLight));

  display->drawString(x, correct_y(10), String(atmPressure) + " Pa");
  display->drawString(64+x, correct_y(10), "ALT " + String(atmAlt) + " m");
  
  display->drawString(x, correct_y(20), "TVOC " + String(eTVOC));
  display->drawString(64+x, correct_y(20), "eCO2 " + String(eCO2));
}

void drawDateTime(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  if(!display->isAwake()){return;}
  
  char time_str[11];
  /*time_t*/ now = time(nullptr);
  struct tm * timeinfo = localtime (&now);

  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  String date = ctime(&now);
  date = date.substring(0,11) + String(1900+timeinfo->tm_year);
  int textWidth = display->getStringWidth(date);
  display->drawString(64 + x, 5 + y, date);
  //display->setFont(DSEG7_Classic_Bold_21);
  display->setTextAlignment(TEXT_ALIGN_RIGHT);

  #ifdef STYLE_24HR
    sprintf(time_str, "%02d:%02d:%02d\n",timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
    display->drawString(108 + x, 19 + y, time_str);
  #else
    int hour = (timeinfo->tm_hour+11)%12+1;  // take care of noon and midnight
    sprintf(time_str, "%2d:%02d:%02d\n",hour, timeinfo->tm_min, timeinfo->tm_sec);
    display->drawString(101 + x, 19 + y, time_str);
  #endif

  /*display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  #ifdef STYLE_24HR
    sprintf(time_str, "%s", dstAbbrev);
    display->drawString(108 + x, 27 + y, time_str);  // Known bug: Cuts off 4th character of timezone abbreviation
  #else
    sprintf(time_str, "%s\n%s", dstAbbrev, timeinfo->tm_hour>=12?"pm":"am");
    display->drawString(102 + x, 18 + y, time_str);
  #endif*/
}

void drawCurrentWeather(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  if(!display->isAwake()){return;}
  //  PRINTF("drawCurrentWeather: %d",display->isAwake());
  
  display->setFont(ArialMT_Plain_10);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(64 + x, correct_y(38), currentWeather.description);

  display->setFont(ArialMT_Plain_24);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  String _temp = String(currentWeather.temp, 1) + (IS_METRIC ? "°C" : "°F");
  display->drawString(60 + x, correct_y(5), _temp);

  display->setFont(Meteocons_Plain_36);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(32 + x, correct_y(0), currentWeather.iconMeteoCon);
}


void drawForecast(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  if(!display->isAwake()){return;}
   // PRINTF(" drawForecast: %d",display->isAwake());
  
  drawForecastDetails(display, x, y, 0);
  drawForecastDetails(display, x + 44, y, 1);
  drawForecastDetails(display, x + 88, y, 2);
}

void drawForecastDetails(OLEDDisplay *display, int x, int y, int dayIndex) {
  if(!display->isAwake()){return;}
  
  time_t observationTimestamp = forecasts[dayIndex].observationTime;
  struct tm* timeInfo;
  timeInfo = localtime(&observationTimestamp);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  display->drawString(x + 20, correct_y(y), WDAY_NAMES[timeInfo->tm_wday]);

  display->setFont(Meteocons_Plain_21);
  display->drawString(x + 20, correct_y(y) + 12, forecasts[dayIndex].iconMeteoCon);
  String temp = String(forecasts[dayIndex].temp, 0) + (IS_METRIC ? "°C" : "°F");
  display->setFont(ArialMT_Plain_10);
  display->drawString(x + 20, correct_y(y) + 34, temp);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
}

void drawHeaderOverlay(OLEDDisplay *display, OLEDDisplayUiState* state) {
  if(!display->isAwake()){return;}
    //PRINTF("drawOverlay: %d",display->isAwake());
  
  char time_str[11];
  time_t now = time(nullptr);
  struct tm * timeinfo = localtime (&now);
  
  #ifdef STYLE_24HR
    sprintf(time_str, "%02d:%02d:%02d\n",timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
  #else
    int hour = (timeinfo->tm_hour+11)%12+1;  // take care of noon and midnight
    sprintf(time_str, "%2d:%02d:%02d%s\n",hour, timeinfo->tm_min, timeinfo->tm_sec, timeinfo->tm_hour>=12?"pm":"am");
  #endif

  display->setColor(WHITE);
  display->setFont(ArialMT_Plain_10);
  
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(5, OVERLAY_Y, time_str);
  
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  String _temp = String(currentWeather.temp, 1) + "/" + String(temp, 1) + (IS_METRIC ? "°C" : "°F");
  display->drawString(90, OVERLAY_Y, _temp);//101


  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  int8_t quality = getWifiQuality();
  for (int8_t i = 0; i < 4; i++) {
    for (int8_t j = 0; j < 2 * (i + 1); j++) {
      if (quality > i * 25 || j == 0) {
        #ifdef TOP_OVERLAY
          display->setPixel(120 + 2 * i, OVERLAY_HEIGHT - 4 - j);
        #else
          display->setPixel(120 + 2 * i, 61 - j);
        #endif
      }
    }
  }

  display->drawHorizontalLine(0, correct_y(0), display->getWidth());
}
