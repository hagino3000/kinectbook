#include "testApp.h"

void testApp::setup() {
	ofSetLogLevel(OF_LOG_VERBOSE);
	ofSetWindowShape(640, 480);
	ofBackground(0, 0, 0);

	// ofxOpenNI�̏�����
	ofxOpenNICtx.setupUsingXMLFile(ofToDataPath("myOpenNIConfig.xml", true));
	depthGenetator.setup(&ofxOpenNICtx);
	imageGenetator.setup(&ofxOpenNICtx);	
	handGenerator.setup(&ofxOpenNICtx, 2);
	handGenerator.setSmoothing(0.1f);
	
	ofxOpenNICtx.toggleRegisterViewport();
	ofxOpenNICtx.toggleMirror();
}

void testApp::update(){
	
	// OpenNI�m�[�h�̃A�b�v�f�[�g
	ofxOpenNICtx.update();
	depthGenetator.update();
	imageGenetator.update();	
	
	// ���o���Ă����̍��W��vector�ɓ���ĕێ�����
	int trackedHands = handGenerator.getNumTrackedHands();
	for (int i=0; i<trackedHands; i++) {
		ofxTrackedHand * hand = handGenerator.getHand(i);
		if (hand != NULL) {
			ofPoint * handPos = new ofPoint(hand->projectPos.x, 
											hand->projectPos.y, 
											hand->projectPos.z);
			handTrackedPoints.push_back(handPos);
			// �����_���̐F�����
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
	
	// �ۑ�������̍��W�̋O�Ղɉ~��`�悷��
	for (int i=0; i<handTrackedPoints.size(); i++) {
		ofPoint * handPos = handTrackedPoints[i];
		ofColor * col = handTrackedColors[i];
		ofSetColor(col->r, col->g, col->b, 200);
		// ��̋����œh��Ԃ��傫����ς���
		ofCircle(handPos->x, 
				 handPos->y, 
				 max(2000 - handPos->z, 400.0f)/500*7);
	}
}


void testApp::exit() {
	//ofxOpenNICtx.shutdown();	
	ofLog(OF_LOG_NOTICE, "End Application");
}
