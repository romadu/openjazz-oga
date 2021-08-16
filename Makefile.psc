# OpenJazz SDL2
include openjazz.mk

# Sane defaults
CXX ?= arm-linux-gnueabihf-g++-6
CXXFLAGS ?= -g -Wall -O3 -marm -mtune=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard
CPPFLAGS = -Isrc -DSCALE -Iext/scale2x -Iext/psmplug -Iext/miniz -DHOMEDIR -DFULLSCREEN_ONLY

# Network support
CXXFLAGS += -DUSE_SOCKETS

# SDL
CXXFLAGS += $(shell sdl2-config --cflags)
LIBS += $(shell sdl2-config --libs)

LIBS += -lm

.PHONY: clean

OpenJazz: $(OBJS)
	@-echo [LD] $@
	@$(CXX) -o OpenJazz $(LDFLAGS) $(OBJS) $(LIBS)

%.o: %.cpp
	@-echo [CXX] $<
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

clean:
	@-echo Cleaning...
	@rm -f OpenJazz $(OBJS)
