#ifndef MPL3115A2_HPP
#define MPL3115A2_HPP

#include <string>


class MPL3115A2
{
    public:
        // Attempts to open the i2c connection at the adapterNumber
        MPL3115A2(unsigned int adapterNumber);
    private:
        std::string m_i2cFilename;
        int m_i2cFile;
};

#endif
