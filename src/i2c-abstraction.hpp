#ifndef I2C_ABSTRACTION_HPP
#define I2C_ABSTRACTION_HPP

#include <stdint.h>
#include <string>
#include <vector>


// This class is an abstraction for a i2c connection to a single device.
// The current implementation is based on linux
// readBytes provides a function to read sequential registers from a device.
// writeByte provides a way to write to a register.
class I2cAbstraction
{
    public:
        I2cAbstraction(const unsigned int adapterNumber, const uint8_t deviceAddress);
        std::vector<uint8_t> readBytes(uint8_t reg, unsigned int size) const;
        void writeByte(uint8_t reg, uint8_t data) const;
    private:
        std::string m_i2cFilename;
        int m_i2cFile;
        uint8_t m_deviceAddress;
};

#endif
