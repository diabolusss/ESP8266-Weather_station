/*!
 * @file Adafruit_BMP085.h
 *
 * This is a library for the Adafruit BMP085/BMP180 Barometric Pressure + Temp
 * sensor
 *
 * Designed specifically to work with the Adafruit BMP085 or BMP180 Breakout
 * ----> http://www.adafruit.com/products/391
 * ----> http://www.adafruit.com/products/1603
 *
 * These displays use I2C to communicate, 2 pins are required to
 * interface
 * Adafruit invests time and resources providing this open source code,
 * please support Adafruit and open-source hardware by purchasing
 * products from Adafruit!
 *
 * Written by Limor Fried/Ladyada for Adafruit Industries.
 * BSD license, all text above must be included in any redistribution
 */

#ifndef ADAFRUIT_BMP085_H
#define ADAFRUIT_BMP085_H

#include <Adafruit_I2CDevice.h>
#include <Arduino.h>

/**
  * Pseudo calibration mechanism based on datasheet values and excel solver
  * Useful in case module has faulty calibration or it was overheated while soldering
  *
  * @see datasheet p.26 "The soldering process can lead to an offset shift."
 **/
//#define TEMP_CALIBRATION_FIX
#ifdef TEMP_CALIBRATION_FIX //could be used to make offset from factory calibration
			    // i.e. if factory calibration have significant error ( > *1C)
			    // find custom values to fix that
	//create -5*C offset in diapazon [-10:28] compared to factory calibration readings
		/*#define AC5 	24997
		#define AC6 	19875
		#define MC 	-10452
		#define MD 	2699*/
		
	//~-7*C offset	
		#define AC5 	24140
		#define AC6 	19911
		#define MC 	-10237
		#define MD 	2446
#endif

//#define PRESSURE_CALIBRATION_FIX
#ifdef PRESSURE_CALIBRATION_FIX //if TEMP_CALIBRATION_FIX is used, then to correct 
				// pressure calculations we need to provide new calibration values
	//for ~-7*C Temp fix we need to fix resulting offset (~2.2kPa)
		//~150Pa err <- good
		/*#define AC1 	7580
		#define AC2 	-1180
		#define AC3 	-9179
		#define AC4 	33238
		#define _B1 	6199
		#define _B2 	45*/

		//~170Pa err
		/*#define AC1 	7713
		#define AC2 	-974
		#define AC3 	-12933
		#define AC4 	32985
		#define _B1 	6242
		#define _B2 	-483*/
		
		//< 150 Pa error at ~15*C
		#define AC1 	7581
		#define AC2 	-1088
		#define AC3 	-9297
		#define AC4 	33263
		#define _B1 	7441
		#define _B2 	-14

#endif

#define BMP085_DEBUG 0 //!< Debug mode

#define BMP085_I2CADDR 0x77 //!< BMP085 I2C address

#define BMP085_ULTRALOWPOWER 0 //!< Ultra low power mode
#define BMP085_STANDARD 1      //!< Standard mode
#define BMP085_HIGHRES 2       //!< High-res mode
#define BMP085_ULTRAHIGHRES 3  //!< Ultra high-res mode
#define BMP085_CAL_AC1 0xAA    //!< R   Calibration data (16 bits)
#define BMP085_CAL_AC2 0xAC    //!< R   Calibration data (16 bits)
#define BMP085_CAL_AC3 0xAE    //!< R   Calibration data (16 bits)
#define BMP085_CAL_AC4 0xB0    //!< R   Calibration data (16 bits)
#define BMP085_CAL_AC5 0xB2    //!< R   Calibration data (16 bits)
#define BMP085_CAL_AC6 0xB4    //!< R   Calibration data (16 bits)
#define BMP085_CAL_B1 0xB6     //!< R   Calibration data (16 bits)
#define BMP085_CAL_B2 0xB8     //!< R   Calibration data (16 bits)
#define BMP085_CAL_MB 0xBA     //!< R   Calibration data (16 bits)
#define BMP085_CAL_MC 0xBC     //!< R   Calibration data (16 bits)
#define BMP085_CAL_MD 0xBE     //!< R   Calibration data (16 bits)

#define BMP085_CONTROL 0xF4         //!< Control register
#define BMP085_TEMPDATA 0xF6        //!< Temperature data register
#define BMP085_PRESSUREDATA 0xF6    //!< Pressure data register
#define BMP085_READTEMPCMD 0x2E     //!< Read temperature control register value
#define BMP085_READPRESSURECMD 0x34 //!< Read pressure control register value

/*!
 * @brief Main BMP085 class
 */
class Adafruit_BMP085 {
public:
  Adafruit_BMP085();
  /*!
   * @brief Starts I2C connection
   * @param mode Mode to set, ultra high-res by default
   * @param wire The I2C interface to use, defaults to Wire
   * @return Returns true if successful
   */
  bool begin(uint8_t mode = BMP085_ULTRAHIGHRES, TwoWire *wire = &Wire);
  /*!
   * @brief Gets the temperature over I2C from the BMP085
   * @return Returns the temperature
   */
  float readTemperature(void);
  
  /*!
   * @brief Gets the temperature over I2C from the BMP
   * @return Returns calculated temperature using custom calibration params
   */
  float readCustomCalibratedTemperature(uint16_t ac5, uint16_t ac6, int16_t mc, int16_t md);
  
  /*!
   * @brief Gets the pressure over I2C from the BMP085
   * @return Returns the pressure
   */
  int32_t readPressure(void);
  /*!
   * @brief Calculates the pressure at sea level
   * @param altitude_meters Current altitude (in meters)
   * @return Returns the calculated pressure at sea level
   */
  int32_t readSealevelPressure(float altitude_meters = 0);
  /*!
   * @brief Reads the altitude
   * @param sealevelPressure Pressure at sea level, measured in pascals
   * @return Returns the altitude
   */
  float readAltitude(float sealevelPressure = 101325); // std atmosphere
  /*!
   * @brief Reads the raw temperature
   * @return Returns signed 16-bit integer of the raw temperature
   */
  uint16_t readRawTemperature(void);
  /*!
   * @brief Reads the raw pressure
   * @return Returns signed 32-bit integer of the raw temperature
   */
  uint32_t readRawPressure(void);
  
  int16_t getAC1(){
  	return ac1;
  }
  int16_t getAC2(){
  	return ac2;
  }
  int16_t getAC3(){
  	return ac3;
  }
  uint16_t getAC4(){
  	return ac4;
  }
  uint16_t getAC5(){
  	return ac5;
  }
  uint16_t getAC6(){
  	return ac6;
  }
  
  int16_t getMC(){
  	return mc;
  }
  int16_t getMD(){
  	return md;
  }
  int16_t getB1(){
  	return b1;
  }
  int16_t getB2(){
  	return b2;
  }
  
private:
  int32_t computeB5(int32_t UT);
  int32_t computeB5(int32_t UT, uint16_t ac5, uint16_t ac6, int16_t mc, int16_t md);
  
  uint8_t read8(uint8_t addr);
  uint16_t read16(uint8_t addr);
  void write8(uint8_t addr, uint8_t data);
  
  Adafruit_I2CDevice *i2c_dev;
  uint8_t oversampling;

  int16_t 
  	ac1, ac2, ac3, b1, b2, //pressure calibration (assuming at 15*C)
  	//mb, //not used
  	mc, md  //Temperature calibration
  	;
  uint16_t 
  	ac4,  //pressure calibration
  	ac5, ac6  //Temperature calibration
  	;
};

#endif //  ADAFRUIT_BMP085_H
