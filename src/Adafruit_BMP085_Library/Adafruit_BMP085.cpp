/*!
 * @file Adafruit_BMP085.cpp
 *
 * @mainpage Adafruit BMP085 Library
 *
 * @section intro_sec Introduction
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
 * @section author Author
 *
 * Written by Limor Fried/Ladyada for Adafruit Industries.
 * Updated by Samy Kamkar for cross-platform support.
 *
 * @section license License
 *
 * BSD license, all text above must be included in any redistribution
 */

#include "Adafruit_BMP085.h"
#include <Adafruit_I2CDevice.h>

Adafruit_BMP085::Adafruit_BMP085() { i2c_dev = nullptr; }

bool Adafruit_BMP085::begin(uint8_t mode, TwoWire *wire) {
  if (mode > BMP085_ULTRAHIGHRES)
    mode = BMP085_ULTRAHIGHRES;
  oversampling = mode;

  if (i2c_dev) {
    delete i2c_dev; // remove old interface
  }

  i2c_dev = new Adafruit_I2CDevice(BMP085_I2CADDR, wire);

  if (!i2c_dev->begin()) {
    return false;
  }

  if (read8(0xD0) != 0x55)
    return false;

  /* read calibration data */
  ac1 = read16(BMP085_CAL_AC1);
  ac2 = read16(BMP085_CAL_AC2);
  ac3 = read16(BMP085_CAL_AC3);
  ac4 = read16(BMP085_CAL_AC4);
  ac5 = read16(BMP085_CAL_AC5);
  ac6 = read16(BMP085_CAL_AC6);

  b1 = read16(BMP085_CAL_B1);
  b2 = read16(BMP085_CAL_B2);

  //mb = read16(BMP085_CAL_MB); //-32768, unused
  mc = read16(BMP085_CAL_MC);
  md = read16(BMP085_CAL_MD);
  
#if (BMP085_DEBUG >= 1)
  Serial.println(" BMP >>> factory calibration data ");
  Serial.print(" ac1 = ");
  Serial.println(ac1, DEC);
  Serial.print(" ac2 = ");
  Serial.println(ac2, DEC);
  Serial.print(" ac3 = ");
  Serial.println(ac3, DEC);
  Serial.print(" ac4 = ");
  Serial.println(ac4, DEC);
  Serial.print(" ac5 = ");
  Serial.println(ac5, DEC);
  Serial.print(" ac6 = ");
  Serial.println(ac6, DEC);

  Serial.print(" b1 = ");
  Serial.println(b1, DEC);
  Serial.print(" b2 = ");
  Serial.println(b2, DEC);

  Serial.print(" mb = ");
  Serial.println(read16(BMP085_CAL_MB), DEC);
  Serial.print(" mc = ");
  Serial.println(mc, DEC);
  Serial.print(" md = ");
  Serial.println(md, DEC);
#endif

#if defined(TEMP_CALIBRATION_FIX) 
	#if !defined(AC5) || AC5 <= 0 || !defined(AC6) || AC6 <= 0 || !defined(MC) || MC == 0 || !defined(MD) || MD == 0
		#error "BMP085 >>> Disable TEMP_CALIBRATION_FIX if overriding is not needed!"
	#endif
  ac5 = AC5;
  ac6 = AC6;
  mc = MC;
  md = MD;
  	#if (BMP085_DEBUG >= 1)
	  Serial.println("Overriding temperature calibration ");
		
	  Serial.print(" ac5 = ");
	  Serial.println(ac5, DEC);
	  Serial.print(" ac6 = ");
	  Serial.println(ac6, DEC);
	  Serial.print(" mc = ");
	  Serial.println(mc, DEC);
	  Serial.print(" md = ");
	  Serial.println(md, DEC);
	#endif
#endif

#if defined(PRESSURE_CALIBRATION_FIX) 
	#if !defined(AC1) || AC1 == 0 || !defined(AC2) || AC2 == 0 || !defined(AC3) || AC3 == 0 || !defined(AC4) || AC4 <= 0 || !defined(_B1) || _B1 == 0 || !defined(_B2) || _B2 == 0 
		#error "BMP085 >>> Disable PRESSURE_CALIBRATION_FIX if overriding is not needed!"
	#endif
  ac1 = AC1;
  ac2 = AC2;
  ac3 = AC3;
  ac4 = AC4;
  b1 = _B1;
  b2 = _B2;
  	#if (BMP085_DEBUG >= 1)
	  Serial.println("Overriding pressure calibration ");
		
	  Serial.print(" ac1 = ");
	  Serial.println(ac1, DEC);
	  Serial.print(" ac2 = ");
	  Serial.println(ac2, DEC);
	  Serial.print(" ac3 = ");
	  Serial.println(ac3, DEC);
	  Serial.print(" ac4 = ");
	  Serial.println(ac4, DEC);
	  Serial.print(" b1 = ");
	  Serial.println(b1, DEC);
	  Serial.print(" b2 = ");
	  Serial.println(b2, DEC);
	#endif
#endif

  return true;
}

int32_t Adafruit_BMP085::computeB5(int32_t UT) {
	return computeB5(UT, ac5, ac6, mc, md);
}

int32_t Adafruit_BMP085::computeB5(int32_t UT, uint16_t ac5, uint16_t ac6, int16_t mc, int16_t md) {
  int32_t X1 = (UT - (int32_t)ac6) * ((int32_t)ac5) >> 15;
  int32_t X2 = ((int32_t)mc << 11) / (X1 + (int32_t)md);

#if defined(BMP085_TEST_DATASHEET) && BMP085_TEST_DATASHEET == 1
  //get_pressure
  //X1 = (UT - (int32_t)ac6) * ((int32_t)ac5) >> 15 == 4743
  //X2 = ((int32_t)mc << 11) / (X1 + (int32_t)md)   == -2344
  //B5 = X1 + X2				    == 2399
#endif

  return X1 + X2;
}

uint16_t Adafruit_BMP085::readRawTemperature(void) {
  write8(BMP085_CONTROL, BMP085_READTEMPCMD);
  delay(5);
#if BMP085_DEBUG > 1 
  Serial.print("Raw temp: ");
  Serial.println(read16(BMP085_TEMPDATA));
#endif
  return read16(BMP085_TEMPDATA);
}

uint32_t Adafruit_BMP085::readRawPressure(void) {
  uint32_t raw;

  write8(BMP085_CONTROL, BMP085_READPRESSURECMD + (oversampling << 6));

  if (oversampling == BMP085_ULTRALOWPOWER)
    delay(5);
  else if (oversampling == BMP085_STANDARD)
    delay(8);
  else if (oversampling == BMP085_HIGHRES)
    delay(14);
  else
    delay(26);

  raw = read16(BMP085_PRESSUREDATA);

  raw <<= 8;
  raw |= read8(BMP085_PRESSUREDATA + 2);
  raw >>= (8 - oversampling);

  /* this pull broke stuff, look at it later?
   if (oversampling==0) {
     raw <<= 8;
     raw |= read8(BMP085_PRESSUREDATA+2);
     raw >>= (8 - oversampling);
   }
  */

#if BMP085_DEBUG > 1
  Serial.print("Raw pressure: ");
  Serial.println(raw);
#endif
  return raw;
}

int32_t Adafruit_BMP085::readPressure(void) {
  int32_t UT, UP, B3, B5, B6, X1, X2, X3, p;
  uint32_t B4, B7;

#if defined(BMP085_TEST_DATASHEET) && BMP085_TEST_DATASHEET == 1
  // use datasheet numbers!
  UT = 27898;
  UP = 23843;
  
  ac1 = 408;
  ac2 = -72;
  ac3 = -14383;
  ac4 = 32741;
  ac5 = 32757;
  ac6 = 23153;
  
  b1 = 6190;
  b2 = 4;
  
  //mb = -32768; //not used
  mc = -8711;
  md = 2868;
  oversampling = 0;
#else
  UT = readRawTemperature();
  UP = readRawPressure();
#endif

  B5 = computeB5(UT);

  // do pressure calcs
  B6 = B5 - 4000;
  X1 = ((int32_t)b2 * ((B6 * B6) >> 12)) >> 11;
  X2 = ((int32_t)ac2 * B6) >> 11;
  X3 = X1 + X2;
  B3 = ((((int32_t)ac1 * 4 + X3) << oversampling) + 2) / 4;
  
  X1 = ((int32_t)ac3 * B6) >> 13;
  X2 = ((int32_t)b1 * ((B6 * B6) >> 12)) >> 16;
  X3 = ((X1 + X2) + 2) >> 2;
  B4 = ((uint32_t)ac4 * (uint32_t)(X3 + 32768)) >> 15;
  B7 = ((uint32_t)UP - B3) * (uint32_t)(50000UL >> oversampling);

  if (B7 < 0x80000000) {
    p = (B7 * 2) / B4;
  } else {
    p = (B7 / B4) * 2;
  }

  X1 = (p >> 8) * (p >> 8);
  X1 = (X1 * 3038) >> 16;
  X2 = (-7357 * p) >> 16;

  p = p + ((X1 + X2 + (int32_t)3791) >> 4);
  
#if defined(BMP085_TEST_DATASHEET) && BMP085_TEST_DATASHEET == 1
  //computeB5
  //X1 = (UT - (int32_t)ac6) * ((int32_t)ac5) >> 15 == 4743
  //X2 = ((int32_t)mc << 11) / (X1 + (int32_t)md)   == -2344
  //B5 = X1 + X2				    == 2399
  
  //get_pressure
  //B6 = B5 - 4000 						== -1601
  //X1 = ((int32_t)b2 * ((B6 * B6) >> 12)) >> 11 		== 1
  //X2 = ((int32_t)ac2 * B6) >> 11 				== 56
  //X3 = X1 + X2 						== 57
  //B3 = ((((int32_t)ac1 * 4 + X3) << oversampling) + 2) / 4 	== 422
  //X1 = ((int32_t)ac3 * B6) >> 13 				== 2810
  //X2 = ((int32_t)b1 * ((B6 * B6) >> 12)) >> 16		== 59
  //X3 = ((X1 + X2) + 2) >> 2					== 717
  //B4 = ((uint32_t)ac4 * (uint32_t)(X3 + 32768)) >> 15		== 33457 
  //B7 = ((uint32_t)UP - B3) * (uint32_t)(50000UL >> oversampling) == 1171050000
  //p = (B7 < 0x80000000) ? ((B7 * 2) / B4) : ((B7 / B4) * 2) 	== 70003
  //X1 = (p >> 8) * (p >> 8)					== 74529
  //X1 = (X1 * 3038) >> 16					== 3454
  //X2 = (-7357 * p) >> 16					== -7859
  //p = p + ((X1 + X2 + (int32_t)3791) >> 4)			== 69964 //Pa
  
#endif

  return p;
}

int32_t Adafruit_BMP085::readSealevelPressure(float altitude_meters) {
  float pressure = readPressure();
  return (int32_t)(pressure / pow(1.0 - altitude_meters / 44330, 5.255));
}

float Adafruit_BMP085::readTemperature(void) {
  int32_t UT, B5; // following ds convention
  float temp;

#if defined(BMP085_TEST_DATASHEET) && BMP085_TEST_DATASHEET == 1
  // use datasheet numbers!
  UT = 27898;
  ac6 = 23153;
  ac5 = 32757;
  mc = -8711;
  md = 2868;
  
#else
  UT = readRawTemperature();
#endif

  B5 = computeB5(UT);
  temp = (B5 + 8) >> 4;
  temp /= 10;
  
#if defined(BMP085_TEST_DATASHEET) && BMP085_TEST_DATASHEET == 1
  //computeB5
  //X1 = (UT - (int32_t)ac6) * ((int32_t)ac5) >> 15 == 4743
  //X2 = ((int32_t)mc << 11) / (X1 + (int32_t)md)   == -2344
  //B5 = X1 + X2				    == 2399
  //T = (B5 + 8) >> 4 				    == 150 *C (in 0.1*C)
#endif

  return temp;
}

float Adafruit_BMP085::readCustomCalibratedTemperature(uint16_t ac5, uint16_t ac6, int16_t mc, int16_t md) {
  int32_t UT = readRawTemperature();
  int32_t B5 = computeB5(UT, ac5, ac6, mc, md);
  float temp = (B5 + 8) >> 4;
  temp /= 10;

  return temp; 
}

float Adafruit_BMP085::readAltitude(float sealevelPressure) {
  float altitude;

  float pressure = readPressure();

  altitude = 44330 * (1.0 - pow(pressure / sealevelPressure, 0.1903));

  return altitude;
}

/*********************************************************************/

uint8_t Adafruit_BMP085::read8(uint8_t a) {
  uint8_t ret;

  // send 1 byte, reset i2c, read 1 byte
  i2c_dev->write_then_read(&a, 1, &ret, 1, true);

  return ret;
}

uint16_t Adafruit_BMP085::read16(uint8_t a) {
  uint8_t retbuf[2];
  uint16_t ret;

  // send 1 byte, reset i2c, read 2 bytes
  // we could typecast uint16_t as uint8_t array but would need to ensure proper
  // endianness
  i2c_dev->write_then_read(&a, 1, retbuf, 2, true);

  // write_then_read uses uint8_t array
  ret = retbuf[1] | (retbuf[0] << 8);

  return ret;
}

void Adafruit_BMP085::write8(uint8_t a, uint8_t d) {
  // send d prefixed with a (a d [stop])
  i2c_dev->write(&d, 1, true, &a, 1);
}
