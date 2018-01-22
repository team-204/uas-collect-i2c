#ifndef MPL3115A2_HPP
#define MPL3115A2_HPP

#include <stdint.h>
#include <string>
#include <vector>


struct MPL3115A2DATA
{
    public:
        double pressure;
        double altitude;
        double temperature;
};


class MPL3115A2
{
    public:

        // Attempts to open the i2c connection at the adapterNumber
        MPL3115A2(const unsigned int adapterNumber);

        MPL3115A2DATA getPressure(void);
        MPL3115A2DATA getAltitude(void);

    private:
        double getTemperature(void);
        uint8_t enterStandbyMode(void) const;
        void configureDataReadyFlag(void) const;
        void configureAltimeterMode(void);
        void configureBarometerMode(void);
        uint8_t readByte(uint8_t reg) const;
        void writeByte(uint8_t reg, uint8_t data) const;

        std::string m_i2cFilename;
        int m_i2cFile;
        bool isAltimeterMode;
        bool isBarometerMode;
};

#endif
