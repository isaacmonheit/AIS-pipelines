#!/bin/bash

gst-launch-1.0 -v -e \
    v4l2src device=/dev/video0 ! video/x-raw, width=640, height=480, framerate=30/1 ! videoconvert ! x264enc ! h264parse ! interpipesink name=source_pipe \
    interpipesrc listen-to=source_pipe ! flvmux ! filesink location=output.flv \
    interpipesrc listen-to=source_pipe ! rtph264pay ! udpsink host=127.0.0.1 port=5000
    #interpipesrc listen-to=source_pipe ! autovideosink