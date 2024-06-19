#!/bin/bash

# DOESN'T WORK


# A forked pipeline that takes in video from a webcam, encodes it in h264, and sends it to both a file and a UDP stream

### To run this script ###
#1. Install gstreamer
#2. Install gstinterpipe {https://github.com/RidgeRun/gst-interpipe}
#3. Ensure that host / port number are set correctly, as well as file output name / location
#4. Run 'chmod +x forked_pipeline_demo.sh'
#5. Run './forked_pipeline_demo.sh'

gst-launch-1.0 -v -e \
    v4l2src device=/dev/video0 ! video/x-raw, width=640, height=480, framerate=30/1 ! videoconvert ! x264enc ! h264parse ! queue ! mp4mux ! filesink location=output.mp4 \
    # interpipesrc listen-to=source_pipe is-live=true format=time ! queue ! videoconvert ! x264enc ! h264parse ! rtph264pay ! udpsink host=127.0.0.1 port=5000
