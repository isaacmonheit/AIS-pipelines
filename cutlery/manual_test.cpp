#include <gst/gst.h>

#include <iostream>

int main(int argc, char *argv[])
{
  GstElement *srcpipeline, *fspipeline;
  GstBus *srcbus, *fsbus;
  GstMessage *msg;
  GstStateChangeReturn ret;

  /* Initialize GStreamer */
  gst_init (&argc, &argv);

  /* Create the empty pipelines */
  srcpipeline = gst_pipeline_new ("src-pipeline");
  //avpipeline = gst_pipeline_new ("av-pipeline");
  fspipeline = gst_pipeline_new ("fs-pipeline");

  /* Create the elements */
  GstElement *source = gst_element_factory_make("v4l2src", "source");
  GstElement *interpipesink = gst_element_factory_make("interpipesink", "interpipesink");

  //GstElement *interpipesrc1 = gst_element_factory_make("interpipesrc", "interpipesrc1");
  //GstElement *vidconvert1 = gst_element_factory_make("videoconvert", "vidconvert1");
  //GstElement *avsink = gst_element_factory_make ("autovideosink", "avsink");

  GstElement *interpipesrc2 = gst_element_factory_make("interpipesrc", "interpipesrc2");
  GstElement *vidconvert2 = gst_element_factory_make("videoconvert", "vidconvert2");
  GstElement *x264enc = gst_element_factory_make("x264enc", "encoder");
  GstElement *filesink = gst_element_factory_make("filesink", "file-output");

  /* Set caps */
  GstCaps *caps = gst_caps_new_simple("video/x-raw",
                                      "width", G_TYPE_INT, 640,
                                      "height", G_TYPE_INT, 480,
                                      "framerate", GST_TYPE_FRACTION, 30, 1,
                                      NULL);

  /* Ensure elements were created */
  if (!srcpipeline || !source || !caps || !interpipesink)
  {
    g_printerr("Not all elements in srcpipeline could be created.\n");
    return -1;
  }

  //if (!interpipesrc1 || !vidconvert1 || !avsink)
  //{
  //  g_printerr("Not all elements in avpipeline could be created.\n");
  //  return -1;
  //}
  
  if (!interpipesrc2 || !vidconvert2 || !x264enc || !filesink)
  {
    g_printerr("Not all elements in fspipeline could be created.\n");
    return -1;
  }
  
  /* Modify the elements' properties */
  g_object_set(source, "device", "/dev/video0", NULL);

  g_object_set(filesink, "location", "output.h264", NULL);

  g_object_set(interpipesink, "name", "source_pipe", NULL);
  //g_object_set(interpipesrc1, "listen-to", "source_pipe", NULL);
  g_object_set(interpipesrc2, "listen-to", "source_pipe", NULL);

  /* Add elements to pipelines */
  gst_bin_add_many(GST_BIN(srcpipeline), source, interpipesink, NULL);
  //gst_bin_add_many(GST_BIN(avpipeline), interpipesrc1, vidconvert1, avsink, NULL);
  gst_bin_add_many(GST_BIN(fspipeline), interpipesrc2, vidconvert2, x264enc, filesink, NULL);

  /* Link elements in srcpipeline */
  if (gst_element_link_filtered(source, interpipesink, caps) != TRUE) {
    g_printerr ("source and interpipesink could not be linked.\n");
    gst_object_unref (srcpipeline);
    //gst_object_unref (avpipeline);
    gst_object_unref (fspipeline);
    gst_caps_unref(caps);
    return -1;
  }

  gst_caps_unref(caps);

  /* Link elements in avpipeline */
  //if (gst_element_link_many(interpipesrc1, vidconvert1, avsink, NULL) != TRUE) {
  //  g_printerr("interpipesrc1, vidconvert1, and avsink could not be linked.\n");
  //  gst_object_unref (srcpipeline);
  //  gst_object_unref (avpipeline);
  //  gst_object_unref (fspipeline);
  //  return -1;
  //}

  /* Link elements in fspipeline */
  if (gst_element_link_many(interpipesrc2, vidconvert2, x264enc, filesink, NULL) != TRUE) {
    g_printerr("interpipesrc2, vidconvert2, x264enc, and filesink could not be linked.\n");
    gst_object_unref (srcpipeline);
    //gst_object_unref (avpipeline);
    gst_object_unref (fspipeline);
    return -1;
  }

  /* Start playing */
  ret = gst_element_set_state (srcpipeline, GST_STATE_PLAYING);
  if (ret == GST_STATE_CHANGE_FAILURE) {
    g_printerr ("Unable to set the srcpipeline to the playing state.\n");
    gst_object_unref (srcpipeline);
    //gst_object_unref (avpipeline);
    gst_object_unref (fspipeline);
    return -1;
  }
  //ret = gst_element_set_state (avpipeline, GST_STATE_PLAYING);
  //if (ret == GST_STATE_CHANGE_FAILURE) {
  //  g_printerr ("Unable to set the avpipeline to the playing state.\n");
  //  gst_object_unref (srcpipeline);
  //  gst_object_unref (avpipeline);
  //  gst_object_unref (fspipeline);
  //  return -1;
  //}
  ret = gst_element_set_state (fspipeline, GST_STATE_PLAYING);
  if (ret == GST_STATE_CHANGE_FAILURE) {
    g_printerr ("Unable to set the fspipeline to the playing state.\n");
    gst_object_unref (srcpipeline);
    //gst_object_unref (avpipeline);
    gst_object_unref (fspipeline);
    return -1;
  }

  /* Wait until error or EOS */
  srcbus = gst_element_get_bus(srcpipeline);
  //avbus = gst_element_get_bus(avpipeline);
  fsbus = gst_element_get_bus(fspipeline);

  std::cerr << "5\n";

  while (true)
  {
    msg = gst_bus_timed_pop_filtered(srcbus, 100 * GST_MSECOND, static_cast<GstMessageType>(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));
    //if (!msg)
    //  msg = gst_bus_timed_pop_filtered(avbus, 100 * GST_MSECOND, static_cast<GstMessageType>(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));
    if (!msg)
      msg = gst_bus_timed_pop_filtered(fsbus, 100 * GST_MSECOND, static_cast<GstMessageType>(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

    if (msg != NULL)
    {
      GError *err;
      gchar *debug_info;

      switch (GST_MESSAGE_TYPE(msg))
      {
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
      break;
    }
  }

  /* Free resources */
  gst_object_unref(srcbus);
  //gst_object_unref(avbus);
  gst_object_unref(fsbus);

  gst_element_set_state(srcpipeline, GST_STATE_NULL);
  //gst_element_set_state(avpipeline, GST_STATE_NULL);
  gst_element_set_state(fspipeline, GST_STATE_NULL);

  gst_object_unref(srcpipeline);
  //gst_object_unref(avpipeline);
  gst_object_unref(fspipeline);

  return 0;
}
