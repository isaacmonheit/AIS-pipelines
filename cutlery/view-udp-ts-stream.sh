gst-launch-1.0 -v -e \
  udpsrc address=127.0.0.1 port=5000 caps='application/x-rtp,media=video,clock-rate=90000,encoding-name=MP2T' ! \
  rtpjitterbuffer latency=100 ! \
  rtpmp2tdepay ! \
  decodebin3 ! \
  videoconvert ! \
  autovideosink
