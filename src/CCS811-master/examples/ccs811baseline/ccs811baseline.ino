#include <Wire.h>
#include "ccs811.h" // CCS811 library - https://github.com/maarten-pennings/CCS811
#include <EEPROM.h>

/*
  CCS811 -- ESP32
  VCC - 3.3V
  GND - G
  SCL - D1 -- GPIO 19
  SDA - D2 -- GPIO 18
  WAK - D3 -- GPIO 23
*/

// CCS811&HDC1080 config
#ifdef ARDUINO_ARCH_ESP32
#define CCS811_SDA 18
#define CCS811_SCL 19
#define CCS811_WAK 23
CCS811 ccs811(CCS811_WAK); // nWAKE on IO23
#endif

// define the number of bytes you want to access
#define EEPROM_SIZE 4

uint16_t currenteCO2, currenteTVOC, errstat, raw;

void setup()
{
  Serial.begin(115200);

  // initialize EEPROM with predefined size
  EEPROM.begin(EEPROM_SIZE);

  Serial.println();
  Serial.println("CCS811 Baseline Example");

  // CCS811 setup - START
  Wire.begin(CCS811_SDA, CCS811_SCL);
  Serial.println("CCS811 test");
  // Enable CCS811
  ccs811.set_i2cdelay(50); // Needed for ESP8266 because it doesn't handle I2C clock stretch correctly
  bool ok = ccs811.begin();
  if ( !ok ) Serial.println("setup: CCS811 begin FAILED");

  // Print CCS811 versions
  Serial.print("setup: hardware    version: "); Serial.println(ccs811.hardware_version(), HEX);
  Serial.print("setup: bootloader  version: "); Serial.println(ccs811.bootloader_version(), HEX);
  Serial.print("setup: application version: "); Serial.println(ccs811.application_version(), HEX);

  // Start measuring
  ok = ccs811.start(CCS811_MODE_1SEC);
  if ( !ok ) Serial.println("init: CCS811 start FAILED");
  Serial.println("\n");
  // CCS811 setup - END

  //Wire.begin();
  Wire.begin(CCS811_SDA, CCS811_SCL);

  //This looks for previously saved data in the eeprom at program start
  if ((EEPROM.read(0) == 0xA5) && (EEPROM.read(1) == 0xB2))
  {
    Serial.println("EEPROM contains saved data.");
  }
  else
  {
    Serial.println("Saved data not found!");
  }
  Serial.println();

  Serial.println("Program running.  Send the following characters to operate:");
  Serial.println(" 's' - save baseline into EEPROM");
  Serial.println(" 'l' - load and apply baseline from EEPROM");
  Serial.println(" 'c' - clear baseline from EEPROM");
  Serial.println(" 'r' - read and print sensor data");

}

void loop()
{
  char c;
  unsigned int result;
  unsigned int baselineToApply;
  if (Serial.available())
  {
    c = Serial.read();
    switch (c)
    {
      case 's':
        Serial.println("saving baseline...");
        //This gets the latest baseline from the sensor
        result = ccs811.get_baseline();
        Serial.print("baseline for this sensor: 0x");
        if (result < 0x100) Serial.print("0");
        if (result < 0x10) Serial.print("0");
        Serial.println(result, HEX);
        //The baseline is saved (with valid data indicator bytes)
        EEPROM.write(0, 0xA5);
        EEPROM.write(1, 0xB2);
        EEPROM.write(2, (result >> 8) & 0x00FF);
        EEPROM.write(3, result & 0x00FF);
        EEPROM.commit();

        Serial.println(EEPROM.read(0));
        Serial.println(EEPROM.read(1));
        Serial.println(EEPROM.read(2));
        Serial.println(EEPROM.read(3));

        break;
      case 'l':
        Serial.println("loading baseline...");

        Serial.println(EEPROM.read(0));
        Serial.println(EEPROM.read(1));
        Serial.println(EEPROM.read(2));
        Serial.println(EEPROM.read(3));

        if ((EEPROM.read(0) == 0xA5) && (EEPROM.read(1) == 0xB2)) {
          Serial.println("EEPROM contains saved data.");
          //The recovered baseline is packed into a 16 bit word
          baselineToApply = ((unsigned int)EEPROM.read(2) << 8) | EEPROM.read(3);
          Serial.print("Saved baseline: 0x");
          if (baselineToApply < 0x100) Serial.print("0");
          if (baselineToApply < 0x10) Serial.print("0");
          Serial.println(baselineToApply, HEX);
          ccs811.set_baseline( baselineToApply );
        } else {
          Serial.println("Saved data not found!");
        }
        break;
      case 'c':
        //Clear data indicator and data from the eeprom
        Serial.println("Clearing EEPROM space.");
        EEPROM.write(0, 0x00);
        EEPROM.write(1, 0x00);
        EEPROM.write(2, 0x00);
        EEPROM.write(3, 0x00);
        EEPROM.commit();
        break;
      case 'r':
        //Simply print the last sensor data
        ccs811.read(&currenteCO2, &currenteTVOC, &errstat, &raw);
        if ( errstat == CCS811_ERRSTAT_OK ) {
          Serial.print("CO2[");
          Serial.print(currenteCO2);
          Serial.print("] tVOC[");
          Serial.print(currenteTVOC);
          Serial.print("]");
          Serial.println();
        }
        else
        {
          Serial.println("Sensor data not available.");
        }
        break;
      default:
        break;
    }
    delay(10);
  }
}
