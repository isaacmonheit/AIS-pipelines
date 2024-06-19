ARGS = -Wall `pkg-config --cflags --libs gstreamer-1.0`

fork: fork.cpp
	g++ -o $@ $^ $(ARGS)

knife: knife.cpp
	g++ -o $@ $^ $(ARGS)

spoon: spoon.cpp
	g++ -o $@ $^ $(ARGS)

clean:
	rm fork output_files/*

.PHONY: clean