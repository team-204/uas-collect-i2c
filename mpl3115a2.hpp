#ifndef MPL3115A2_HPP
#define MPL3115A2_HPP

#include <stdint.h>
#include <string>
#include <vector>


class MPL3115A2
{
    public:
        // Attempts to open the i2c connection at the adapterNumber
        MPL3115A2(const unsigned int adapterNumber);
    private:
        uint8_t readByte(uint8_t reg) const;
        void writeByte(uint8_t reg, uint8_t data) const;
        std::string m_i2cFilename;
        int m_i2cFile;
};

#endif
