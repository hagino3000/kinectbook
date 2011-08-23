#include <iostream>
#include <XnCppWrapper.h>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

// 設定ファイルのパス 自分の環境にあわせて書き変えてください
const char* CONFIG_XML_PATH = "/Users/takashi/dev/workspace/kinectbook/apps/video_view_openni/config.xml";

int main (int argc, char * const argv[]) {
	
	xn::Context context;
	IplImage *rgbImage = NULL;
	
	try {
		// OpenNIコンテキストの作成
		xn::EnumerationErrors ctxErrors;
		XnStatus rc = context.InitFromXmlFile(CONFIG_XML_PATH, &ctxErrors);
		if (rc != XN_STATUS_OK) {
			XnChar strError[1024];
			ctxErrors.ToString(strError, 1024);
			std::cout << strError << std::endl;
			return 1;
		}
		context.SetGlobalMirror(false);
		// ImageGeneratorの作成
		xn::ImageGenerator image;
		rc = context.FindExistingNode(XN_NODE_TYPE_IMAGE, image);
		if (rc != XN_STATUS_OK) {
			throw std::runtime_error(xnGetStatusString(rc));
		}
		
		XnMapOutputMode mapMode;
		image.GetMapOutputMode(mapMode);
		
		int width = mapMode.nXRes;
		int height = mapMode.nYRes;
								
		// 8ビット3チャンネルの画像データの作成
		rgbImage = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
		
		while(1) {
			// ノードの更新を待つ
			context.WaitAndUpdateAll();
			
			// ビデオカメラ画像の生データを取得
			xn::ImageMetaData imageMetaData;
			image.GetMetaData(imageMetaData);
			
			// メモリコピー
			xnOSMemCopy(rgbImage->imageData, imageMetaData.RGB24Data(), rgbImage->imageSize);
			
			// BGRからRGBに変換して表示
			cvCvtColor(rgbImage, rgbImage, CV_RGB2BGR);
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
