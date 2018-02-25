""" An example data client for the data-server.
This is a simple data client to interact with the data-server created with
the makefile. Basically all that it does is send an arbitrary request message
to the server, and then prints the response. It does this 10 times and then
exits. This provides a really simple way of getting the i2c data from our c++
program. It is worth noting that zeromq allows the client to start before the
server even, allowing us to start these in any order.
"""
import zmq

context = zmq.Context()

#  Socket to talk to server
print("Connecting to data-server...")
socket = context.socket(zmq.REQ)
socket.connect("tcp://localhost:5555")

#  Do 10 requests, waiting each time for a response
for request in range(10):
    print("Sending request {}...".format(request))
    socket.send(b"Can I please have data?")

    #  Get the reply (this is a blocking call)
    message = socket.recv()
    print("Received reply {} [ {} ]".format(request, message))
