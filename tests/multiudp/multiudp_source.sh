#!/bin/bash

gst-launch-1.0 -v -e \
    v4l2src device=/dev/video0 ! video/x-raw, width=640, height=480, framerate=30/1 ! videoconvert ! x264enc ! h264parse ! rtph264pay ! multiudpsink clients=127.0.0.1:5000,127.0.0.1:5001