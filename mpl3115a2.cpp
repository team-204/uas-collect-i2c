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
    uint8_t whoIsThis = readByte(WHO_AM_I);
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
        close(m_i2cFile);
    }
}


uint8_t MPL3115A2::readByte(uint8_t reg) const
{
    struct i2c_rdwr_ioctl_data packagedMessages;
    struct i2c_msg messages[2];

    // This message is responsible for telling the sensor which register we want data from
    messages[0].addr = MPL3115A2_ADDRESS;
    messages[0].flags = 0;
    messages[0].len = sizeof(reg);
    messages[0].buf = &reg;

    // This message contains the data from the register
    uint8_t data = 0;
    messages[1].addr  = MPL3115A2_ADDRESS;
    messages[1].flags = I2C_M_RD;
    messages[1].len   = sizeof(data);
    messages[1].buf   = &data;

    // Ask for the transaction to take place
    packagedMessages.msgs = messages;
    packagedMessages.nmsgs = 2;
    if(ioctl(m_i2cFile, I2C_RDWR, &packagedMessages) < 0)
    {
        std::ostringstream err;
        err << "Could not perform read" << std::endl << strerror(errno);
        throw std::runtime_error(err.str());
    }
    return data;
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
