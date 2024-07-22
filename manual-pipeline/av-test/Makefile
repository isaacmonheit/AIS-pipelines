ARGS = -Wall `pkg-config --cflags --libs gstreamer-1.0`

SOURCES := $(wildcard *.cpp)
TARGETS := $(SOURCES:.cpp=)

# Rule to compile all .cpp files
all: $(TARGETS)

%: %.cpp
	g++ -o $@ $< $(ARGS) 

clean:
	rm -f output.*

.PHONY: clean all
