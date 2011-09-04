#ifndef _KINECT_WEB
#define _KINECT_WEB

#include "ofMain.h"
#include "ofxOpenNI.h"
#include <zmq.hpp>
#include <queue>

class kinectWeb : public ofBaseApp{
    
public:
    void setup();
    void update();
    void draw();
    void exit();
    
    ofxOpenNIContext    context;
    ofxDepthGenerator   depthGenerator;    
    ofxImageGenerator   imageGenerator; 
    ofxUserGenerator    userGenerator;    
    
    XN_THREAD_HANDLE    threadHandle;
    
    ofImage             userMask, saveImage;
};

#endif
