#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include "lsm9ds1.hpp"


int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "Please give the i2c adapter number (found using i2cdetect -l)" << std::endl;
        return 1;
    }
    std::string adapter(argv[1]);
    LSM9DS1 lsm9ds1(stoi(adapter));

    for (;;)
    {
        std::vector<int16_t> accel, gyro, mag;
        accel = lsm9ds1.getAccel();
        gyro = lsm9ds1.getGyro();
        mag = lsm9ds1.getMag();
        std::cout << "Acceleration(X, Y, Z): ";
        std::cout << accel[0] << " " << accel[1] << " " << accel[2] << std::endl;
        std::cout << "Gyro(X, Y, Z): ";
        std::cout << gyro[0] << " " << gyro[1] << " " << gyro[2] << std::endl;
        std::cout << "Mag(X, Y, Z): ";
        std::cout << mag[0] << " " << mag[1] << " " << mag[2] << std::endl;
        std::chrono::seconds timespan(1);
        std::this_thread::sleep_for(timespan);
    }

    return 0;
}
