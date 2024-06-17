gst-launch-1.0 -v -e \
    v4l2src device=/dev/video0 ! video/x-raw, width=640, height=480, framerate=30/1 ! interpipesink name=source_pipe sync=false async=false \
    interpipesrc listen-to=source_pipe is-live=true format=time ! queue ! videoconvert ! x264enc ! h264parse ! mp4mux ! filesink location=output.mp4 \
    interpipesrc listen-to=source_pipe is-live=true format=time ! queue ! videoconvert ! x264enc ! h264parse ! rtph264pay ! udpsink host=127.0.0.1 port=5000