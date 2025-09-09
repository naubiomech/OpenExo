/*!
 *  @file Adafruit_INA260.cpp
 *
 *  @mainpage Adafruit INA260 I2C Current and Power sensor
 *
 *  @section intro_sec Introduction
 *
 * 	I2C Driver for the INA260 I2C Current and Power sensor
 *
 * 	This is a library for the Adafruit INA260 breakout:
 * 	http://www.adafruit.com/products/4226
 *
 * 	Adafruit invests time and resources providing this open source code,
 *  please support Adafruit and open-source hardware by purchasing products from
 * 	Adafruit!
 *
 *  @section dependencies Dependencies
 *
 *  This library depends on the Adafruit BusIO library
 *
 *  @section author Author
 *
 *  Bryan Siepert for Adafruit Industries
 *
 * 	@section license License
 *
 * 	BSD (see license.txt)
 *
 * 	@section  HISTORY
 *
 *     v1.0 - First release
 */

#include "Arduino.h"
#include <Wire.h>

#include "Adafruit_INA260.h"

/*!
 *    @brief  Instantiates a new INA260 class
 */
Adafruit_INA260::Adafruit_INA260(void) {}

/*!
 *    @brief  Sets up the HW
 *    @param  i2c_address
 *            The I2C address to be used.
 *    @param  theWire
 *            The Wire object to be used for I2C connections.
 *    @return True if initialization was successful, otherwise false.
 */
bool Adafruit_INA260::begin(uint8_t i2c_address, TwoWire *theWire) {
  i2c_dev = new Adafruit_I2CDevice(i2c_address, theWire);

  if (!i2c_dev->begin()) {
    return false;
  }

  Adafruit_I2CRegister *die_register =
      new Adafruit_I2CRegister(i2c_dev, INA260_REG_DIE_UID, 2, MSBFIRST);
  Adafruit_I2CRegister *mfg_register =
      new Adafruit_I2CRegister(i2c_dev, INA260_REG_MFG_UID, 2, MSBFIRST);
  Adafruit_I2CRegisterBits *device_id =
      new Adafruit_I2CRegisterBits(die_register, 12, 4);

  // make sure we're talking to the right chip
  if ((mfg_register->read() != 0x5449) || (device_id->read() != 0x227)) {
    return false;
  }

  Config = new Adafruit_I2CRegister(i2c_dev, INA260_REG_CONFIG, 2, MSBFIRST);
  MaskEnable =
      new Adafruit_I2CRegister(i2c_dev, INA260_REG_MASK_ENABLE, 2, MSBFIRST);
  AlertLimit =
      new Adafruit_I2CRegister(i2c_dev, INA260_REG_ALERT_LIMIT, 2, MSBFIRST);

  reset();
  delay(2); // delay 2ms to give time for first measurement to finish
  return true;
}
/**************************************************************************/
/*!
    @brief Resets the harware. All registers are set to default values,
    the same as a power-on reset.
*/
/**************************************************************************/
void Adafruit_INA260::reset(void) {
  Adafruit_I2CRegisterBits reset = Adafruit_I2CRegisterBits(Config, 1, 15);
  reset.write(1);
}
/**************************************************************************/
/*!
    @brief Reads and scales the current value of the Current register.
    @return The current current measurement in mA
*/
/**************************************************************************/
float Adafruit_INA260::readCurrent(void) {
  Adafruit_I2CRegister current =
      Adafruit_I2CRegister(i2c_dev, INA260_REG_CURRENT, 2, MSBFIRST);
  return (int16_t)current.read() * 1.25;
}
/**************************************************************************/
/*!
    @brief Reads and scales the current value of the Bus Voltage register.
    @return The current bus voltage measurement in mV
*/
/**************************************************************************/
float Adafruit_INA260::readBusVoltage(void) {
  Adafruit_I2CRegister bus_voltage =
      Adafruit_I2CRegister(i2c_dev, INA260_REG_BUSVOLTAGE, 2, MSBFIRST);
  return bus_voltage.read() * 1.25;
}
/**************************************************************************/
/*!
    @brief Reads and scales the current value of the Power register.
    @return The current Power calculation in mW
*/
/**************************************************************************/
float Adafruit_INA260::readPower(void) {
  Adafruit_I2CRegister power =
      Adafruit_I2CRegister(i2c_dev, INA260_REG_POWER, 2, MSBFIRST);
  return power.read() * 10;
}
/**************************************************************************/
/*!
    @brief Returns the current measurement mode
    @return The current mode
*/
/**************************************************************************/
INA260_MeasurementMode Adafruit_INA260::getMode(void) {
  Adafruit_I2CRegisterBits mode = Adafruit_I2CRegisterBits(Config, 3, 0);
  return (INA260_MeasurementMode)mode.read();
}
/**************************************************************************/
/*!
    @brief Sets a new measurement mode
    @param new_mode
           The new mode to be set
*/
/**************************************************************************/
void Adafruit_INA260::setMode(INA260_MeasurementMode new_mode) {
  Adafruit_I2CRegisterBits mode = Adafruit_I2CRegisterBits(Config, 3, 0);
  mode.write(new_mode);
}
/**************************************************************************/
/*!
    @brief Reads the current number of averaging samples
    @return The current number of averaging samples
*/
/**************************************************************************/
INA260_AveragingCount Adafruit_INA260::getAveragingCount(void) {
  Adafruit_I2CRegisterBits averaging_count =
      Adafruit_I2CRegisterBits(Config, 3, 9);
  return (INA260_AveragingCount)averaging_count.read();
}
/**************************************************************************/
/*!
    @brief Sets the number of averaging samples
    @param count
           The number of samples to be averaged
*/
/**************************************************************************/
void Adafruit_INA260::setAveragingCount(INA260_AveragingCount count) {
  Adafruit_I2CRegisterBits averaging_count =
      Adafruit_I2CRegisterBits(Config, 3, 9);
  averaging_count.write(count);
}
/**************************************************************************/
/*!
    @brief Reads the current current conversion time
    @return The current current conversion time
*/
/**************************************************************************/
INA260_ConversionTime Adafruit_INA260::getCurrentConversionTime(void) {
  Adafruit_I2CRegisterBits current_conversion_time =
      Adafruit_I2CRegisterBits(Config, 3, 3);
  return (INA260_ConversionTime)current_conversion_time.read();
}
/**************************************************************************/
/*!
    @brief Sets the current conversion time
    @param time
           The new current conversion time
*/
/**************************************************************************/
void Adafruit_INA260::setCurrentConversionTime(INA260_ConversionTime time) {
  Adafruit_I2CRegisterBits current_conversion_time =
      Adafruit_I2CRegisterBits(Config, 3, 3);
  current_conversion_time.write(time);
}
/**************************************************************************/
/*!
    @brief Reads the current bus voltage conversion time
    @return The current bus voltage conversion time
*/
/**************************************************************************/
INA260_ConversionTime Adafruit_INA260::getVoltageConversionTime(void) {
  Adafruit_I2CRegisterBits voltage_conversion_time =
      Adafruit_I2CRegisterBits(Config, 3, 6);
  return (INA260_ConversionTime)voltage_conversion_time.read();
}
/**************************************************************************/
/*!
    @brief Sets the bus voltage conversion time
    @param time
           The new bus voltage conversion time
*/
/**************************************************************************/
void Adafruit_INA260::setVoltageConversionTime(INA260_ConversionTime time) {
  Adafruit_I2CRegisterBits voltage_conversion_time =
      Adafruit_I2CRegisterBits(Config, 3, 6);
  voltage_conversion_time.write(time);
}

/**************************************************************************/
/*!
    @brief Checks if the most recent one shot measurement has completed
    @return true if the conversion has completed
*/
/**************************************************************************/
bool Adafruit_INA260::conversionReady(void) {
  Adafruit_I2CRegisterBits conversion_ready =
      Adafruit_I2CRegisterBits(MaskEnable, 1, 3);
  return conversion_ready.read();
}
/**************************************************************************/
/*!
    @brief Reads the current parameter that asserts the ALERT pin
    @return The current parameter that asserts the ALERT PIN
*/
/**************************************************************************/
INA260_AlertType Adafruit_INA260::getAlertType(void) {
  Adafruit_I2CRegisterBits alert_type =
      Adafruit_I2CRegisterBits(MaskEnable, 6, 10);
  return (INA260_AlertType)alert_type.read();
}
/**************************************************************************/
/*!
    @brief Sets which parameter asserts the ALERT pin
    @param alert
           The parameter which asserts the ALERT pin
*/
/**************************************************************************/
void Adafruit_INA260::setAlertType(INA260_AlertType alert) {
  Adafruit_I2CRegisterBits alert_type =
      Adafruit_I2CRegisterBits(MaskEnable, 6, 10);
  alert_type.write(alert);
}
/**************************************************************************/
/*!
    @brief Reads the current alert limit setting
    @return The current bus alert limit setting
*/
/**************************************************************************/
float Adafruit_INA260::getAlertLimit(void) {
  Adafruit_I2CRegisterBits alert_limit =
      Adafruit_I2CRegisterBits(AlertLimit, 16, 0);
  return (float)alert_limit.read() * 1.25;
}
/**************************************************************************/
/*!
    @brief Sets the Alert Limit
    @param limit
           The new limit that triggers the alert
*/
/**************************************************************************/
void Adafruit_INA260::setAlertLimit(float limit) {
  Adafruit_I2CRegisterBits alert_limit =
      Adafruit_I2CRegisterBits(AlertLimit, 16, 0);
  alert_limit.write((int16_t)(limit / 1.25));
}
/**************************************************************************/
/*!
    @brief Reads the current alert polarity setting
    @return The current bus alert polarity setting
*/
/**************************************************************************/
INA260_AlertPolarity Adafruit_INA260::getAlertPolarity(void) {
  Adafruit_I2CRegisterBits alert_polarity =
      Adafruit_I2CRegisterBits(MaskEnable, 1, 1);
  return (INA260_AlertPolarity)alert_polarity.read();
}
/**************************************************************************/
/*!
    @brief Sets Alert Polarity Bit
    @param polarity
           The polarity of the alert pin
*/
/**************************************************************************/
void Adafruit_INA260::setAlertPolarity(INA260_AlertPolarity polarity) {
  Adafruit_I2CRegisterBits alert_polarity =
      Adafruit_I2CRegisterBits(MaskEnable, 1, 1);
  alert_polarity.write(polarity);
}
/**************************************************************************/
/*!
    @brief Reads the current alert latch setting
    @return The current bus alert latch setting
*/
/**************************************************************************/
INA260_AlertLatch Adafruit_INA260::getAlertLatch(void) {
  Adafruit_I2CRegisterBits alert_latch =
      Adafruit_I2CRegisterBits(MaskEnable, 1, 0);
  return (INA260_AlertLatch)alert_latch.read();
}
/**************************************************************************/
/*!
    @brief Sets Alert Latch Bit
    @param state
           The parameter which asserts the ALERT pin
*/
/**************************************************************************/
void Adafruit_INA260::setAlertLatch(INA260_AlertLatch state) {
  Adafruit_I2CRegisterBits alert_latch =
      Adafruit_I2CRegisterBits(MaskEnable, 1, 0);
  alert_latch.write(state);
}
/**************************************************************************/
/*!
    @brief Checks if the Alert Flag is set
    @return true if the flag is set
*/
/**************************************************************************/
bool Adafruit_INA260::alertFunctionFlag(void) {
  Adafruit_I2CRegisterBits alert_function_flag =
      Adafruit_I2CRegisterBits(MaskEnable, 1, 4);
  return alert_function_flag.read();
}