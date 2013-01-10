DEFINES  ?= OS_USE_LIBEDIT
CXXFLAGS  = -Wall -ggdb -O0 -Wno-comment -Wno-switch
SOURCES  := source/objectscript.cpp source/main.cpp
OBJECTS  := $(patsubst %.cpp,%.o,$(SOURCES))
CPPFLAGS += $(patsubst %,-D%,$(DEFINES))
LD        = $(CXX)
LDLIBS    = -lm -lstdc++

all: bin/os

bin/os: $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^ $(LDLIBS)

clean:
	$(RM) $(OBJECTS) bin/os

