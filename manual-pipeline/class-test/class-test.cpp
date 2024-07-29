#include <gst/gst.h>
#include <iostream>
#include <csignal>

void signalHandler(int signum);
gboolean busCallback(GstBus *bus, GstMessage *msg, gpointer data);
void linkElements(GstElement* element,GstPad* sourcePad, gpointer sinkElement);

GMainLoop *loop;

GstElement *pipeline0;
GstElement *pipeline1;

bool stopped = false;

int main(int argc, char *argv[])
{
    GstBus *bus;
    GstStateChangeReturn ret;

    guint bwid0, bwid1;
  
    /* Signal handler to catch Ctrl-C */
    signal(SIGINT, signalHandler);
  
    /* Initialize GStreamer */
    gst_init(&argc, &argv);
  
    /* Create the empty pipelines */
    pipeline0 = gst_pipeline_new("pipeline0");
    pipeline1 = gst_pipeline_new("pipeline1");
  
    /* Create the elements */
    GstElement *source = gst_element_factory_make("v4l2src", "source");
    GstElement *interpipesink = gst_element_factory_make("interpipesink", "interpipesink");
  
    GstElement *interpipesrc1 = gst_element_factory_make("interpipesrc", "interpipesrc1");
    GstElement *vidconvert = gst_element_factory_make("videoconvert", "vidconvert");
    GstElement *avsink = gst_element_factory_make("autovideosink", "avsink");
  
    /* Set caps */
    GstCaps *srccaps = gst_caps_new_simple("video/x-raw",
                                           "width", G_TYPE_INT, 640,
                                           "height", G_TYPE_INT, 480,
                                           "framerate", GST_TYPE_FRACTION, 30, 1,
                                           NULL);

    /* Ensure elements were created */
    if (!pipeline0 || !source || !srccaps || !interpipesink)
    {
        g_printerr("Not all elements in pipeline0 could be created.\n");
        return -1;
    }
  
    if (!pipeline1 || !interpipesrc1 || !vidconvert || !avsink)
    {
        g_printerr("Not all elements in pipeline1 could be created.\n");
        std::cerr << (bool) pipeline1 << (bool) interpipesrc1 << (bool) vidconvert << (bool) avsink << "\n";
        return -1;
    }
    
    /* Modify the elements' properties */
    g_object_set(source, "device", "/dev/video0", NULL);
    g_object_set(interpipesink, "name", "source_pipe",
                                "forward-eos", TRUE,
                                "forward-events", TRUE,
                                NULL);
  
    g_object_set(interpipesrc1, "listen-to", "source_pipe", NULL);

    /* Add elements to the pipeline */
    gst_bin_add_many(GST_BIN(pipeline0), source, interpipesink, NULL);
    gst_bin_add_many(GST_BIN(pipeline1), interpipesrc1, vidconvert, avsink, NULL);
  
    /* Link pipeline 0 */
    if (gst_element_link_filtered(source, interpipesink, srccaps) != TRUE) {
        g_printerr("source and interpipesink could not be linked.\n");
  
        gst_object_unref(pipeline0);
        gst_object_unref(pipeline1);
  
        gst_caps_unref(srccaps);
        return -1;
    }
    gst_caps_unref(srccaps);

    /* Link pipeline 1 */
    if (gst_element_link_many(interpipesrc1, vidconvert, avsink, NULL) != TRUE) {
        g_printerr("interpipesrc1, vidconvert, and avsink could not be linked.\n");
  
        gst_object_unref(pipeline0);
        gst_object_unref(pipeline1);
        return -1;
    }


    /* Start playing */
    ret = gst_element_set_state(pipeline0, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Unable to set the pipeline to the playing state.\n");
  
        gst_object_unref(pipeline0);
        gst_object_unref(pipeline1);
  
        return -1;
    }
    ret = gst_element_set_state(pipeline1, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Unable to set the pipeline to the playing state.\n");
  
        gst_element_set_state(pipeline0, GST_STATE_NULL);
  
        gst_object_unref(pipeline0);
        gst_object_unref(pipeline1);
        return -1;
    }

    /* Get each pipeline's bus */
    bus = gst_element_get_bus(pipeline0);
    bwid0 = gst_bus_add_watch(bus, busCallback, NULL);

    bus = gst_element_get_bus(pipeline1);
    bwid1 = gst_bus_add_watch(bus, busCallback, NULL);

    gst_object_unref(bus);
  
    /* Create and begin running a main loop with each bus callback */
    loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(loop);
      

    /* Clean up */
    gst_element_set_state(pipeline0, GST_STATE_NULL);
    gst_element_set_state(pipeline1, GST_STATE_NULL);
  
    gst_object_unref(pipeline0);
    gst_object_unref(pipeline1);

    g_source_remove(bwid0);
    g_source_remove(bwid1);

    g_main_loop_unref(loop);
  
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

gboolean busCallback(GstBus *bus, GstMessage *msg, gpointer data)
{
    //g_print("Got %s message\n", GST_MESSAGE_TYPE_NAME (msg));

    switch (GST_MESSAGE_TYPE (msg))
    {
        case GST_MESSAGE_ERROR:
            GError *err;
            gchar *debug_info;
  
            gst_message_parse_error(msg, &err, &debug_info);
            g_printerr("Error received from element %s: %s\n",
                    GST_OBJECT_NAME (msg->src), err->message);
            g_printerr("Debugging information: %s\n",
                    debug_info ? debug_info : "none");

            g_clear_error(&err);

            g_error_free(err);
            g_free(debug_info);

            g_main_loop_quit(loop);

            break;

        case GST_MESSAGE_EOS:
            g_print("End-Of-Stream reached.\n");

            g_main_loop_quit(loop);
            // TODO: Get the source of the message; if it's pipeline0, tell
            // the main loop to stop. Otherwise, just chill out.
            // We could perhaps do this with msg-src, but there might be a
            // better way
            // TODO: Wait until all elements have received the EOS to end the
            // main loop
            break;

        default:
            // Nothing should happen here... I might add functionality later
            // for debugging though.
            break;
    }

    return TRUE;
}

void linkElements(GstElement* element, GstPad* sourcePad, gpointer sinkElement)
{
    GstPad* sinkPad = gst_element_get_static_pad(GST_ELEMENT(sinkElement), "sink");
    if (!sinkPad) {
        g_printerr("Sink pad could not be obtained.\n");
        return;
    }

    GstPadLinkReturn ret = gst_pad_link(sourcePad, sinkPad);
    gst_object_unref(sinkPad);

    std::cerr << "Link here pineapple\n";

    if (ret != GST_PAD_LINK_OK) {
        g_printerr("Pad linking failed.\n");
    } else {
        g_print("Pad linked successfully.\n");
    }
}
