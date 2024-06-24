ARGS = -Wall `pkg-config --cflags --libs gstreamer-1.0`

all: fork knife spoon

fork: fork.cpp
	g++ -o $@ $^ $(ARGS)

knife: knife.cpp
	g++ -o $@ $^ $(ARGS)

spoon: spoon.cpp
	g++ -o $@ $^ $(ARGS)

clean:
	rm -f fork knife spoon output.*

.PHONY: clean all
