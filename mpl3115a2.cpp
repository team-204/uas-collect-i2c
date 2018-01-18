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

#include <errno.h>
#include <fcntl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <unistd.h>

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


MPL3115A2::MPL3115A2(const unsigned int adapterNumber)
{
    std::ostringstream oss;
    oss << "/dev/i2c-" << adapterNumber;
    m_i2cFilename = oss.str();
    m_i2cFile = open(m_i2cFilename.c_str(), O_RDWR);
    if (m_i2cFile < 0)
    {
        std::ostringstream err;
        err << "Could not open " << m_i2cFilename << std::endl << strerror(errno);
        throw std::runtime_error(err.str());
    }
    if (ioctl(m_i2cFile, I2C_SLAVE, MPL3115A2_ADDRESS) < 0)
    {
        std::ostringstream err;
        err << "Could not set slave address\n" << strerror(errno);
        throw std::runtime_error(err.str());
    }
    unsigned long funcs = 0;
    if (ioctl(m_i2cFile, I2C_FUNCS, &funcs) < 0)
    {
        std::ostringstream err;
        err << "Could not get available functionality from the i2c adapter"
            << std::endl << strerror(errno);
        throw std::runtime_error(err.str());
    }
    if (!(funcs & I2C_FUNC_I2C))
    {
        std::ostringstream err;
        err << "The adapter does not support a mixed read write transaction"
            << std::endl << strerror(errno);
        throw std::runtime_error(err.str());
    }

    // Confirm that the device at this address is indeed the MPL3115A2
    uint8_t whoIsThis = readBytes(WHO_AM_I, 1)[0];
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
        std::cout << "MPL3115A2 confirmed to be on I2C bus represented by "
                  << m_i2cFilename << std::endl;
    }

    configureDataReadyFlag();
    configureAltimeterMode();
}


uint8_t MPL3115A2::enterStandbyMode(void) const
{
    // Enter standby mode to allow register writing
    // Return the register data to allow it to be restored to previous state
    uint8_t controlRegisterData = readBytes(CTRL_REG1, 1)[0];
    writeByte(CTRL_REG1, controlRegisterData & ~STANDBY_BAR_MASK);  // Clear standby bar bit
    return controlRegisterData;
}


void MPL3115A2::configureAltimeterMode(void)
{
    // Clear the standby-bar bit to activate standby mode
    uint8_t controlRegisterData = enterStandbyMode();

    // Set the mode to altimeter and set the standby-bar bit to deactivate standby mode
    writeByte(CTRL_REG1, controlRegisterData | ALTIMETER_MASK | STANDBY_BAR_MASK);
    isAltimeterMode = true;
    isBarometerMode = false;
}


void MPL3115A2::configureBarometerMode(void)
{
    // Clear the standby-bar bit to activate standby mode
    uint8_t controlRegisterData = enterStandbyMode();

    // Set the mode to barometer and set the standby-bar bit to deactivate standby mode
    writeByte(CTRL_REG1, (controlRegisterData & ~ALTIMETER_MASK) | STANDBY_BAR_MASK);
    isAltimeterMode = false;
    isBarometerMode = true;
}


void MPL3115A2::configureDataReadyFlag(void) const
{
    // Clear the standby-bar bit to activate standby mode
    // Entering standby mode maybe unnecessary to change this register but we'll do it anyways
    uint8_t controlRegisterData = enterStandbyMode();

    // Configure the sensor data register to raise a status flag when any new data is available
    writeByte(PT_DATA_CFG, PT_DATA_CFG_DREM_MASK | PT_DATA_CFG_TDEFE_MASK | PT_DATA_CFG_PDEFE_MASK);

    // Set device back to active mode
    writeByte(CTRL_REG1, controlRegisterData | STANDBY_BAR_MASK);
}


std::vector<uint8_t> MPL3115A2::readBytes(uint8_t reg, unsigned int size) const
{
    struct i2c_rdwr_ioctl_data packagedMessages;
    struct i2c_msg messages[2];

    // This message is responsible for telling the sensor which register we want data from
    messages[0].addr = MPL3115A2_ADDRESS;
    messages[0].flags = 0;
    messages[0].len = sizeof(reg);
    messages[0].buf = &reg;

    // This message contains the data from the register
    std::unique_ptr<uint8_t[]> data(new uint8_t[size]);
    messages[1].addr  = MPL3115A2_ADDRESS;
    messages[1].flags = I2C_M_RD;
    messages[1].len   = size;
    messages[1].buf   = data.get();

    // Ask for the transaction to take place
    packagedMessages.msgs = messages;
    packagedMessages.nmsgs = 2;
    if(ioctl(m_i2cFile, I2C_RDWR, &packagedMessages) < 0)
    {
        std::ostringstream err;
        err << "Could not perform read" << std::endl << strerror(errno);
        throw std::runtime_error(err.str());
    }
    return std::vector<uint8_t>(data.get(), data.get() + size);
}


void MPL3115A2::writeByte(uint8_t reg, uint8_t data) const
{
    struct i2c_rdwr_ioctl_data packagedMessages;
    struct i2c_msg message;

    // This message contains the register to write to as well as the data to write
    uint8_t out[2];
    out[0] = reg;
    out[1] = data;
    message.addr = MPL3115A2_ADDRESS;
    message.flags = 0;
    message.len = sizeof(out);
    message.buf = out;

    packagedMessages.msgs = &message;
    packagedMessages.nmsgs = 1;

    if(ioctl(m_i2cFile, I2C_RDWR, &packagedMessages) < 0)
    {
        std::ostringstream err;
        err << "Could not perform write" << std::endl << strerror(errno);
        throw std::runtime_error(err.str());
    }
}


MPL3115A2DATA MPL3115A2::getPressure(void)
{
    if (!isBarometerMode)
    {
        configureBarometerMode();
    }

    // Poll until there is data available
    uint8_t status = readBytes(STATUS, 1)[0];
    while (!(status & STATUS_PTDR_MASK))
    {
        status = readBytes(STATUS, 1)[0];
        std::chrono::milliseconds timespan(10);
        std::this_thread::sleep_for(timespan);
    }

    // Upper two bytes + top two bits in LSB represent the 18 bit unsigned integer portion in Pascals
    // Bits 5-4 of LSB represent fractional portion
    uint8_t MSB = readBytes(PRESSURE_MSB, 1)[0];
    uint8_t CSB = readBytes(PRESSURE_CSB, 1)[0];
    uint8_t LSB = readBytes(PRESSURE_LSB, 1)[0];
    double temperature = getTemperature();

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
    data.temperature = temperature;
    return data;
}


MPL3115A2DATA MPL3115A2::getAltitude(void)
{
    if (!isAltimeterMode)
    {
        configureAltimeterMode();
    }
    uint8_t status = readBytes(STATUS, 1)[0];
    while (!(status & STATUS_PTDR_MASK))
    {
        status = readBytes(STATUS, 1)[0];
        std::chrono::milliseconds timespan(10);
        std::this_thread::sleep_for(timespan);
    }

    // MSB and CSB represent signed int portion in meters, bits 7-4 represent fractional portion
    uint8_t MSB = readBytes(PRESSURE_MSB, 1)[0];
    uint8_t CSB = readBytes(PRESSURE_CSB, 1)[0];
    uint8_t LSB = readBytes(PRESSURE_LSB, 1)[0];
    double temperature = getTemperature();

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
    data.temperature = temperature;
    return data;
}


double MPL3115A2::getTemperature(void)
{
  // MSB is integer portion, LSB 7-4 is fractional, in Celcius
  int8_t intPortion = readBytes(TEMPERATURE_MSB, 1)[0];
  uint8_t fractionalPortion = readBytes(TEMPERATURE_LSB, 1)[0];

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
