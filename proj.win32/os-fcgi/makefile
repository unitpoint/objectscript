DEFINES  ?= 
CXXFLAGS  = -Wall -ggdb -O0 -Wno-comment -Wno-switch
SOURCES  := ./../../source/objectscript.cpp os-fcgi.cpp
SOURCES	 += fcgi-2.4.1/libfcgi/os_unix.c fcgi-2.4.1/libfcgi/fcgiapp.c
SOURCES	 += MPFDParser-1.0/Parser.cpp MPFDParser-1.0/Field.cpp MPFDParser-1.0/Exception.cpp
OBJECTS  := $(patsubst %.cpp,%.o,$(SOURCES))
OBJECTS  := $(patsubst %.c,%.o,$(OBJECTS))
CPPFLAGS += $(patsubst %,-D%,$(DEFINES))
CPPFLAGS += -Ifcgi-2.4.1/include -IMPFDParser-1.0
LD        = $(CXX)
LDLIBS    = -lm -lstdc++

all: ./../../bin/os-fcgi

./../../bin/os-fcgi: $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^ $(LDLIBS)

clean:
	$(RM) $(OBJECTS) ./../../bin/os-fcgi

