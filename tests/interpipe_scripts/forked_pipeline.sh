#!/bin/bash

# this one works, encoding and decoding using queue and sending to udp

gst-launch-1.0 -v -e \
    v4l2src device=/dev/video0 ! video/x-raw,width=640,height=480,framerate=30/1 ! videoconvert ! x264enc ! video/x-h264,stream-format=byte-stream,alignment=au ! mpegtsmux ! queue ! tsdemux ! h264parse ! video/x-h264,stream-format=byte-stream,alignment=au ! rtph264pay ! udpsink host=127.0.0.1 port=5000
    # udpsrc port=5000 ! avdec_h264 ! videoconvert ! autovideosink \
    # interpipesrc listen-to=source_pipe ! rtph264pay ! udpsink host=localhost port=5000GstElement
