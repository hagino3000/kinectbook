#ifndef _TEST_APP
#define _TEST_APP

#include "ofMain.h"
#include "ofxOpenNI.h"

class testApp : public ofBaseApp{
	
public:
	void setup();
	void update();
	void draw();
	void exit();	
		
	vector <ofPoint*> handTrackedPoints;
	vector <ofColor*> handTrackedColors;
	
	ofxOpenNIContext	ofxOpenNICtx;
	ofxDepthGenerator	depthGenetator;	
	ofxImageGenerator	imageGenetator; 
	ofxHandGenerator	handGenerator;
			
};

#endif
