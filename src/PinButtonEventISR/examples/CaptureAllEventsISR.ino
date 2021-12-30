/*
   Capture debounced button events 
   This example code is in the public domain.
*/

#define DEBUG
#include <PinButtonEventISR.h>

PinButtonEventISR D7Button(D7); //if you are getting WDT reset, change pin notation, i.e. from raw number into Dn

void setup() {
  // initialize serial and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
}

void loop() {
  Serial.print(".");
  
  if(D7Button.hasEvent()){
    Serial.print("event ready: ");
    
    if(D7Button.isLongPress()){
      Serial.println("LONG PRESS");
      
    }else if(D7Button.isPress()){
      Serial.println("PRESS");
      
    }else if(D7Button.isClick()){
      if(D7Button.isDoubleClick()){
        Serial.println("DBL CLICK");
        
      }else if(D7Button.isSingleClick()){
        Serial.println("SNGL CLICK");
      }
      
    }
    
  }

  delay(500);
}
