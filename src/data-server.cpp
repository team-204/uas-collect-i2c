// Data server awaits an arbitrary request from another zeromq application.
// It then collects data from the mpl3115a2 device hanging off the i2c adapter
// number provided. Finally it sends that data to the requester and awaits
// another request for data.

#include <iostream>
#include <sstream>
#include <string>
#include <zmq.hpp>

#include "mpl3115a2.hpp"


int main(int argc, char **argv)
{
    // Get the adapter number
    if (argc < 2)
    {
        std::cerr << "Please give the i2c adapter number (found using i2cdetect -l)" << std::endl;
        return 1;
    }
    std::string adapter(argv[1]);
    MPL3115A2 mpl3115a2(stoi(adapter));

    //  Prepare our context and socket to setup as a server
    zmq::context_t context (1);
    zmq::socket_t socket (context, ZMQ_REP);
    socket.bind ("tcp://*:5555");

    // Infinite loop waiting for a client request and then responding
    // with the temperature and altitude data
    for (;;)
    {
        zmq::message_t request;

        //  Wait for next request from client
        socket.recv (&request);
        std::cout << "Received request from client" << std::endl;

        // Get the data
        MPL3115A2DATA data = mpl3115a2.getAltitude();
        std::cout << "Temperature: " << data.temperature << " (C)"<< std::endl;
        std::cout << "Altitude: " << data.altitude << " (m)"<< std::endl;
        std::ostringstream os;
        os << "Temperature: " << data.temperature << " Altitude: " << data.altitude;
        std::string replyString = os.str();

        //  Send reply back to client
        zmq::message_t reply(replyString.size());  // length of reply data
        memcpy(reply.data(), replyString.c_str(), replyString.size());
        socket.send(reply);
    }

    return 0;
}
