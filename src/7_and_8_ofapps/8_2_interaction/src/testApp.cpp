#include "testApp.h"

#define MAX_CIRCLE_NUM  90
#define MAX_RECT_NUM  90

void ofKinectInteraction::setup() {	
	ofSetLogLevel(OF_LOG_VERBOSE);
	ofLog(OF_LOG_VERBOSE, "Start setup()");
	debug = false;
	
	// 画面の設定
    ofSetFrameRate(30);
	ofSetVerticalSync(true);
    ofSetCircleResolution(128);
    ofBackground(100, 100, 100);
	ofSetWindowShape(1024, 768);
    ofEnableSmoothing();
	
	// box2dの設定
	box2d.init();
	box2d.setGravity(0, 2);
	box2d.createFloor(640, 480);
	box2d.setFPS(30);
	
	// Kinectの初期化
	kinect.init();
	kinect.open();
	tiltAngle = 0;
	
	kinect.setCameraTiltAngle(tiltAngle);
	// 深度180~220の範囲を使う
	maxThreshold = 220;
	minThreshold = 180;
	
	// メモリ確保
	grayImage.allocate(kinect.width, kinect.height);
	binaryImage.allocate(kinect.width, kinect.height);
}

void ofKinectInteraction::update() {
	// box2dの更新
	box2d.update();
	createBox2dObjects();
	
	// Kinectの更新
	kinect.update();
	if(kinect.isFrameNew())	{
		
		// 深度画像の取得
		grayImage.setFromPixels(kinect.getDepthPixels(), 
								kinect.width, 
								kinect.height);
		grayImage.flagImageChanged();
		
		//必要な時は反転させる
		//grayImage.mirror(false, true);
		
		// 二値化した画像の作成
		int numPixels = kinect.width * kinect.height;
		unsigned char * grayPix = grayImage.getPixels();
		unsigned char * binaryPix = binaryImage.getPixels();
		
		for (int i = 0; i < numPixels; i++) {
			if (minThreshold < grayPix[i] && 
				grayPix[i] < maxThreshold) {
				
				binaryPix[i] = 255; // 白
			} else {
				binaryPix[i] = 0; // 黒
			}
		}
		binaryImage.flagImageChanged();
		
		// 二値化した画像から輪郭の抽出
		// 最大5個まで、 250ピクセルより小さい物は除外する
		contourFinder.findContours(binaryImage,
								   250, 
								   (640*480)/2, 
								   5, 
								   false);

		// 前回作った当り判定用シェイプをクリア
		for (int i=0; i<contorParticles.size(); i++) {
			contorParticles[i].destroyShape();
		}
		contorParticles.clear();
		
		// 抽出した輪郭に沿って当り判定用シェイプを配置する
		for (int j = 0; j < contourFinder.nBlobs; j++){			
			for (int i = 0; i < contourFinder.blobs[j].nPts; i+=5){
				ofxBox2dCircle p = ofxBox2dCircle();
				p.setup(box2d.getWorld(), 
						contourFinder.blobs[j].pts[i].x, 
						contourFinder.blobs[j].pts[i].y, 
						5, true);
				p.setPhysics(1.0, 2.0, 1.0);
				contorParticles.push_back(p);
			}
		}
	}
}

void ofKinectInteraction::draw() {
	
	if (debug) {
		// デバッグ表示
		glScalef(0.8f, 0.8f, 1.0f);
		ofPushMatrix();
		
		// 深度画像の描画
		grayImage.draw(0, 0, 640, 480);
		// 二値化画像の描画
		binaryImage.draw(0, 480, 640, 480);

		ofTranslate(640, 0, 0);	
		
		// 輪郭の描画
		contourFinder.draw();
		
		ofTranslate(0, 480, 0);
		
		// 当り判定用シェイプの描画
		for(int i = 0; i < contorParticles.size(); i++) {
			contorParticles[i].draw();
		}
		ofPopMatrix();
		
	} else {
		// 画面サイズにあわせる
		glScalef(1024.0f/640.0f, 768.0f/480.0f, 1.0f);
		// 背景の描画
		drawBackground();
		// 深度像の描画
		drawDepth();
		// box2オブジェクトの描画
		for(int i = 0; i < box2dCircles.size(); i++) {
			box2dCircles[i].draw();
		}
		for(int i = 0; i < box2dRects.size(); i++) {
			box2dRects[i].draw();
		}			
	}
}

// 背景の描画
void ofKinectInteraction::drawBackground() {
	ofFill();
	int rowHeight = 10;
	int winHeight = ofGetHeight();
	int winWidth = ofGetWidth();
	for (int i = 0; i < winHeight; i+=rowHeight) {
		ofSetColor(0, 0, 
				   abs(255.0f - (float)i*3/winHeight*255.0f), 
				   255);
		ofRect(0, i, winWidth, rowHeight);
	}
	ofNoFill();
}

// 深度像の描画
void ofKinectInteraction::drawDepth() {
	
	ofFill();
	
	unsigned char * grayPix = grayImage.getPixels();	
	int step = 6;
	int width = grayImage.getWidth();
	int height = grayImage.getHeight();
	int drawMinThreshold = minThreshold - 20;
	float scale = maxThreshold - drawMinThreshold;
	
	for (int x = 0; x < width; x+=step) {
		for (int y = 0; y < height; y+=step) {
			int i = y * width + x;
			if (drawMinThreshold < grayPix[i] && 
				grayPix[i] < maxThreshold) {
				
				float size = grayPix[i] - drawMinThreshold;
				// 近い所は赤、遠い所は緑にする
				ofSetColor(min(size/scale*255.0f, 255.0f), 
						   max(255.0f-size/scale*255.0f, 0.0f), 
						   0, 
						   min(150.0f + size/scale*255.0f, 255.0f));
				// 円の描画
				ofCircle(x, y, 3);
			}
		}
	}
}

// Box2dシェイプの作成
void ofKinectInteraction::createBox2dObjects() {
	int x = ofRandom(0, 640);
	int y = 0;
	
	// 4フレーム毎に円を作る
    if (ofGetFrameNum() % 4 == 0) {
        ofxBox2dCircle c = ofxBox2dCircle();
        c.setPhysics(2, 0.8, 2);
        c.setup(box2d.getWorld(), x, y, ofRandom(3, 14));
        box2dCircles.push_back(c);
		
		// 指定最大値以上に増えたら古い物から削除
        if (box2dCircles.size() > MAX_CIRCLE_NUM) {
            box2dCircles.at(0).destroyShape();
            box2dCircles.erase(box2dCircles.begin());
        }
	// 5フレーム毎に四角形を作る
    } else if (ofGetFrameNum() % 5 == 0) {
        int size = ofRandom(5, 20);
        ofxBox2dRect c = ofxBox2dRect();
        c.setPhysics(2, 0.8, 10);
        c.setup(box2d.getWorld(), x, y, size, size);
        box2dRects.push_back(c);
		
		// 指定最大値以上に増えたら古い物から削除
        if (box2dRects.size() > MAX_RECT_NUM) {
            box2dRects.at(0).destroyShape();
            box2dRects.erase(box2dRects.begin());
        }
    }
}

void ofKinectInteraction::keyPressed(int key) {
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
		case 'd':
			// デバッグモードにする
			debug = !debug;
			break;
	}
}

void ofKinectInteraction::exit() {
	// プログラム終了時にKinect接続を解除する
	kinect.setCameraTiltAngle(0);
	kinect.close();
	ofLog(OF_LOG_VERBOSE, "End Application");
}


