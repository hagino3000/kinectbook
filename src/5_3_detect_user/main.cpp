#include <iostream>
#include <XnCppWrapper.h>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

// 設定ファイルのパス 自分の環境にあわせて書き変えてください
const char* CONFIG_XML_PATH = "/Users/takashi/dev/workspace/kinectbook/apps/openni_introduction/config.xml";

void errorCheck(XnStatus status) {
	if (status != XN_STATUS_OK) {
		throw std::runtime_error(xnGetStatusString(status));
	}	
}

// 新たにユーザーを検出した時のコールバック
void XN_CALLBACK_TYPE NewUser(xn::UserGenerator& generator, XnUserID userId, void* pCookie) {
	std::cout << "New User:" << userId << std::endl;
}

// ユーザーの消失を検知した時のコールバック
void XN_CALLBACK_TYPE LostUser(xn::UserGenerator& generator, XnUserID userId, void* pCookie) {
	std::cout << "Lost User:" << userId << std::endl;
}


int main (int argc, char* const argv[]) {
	
	xn::Context context;
	
	IplImage *rgbImage = NULL;
	CvFont dfont;
    cvInitFont(&dfont, CV_FONT_HERSHEY_SIMPLEX , 1.0, 1.0, 0.0, 1, CV_AA);	
	
	try {
		// OpenNIコンテキストの作成
		xn::EnumerationErrors ctxErrors;
		XnStatus rc = context.InitFromXmlFile(CONFIG_XML_PATH, &ctxErrors);
		if (rc != XN_STATUS_OK) {
			XnChar strError[1024];
			ctxErrors.ToString(strError, 1024);
			throw std::runtime_error(strError);
		}
		context.SetGlobalMirror(true);
		
		// DepthGeneratorの取得
		xn::DepthGenerator depth;
		rc = context.FindExistingNode(XN_NODE_TYPE_DEPTH, depth);
		errorCheck(rc);
				
		// UserGeneratorの取得
		xn::UserGenerator user;
		rc = context.FindExistingNode(XN_NODE_TYPE_USER, user);
		errorCheck(rc);
		
		// コールバックの登録
		XnCallbackHandle userCallbacks;
		user.RegisterUserCallbacks(NewUser, LostUser, NULL, userCallbacks);
		
		// 表示用の画像データの作成
		XnMapOutputMode mapMode;
		depth.GetMapOutputMode(mapMode);
		rgbImage = cvCreateImage(cvSize(mapMode.nXRes, mapMode.nYRes), IPL_DEPTH_8U, 3);
		
		while(1) {
			// ノードの更新を待つ
			context.WaitAndUpdateAll();
			cvZero(rgbImage);
			
			// UserGeneratorからユーザー識別ピクセルを取得
			xn::SceneMetaData sceneMetaData;
			user.GetUserPixels(0, sceneMetaData);
			
			int pos = 0;
			for (int y=0; y<sceneMetaData.YRes(); y++) {
				for (int x=0; x<sceneMetaData.XRes(); x++) {
					XnLabel label = sceneMetaData(x, y);
					
					if (label > 0) {
						// 適当な色を付ける
						rgbImage->imageData[pos+0] = label%2 == 0 ? 0 : 255;
						rgbImage->imageData[pos+1] = label%2 == 0 ? 255 : 0;
						rgbImage->imageData[pos+2] = label%3 == 0 ? 255 : 0;
					}
					pos += 3;
				}
			}
						
			// 現在検出中のユーザー数を描画する
			XnUInt16 userNums = user.GetNumberOfUsers();
			stringstream ss;
			ss << "Detecting " << userNums << " Users";
			cvPutText(rgbImage, ss.str().c_str(), cvPoint(10, 40), &dfont, CV_RGB(255, 255, 255));
						
			// 表示
			cvShowImage("Video View", rgbImage);
					
			// ESCもしくはqが押されたら終了させる
			char key = cvWaitKey(10);
			if (key == 27 || key == 'q') {
				break;
			}
		}

	} catch (std::exception& ex) {
		std::cout << ex.what() << std::endl;
	}
	
	if (rgbImage != NULL) {
		cvReleaseImage(&rgbImage);	
	}
	context.Shutdown();
	
	return 0;
}
