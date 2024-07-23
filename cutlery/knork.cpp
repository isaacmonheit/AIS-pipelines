#include <gst/gst.h>
#include <iostream>
#include <csignal>

void parseMessage(GstMessage *msg);
void signalHandler(int signum);

GstElement *pipeline0;
GstElement *pipeline1;
GstElement *pipeline2;

bool stopped = false;

int main(int argc, char *argv[])
{
    GstBus *bus0, *bus1, *bus2;
    GstMessage *msg0, *msg1, *msg2;
    GstStateChangeReturn ret;
  
    /* Signal handler to catch Ctrl-C */
    signal(SIGINT, signalHandler);
  
    /* Initialize GStreamer */
    gst_init(&argc, &argv);
  
    /* Create the empty pipelines */
    pipeline0 = gst_pipeline_new("pipeline0");
    pipeline1 = gst_pipeline_new("pipeline1");
    pipeline2 = gst_pipeline_new("pipeline2");
  
    /* Create the elements */
    GstElement *source = gst_element_factory_make("v4l2src", "source");
    GstElement *vidconvert = gst_element_factory_make("videoconvert", "vidconvert");
    GstElement *x264enc = gst_element_factory_make("x264enc", "encoder");
    GstElement *interpipesink = gst_element_factory_make("interpipesink", "interpipesink");
  
    GstElement *interpipesrc1 = gst_element_factory_make("interpipesrc", "interpipesrc1");
    GstElement *fakesink1 = gst_element_factory_make("fakesink", "fakesink1");
    GstElement *rtph264pay = gst_element_factory_make("rtph264pay", "rtph264pay");
    GstElement *udpsink = gst_element_factory_make("udpsink", "udpsink");
  
    GstElement *interpipesrc2 = gst_element_factory_make("interpipesrc", "interpipesrc2");
    GstElement *fakesink2 = gst_element_factory_make("fakesink", "fakesink2");
    GstElement *mpegtsmux = gst_element_factory_make("mpegtsmux", "mpegtsmux");
    GstElement *filesink = gst_element_factory_make("filesink", "file-output");
  
    /* Set caps */
    GstCaps *srccaps = gst_caps_new_simple("video/x-raw",
                                           "width", G_TYPE_INT, 640,
                                           "height", G_TYPE_INT, 480,
                                           "framerate", GST_TYPE_FRACTION, 30, 1,
                                           NULL);
  
    GstCaps *enccaps = gst_caps_new_simple("video/x-h264",
                                           "stream-format", G_TYPE_STRING, "byte-stream",
                                           "alignment", G_TYPE_STRING, "au",
                                           NULL);

    /* Ensure elements were created */
    if (!pipeline0 || !source || !srccaps || !vidconvert || !x264enc || !x264enc || !interpipesink)
    {
        g_printerr("Not all elements in pipeline0 could be created.\n");
        return -1;
    }
  
    std::cerr << "a\n";
    if (!pipeline1 || !interpipesrc1 || !rtph264pay || !udpsink)
    {
        g_printerr("Not all elements in pipeline1 could be created.\n");
        return -1;
    }
    std::cerr << "b\n";
    
    if (!pipeline2 || !interpipesrc2 || !mpegtsmux || !filesink)
    {
        g_printerr("Not all elements in pipeline2 could be created.\n");
        return -1;
    }
    
    /* Modify the elements' properties */
    g_object_set(source, "device", "/dev/video0", NULL);
    g_object_set(interpipesink, "name", "source_pipe",
                                "forward-eos", TRUE,
                                "forward-events", TRUE,
                                NULL);
  
    g_object_set(interpipesrc1, "listen-to", "source_pipe", NULL);
    g_object_set(udpsink, "host", "localhost",
                          "port", 5001,
                          NULL);
  
    g_object_set(interpipesrc2, "listen-to", "source_pipe",
                                "accept-events", TRUE,
                                "format", GST_FORMAT_TIME,
                                NULL);
    g_object_set(filesink, "location", "output.ts", NULL);

    /* Add elements to the pipeline */
    gst_bin_add_many(GST_BIN(pipeline0), source, vidconvert, x264enc, interpipesink, NULL);
    gst_bin_add_many(GST_BIN(pipeline1), interpipesrc1, fakesink1, rtph264pay, udpsink, NULL);
    gst_bin_add_many(GST_BIN(pipeline2), interpipesrc2, fakesink2, mpegtsmux, filesink, NULL);
  
    /* Link pipeline 0 */
    if (gst_element_link_filtered(source, vidconvert, srccaps) != TRUE) {
        g_printerr("source and interpipesink could not be linked.\n");
  
        gst_object_unref(pipeline0);
        gst_object_unref(pipeline1);
        gst_object_unref(pipeline2);
  
        gst_caps_unref(srccaps);
        gst_caps_unref(enccaps);
        return -1;
    }
    gst_caps_unref(srccaps);

    if (gst_element_link(vidconvert, x264enc) != TRUE) {
        g_printerr("vidconvert and x264enc could not be linked.\n");
  
        gst_object_unref(pipeline0);
        gst_object_unref(pipeline1);
        gst_object_unref(pipeline2);

        gst_caps_unref(enccaps);
        return -1;
    }
  
    if (gst_element_link_filtered(x264enc, interpipesink, enccaps) != TRUE) {
        g_printerr("x264enc and interpipesink could not be linked.\n");
  
        gst_object_unref(pipeline0);
        gst_object_unref(pipeline1);
        gst_object_unref(pipeline2);
  
        gst_caps_unref(enccaps);
        return -1;
    }
    gst_caps_unref(enccaps);
  
    /* Link pipeline 1 */
    if (gst_element_link_many(interpipesrc1, rtph264pay, udpsink, NULL) != TRUE) {
        g_printerr("interpipesrc1, rtph264pay, and udpsink could not be linked.\n");
  
        gst_object_unref(pipeline0);
        gst_object_unref(pipeline1);
        gst_object_unref(pipeline2);
  
        gst_caps_unref(enccaps);
        return -1;
    }
  
    /* Link pipeline 2 */
    if (gst_element_link(interpipesrc2, fakesink2) != TRUE) {
        g_printerr("interpipesrc2, mpegtsmux, and filesink could not be linked.\n");
  
        gst_object_unref(pipeline0);
        gst_object_unref(pipeline1);
        gst_object_unref(pipeline2);
  
        gst_caps_unref(enccaps);
        return -1;
    }
  
    /* Start playing */
    ret = gst_element_set_state(pipeline0, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Unable to set the pipeline to the playing state.\n");
  
        gst_object_unref(pipeline0);
        gst_object_unref(pipeline1);
        gst_object_unref(pipeline2);
  
        return -1;
    }
    ret = gst_element_set_state(pipeline1, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Unable to set the pipeline to the playing state.\n");
  
        gst_element_set_state(pipeline0, GST_STATE_NULL);
  
        gst_object_unref(pipeline0);
        gst_object_unref(pipeline1);
        gst_object_unref(pipeline2);
        return -1;
    }
    ret = gst_element_set_state(pipeline2, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Unable to set the pipeline to the playing state.\n");
  
        gst_element_set_state(pipeline0, GST_STATE_NULL);
        gst_element_set_state(pipeline1, GST_STATE_NULL);
  
        gst_object_unref(pipeline0);
        gst_object_unref(pipeline1);
        gst_object_unref(pipeline2);
        return -1;
    }
  
    /* Wait until error or EOS */
    bus0 = gst_element_get_bus(pipeline0);
    bus1 = gst_element_get_bus(pipeline1);
    bus2 = gst_element_get_bus(pipeline2);
  
    /* The order doesn't seem to matter */
    msg0 =
            gst_bus_timed_pop_filtered(bus0, GST_CLOCK_TIME_NONE,
            static_cast<GstMessageType>(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));
    msg1 =
            gst_bus_timed_pop_filtered(bus1, GST_CLOCK_TIME_NONE,
            static_cast<GstMessageType>(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));
    msg2 =
            gst_bus_timed_pop_filtered(bus2, GST_CLOCK_TIME_NONE,
            static_cast<GstMessageType>(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));
  
      
    parseMessage(msg0);
    parseMessage(msg1);
    parseMessage(msg2);
  
    /* Free resources */
    gst_object_unref(bus0);
    gst_object_unref(bus1);
    gst_object_unref(bus2);
  
    gst_element_set_state(pipeline0, GST_STATE_NULL);
    gst_element_set_state(pipeline1, GST_STATE_NULL);
    gst_element_set_state(pipeline2, GST_STATE_NULL);
  
    gst_object_unref(pipeline0);
    gst_object_unref(pipeline1);
    gst_object_unref(pipeline2);
  
    return 0;
}

void signalHandler(int signum) {
    if (not stopped)
    {
        stopped = true;
        gst_element_send_event(pipeline0, gst_event_new_eos());
    }
    else
    {
        exit(signum);
    }
}

void parseMessage(GstMessage *msg)
{
    if (msg != NULL) {
        GError *err;
        gchar *debug_info;
  
        switch (GST_MESSAGE_TYPE (msg)) {
            case GST_MESSAGE_ERROR:
                gst_message_parse_error(msg, &err, &debug_info);
                g_printerr("Error received from element %s: %s\n",
                        GST_OBJECT_NAME (msg->src), err->message);
                g_printerr("Debugging information: %s\n",
                        debug_info ? debug_info : "none");
                g_clear_error(&err);
                g_free(debug_info);
                break;
            case GST_MESSAGE_EOS:
                g_print("End-Of-Stream reached.\n");
                break;
            default:
                /* We should not reach here because we only asked for ERRORs and EOS */
                g_printerr("Unexpected message received.\n");
                break;
        }
        gst_message_unref(msg);
    }
}
