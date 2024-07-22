ARGS = -Wall `pkg-config --cflags --libs gstreamer-1.0`

all: knork fork knife spoon

%: %.cpp
	g++ -o $@ $< $(ARGS)

clean:
	rm -f knork fork knife spoon output.*

.PHONY: clean all
