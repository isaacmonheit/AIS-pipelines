ARGS = -Wall `pkg-config --cflags --libs gstreamer-1.0`

demo_c: demo_pipeline.c
	gcc demo_pipeline.c -o demo_c $(ARGS)

demo_cpp: demo_pipeline.cpp
	g++ demo_pipeline.cpp -o demo_cpp $(ARGS)

demo_appss: demo_appss.cpp
	g++ demo_appss.cpp -o demo_appss $(ARGS)

grog: scratch.cpp
	g++ scratch.cpp -o grog $(ARGS)

clean:
	rm -f demo_c demo_cpp demo_appss