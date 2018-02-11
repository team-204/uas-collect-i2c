// TODO: Add check for data available before getting data
// TODO: Add some form of filtering for the data
// TODO: Add some kind of interpretation of the data
#ifndef LSM9DS1_HPP
#define LSM9DS1_HPP

#include <memory>

#include "i2c-abstraction.hpp"


// This class represents the LSM9DS1.
class LSM9DS1
{
    public:
        // Attempts to open the i2c connections at the adapterNumber
        LSM9DS1(const unsigned int adapterNumber);

        // Returns the raw values of acceleration x, y, z
        // These really need to be filtered (low pass for accelerometer)
        std::vector<int16_t> getAccel(void) const;

        // Returns the raw values of angular something something TODO
        // These should be filtered with a high pass filter and error
        // corrected for drift
        std::vector<int16_t> getGyro(void) const;

        // Returns the raw values of magnetic something TODO
        // These should be filtered TODO
        std::vector<int16_t> getMag(void) const;

    private:
        std::unique_ptr<I2cAbstraction> m_magConn;
        std::unique_ptr<I2cAbstraction> m_xlgConn;
        //bool isAccelReady(void) const;
        //bool isGyroReady(void) const;
        //bool isMagReady(void) const;
};

#endif
