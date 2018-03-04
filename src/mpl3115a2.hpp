#ifndef MPL3115A2_HPP
#define MPL3115A2_HPP

#include <memory>
#include <stdint.h>
#include <string>
#include <vector>

#include "i2c-abstraction.hpp"


// This struct represents the data available from the device.
// Note that the get functions in MPL3115A2 only fill two fields,
// the device can only read pressure and temperature, or altitude and
// temperature.
struct MPL3115A2DATA
{
    public:
        double pressure;
        double altitude;
        double temperature;
};


// This class represents the MPL3115A2.
// Using getPressure and getPressure/getAltitude will return a data struct with
// temperature and pressure/altitude data, depending on the function used.
class MPL3115A2
{
    public:
        // Attempts to open the i2c connection at the adapterNumber
        MPL3115A2(const unsigned int adapterNumber);
        MPL3115A2DATA getPressure(void);
        MPL3115A2DATA getAltitude(void);

    private:
        double calculateTemperature(uint8_t MSB, uint8_t LSB);
        uint8_t enterStandbyMode(void) const;
        void configureDataReadyFlag(void) const;
        void configureAltimeterMode(void);
        void configureBarometerMode(void);
        std::vector<uint8_t> getData(void) const;
        std::unique_ptr<I2cAbstraction> m_connection;
        bool isAltimeterMode;
        bool isBarometerMode;
};

#endif
