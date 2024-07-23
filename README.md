Each folder contains its own makefile for compiling. This is not very structured  

If you're running the pipeline-class tests, make sure that you've created the "outputs" directory or accordingly changed the output filepath in Pipeline.h

The newest code in the repo right now is all in "cutlery", named for the fork in
forked pipeline. The newest version of the code is in knork.cpp  

knork takes in video, saves it to a file, and also outputs that video into
avsink. Work to change this to a udpsink is being done in the branch "tee".  

Once something's working here, it'll be put into the gstreamerPipeline class in
Mete's code.
