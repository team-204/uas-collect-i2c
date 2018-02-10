.PHONY: clean
CXX=g++
ADDRESSSANITIZER=-fsanitize=address -fno-omit-frame-pointer
GDB=-g -O0
CXXFLAGS=-I. -Wall -Wextra -std=c++11 $(GDB) $(ADDRESSSANITIZER)
LIBS=
BUILDDIR = build/
SRCDIR = src/
DEPS = $(addprefix $(SRCDIR),mpl3115a2.hpp i2c-abstraction.hpp lsm9ds1.hpp)
MPL3115A2-TESTOBJS = mpl3115a2-test.o mpl3115a2.o i2c-abstraction.o
LSM9DS1-TESTOBJS = lsm9ds1-test.o lsm9ds1.o i2c-abstraction.o
OBJS = $(addprefix $(BUILDDIR),$(MPL3115A2-TESTOBJS))

all: mpl3115a2-test lsm9ds1-test

lsm9ds1-test: $(addprefix $(BUILDDIR),$(LSM9DS1-TESTOBJS))
		$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

mpl3115a2-test: $(addprefix $(BUILDDIR),$(MPL3115A2-TESTOBJS))
		$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

$(BUILDDIR)%.o: $(SRCDIR)%.cpp $(DEPS)
		$(CXX) -c -o $@ $< $(CXXFLAGS)

clean:
		rm -f $(OBJS) mpl3115a2-test lsm9ds1-test

$(OBJS): | $(BUILDDIR)

$(BUILDDIR):
		mkdir $(BUILDDIR)
