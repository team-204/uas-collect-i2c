#include <chrono>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdint.h>
#include <stdexcept>
#include <string>
#include <string.h>
#include <thread>
#include <vector>

#include "mpl3115a2.hpp"


// Register addresses
constexpr uint8_t MPL3115A2_ADDRESS = 0x60;
constexpr uint8_t STATUS = 0x00;
constexpr uint8_t DATA_READY = 0x06;
constexpr uint8_t WHO_AM_I = 0x0C;
constexpr uint8_t PT_DATA_CFG = 0x13;
constexpr uint8_t CTRL_REG1 = 0x26;
constexpr uint8_t PRESSURE_MSB = 0x01;
constexpr uint8_t PRESSURE_CSB = 0x02;
constexpr uint8_t PRESSURE_LSB = 0x03;
constexpr uint8_t TEMPERATURE_MSB = 0x04;
constexpr uint8_t TEMPERATURE_LSB = 0x05;

// Register defaults
constexpr uint8_t DEVICE_ID = 0xC4;

// Register masks
constexpr uint8_t STANDBY_BAR_MASK = 0x01;  // Standby bit of ctrl register 1
constexpr uint8_t ALTIMETER_MASK = 0x80;  // If bit is set device is measuring altitude otherwise pressure
constexpr uint8_t PT_DATA_CFG_DREM_MASK = 0x04;
constexpr uint8_t PT_DATA_CFG_TDEFE_MASK  = 0x01;
constexpr uint8_t PT_DATA_CFG_PDEFE_MASK  = 0x02;
constexpr uint8_t STATUS_TDR_MASK = 0x02;
constexpr uint8_t STATUS_PDR_MASK =  0x04;
constexpr uint8_t STATUS_PTDR_MASK = 0x08;


MPL3115A2::MPL3115A2(const unsigned int adapterNumber) :
    m_connection(new I2cAbstraction(adapterNumber, MPL3115A2_ADDRESS))
{
    // Confirm that the device at this address is indeed the MPL3115A2
    uint8_t whoIsThis = m_connection->readBytes(WHO_AM_I, 1)[0];
    if (whoIsThis != DEVICE_ID)
    {
        std::ostringstream err;
        err << "WHO_AM_I register contained: " << std::hex
            << whoIsThis << std::endl
            << "Expected: " << std::hex << DEVICE_ID << std::endl;
        throw std::runtime_error(err.str());
    }
    else
    {
        std::cout << "MPL3115A2 confirmed to be on I2C bus adapter "
                  << adapterNumber << std::endl;
    }

    configureDataReadyFlag();
    configureAltimeterMode();
}


uint8_t MPL3115A2::enterStandbyMode(void) const
{
    // Enter standby mode to allow register writing
    // Return the register data to allow it to be restored to previous state
    uint8_t controlRegisterData = m_connection->readBytes(CTRL_REG1, 1)[0];
    m_connection->writeByte(CTRL_REG1, controlRegisterData & ~STANDBY_BAR_MASK);  // Clear standby bar bit
    return controlRegisterData;
}


void MPL3115A2::configureAltimeterMode(void)
{
    // Clear the standby-bar bit to activate standby mode
    uint8_t controlRegisterData = enterStandbyMode();

    // Set the mode to altimeter and set the standby-bar bit to deactivate standby mode
    m_connection->writeByte(CTRL_REG1, controlRegisterData | ALTIMETER_MASK | STANDBY_BAR_MASK);
    isAltimeterMode = true;
    isBarometerMode = false;
}


void MPL3115A2::configureBarometerMode(void)
{
    // Clear the standby-bar bit to activate standby mode
    uint8_t controlRegisterData = enterStandbyMode();

    // Set the mode to barometer and set the standby-bar bit to deactivate standby mode
    m_connection->writeByte(CTRL_REG1, (controlRegisterData & ~ALTIMETER_MASK) | STANDBY_BAR_MASK);
    isAltimeterMode = false;
    isBarometerMode = true;
}


void MPL3115A2::configureDataReadyFlag(void) const
{
    // Clear the standby-bar bit to activate standby mode
    // Entering standby mode maybe unnecessary to change this register but we'll do it anyways
    uint8_t controlRegisterData = enterStandbyMode();

    // Configure the sensor data register to raise a status flag when any new data is available
    m_connection->writeByte(PT_DATA_CFG, PT_DATA_CFG_DREM_MASK | PT_DATA_CFG_TDEFE_MASK | PT_DATA_CFG_PDEFE_MASK);

    // Set device back to active mode
    m_connection->writeByte(CTRL_REG1, controlRegisterData | STANDBY_BAR_MASK);
}


MPL3115A2DATA MPL3115A2::getPressure(void)
{
    if (!isBarometerMode)
    {
        configureBarometerMode();
    }

    // Poll until there is data available
    uint8_t status = m_connection->readBytes(STATUS, 1)[0];
    while (!(status & STATUS_PTDR_MASK))
    {
        status = m_connection->readBytes(STATUS, 1)[0];
        std::chrono::milliseconds timespan(10);
        std::this_thread::sleep_for(timespan);
    }

    std::vector<uint8_t> rawData = getData();
    // Upper two bytes + top two bits in LSB represent the 18 bit unsigned integer portion in Pascals
    // Bits 5-4 of LSB represent fractional portion
    uint8_t MSB = rawData[0];
    uint8_t CSB = rawData[1];
    uint8_t LSB = rawData[2];

    // Get integer portion
    unsigned int intPortion = MSB;
    intPortion <<= 8;
    intPortion |= CSB;
    intPortion <<= 2;
    intPortion |= (LSB >> 6);

    // Isolate fractional portion
    uint8_t fractionalPortion = LSB;
    fractionalPortion <<= 2;
    fractionalPortion >>= 6;

    // Calculate pressure
    double fraction = LSB / 4.0;
    double pressure = intPortion + fraction;

    MPL3115A2DATA data;
    data.pressure = pressure;
    data.temperature = calculateTemperature(rawData[3], rawData[4]);
    return data;
}


MPL3115A2DATA MPL3115A2::getAltitude(void)
{
    if (!isAltimeterMode)
    {
        configureAltimeterMode();
    }
    uint8_t status = m_connection->readBytes(STATUS, 1)[0];
    while (!(status & STATUS_PTDR_MASK))
    {
        status = m_connection->readBytes(STATUS, 1)[0];
        std::chrono::milliseconds timespan(10);
        std::this_thread::sleep_for(timespan);
    }

    std::vector<uint8_t> rawData = getData();
    // MSB and CSB represent signed int portion in meters, bits 7-4 represent fractional portion
    uint8_t MSB = rawData[0];
    uint8_t CSB = rawData[1];
    uint8_t LSB = rawData[2];

    // Get signed int portion
    int16_t intPortion = MSB;
    intPortion <<= 8;
    intPortion |= CSB;

    // Get fraction
    uint8_t temp = LSB;
    temp >>= 4;
    double fraction = temp / 16.0;
    if (intPortion < 0)
    {
      fraction *= -1;
    }

    double altitude = intPortion + fraction;

    MPL3115A2DATA data;
    data.altitude = altitude;
    data.temperature = calculateTemperature(rawData[3], rawData[4]);
    return data;
}


double MPL3115A2::calculateTemperature(uint8_t MSB, uint8_t LSB)
{
  // MSB is integer portion, LSB 7-4 is fractional, in Celcius
  int8_t intPortion = MSB;
  uint8_t fractionalPortion = LSB;

  // Get fraction
  uint8_t temp = fractionalPortion;
  temp >>= 4;
  double fraction = temp / 16.0;
  if (intPortion < 0)
  {
      fraction *= -1;
  }

  double temperature = intPortion + fraction;
  return temperature;
}


std::vector<uint8_t> MPL3115A2::getData(void) const
{
    // Get all the data in one transaction
    return m_connection->readBytes(PRESSURE_MSB, 5);
}
