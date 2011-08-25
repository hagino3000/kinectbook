#ifndef _OF_KINECT_INTERACTION
#define _OF_KINECT_INTERACTION

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxKinect.h"
#include "ofxBox2d.h"

class ofKinectInteraction : public ofBaseApp {
	
public:
	void setup();
	void update();
	void draw();
	void exit();
			
	void keyPressed  (int key);
	
	void createBox2dObjects();
	void drawBackground();
	void drawDepth();

	ofxKinect kinect;
	int tiltAngle;	
	
	// 画像処理用
	int minThreshold;
	int	maxThreshold;
	ofxCvGrayscaleImage grayImage, binaryImage;
    ofxCvContourFinder 	contourFinder;
	
	// Box2dシェイプ用
	ofxBox2d box2d;
	vector <ofxBox2dCircle> box2dCircles;
	vector <ofxBox2dRect> box2dRects;
	vector <ofxBox2dCircle> contorParticles;
	
	bool debug;	
};

#endif
