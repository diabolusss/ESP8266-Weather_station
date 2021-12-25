#ifndef __UIHELPER_H_
#define __UIHELPER_H_

#include "settings.h"

#define TOP_OVERLAY
#define OVERLAY_HEIGHT 14
#ifdef TOP_OVERLAY
  #define OVERLAY_Y 0
  #define correct_y(Y) (OVERLAY_HEIGHT + (Y) )
#else
  #define OVERLAY_Y 52
  #define correct_y(Y) (Y)
#endif

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
    uint8_t numberOfFrames = 3;
    
    OverlayCallback overlays[] = { drawHeaderOverlay };
    uint8_t numberOfOverlays = 1;   

    bool isDisplayOn = true; // we start with enabled display
/******************************
 * End of Basic init
 *****************************/
 
#endif /*define __UIHELPER_H_*/
