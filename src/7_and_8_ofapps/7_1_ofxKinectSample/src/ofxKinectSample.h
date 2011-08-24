#ifndef _TEST_APP
#define _TEST_APP


#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxKinect.h"

class ofxKinectSample : public ofBaseApp {
	
public:
	void setup();
	void update();
	void draw();
	void exit();
			
	void keyPressed  (int key);
	
	ofxCvGrayscaleImage grayImage;	
	ofxKinect kinect;
	int tiltAngle;
	ofTrueTypeFont msgFont;
};

#endif
