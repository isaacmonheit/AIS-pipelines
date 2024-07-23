#!/bin/bash

# THIS IS BROKEN !!

gst-launch-1.0 -v -e \
    v4l2src device=/dev/video0 ! \
    video/x-raw,width=640,height=480,framerate=30/1 ! \
    videoconvert ! \
    x264enc ! \
    video/x-h264,stream-format=byte-stream,alignment=au ! \
    interpipesink name=source_pipe sync=false async=false forward-eos=true forward-events=true &

gst-launch-1.0 -v -e \
    interpipesrc listen-to=source_pipe accept-events=true format=time ! \
    rtph264pay ! \
    udpsink host=127.0.0.1 port=5000 &

gst-launch-1.0 -v -e \
    interpipesrc listen-to=source_pipe accept-events=true format=time ! \
    mpegtsmux ! \
    filesink location=output.ts
