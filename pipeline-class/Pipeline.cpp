#include "Pipeline.h"

Pipeline::Pipeline(Pipeline::PipelineType pt)
{
    // Initialize GStreamer
    gst_init (nullptr, nullptr);

    // Init err as nullptr just to be sure
    err = nullptr;

    // Build the pipeline
    switch (pt)
    {
        case Fork:
            std::cerr << "Squeeze me like the Kennedys\n";
            pipeline =
                gst_parse_launch("v4l2src device=/dev/video0 ! video/x-raw,width=640,height=480,framerate=30/1 ! "
                        "videoconvert ! x264enc ! video/x-h264,stream-format=byte-stream,alignment=au ! "
                        "rtph264pay ! multiudpsink clients=127.0.0.1:5000,127.0.0.1:5001",
                        &err);
            std::cerr << "Call me the flower twice\n";
            break;
        case Knife:
            pipeline =
                gst_parse_launch("udpsrc port=5001 caps=\"application/x-rtp,media=video,clock-rate=90000,encoding-name=H264,payload=96\" ! "
                        "rtph264depay ! video/x-h264,stream-format=byte-stream,alignment=au ! mpegtsmux ! filesink location=output.ts", 
                        &err);
            break;
        case Spoon:
            pipeline =
                gst_parse_launch("udpsrc port=5000 caps = \"application/x-rtp, media=(string)video, clock-rate=(int)90000, "
                        "encoding-name=(string)H264, payload=(int)96\" ! rtph264depay ! decodebin ! videoconvert ! autovideosink",
                        &err);
            break;
        default:
            std::cerr << "You should not be here. (Pipeline type invalid)" << std::endl;
            exit(EXIT_FAILURE);
    }

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
    // Actually stop the pipeline
    gst_element_set_state (pipeline, GST_STATE_NULL);
    gst_element_send_event(pipeline, gst_event_new_eos());


    // Get that end of pipeline message, girl!
    msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE,
            static_cast<GstMessageType>(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));
    std::cout << "Fries are better than marshmallows\n";
    
    
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
        std::cout << "Fries are better than marshmallows\n";

        gst_message_unref(msg);
    }


    std::cout << "don't hurt tod!\n";

    // Free resources
    gst_message_unref(msg);
    gst_object_unref(bus);
    gst_object_unref(pipeline);
}

void Pipeline::signalHandler(int signum) {
    std::cout << "\nInterrupt signal (" << signum << ") received.\n";

    // Clean up and close up stuff here
    // fizz buzz

    // Terminate program
    std::exit(signum);
}