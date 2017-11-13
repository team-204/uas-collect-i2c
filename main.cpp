#include <iostream>
#include <string>
#include "mpl3115a2.hpp"


int main(int argc, char **argv)
{
    std::cout << "Hello world!" << std::endl;
    if (argc != 2)
    {
        std::cerr << "Please give the i2c adapter number (found using i2cdetect -l)" << std::endl;
        return 1;
    }
    std::string adapter(argv[1]);
    MPL3115A2 temp(stoi(adapter));
    return 0;
}
