cxxflags := $(CPPFLAGS) -Isrc $(CXXFLAGS) -std=c++23 -pedantic -Wall -Wextra -Werror
ldflags := $(LDFLAGS)

sources := $(wildcard src/*.cpp)
objects := $(sources:.cpp=.o)
depends := $(sources:.cpp=.d)

all: sodac

clean:
	$(RM) sodac src/*.[do]

sodac: $(objects)
	$(CXX) $(strip $(cxxflags) -o $@ $(objects) $(ldflags))

.cpp.o:
	$(CXX) $(strip $(cxxflags) -c -MMD -o $@ $<)

-include $(depends)

.PHONY: all clean
