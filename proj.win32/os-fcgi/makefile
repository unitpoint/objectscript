DEFINES  ?= 
CXXFLAGS  = -Wall -ggdb -O0 -Wno-comment -Wno-switch
SOURCES  := ./../../source/objectscript.cpp os-fcgi.cpp fcgi-2.4.1/libfcgi/os_unix.c fcgi-2.4.1/libfcgi/fcgiapp.c
OBJECTS  := $(patsubst %.cpp,%.o,$(SOURCES))
OBJECTS  := $(patsubst %.c,%.o,$(OBJECTS))
CPPFLAGS += $(patsubst %,-D%,$(DEFINES))
LD        = $(CXX)
LDLIBS    = -lm -lstdc++

all: ./../../bin/os-fcgi

./../../bin/os-fcgi: $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^ $(LDLIBS)

clean:
	$(RM) $(OBJECTS) ./../../bin/os-fcgi

