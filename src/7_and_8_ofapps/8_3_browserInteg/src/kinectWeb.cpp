#include "kinectweb.h"

// ZMQ Sokcet
zmq::context_t ctx (1);
zmq::socket_t clients (ctx, ZMQ_REP);

unsigned char *grayPixels;
queue<int> savedFileNames;
int tiltAngle = 0;

// チルト角の制御はMaxOS Xのみ動作する
#ifdef TARGET_OSX
ofxHardwareDriver	hardware;
#endif

// ZMQソケット送信
bool s_send (zmq::socket_t& socket, const std::string& string) {
    zmq::message_t message(string.size());
    xnOSMemCopy(message.data(), string.data(), string.size());
    bool rc = socket.send(message);
    return (rc);
}

// データ受信用ワーカー
XN_THREAD_PROC workerThread(XN_THREAD_PARAM) {
	
	ofLog(OF_LOG_NOTICE, "Start receiver thread");
	
    while (1) {
        zmq::message_t request;
        clients.recv (&request);
		
		string clientRequest = static_cast<const char*>(request.data());
		ofLog(OF_LOG_NOTICE, clientRequest);
		
		stringstream replyStr;
		
		if (clientRequest.find("GET_DEPTH", 0) != string::npos) {
			
			// 深度情報をJSON文字列にする
			int width = 640;
			int height = 480;
			replyStr << "{\"type\":\"depth\", \"data\":\"";
			for(int y = 0; y < height; y+=2) {
				for(int x = 0; x < width; x+=2) {
					replyStr << ofToString((int)grayPixels[x+y*width]) << ',';
				}
			}
			replyStr << "\", \"imageFiles\":[";
			int files = savedFileNames.size();
			for (int i=0; i<files; i++) {
				int fileNo = (int)savedFileNames.front();
				if (i > 0) {
					replyStr << ',';
				}
				replyStr << fileNo;
				savedFileNames.pop();
			}
			replyStr << "]}";
            
		} else if (clientRequest.find("UP_ANGLE", 0) != string::npos) {
			
			// チルト角を増やす
#ifdef TARGET_OSX
			tiltAngle = min(++tiltAngle, 30);
			hardware.setTiltAngle(tiltAngle);
#endif
			
			// 現在の角度をレスポンスとして返す
			replyStr << "{\"type\":\"tilt\",\"angle\":" 
            << tiltAngle 
            << "}";			
			
		} else if (clientRequest.find("DOWN_ANGLE", 0) != string::npos) {
			
			// チルト角を減らす
#ifdef TARGET_OSX
			tiltAngle = max(--tiltAngle, -30);
			hardware.setTiltAngle(tiltAngle);
#endif
			
			// 現在の角度をレスポンスとして返す
			replyStr << "{\"type\":\"tilt\",\"angle\":" 
            << tiltAngle 
            << "}";
			
		} else {
			// 未知のイベント
			ofLog(OF_LOG_ERROR, "Unknown Event");
			replyStr << "Unknown Event" << endl;
		}
		
		// レスポンスを返す
		s_send(clients, replyStr.str());
    }
	XN_THREAD_PROC_RETURN(XN_STATUS_OK);
}

void kinectWeb::setup() {
	ofSetLogLevel(OF_LOG_VERBOSE);
	ofLog(OF_LOG_VERBOSE, "Start setup()");
    
	// 画面の初期化
	ofSetFrameRate(30);
    ofBackground(255, 255, 255);
	ofSetWindowShape(640, 480);
    ofEnableSmoothing();	
	
	// ofxOpenNIの初期化
	context.setup();
	depthGenerator.setup(&context);
	imageGenerator.setup(&context);
	userGenerator.setup(&context);
	userGenerator.setUseMaskPixels(true);
	context.toggleRegisterViewport();
	context.toggleMirror();
	
#ifdef TARGET_OSX
	hardware.setup();
	hardware.setLedOption(LED_BLINK_GREEN);
#endif
    
	// メモリ確保
	grayPixels = new unsigned char[640 * 480];
	xnOSMemSet(grayPixels, 0, 640 * 480 * sizeof(unsigned char));
	saveImage.allocate(640, 480, OF_IMAGE_COLOR);
	
	// ソケットの初期化
	try {
		clients.bind("tcp://*:5555");
		ofLog(OF_LOG_NOTICE, "Open ZMQ Sokcet at 5555");
	} catch (zmq::error_t e) {
		ofLog(OF_LOG_ERROR, (string)e.what());
	}
	
	// 通信用ワーカースレッドの作成
	XnStatus rc = xnOSCreateThread(workerThread, 
								   (XN_THREAD_PARAM)NULL, 
								   &threadHandle); 
	if (rc != XN_STATUS_OK) {
		ofLog(OF_LOG_ERROR, xnGetStatusString(rc));
	}
}

void kinectWeb::update(){
	
#ifdef TARGET_OSX
	hardware.update();
#endif
	
	// OpenNIの更新
	context.update();
	depthGenerator.update();
	imageGenerator.update();
	userGenerator.update();
    
	// ユーザーマスクの取得
	userMask.setFromPixels(userGenerator.getUserPixels(), 
						   userGenerator.getWidth(), 
						   userGenerator.getHeight(), 
						   OF_IMAGE_GRAYSCALE);	
    
	// 生のDepthGeneratorから深度情報を取得する
	xn::DepthGenerator& xnDGen = depthGenerator.getXnDepthGenerator();
	xn::DepthMetaData depthMetaData;
	xnDGen.GetMetaData(depthMetaData);
	const XnDepthPixel* depthPixels = depthMetaData.Data();
	int numPixels = depthMetaData.YRes() * depthMetaData.XRes();
	
	for (int pos = 0; pos < numPixels; pos++, depthPixels++) {
		// 0~255の値に変換して保管
		// 0mm -> 0
		// 5000mm -> 255
		grayPixels[pos] = min(255, (int)(*depthPixels * 255 / 5000));
	}
    
	// ユーザー検出中の場合、15フレームに一度画像を保存する
	if (userGenerator.getNumberOfTrackedUsers() > 0 && 
		ofGetFrameNum() % 15 == 0) {
		
		// ファイル名はフレーム番号 + ".png" とする
		string fileName = ofToString(ofGetFrameNum()) + ".png";
		saveImage.grabScreen(0, 0, 640, 480);
		saveImage.saveImage(fileName);
		ofLog(OF_LOG_NOTICE, "Saved!! " + fileName);
		
		savedFileNames.push(ofGetFrameNum());
	}
}

void kinectWeb::draw(){
	// 検出中のユーザー領域のみ深度とビデオ画像を重ねあわせて描画する
	depthGenerator.draw(0,0,640,480);
	glEnable(GL_BLEND);
	glBlendFunc(GL_DST_COLOR, GL_ZERO);
	userMask.draw(0, 0);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);		
	imageGenerator.draw(0, 0, 640, 480);
	glDisable(GL_BLEND);
    
	
	// 検出ユーザー数の表示
	stringstream info;
	info << "Traking Users:" 
    << userGenerator.getNumberOfTrackedUsers() 
    << endl;
	ofDrawBitmapString(info.str(), 20, 20);
}

void kinectWeb::exit() {
	// 終了処理
    clients.close();
    xnOSCloseThread(&threadHandle);
    context.shutdown();
	ofLog(OF_LOG_NOTICE, "End Application");
}

