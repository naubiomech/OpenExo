/**
 * @file I2CHandler.h
 * @author Chancelor Cuddeback
 * @brief This file defines the I2CHandler which is using the singleton pattern. To use you must first get a referance to the singleton instance. 
 * Do this with the line 'I2C* instance = I2C::get_instance();'. Then you can read with 'instance->read_i2c(...)'
 * @date 2023-07-18
 * 
 */

#ifndef I2CHANDLER_H
#define I2CHANDLER_H

#include <Arduino.h>

#if defined(ARDUINO_ARDUINO_NANO33BLE)

#include <Wire.h>

class I2C
{
    public:
        static I2C* get_instance()
        {
            static I2C* instance = new I2C();
            return instance;
        }

        void read_i2c(uint8_t* ret, uint8_t addr, uint8_t reg, uint8_t len)
        {
            // logger::print("Reading from I2C device: ");
            // logger::print(addr);
            // logger::print(" at register: ");
            // logger::print(reg);
            // logger::print(" with length: ");
            // logger::println(len);

            Wire.beginTransmission(addr);
            Wire.write(reg);
            Wire.endTransmission();
            Wire.requestFrom(addr, 2, false);
            for (uint8_t i=0; i<len; i++)
            {
                ret[i] = Wire.read();
            }
            Wire.endTransmission();
        }

        void write_i2c(uint8_t addr, uint8_t reg, uint8_t val)
        {
            Wire.beginTransmission(addr);
            Wire.write(reg);
            Wire.write(val);
            Wire.endTransmission();
        }

    private:
        I2C()
        {
          Wire.begin();
        }
};

#endif


namespace i2c_cmds
{
    // namespace smart
    // {
        // namespace get_battery_voltage
        // {
            // const uint8_t addr = 0x40;
            // const uint8_t reg = 0x02;
            // const uint8_t len = 2;
        // }
        // namespace get_battery_soc
        // {
            // const uint8_t addr = 0x40;
            // const uint8_t reg = 0x0e;
            // const uint8_t len = 2;
        // }
    // }

    // namespace rc
    // {
        // namespace calibrate
        // {
            // const uint8_t addr = 0x40;
            // const uint8_t reg = 0x05;
            // const uint16_t val = 0x5000;
        // }
        // namespace get_battery_voltage
        // {
            // const uint8_t addr = 0x40;
            // const uint8_t reg = 0x02;
            // const uint8_t len = 2;
        // }
    // }

    namespace thigh_imu
    {
        const uint8_t left_addr = 0x01;
        const uint8_t right_addr = 0x02;

        namespace handshake
        {
            const uint8_t reg = 0x01;
            const uint8_t len = 1;
        }
        namespace get_angle
        {
            const uint8_t reg = 0x02;
            const uint8_t len = 1;
        }
    }

    namespace ankle_angles
    {
        const uint8_t addr = 0x04; //Confirm that peripheral MCU has the same address
        namespace handshake
        {
            const uint8_t reg = 0x01;
            const uint8_t len = 1;
        }
        namespace get_left_angle
        {
            const uint8_t reg = 0x02;
            const uint8_t len = 2;
        }
        namespace get_right_angle
        {
            const uint8_t reg = 0x03;
            const uint8_t len = 2;
        }
    }
}

// int INA219_ADR = 0x40;           //Address of INA219 for writing defined in 7 bits. The 8th bit is automatically included by Wire.read() or Wire.write()
// int INA219_CONFIG = 0x00;        //All-register reset, bus voltage range, PGA gain, ADC resolution/averaging. Typically does not need modification
// int INA219_SHUNT = 0x01;         //Shunt voltage measurement - use this to get the shunt resistor voltage
// int INA219_BUS = 0x02;           //Bus voltage measurement - use this to get the battery voltage relative to ground
// int INA219_PWR = 0x03;           //Power measurement - use this to get calibrated power measurements
// int INA219_CUR = 0x04;           //Current measurement - use this to get the current flowing through the shunt
// int INA219_CAL = 0x05;           //Set full scale range and LSB of current/power measurements. Needed for power and current measurements
// int CurrentLSB = 1;              //mA/bit. This value is used to multiply the current reading from the INA219 to obtain actual current in mA
// int PowerLSB = 20 * CurrentLSB;  //mW/bit. This value is used to multiply to power reading from the INA219 to obtain actual power in mW
// int ShuntLSB = 0.01;             //mV. This is the default multiplier for the shunt voltage reading from the INA219.
// int BusLSB = 4;                  //mV. This is the multiplier for the bus (battery) voltage reading from the INA219.
// int Cal = 0x5000;                //Calibration value in hex. Cal = 0.04096/(CurrentLSB*ShuntResistance). Shunt resistance on Rev3/4 is 2mOhm.

#endif