#include <gst/gst.h>
#include <stdio.h>
#include <iostream>
#include <csignal>

class Pipeline
{
public:
    enum PipelineType {
        Fork,
        Knife,
        Spoon
    };

    Pipeline(Pipeline::PipelineType pt);
    ~Pipeline();

private:
    GstElement *pipeline;
    GstBus *bus;
    GstMessage *msg;
    GError *err;

    static void signalHandler(int signum);

    std::string next_available_filepath();

    const std::string base_filepath = "outputs/output";
};
