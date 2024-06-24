#include <gst/gst.h>
#include <stdio.h>
#include <iostream>

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
};