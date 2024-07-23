#include <gst/gst.h>
#include <iostream>

int main(int argc, char *argv[]) {
    GstElement *pipeline, *source, *capsfilter, *videoconvert, *encoder, *h264filter, *tee;
    GstElement *queue1, *rtph264pay, *udpsink, *queue2, *mpegtsmux, *filesink;
    GstCaps *caps;
    GstBus *bus;
    GstMessage *msg;
    GstStateChangeReturn ret;

    gst_init(&argc, &argv);

    pipeline = gst_pipeline_new("video-streamer");

    source = gst_element_factory_make("v4l2src", "source");
    capsfilter = gst_element_factory_make("capsfilter", "capsfilter");
    videoconvert = gst_element_factory_make("videoconvert", "videoconvert");
    encoder = gst_element_factory_make("x264enc", "encoder");
    h264filter = gst_element_factory_make("capsfilter", "h264filter");
    tee = gst_element_factory_make("tee", "tee");
    queue1 = gst_element_factory_make("queue", "queue1");
    rtph264pay = gst_element_factory_make("rtph264pay", "rtph264pay");
    udpsink = gst_element_factory_make("udpsink", "udpsink");
    queue2 = gst_element_factory_make("queue", "queue2");
    mpegtsmux = gst_element_factory_make("mpegtsmux", "mpegtsmux");
    filesink = gst_element_factory_make("filesink", "filesink");

    if (!pipeline || !source || !capsfilter || !videoconvert || !encoder || !h264filter || !tee ||
        !queue1 || !rtph264pay || !udpsink || !queue2 || !mpegtsmux || !filesink) {
        g_printerr("Not all elements could be created.\n");
        return -1;
    }

    g_object_set(source, "device", "/dev/video0", NULL);

    caps = gst_caps_new_simple("video/x-raw",
                               "width", G_TYPE_INT, 640,
                               "height", G_TYPE_INT, 480,
                               "framerate", GST_TYPE_FRACTION, 30, 1,
                               NULL);
    g_object_set(capsfilter, "caps", caps, NULL);
    gst_caps_unref(caps);

    caps = gst_caps_new_simple("video/x-h264",
                               "stream-format", G_TYPE_STRING, "byte-stream",
                               "alignment", G_TYPE_STRING, "au",
                               NULL);
    g_object_set(h264filter, "caps", caps, NULL);
    gst_caps_unref(caps);

    g_object_set(udpsink, "host", "127.0.0.1", "port", 5000, NULL);
    g_object_set(filesink, "location", "output.ts", NULL);

    gst_bin_add_many(GST_BIN(pipeline), source, capsfilter, videoconvert, encoder, h264filter, tee, queue1, rtph264pay, udpsink, queue2, mpegtsmux, filesink, NULL);
    if (!gst_element_link_many(source, capsfilter, videoconvert, encoder, h264filter, tee, NULL) ||
        !gst_element_link_many(queue1, rtph264pay, udpsink, NULL) ||
        !gst_element_link_many(queue2, mpegtsmux, filesink, NULL)) {
        g_printerr("Elements could not be linked.\n");
        gst_object_unref(pipeline);
        return -1;
    }

    gst_element_link(tee, queue1);
    gst_element_link(tee, queue2);

    ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Unable to set the pipeline to the playing state.\n");
        gst_object_unref(pipeline);
        return -1;
    }

    bus = gst_element_get_bus(pipeline);
    while (true) {
        msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, static_cast<GstMessageType>(GST_MESSAGE_ERROR | GST_MESSAGE_EOS | GST_MESSAGE_STATE_CHANGED));

        if (msg != NULL) {
            GError *err;
            gchar *debug_info;

            switch (GST_MESSAGE_TYPE(msg)) {
                case GST_MESSAGE_ERROR:
                    gst_message_parse_error(msg, &err, &debug_info);
                    g_printerr("Error received from element %s: %s\n", GST_OBJECT_NAME(msg->src), err->message);
                    g_printerr("Debugging information: %s\n", debug_info ? debug_info : "none");
                    g_clear_error(&err);
                    g_free(debug_info);
                    break;
                case GST_MESSAGE_EOS:
                    g_print("End-Of-Stream reached.\n");
                    break;
                case GST_MESSAGE_STATE_CHANGED:
                    if (GST_MESSAGE_SRC(msg) == GST_OBJECT(pipeline)) {
                        GstState old_state, new_state, pending_state;
                        gst_message_parse_state_changed(msg, &old_state, &new_state, &pending_state);
                        g_print("Pipeline state changed from %s to %s:\n",
                                gst_element_state_get_name(old_state), gst_element_state_get_name(new_state));
                    }
                    break;
                default:
                    g_printerr("Unexpected message received.\n");
                    break;
            }
            gst_message_unref(msg);
        }
    }

    gst_object_unref(bus);
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    return 0;
}

