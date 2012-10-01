
SOURCES := source/objectscript.cpp source/main.cpp
OBJECTS := $(patsubst %.cpp,%.o,$(SOURCES))
CXXFLAGS = -Wall -ggdb -O0
LD       = $(CXX)

all: bin/os

bin/os: $(OBJECTS) -ledit -lstdc++ -lm
	$(LD) $(LDFLAGS) -o $@ $^

clean:
	$(RM) $(OBJECTS) bin/os

