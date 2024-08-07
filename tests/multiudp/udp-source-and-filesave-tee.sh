#!/bin/bash

gst-launch-1.0 -v -e \
    v4l2src device=/dev/video0 ! \
    video/x-raw,width=640,height=480,framerate=30/1 ! \
    videoconvert ! \
    x264enc ! \
    video/x-h264,stream-format=byte-stream,alignment=au ! \
    tee name=t ! \
    queue ! \
    rtph264pay ! \
    udpsink host=127.0.0.1 port=5000 \
    t. ! \
    queue ! \
    mpegtsmux ! \
    filesink location=output.ts
