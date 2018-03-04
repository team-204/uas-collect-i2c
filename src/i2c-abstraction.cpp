#include <iostream>
#include <memory>
#include <stdexcept>
#include <stdint.h>
#include <sstream>
#include <string>
#include <string.h>
#include <vector>

#include <errno.h>
#include <fcntl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "i2c-abstraction.hpp"


I2cAbstraction::I2cAbstraction(const unsigned int adapterNumber, const uint8_t deviceAddress)
{
    // Basically setup an dev file to be used for i2c and handle errors
    m_deviceAddress = deviceAddress;
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
    if (ioctl(m_i2cFile, I2C_SLAVE, m_deviceAddress) < 0)
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
}


std::vector<uint8_t> I2cAbstraction::readBytes(uint8_t reg, unsigned int size) const
{
    struct i2c_rdwr_ioctl_data packagedMessages;
    struct i2c_msg messages[2];

    // This message is responsible for telling the device which register we want data from
    messages[0].addr = m_deviceAddress;
    messages[0].flags = 0;
    messages[0].len = sizeof(reg);
    messages[0].buf = &reg;

    // This message contains the data from the register
    std::unique_ptr<uint8_t[]> data(new uint8_t[size]);
    messages[1].addr  = m_deviceAddress;
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


void I2cAbstraction::writeByte(uint8_t reg, uint8_t data) const
{
    struct i2c_rdwr_ioctl_data packagedMessages;
    struct i2c_msg message;

    // This message contains the register to write to as well as the data to write
    uint8_t out[2];
    out[0] = reg;
    out[1] = data;
    message.addr = m_deviceAddress;
    message.flags = 0;
    message.len = sizeof(out);
    message.buf = out;


    // Ask for the transaction to take place
    packagedMessages.msgs = &message;
    packagedMessages.nmsgs = 1;
    if(ioctl(m_i2cFile, I2C_RDWR, &packagedMessages) < 0)
    {
        std::ostringstream err;
        err << "Could not perform write" << std::endl << strerror(errno);
        throw std::runtime_error(err.str());
    }
}
