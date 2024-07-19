#!/bin/bash

gst-launch-1.0 -v -e \
    v4l2src device=/dev/video0 ! video/x-raw, width=640, height=480, framerate=30/1 ! videoconvert ! x264enc ! interpipesink name=source_pipe \
    interpipesrc listen-to=source_pipe ! filesink location=output.h264 \
    interpipesrc listen-to=source_pipe ! rtph264pay ! udpsink host=localhost port=5000
