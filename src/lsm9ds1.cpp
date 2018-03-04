// This device can be a bit confusing due to the fact that
// the LSM9DS1 is actually 2 separate devices, an accelerometer/gyro and
// a magnetometer. You'll note a lot of variables ending in XL or G or M.
// These stand for accelerometer, gyroscope, and magnetometer.
// The addressing here also assumes that the SDO pins are tied to ground
// making the LSB for both XL/G and M 0 when doing i2c addressing.


#include <stdint.h>
#include <sstream>

#include "i2c-abstraction.hpp"
#include "lsm9ds1.hpp"


// Register addresses
constexpr uint8_t LSM9DS1_XLG_ADDRESS = 0x6A;
constexpr uint8_t LSM9DS1_M_ADDRESS = 0x1C;
constexpr uint8_t WHO_AM_I_M = 0x0F;
constexpr uint8_t DEVICE_ID_M = 0x3D;
constexpr uint8_t WHO_AM_I_XLG = 0x0F;
constexpr uint8_t DEVICE_ID_XLG = 0x68;
constexpr uint8_t OUT_X_L_G = 0x18;
constexpr uint8_t OUT_X_H_G = 0x19;
constexpr uint8_t OUT_Y_L_G = 0x1A;
constexpr uint8_t OUT_Y_H_G = 0x1B;
constexpr uint8_t OUT_Z_L_G = 0x1C;
constexpr uint8_t OUT_Z_H_G = 0x1D;
constexpr uint8_t OUT_X_L_XL = 0x28;
constexpr uint8_t OUT_X_H_XL = 0x29;
constexpr uint8_t OUT_Y_L_XL = 0x2A;
constexpr uint8_t OUT_Y_H_XL = 0x2B;
constexpr uint8_t OUT_Z_L_XL = 0x2C;
constexpr uint8_t OUT_Z_H_XL = 0x2D;
constexpr uint8_t OUT_X_L_M = 0x28;
constexpr uint8_t OUT_X_H_M = 0x29;
constexpr uint8_t OUT_Y_L_M = 0x2A;
constexpr uint8_t OUT_Y_H_M = 0x2B;
constexpr uint8_t OUT_Z_L_M = 0x2C;
constexpr uint8_t OUT_Z_H_M = 0x2D;


LSM9DS1::LSM9DS1(const unsigned int adapterNumber) :
    m_magConn(new I2cAbstraction(adapterNumber, LSM9DS1_M_ADDRESS)),
    m_xlgConn(new I2cAbstraction(adapterNumber, LSM9DS1_XLG_ADDRESS))
{
    // Confirm that the device at this address is indeed the LSM9DS1
    uint8_t whoIsThis = m_magConn->readBytes(WHO_AM_I_M, 1)[0];
    if (whoIsThis != DEVICE_ID_M)
    {
        std::ostringstream err;
        err << "WHO_AM_I_M register contained: " << std::hex
            << whoIsThis << std::endl
            << "Expected: " << std::hex << DEVICE_ID_M << std::endl;
        throw std::runtime_error(err.str());
    }
    whoIsThis = m_xlgConn->readBytes(WHO_AM_I_XLG, 1)[0];
    if (whoIsThis != DEVICE_ID_M)
    {
        std::ostringstream err;
        err << "WHO_AM_I_XLG register contained: " << std::hex
            << whoIsThis << std::endl
            << "Expected: " << std::hex << DEVICE_ID_XLG << std::endl;
        throw std::runtime_error(err.str());
    }
}


std::vector<int16_t> LSM9DS1::getAccel(void) const
{
    std::vector<uint8_t> data = m_xlgConn->readBytes(OUT_X_L_XL, 6);
    int16_t accelerationX, accelerationY, accelerationZ;
    accelerationX = (data[1] << 8) | data[0];
    accelerationY = (data[3] << 8) | data[2];
    accelerationZ = (data[5] << 8) | data[4];
    std::vector<int16_t> accelData;
    accelData[0] = accelerationX;
    accelData[1] = accelerationY;
    accelData[2] = accelerationZ;
    return accelData;
}


std::vector<int16_t> LSM9DS1::getGyro(void) const
{
    std::vector<uint8_t> data = m_xlgConn->readBytes(OUT_X_L_G, 6);
    int16_t rotationX, rotationY, rotationZ;
    rotationX = (data[1] << 8) | data[0];
    rotationY = (data[3] << 8) | data[2];
    rotationZ = (data[5] << 8) | data[4];
    std::vector<int16_t> gyroData;
    gyroData[0] = rotationX;
    gyroData[1] = rotationY;
    gyroData[2] = rotationZ;
    return gyroData;
}


std::vector<int16_t> LSM9DS1::getMag(void) const
{
    std::vector<uint8_t> data = m_magConn->readBytes(OUT_X_L_M, 6);
    int16_t magX, magY, magZ;
    magX = (data[1] << 8) | data[0];
    magY = (data[3] << 8) | data[2];
    magZ = (data[5] << 8) | data[4];
    std::vector<int16_t> magData;
    magData[0] = magX;
    magData[1] = magY;
    magData[2] = magZ;
    return magData;
}
