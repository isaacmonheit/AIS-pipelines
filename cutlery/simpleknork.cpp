#include <gst/gst.h>

void signalHandler(int signum);

GstElement *pipeline;

bool stopped = false;

int main(int argc, char *argv[]) {
    GstBus *bus;
    GstMessage *msg;
    GstStateChangeReturn ret;

    /* Signal handler to catch Ctrl-C */
    signal(SIGINT, signalHandler);
  
    gst_init(&argc, &argv);

    pipeline = gst_pipeline_new("video-streamer");

    GstElement *source = gst_element_factory_make("v4l2src", "source");
    GstElement *capsfilter = gst_element_factory_make("capsfilter", "capsfilter");
    GstElement *videoconvert = gst_element_factory_make("videoconvert", "videoconvert");
    GstElement *encoder = gst_element_factory_make("x264enc", "encoder");
    GstElement *h264filter = gst_element_factory_make("capsfilter", "h264filter");
    GstElement *tee = gst_element_factory_make("tee", "tee");
    GstElement *queue1 = gst_element_factory_make("queue", "queue1");
    GstElement *rtph264pay = gst_element_factory_make("rtph264pay", "rtph264pay");
    GstElement *udpsink = gst_element_factory_make("udpsink", "udpsink");
    GstElement *queue2 = gst_element_factory_make("queue", "queue2");
    GstElement *mpegtsmux = gst_element_factory_make("mpegtsmux", "mpegtsmux");
    GstElement *filesink = gst_element_factory_make("filesink", "filesink");

    if (!pipeline || !source || !capsfilter || !videoconvert || !encoder || !h264filter || !tee ||
        !queue1 || !rtph264pay || !udpsink || !queue2 || !mpegtsmux || !filesink) {
        g_printerr("Not all elements could be created.\n");
        return -1;
    }

    g_object_set(source, "device", "/dev/video0", NULL);

    GstCaps *caps = gst_caps_new_simple("video/x-raw",
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
    msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, static_cast<GstMessageType>(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

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
            default:
                g_printerr("Unexpected message received.\n");
                break;
        }
        gst_message_unref(msg);
    }

    gst_object_unref(bus);
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    return 0;
}

void signalHandler(int signum) {
    if (not stopped)
    {
        stopped = true;
        gst_element_send_event(pipeline, gst_event_new_eos());
    }
    else
    {
        exit(signum);
    }
}
