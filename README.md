# uas-control-collect
This project consists of a few different parts
- i2c wrapper (which calls the linux i2c libraries)
- MPL3115A2 library
- LSM9DS1 library (eventually)
- GPS library (eventually)
- Communication library via mavlink to pixhawk (eventually)
- Controller to control/navigate the pixhawk

This was all written specifically to run on a beaglebone black.
We're planning on implementing navigation by receiving waypoints from a
ground control station. This will be done (most likely) by a separate process
and leverage some form of IPC to communicate with this program. If these libraries
start to get out of control we may decide to move them to their own process as well
and use IPC to receive all data, essentially leaving this as the "middle man" navigation
process.
