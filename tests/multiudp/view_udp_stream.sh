#!/bin/bash

# Used in order to view UDP Stream sent from forked_pipeline_demo.sh

### To run this script ###
#1. Install gstreamer
#2. Ensure that port number is set correctly
#4. Run 'chmod +x view_udp_stream.sh'
#5. Run './view_udp_stream.sh'

gst-launch-1.0 -v \
    udpsrc port=5000 caps="application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264, payload=(int)96" ! \
    rtph264depay ! decodebin ! videoconvert ! autovideosink
