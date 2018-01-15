#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include "mpl3115a2.hpp"


int main(int argc, char **argv)
{
    bool logToFile = false;
    char *filename = nullptr;
    if (argc < 2)
    {
        std::cerr << "Please give the i2c adapter number (found using i2cdetect -l)" << std::endl;
        return 1;
    }
    else if (argc == 3)
    {
        logToFile = true;
        filename = argv[2];
    }
    std::string adapter(argv[1]);
    MPL3115A2 mpl3115a2(stoi(adapter));

    if (logToFile)
    {
        std::ofstream file(filename);
        file << "Temperature (C), Altitude (m)" << std::endl;
        for (;;)
        {
            // No idea how fast this will actually poll, we don't really care for this data
            MPL3115A2DATA data = mpl3115a2.getAltitude();
            file << data.temperature << "," << data.altitude << std::endl;
            std::chrono::milliseconds timespan(50);
            std::this_thread::sleep_for(timespan);
        }
    }
    else
    {
        for (;;)
        {
            MPL3115A2DATA data = mpl3115a2.getAltitude();
            std::cout << "Temperature: " << data.temperature << " (C)"<< std::endl;
            std::cout << "Altitude: " << data.altitude << " (m)"<< std::endl;
            std::chrono::seconds timespan(1);
            std::this_thread::sleep_for(timespan);
        }
    }

    return 0;
}
