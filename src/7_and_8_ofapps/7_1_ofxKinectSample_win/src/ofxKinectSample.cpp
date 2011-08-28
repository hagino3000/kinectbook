#include "ofxKinectSample.h"

void ofxKinectSample::setup() {	
	ofSetLogLevel(OF_LOG_VERBOSE);
	ofLog(OF_LOG_VERBOSE, "Start setup()");
	
	// ��ʂ̐ݒ�
    ofSetFrameRate(60);
    ofSetCircleResolution(128);
    ofBackground(255, 255, 255);
	ofSetWindowShape(640, 480);
    ofEnableSmoothing();
	
	// Kinect�̏�����
	kinect.init();
	kinect.open();
	tiltAngle = 0;
	kinect.setCameraTiltAngle(tiltAngle);
	
	// �t�H���g�̃��[�h
	msgFont.loadFont("Courier New.ttf",18, false, false);
    msgFont.setLineHeight(30.0f);
	
	// �������m��
	grayImage.allocate(kinect.width, kinect.height);
}

void ofxKinectSample::update() {
	kinect.update();
	if(kinect.isFrameNew())	{
		// Kinect�̃f�[�^���X�V����Ă�����s�N�Z���f�[�^���ăZ�b�g����
		grayImage.setFromPixels(kinect.getDepthPixels(), kinect.width, kinect.height);
		grayImage.flagImageChanged();
	}
}

void ofxKinectSample::draw() {
	
	// �[�x�摜�̕`��
	ofSetColor(255, 0, 200);
	grayImage.draw(0, 0, 640, 480);
	
	// �K�C�h�̕`��
	ofSetColor(0, 255, 255);
	ofNoFill();
	ofCircle(320, 240, 10);
	ofLine(320, 240, 340, 220);
	ofLine(340,	220, 590, 220);
	
	// ���S�̋����̕`��
	stringstream ss;
	ss << "CenterDepth=" << kinect.getDistanceAt(320, 240) << "cm";
	msgFont.drawString(ss.str(), 330, 215);
}


void ofxKinectSample::keyPressed(int key) {
	// �㉺�L�[�Ń`���g���[�^�𓮂���
	switch (key) {
		case OF_KEY_UP:
			tiltAngle = min(++tiltAngle, 30);
			kinect.setCameraTiltAngle(tiltAngle);
			break;
			
		case OF_KEY_DOWN:
			tiltAngle = max(--tiltAngle, -30);
			kinect.setCameraTiltAngle(tiltAngle);
			break;
	}	

}

void ofxKinectSample::exit() {
	// �v���O�����I������Kinect�ڑ�����������
	kinect.setCameraTiltAngle(0);
	kinect.close();
	ofLog(OF_LOG_VERBOSE, "End Application");
}


