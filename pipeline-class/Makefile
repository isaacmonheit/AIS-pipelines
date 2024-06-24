ARGS = -Wall `pkg-config --cflags --libs gstreamer-1.0`

main: main.cpp Pipeline.cpp
	g++ -o $@ $^ $(ARGS)

clean:
	rm -f output.ts main

.PHONY: clean