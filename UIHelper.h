#ifndef __UIHELPER_H_
#define __UIHELPER_H_

#include "settings.h"

/******************************
 * UI Prototypes
 *****************************/
 
  void displayBlank(OLEDDisplay *display);
  
  void drawBootWelcome(OLEDDisplay *display, String title);
  void drawProgress(OLEDDisplay *display, int percentage, String label);
  void drawWifiConnecting(OLEDDisplay *display, int counter);
  void drawDateTime(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
  
  void drawIndoor(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
  void drawCurrentWeather(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
  void drawForecast(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
  void drawForecastDetails(OLEDDisplay *display, int x, int y, int dayIndex);
  void drawHeaderOverlay(OLEDDisplay *display, OLEDDisplayUiState* state);
/******************************
 * End of UI Prototypes
 *****************************/

 /******************************
 * Basic init
 *****************************/
    // Add frames
    // this array keeps function pointers to all frames
    // frames are the single views that slide from right to left
    FrameCallback frames[] = { drawIndoor, drawCurrentWeather, drawForecast };
    int numberOfFrames = 3;
    
    OverlayCallback overlays[] = { drawHeaderOverlay };
    int numberOfOverlays = 1;   
/******************************
 * End of Basic init
 *****************************/
 
#endif /*define __UIHELPER_H_*/
