#include <gst/gst.h>
#include <stdio.h>

int main (int argc, char *argv[])
{
    GstElement *pipeline;
    GstBus *bus;
    GstMessage *msg;

    /* Initialize GStreamer */
    gst_init (&argc, &argv);

    /* Create pipeline */
    GstElement *pipeline = gst_pipeline_new("video-pipeline");

    /* Create elements */
    GstElement *v4l2src = gst_element_factory_make("v4l2src", "source");
    GstElement *videoconvert = gst_element_factory_make("videoconvert", "convert");
    GstElement *x264enc = gst_element_factory_make("x264enc", "encoder");
    GstElement *interpipesink = gst_element_factory_make("interpipesink", "interpipesink");

    GstElement *interpipesrc1 = gst_element_factory_make("interpipesrc", "interpipesrc1");
    GstElement *filesink = gst_element_factory_make("filesink", "file-output");

    GstElement *interpipesrc2 = gst_element_factory_make("interpipesrc", "interpipesrc2");
    GstElement *rtph264pay = gst_element_factory_make("rtph264pay", "payloader");
    GstElement *udpsink = gst_element_factory_make("udpsink", "udp-output");

    if (!pipeline || !v4l2src || !videoconvert || !x264enc || !interpipesink ||
        !interpipesrc1 || !filesink || !interpipesrc2 || !rtph264pay || !udpsink)
    {
        g_printerr("Not all elements could be created.\n");
        return -1;
    }

    /* Set caps */
    GstCaps *caps = gst_caps_new_simple("video/x-raw",
                                        "width", G_TYPE_INT, 640,
                                        "height", G_TYPE_INT, 480,
                                        "framerate", GST_TYPE_FRACTION, 30, 1,
                                        NULL);

    /* Set properties */
    g_object_set(v4l2src, "device", "/dev/video0", NULL);
    g_object_set(filesink, "location", "output.h264", NULL);
    g_object_set(udpsink, "host", "localhost", "port", 5000, NULL);

    g_object_set(interpipesink, "name", "source_pipe", NULL);
    g_object_set(interpipesrc1, "listen-to", "source_pipe", NULL);
    g_object_set(interpipesrc2, "listen-to", "source_pipe", NULL);

    /* Add elements to the pipeline */
    // add caps too?
    gst_bin_add_many(GST_BIN(pipeline), v4l2src, videoconvert, x264enc, interpipesink,
                     interpipesrc1, filesink, interpipesrc2, rtph264pay, udpsink, NULL);

    /* Link elements */
    if (!gst_element_link_filtered(v4l2src, videoconvert, caps))
    {
        g_printerr("Failed to link v4l2src and videoconvert with caps.\n");
        gst_object_unref(pipeline);
        return -1;
    }
    gst_caps_unref(caps);

    if (!gst_element_link_many(videoconvert, x264enc, interpipesink, NULL))
    {
        g_printerr("Failed to link videoconvert, x264enc, and interpipesink.\n");
        gst_object_unref(pipeline);
        return -1;
    }

    if (!gst_element_link(interpipesrc1, filesink))
    {
        g_printerr("Failed to link interpipesrc1 and filesink.\n");
        gst_object_unref(pipeline);
        return -1;
    }

    if (!gst_element_link_many(interpipesrc2, rtph264pay, udpsink, NULL))
    {
        g_printerr("Failed to link interpipesrc2, rtph264pay, and udpsink.\n");
        gst_object_unref(pipeline);
        return -1;
    }

    /* Start playing */
    GstStateChangeReturn *ret = gst_element_set_state (pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        g_printerr ("Unable to set the pipeline to the playing state.\n");
        gst_object_unref (pipeline);
        return -1;
    }

    /* Wait until error or EOS */
    bus = gst_element_get_bus (pipeline);
    msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE,
            static_cast<GstMessageType>(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

    /* Parse message */
    if (msg != NULL)
    {
        GError *err;
        gchar *debug_info;

        switch (GST_MESSAGE_TYPE (msg)) {
            case GST_MESSAGE_ERROR:
                gst_message_parse_error (msg, &err, &debug_info);
                g_printerr ("Error received from element %s: %s\n",
                        GST_OBJECT_NAME (msg->src), err->message);
                g_printerr ("Debugging information: %s\n",
                        debug_info ? debug_info : "none");
                g_clear_error (&err);
                g_free (debug_info);
                break;
            case GST_MESSAGE_EOS:
                g_print ("End-Of-Stream reached.\n");
                break;
            default:
                /* We should not reach here because we only asked for ERRORs and EOS */
                g_printerr ("Unexpected message received.\n");
                break;
        }
        gst_message_unref (msg);
    }

    /* Free resources */
    gst_message_unref (msg);
    gst_object_unref (bus);
    gst_element_set_state (pipeline, GST_STATE_NULL);
    gst_object_unref (pipeline);
    return 0;
}
