/*  
 *  Pipeline.cpp
 *
 *  ForkedSource creates a pipeline, which, for now, streams video from v4l2src
 *  (default video camera) and into a udpsink which redirects to the machine it
 *  was run on on ports 5000 and 5001.
 *
 *  FileSaver listens on port 5001 and saves the video streamed in to a ts file
 *
 *  Viewer listens on port 5000 and plays the video live on the screen.
 */

#include <filesystem>

#include "Pipeline.h"

Pipeline::Pipeline(Pipeline::PipelineType pt)
{
    // Initialize GStreamer
    gst_init(nullptr, nullptr);

    // Store potential errors here
    err = nullptr;

    std::string pipeline_str = "";

    // Set pipeline string
    switch (pt)
    {
        case ForkedSource:
            pipeline_str = "v4l2src device=/dev/video0 ! video/x-raw,width=640,height=480,framerate=30/1 ! "
                           "videoconvert ! x264enc ! video/x-h264,stream-format=byte-stream,alignment=au ! "
                           "rtph264pay ! multiudpsink clients=127.0.0.1:5000,127.0.0.1:5001";
            break;
        case FileSaver:
            pipeline_str = "udpsrc port=5001 caps=\"application/x-rtp,media=video,clock-rate=90000,encoding-name=H264,payload=96\" ! "
                           "rtph264depay ! video/x-h264,stream-format=byte-stream,alignment=au ! mpegtsmux ! filesink location="
                           + next_available_filepath();
            break;
        case Viewer:
            pipeline_str = "udpsrc port=5000 caps = \"application/x-rtp, media=(string)video, clock-rate=(int)90000, "
                           "encoding-name=(string)H264, payload=(int)96\" ! rtph264depay ! decodebin ! videoconvert ! autovideosink";
            break;
        default:
            g_print("ERROR: Invalid Pipeline object type\n");
            exit(EXIT_FAILURE);
    }

    // Start pipeline
    pipeline = gst_parse_launch(pipeline_str.c_str(), &err);

    if (err != NULL)
    {
        g_print("Could not construct pipeline: %s\n", err->message);
        g_error_free(err);
        exit(-1);
    }

    // Start actual pipeline
    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    
    // Save pipeline bus
    bus = gst_element_get_bus(pipeline);
}

Pipeline::~Pipeline()
{
    // We send an EOS to close each pipeline
    gst_element_send_event(pipeline, gst_event_new_eos());


    // Get the end of pipeline message
    msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE,
            static_cast<GstMessageType>(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));
    
    // Parse and print message
    if (msg != NULL)
    {
        GError *err;
        gchar *debug_info;

        switch (GST_MESSAGE_TYPE (msg))
        {
            case GST_MESSAGE_ERROR:
                gst_message_parse_error (msg, &err, &debug_info);
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
                // We should not reach here because we only asked for ERRORs and EOSs
                g_printerr("Unexpected message received.\n");
                break;
        }

        gst_message_unref(msg);
    }

    gst_element_set_state (pipeline, GST_STATE_NULL);

    // Free resources
    gst_object_unref(bus);
    gst_object_unref(pipeline);
}

std::string Pipeline::next_available_filepath() {
    int file_counter = 0;
    
    while (true) {		
        std::filesystem::path filepath(base_filepath
                + std::to_string(file_counter) + ".ts");

        if (!std::filesystem::exists(filepath)) {
            return filepath.string();
        }

        file_counter++;
    }
}
