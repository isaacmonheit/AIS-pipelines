#include <gst/gst.h>
#include <iostream>
#include <csignal>

GstElement *pipeline;

void signalHandler(int signum) {
    gst_element_send_event(pipeline, gst_event_new_eos());
}

int main (int argc, char *argv[])
{

    GstBus *bus;
    GstMessage *msg;

    /* Signal handler to catch Ctrl-C */
    signal(SIGINT, signalHandler);


    /* Initialize GStreamer */
    gst_init (&argc, &argv);

    /* Build the pipeline */
    pipeline =
        gst_parse_launch(
                "v4l2src device=/dev/video0 "
                "! video/x-raw,width=640,height=480,framerate=30/1 "
                "! videoconvert "
                "! x264enc "
                //"! video/x-h264,stream-format=byte-stream "
                "! mp4mux "
                "! filesink location=output.mp4",
                NULL);

    /* Start playing */
    gst_element_set_state (pipeline, GST_STATE_PLAYING);

    /* Wait until error or EOS */
    bus = gst_element_get_bus (pipeline);
    msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE,
            static_cast<GstMessageType>(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

    /* Parse message */
    if (msg != NULL) {
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
    std::cout << "1\n";
    gst_object_unref (bus);
    std::cout << "3\n";
    gst_element_set_state (pipeline, GST_STATE_NULL);
    std::cout << "4\n";
    gst_object_unref (pipeline);
    std::cout << "5\n";

    return 0;
}
