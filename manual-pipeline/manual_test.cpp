#include <gst/gst.h>

#include <iostream>

int main(int argc, char *argv[])
{
  GstElement *pipeline;
  GstBus *bus;
  GstMessage *msg;
  GstStateChangeReturn ret;

  /* Initialize GStreamer */
  gst_init (&argc, &argv);

  /* Create the empty pipeline */
  pipeline = gst_pipeline_new ("test-pipeline");

  /* Create the elements */
  GstElement *source = gst_element_factory_make("v4l2src", "source");
  GstElement *interpipesink = gst_element_factory_make("interpipesink", "interpipesink");

  GstElement *interpipesrc1 = gst_element_factory_make("interpipesrc", "interpipesrc1");
  GstElement *vidconvert1 = gst_element_factory_make("videoconvert", "vidconvert1");
  GstElement *avsink = gst_element_factory_make ("autovideosink", "avsink");

  //GstElement *interpipesrc2 = gst_element_factory_make("interpipesrc", "interpipesrc2");
  //GstElement *vidconvert2 = gst_element_factory_make("videoconvert", "vidconvert2");
  //GstElement *x264enc = gst_element_factory_make("x264enc", "encoder");
  //GstElement *filesink = gst_element_factory_make("filesink", "file-output");

  /* Set caps */
  GstCaps *caps = gst_caps_new_simple("video/x-raw",
                                      "width", G_TYPE_INT, 640,
                                      "height", G_TYPE_INT, 480,
                                      "framerate", GST_TYPE_FRACTION, 30, 1,
                                      NULL);

  /* Ensure elements were created */
  if (!pipeline || !source || !caps || !interpipesink || !interpipesrc1 || !vidconvert1 || !avsink) //!vidconvert2 || !x264enc || !filesink)
  {
    g_printerr ("Not all elements could be created.\n");
    return -1;
  }

  std::cerr << "0\n";
  
  /* Modify the elements' properties */
  g_object_set(source, "device", "/dev/video0", NULL);

  //g_object_set(filesink, "location", "output.h264", NULL);

  g_object_set(interpipesink, "name", "source_pipe", NULL);
  g_object_set(interpipesrc1, "listen-to", "source_pipe", NULL);
  //g_object_set(interpipesrc2, "listen-to", "source_pipe", NULL);

  std::cerr << "1\n";

  /* Add elements to the pipeline */
  gst_bin_add_many(GST_BIN(pipeline), source, interpipesink, interpipesrc1, vidconvert1, avsink, NULL);
                   //interpipesrc2, vidconvert2, x264enc, filesink, NULL);

  std::cerr << "2\n";

  /* Link elements */
  if (gst_element_link_filtered(source, interpipesink, caps) != TRUE) {
    g_printerr ("source and interpipesink could not be linked.\n");
    gst_object_unref (pipeline);
    gst_caps_unref(caps);
    return -1;
  }

  gst_caps_unref(caps);
  std::cerr << "2.5\n";

  if (gst_element_link_many(interpipesrc1, vidconvert1, avsink, NULL) != TRUE) {
    g_printerr("interpipesrc1, vidconvert1, and avsink could not be linked.\n");
    gst_object_unref (pipeline);
    return -1;
  }

  std::cerr << "3\n";

  /* Start playing */
  ret = gst_element_set_state (pipeline, GST_STATE_PLAYING);
  if (ret == GST_STATE_CHANGE_FAILURE) {
    g_printerr ("Unable to set the pipeline to the playing state.\n");
    gst_object_unref (pipeline);
    return -1;
  }

  std::cerr << "4\n";

  /* Wait until error or EOS */
  bus = gst_element_get_bus (pipeline);

  std::cerr << "5\n";

  msg =
      gst_bus_timed_pop_filtered (bus, GST_CLOCK_TIME_NONE,
      static_cast<GstMessageType>(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

  std::cerr << "7\n";

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

  std::cerr << "8\n";

  /* Free resources */
  gst_object_unref (bus);
  gst_element_set_state (pipeline, GST_STATE_NULL);
  gst_object_unref (pipeline);
  return 0;
}
