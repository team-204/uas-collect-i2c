#include <iostream>
#include <sstream>
#include <stdint.h>
#include <stdexcept>
#include <string>
#include <string.h>

#include <errno.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "mpl3115a2.hpp"


constexpr uint8_t MPL3115A2_ADDRESS = 0x60;
constexpr uint8_t STATUS = 0x00;
constexpr uint8_t DATA_READY = 0x06;
constexpr uint8_t WHO_AM_I = 0x0C;
constexpr uint8_t DEVICE_ID = 0xC4;
constexpr uint8_t PT_DATA_CFG = 0x13;


MPL3115A2::MPL3115A2(unsigned int adapterNumber)
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
    uint8_t buf[1];
    buf[0] = WHO_AM_I;
    if (read(m_i2cFile, buf, 1) != 1)
    {
        std::ostringstream err;
        err << "Could not read WHO_AM_I register" << strerror(errno);
        throw std::runtime_error(err.str());
    }
    if (buf[0] != DEVICE_ID)
    {
        std::ostringstream err;
        err << "WHO_AM_I register contained: " << std::hex
            << static_cast<int>(buf[0]) << std::endl
            << "Expected: " << std::hex << DEVICE_ID << std::endl;
        throw std::runtime_error(err.str());
    }
    else
    {
        std::cout << "MPL3115A2 confirmed to be on I2C bus represented by "
                  << m_i2cFilename << std::endl;
    }
}
