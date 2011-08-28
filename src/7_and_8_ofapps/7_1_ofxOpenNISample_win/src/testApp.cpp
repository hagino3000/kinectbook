#include "testApp.h"

void testApp::setup() {
	ofSetLogLevel(OF_LOG_VERBOSE);
	ofSetWindowShape(640, 480);
	ofBackground(0, 0, 0);

	// ofxOpenNIの初期化
	ofxOpenNICtx.setupUsingXMLFile(ofToDataPath("myOpenNIConfig.xml", true));
	depthGenetator.setup(&ofxOpenNICtx);
	imageGenetator.setup(&ofxOpenNICtx);	
	handGenerator.setup(&ofxOpenNICtx, 2);
	handGenerator.setSmoothing(0.1f);
	
	ofxOpenNICtx.toggleRegisterViewport();
	ofxOpenNICtx.toggleMirror();
}

void testApp::update(){
	
	// OpenNIノードのアップデート
	ofxOpenNICtx.update();
	depthGenetator.update();
	imageGenetator.update();	
	
	// 検出している手の座標をvectorに入れて保持する
	int trackedHands = handGenerator.getNumTrackedHands();
	for (int i=0; i<trackedHands; i++) {
		ofxTrackedHand * hand = handGenerator.getHand(i);
		if (hand != NULL) {
			ofPoint * handPos = new ofPoint(hand->projectPos.x, 
											hand->projectPos.y, 
											hand->projectPos.z);
			handTrackedPoints.push_back(handPos);
			// ランダムの色を作る
			ofColor * col = new ofColor();
			col->r = ofRandom(0, 255);
			col->g = ofRandom(0, 255);
			col->b = ofRandom(0, 255);
			handTrackedColors.push_back(col);
		}
	}
}

//--------------------------------------------------------------
void testApp::draw(){
	
	ofFill();
	glEnable(GL_BLEND);
	imageGenetator.draw(0, 0, 640, 480);
	
	// 保存した手の座標の軌跡に円を描画する
	for (int i=0; i<handTrackedPoints.size(); i++) {
		ofPoint * handPos = handTrackedPoints[i];
		ofColor * col = handTrackedColors[i];
		ofSetColor(col->r, col->g, col->b, 200);
		// 手の距離で塗りつぶす大きさを変える
		ofCircle(handPos->x, 
				 handPos->y, 
				 max(2000 - handPos->z, 400.0f)/500*7);
	}
}


void testApp::exit() {
	//ofxOpenNICtx.shutdown();	
	ofLog(OF_LOG_NOTICE, "End Application");
}
