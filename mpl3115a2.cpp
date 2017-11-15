#include <iostream>
#include <memory>
#include <sstream>
#include <stdint.h>
#include <stdexcept>
#include <string>
#include <string.h>
#include <vector>

#include <errno.h>
#include <fcntl.h>
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
// Register defaults
constexpr uint8_t DEVICE_ID = 0xC4;


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

    // Confirm that the device at this address is indeed the MPL3115A2
    std::vector<uint8_t> whoIsThis = readBytes(WHO_AM_I, 1);
    if (whoIsThis[0] != DEVICE_ID)
    {
        std::ostringstream err;
        err << "WHO_AM_I register contained: " << std::hex
            << whoIsThis[0] << std::endl
            << "Expected: " << std::hex << DEVICE_ID << std::endl;
        throw std::runtime_error(err.str());
    }
    else
    {
        std::cout << "MPL3115A2 confirmed to be on I2C bus represented by "
                  << m_i2cFilename << std::endl;
    }
}


std::vector<uint8_t> MPL3115A2::readBytes(const uint8_t reg, const unsigned int size) const
{
    std::vector<uint8_t> regVector {reg};
    writeBytes(regVector);  // Tell slave where the data we want is
    std::unique_ptr<uint8_t> cArray(new uint8_t[size]);
    if (read(m_i2cFile, cArray.get(), 1) != 1)
    {
        std::ostringstream err;
        err << "Could not read from MPL3115A2" << strerror(errno);
        throw std::runtime_error(err.str());
    }
    std::vector<uint8_t> data;
    data.assign(cArray.get(), cArray.get() + size);
    return data;
}


void MPL3115A2::writeBytes(const std::vector<uint8_t> &data) const
{
    std::unique_ptr<uint8_t> cArray(new uint8_t[data.size()]);
    uint8_t *currentElement = cArray.get();
    for (uint8_t x : data)
    {
        *currentElement = x;
        ++currentElement;
    }
    if (write(m_i2cFile, cArray.get(), data.size()) != 3)
    {
        std::ostringstream err;
        err << "Could not write to MPL3115A2" << strerror(errno);
        throw std::runtime_error(err.str());
    }
}
