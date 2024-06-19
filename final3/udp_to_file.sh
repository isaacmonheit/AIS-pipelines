#!/bin/bash

gst-launch-1.0 -v -e \
    udpsrc port=5001 caps="application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264, payload=(int)96" ! \
    rtph264depay ! h264parse ! mpegtsmux ! filesink location=output.mp4
