gst-launch-1.0 -v -e \
    v4l2src device=/dev/video0 ! \
    video/x-raw, width=640, height=480, framerate=30/1 ! \
    videoconvert ! \
    x264enc ! \
    mpegtsmux ! \
    filesink location=output.ts
