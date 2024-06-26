/* 
 *  Pipeline.h
 *
 *  The Pipeline class contains an instance of a gstreamer pipeline which is
 *  automatically played on contruction of the class and stopped at deletion.
 *
 *  The details of the pipeline are defined by the PipelineType enum passed into
 *  the contructor. Exactly what each pipeline does can be read through in
 *  Pipeline.cpp
 *
 */

#include <gst/gst.h>
#include <stdio.h>
#include <string>

class Pipeline
{
public:
    enum PipelineType {
        ForkedSource,
        FileSaver,
        Viewer
    };

    Pipeline(Pipeline::PipelineType pt);
    ~Pipeline();

private:
    GstElement *pipeline;
    GstBus *bus;
    GstMessage *msg;
    GError *err;

    std::string next_available_filepath();

    const std::string base_filepath = "outputs/output";
};
