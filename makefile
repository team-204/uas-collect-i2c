.PHONY: clean
CXX=g++
ADDRESSSANITIZER=-fsanitize=address -fno-omit-frame-pointer
GDB=-g -O0
CXXFLAGS=-I. -Wall -Wextra -std=c++11 $(GDB) $(ADDRESSSANITIZER)
LIBS=
BUILDDIR = build/
SRCDIR = src/
DEPS = $(addprefix $(SRCDIR),mpl3115a2.hpp i2c-abstraction.hpp)
I2COBJS = main.o mpl3115a2.o i2c-abstraction.o
OBJS = $(addprefix $(BUILDDIR),$(I2COBJS))

all: i2c

i2c: $(OBJS)
		$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

$(BUILDDIR)%.o: $(SRCDIR)%.cpp $(DEPS)
		$(CXX) -c -o $@ $< $(CXXFLAGS)

clean:
		rm -f $(OBJS) i2c

$(OBJS): | $(BUILDDIR)

$(BUILDDIR):
		mkdir $(BUILDDIR)
