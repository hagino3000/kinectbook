#include "ofxKinectSample.h"

void ofxKinectSample::setup() {	
	ofSetLogLevel(OF_LOG_VERBOSE);
	ofLog(OF_LOG_VERBOSE, "Start setup()");
	
	// 画面の設定
    ofSetFrameRate(60);
    ofSetCircleResolution(128);
    ofBackground(255, 255, 255);
	ofSetWindowShape(640, 480);
    ofEnableSmoothing();
	
	// Kinectの初期化
	kinect.init();
	kinect.open();
	tiltAngle = 0;
	kinect.setCameraTiltAngle(tiltAngle);
	
	// フォントのロード
	msgFont.loadFont("Courier New.ttf",18, true, true);
    msgFont.setLineHeight(30.0f);
	
	// メモリ確保
	grayImage.allocate(kinect.width, kinect.height);
}

void ofxKinectSample::update() {
	kinect.update();
	if(kinect.isFrameNew())	{
		// Kinectのデータが更新されていたらピクセルデータを再セットする
		grayImage.setFromPixels(kinect.getDepthPixels(), kinect.width, kinect.height);
		grayImage.flagImageChanged();
	}
}

void ofxKinectSample::draw() {
	
	// 深度画像の描画
	ofSetColor(255, 0, 200);
	grayImage.draw(0, 0, 640, 480);
	
	// ガイドの描画
	ofSetColor(0, 255, 255);
	ofNoFill();
	ofCircle(320, 240, 10);
	ofLine(320, 240, 340, 220);
	ofLine(340,	220, 590, 220);
	
	// 中心の距離の描画
	stringstream ss;
	ss << "CenterDepth=" << kinect.getDistanceAt(320, 240) << "cm";
	msgFont.drawString(ss.str(), 330, 215);
}


void ofxKinectSample::keyPressed(int key) {
	// 上下キーでチルトモータを動かす
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
	// プログラム終了時にKinect接続を解除する
	kinect.setCameraTiltAngle(0);
	kinect.close();
	ofLog(OF_LOG_VERBOSE, "End Application");
}


