# uas-collect-i2c
This project consists of a few different parts
- i2c wrapper (which calls the linux i2c libraries)
- MPL3115A2 library
- LSM9DS1 library (eventually)

It produces 3 executables, one to test the two libraries,
and one more important executable (data-server) that utilizes zeromq
to perform ipc for other applications. In our project we are using this
in our python application to help with data collection and navigation
(mainly because i2c in python wasn't super easy to figure out and we implemented
these c++ libraries first).


An example python program that would interact with data-server is included.
Note that data-server is going to require the mpl3115a2 device hooked up via
i2c, and that it requires zeromq to be installed on the system.
