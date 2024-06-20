#!/bin/bash

# This file saves the streamed video as a .ts file

gst-launch-1.0 -v -e \
    udpsrc port=5001 caps="application/x-rtp,media=video,clock-rate=90000,encoding-name=H264,payload=96" ! \
    rtph264depay ! video/x-h264,stream-format=byte-stream,alignment=au ! mpegtsmux ! filesink location=output.ts
