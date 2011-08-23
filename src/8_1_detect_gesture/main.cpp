#include <iostream>
#include <stdexcept>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <XnCppWrapper.h>
#include <XnVSessionManager.h>
#include <XnVSwipeDetector.h>

#if defined( __APPLE_CC__)
#include <CoreGraphics/CGEventTypes.h>
#include <CoreGraphics/CGEvent.h>
#endif

// 設定ファイルのパス 自分の環境にあわせて書き変えてください
const char* CONFIG_XML_PATH = "/Users/takashi/dev/workspace/kinectbook/apps/gesture_sample/config.xml";

// セッションが開始した時のコールバック
void XN_CALLBACK_TYPE SessionStart(const XnPoint3D& pFocus, 
								   void* UserCxt) {
	std::cout << "Start Hand Session" << std::endl;
}

// セッションが終了した時のコールバック
void XN_CALLBACK_TYPE SessionEnd(void* UserCxt) {
	std::cout << "End Hand Session" << std::endl;
}

// スワイプを検出した時のコールバック
void XN_CALLBACK_TYPE Swipe(XnFloat fVelocity, XnFloat fAngle, 
							void *pUserCxt) {
	std::cout << "Swipe:" << (char*)pUserCxt 
	<< ", Speed:" << fVelocity 
	<< ", Angle:" << fAngle << std::endl;
	
	if ((char*)pUserCxt == "Left") {
		// キーボードイベントを生成する
#if defined( __WIN32__ ) || defined( _WIN32 )
		keybd_event(VK_RIGHT, 0, 0, 0);
#elif defined( __APPLE_CC__ )
		CGEventRef keyEv = CGEventCreateKeyboardEvent(NULL, 
								(CGKeyCode)124, true);
		CGEventPost (kCGHIDEventTap, keyEv);
#endif
	}
	if ((char*)pUserCxt == "Right") {
		// キーボードイベントを生成する
#if defined( __WIN32__ ) || defined( _WIN32 )
		keybd_event(VK_LEFT, 0, 0, 0);
#elif defined( __APPLE_CC__)
		CGEventRef keyEv = CGEventCreateKeyboardEvent(NULL, 
								(CGKeyCode)123, true);
		CGEventPost (kCGHIDEventTap, keyEv);
#endif
	}
}


int main (int argc, char * const argv[]) {
	
	IplImage* camera = 0;
	xn::Context context;
	
	try {
		// OpenNIコンテキストの作成
		xn::EnumerationErrors ctxErrors;
		XnStatus rc = context.InitFromXmlFile(CONFIG_XML_PATH, 
											  &ctxErrors);
		if (rc != XN_STATUS_OK) {
			XnChar strError[1024];
			ctxErrors.ToString(strError, 1024);
			throw std::runtime_error(strError);
		}
		
		// ImageGeneratorの作成
		xn::ImageGenerator image;
		rc = context.FindExistingNode(XN_NODE_TYPE_IMAGE, image);
		if (rc != XN_STATUS_OK) {
			throw std::runtime_error(xnGetStatusString(rc));
		}
						
		XnMapOutputMode outputMode;
		image.GetMapOutputMode(outputMode);
		camera = cvCreateImage(cvSize(outputMode.nXRes, 
									  outputMode.nYRes),
							   IPL_DEPTH_8U, 3);
		
		// NITEのセッション初期化
		XnVSessionManager sessionManager;
		rc = sessionManager.Initialize(&context, 
									   "Wave,Click", 
									   "RaiseHand");
		if (rc != XN_STATUS_OK) {
			throw std::runtime_error(xnGetStatusString(rc));
		}
		
		// セッションの開始終了時のコールバックの登録
		XnVHandle sessionCallback = 
			sessionManager.RegisterSession(0, 
										   &SessionStart, 
										   &SessionEnd);
		
		// 検出器の作成
		XnVSwipeDetector swipeDetector;
		// 検出する最低速度(デフォルトは 0.25m/s)
		swipeDetector.SetMotionSpeedThreshold(0.25);
		// 検出する最低持続時間 (デフォルトは350ms)
		swipeDetector.SetMotionTime(350);
		// X軸方向からのずれの許容角度 (デフォルトは25度)
		swipeDetector.SetXAngleThreshold(25);
		
		// スワイプ検出時のコールバックを登録
		XnCallbackHandle swipeRightCallback = 
			swipeDetector.RegisterSwipeRight((void*)"Right", Swipe);
		XnCallbackHandle swipeLeftCallback = 
			swipeDetector.RegisterSwipeLeft((void*)"Left", Swipe);
		
		// セッションに検出器を登録
		sessionManager.AddListener(&swipeDetector);
		
		// ジェスチャー認識の開始
		context.StartGeneratingAll();
		
		std::cout << "Initialize Success" << std::endl;
		while (1) {
			// ノードの更新を待つ
			context.WaitAndUpdateAll();
			sessionManager.Update(&context);
			
			// ビデオカメラ画像の表示
			xn::ImageMetaData imageMD;
			image.GetMetaData(imageMD);
			xnOSMemCopy(camera->imageData, imageMD.RGB24Data(), 
						camera->imageSize);
			cvCvtColor(camera, camera, CV_BGR2RGB);
			cvShowImage("Video Image", camera);
			
			// ESCもしくはqが押されたら終了させる
			char key = cvWaitKey(10);
			if (key == 27 || key == 'q') {
				break;
			}
		}
		
		// 登録したコールバックの解除
		swipeDetector.UnregisterSwipeRight(swipeRightCallback);
		swipeDetector.UnregisterSwipeLeft(swipeLeftCallback);
		sessionManager.UnregisterSession(sessionCallback);
		
	} catch (std::exception& ex) {
		std::cout << ex.what() << std::endl;
	}
	if (camera != NULL) cvReleaseImage(&camera);
	context.Shutdown();
	return 0;
}
