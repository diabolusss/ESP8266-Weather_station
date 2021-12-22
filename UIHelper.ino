#include "UIHelper.h"

  void drawWifiConnecting(OLEDDisplay *display, int counter){
      display->clear();
      display->flipScreenVertically();
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
    display->flipScreenVertically();
    display->setFont(ArialMT_Plain_10);
    display->setTextAlignment(TEXT_ALIGN_CENTER);
    display->setContrast(255);
  
    display->drawXbm(-6, 5, WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits);
    display->drawString(88, 18, title);
    display->display();
  }
  
void drawProgress(OLEDDisplay *display, int percentage, String label) {
  display->clear();
  //display->flipScreenVertically();
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  display->drawString(64, 10, label);
  display->drawProgressBar(2, 28, 124, 10, percentage);
  display->display();
}

void drawIndoor(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  char FormattedTemperature[10];
  char FormattedHumidity[10];
  char FormattedAltitude[10];
  
  display->flipScreenVertically();
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  //display->drawString(/*64+*/x, 0, "Indoor Sensors" );
  //display->setFont(ArialMT_Plain_16);
  
  dtostrf(temp, 6, 1, FormattedTemperature); //float_value, min_width, num_digits_after_decimal, where_to_store_string
  display->drawString(x, 8,  "T " + String(FormattedTemperature) + (IS_METRIC ? "°C": "°F"));
  
  dtostrf(humi, 6, 1, FormattedHumidity);
  display->drawString(64+x, 8, "H2O " + String(FormattedHumidity) + " %");

  display->drawString(x, 16, " " + String(atmPressure) + " Pa");
  display->drawString(64+x, 16, "Alt " + String(atmAlt) + " m");
  
  display->drawString(x, 32, "LUX " + String(tempLight));
}

void drawDateTime(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
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
  display->setFont(ArialMT_Plain_10);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(64 + x, 38 + y, currentWeather.description);

  display->setFont(ArialMT_Plain_24);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  String temp = String(currentWeather.temp, 1) + (IS_METRIC ? "°C" : "°F");
  display->drawString(60 + x, 5 + y, temp);

  display->setFont(Meteocons_Plain_36);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(32 + x, 0 + y, currentWeather.iconMeteoCon);
}


void drawForecast(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  display->flipScreenVertically();
  drawForecastDetails(display, x, y, 0);
  drawForecastDetails(display, x + 44, y, 1);
  drawForecastDetails(display, x + 88, y, 2);
}

void drawForecastDetails(OLEDDisplay *display, int x, int y, int dayIndex) {
  time_t observationTimestamp = forecasts[dayIndex].observationTime;
  struct tm* timeInfo;
  timeInfo = localtime(&observationTimestamp);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  display->drawString(x + 20, y, WDAY_NAMES[timeInfo->tm_wday]);

  display->setFont(Meteocons_Plain_21);
  display->drawString(x + 20, y + 12, forecasts[dayIndex].iconMeteoCon);
  String temp = String(forecasts[dayIndex].temp, 0) + (IS_METRIC ? "°C" : "°F");
  display->setFont(ArialMT_Plain_10);
  display->drawString(x + 20, y + 34, temp);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
}

void drawHeaderOverlay(OLEDDisplay *display, OLEDDisplayUiState* state) {
  char time_str[11];
  time_t now = /*dstAdjusted.*/time(nullptr);
  struct tm * timeinfo = localtime (&now);
  
  #ifdef STYLE_24HR
    sprintf(time_str, "%02d:%02d:%02d\n",timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
  #else
    int hour = (timeinfo->tm_hour+11)%12+1;  // take care of noon and midnight
    sprintf(time_str, "%2d:%02d:%02d%s\n",hour, timeinfo->tm_min, timeinfo->tm_sec, timeinfo->tm_hour>=12?"pm":"am");
  #endif

  display->flipScreenVertically();
  display->setColor(WHITE);
  display->setFont(ArialMT_Plain_10);
  
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(5, 52, time_str);
  
  //display->setTextAlignment(TEXT_ALIGN_RIGHT);
  
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  String temp = String(currentWeather.temp, 1) + (IS_METRIC ? "°C" : "°F");
  //display->drawString(128, 54, temp);
  display->drawString(101, 52, temp);

  int8_t quality = getWifiQuality();
  for (int8_t i = 0; i < 4; i++) {
    for (int8_t j = 0; j < 2 * (i + 1); j++) {
      if (quality > i * 25 || j == 0) {
        display->setPixel(120 + 2 * i, 61 - j);
      }
    }
  }

  display->drawHorizontalLine(0, 51, 128);
}
