CXX=g++
ADDRESSSANITIZER=-fsanitize=address -fno-omit-frame-pointer
GDB=-g -O0
CXXFLAGS=-I. -Wall -Wextra -std=c++11 $(GDB) $(ADDRESSSANITIZER)
LIBS=

DEPS = mpl3115a2.hpp i2c-abstraction.hpp
I2COBJS = main.o mpl3115a2.o i2c-abstraction.o
OBJS = $(I2COBJS)

all: i2c

i2c: $(OBJS)
		$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

%.o: %.cpp $(DEPS)
		$(CXX) -c -o $@ $< $(CXXFLAGS)

.phony: clean all
clean:
		rm -f $(OBJS) i2c
