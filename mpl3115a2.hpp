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
        std::vector<uint8_t> readBytes(const uint8_t reg, const unsigned int size) const;

        // Expects the first element in data to be the register to be written to
        void writeBytes(const std::vector<uint8_t> &data) const;
        std::string m_i2cFilename;
        int m_i2cFile;
};

#endif
