#include <iostream>
#include <XnCppWrapper.h>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

// 設定ファイルのパス 自分の環境にあわせて書き変えてください
const char* CONFIG_XML_PATH = "/Users/takashi/dev/lib/forKinect/Nite-1.3.1.4/Data/Sample-Tracking.xml";

int main (int argc, char * const argv[]) {
	
	xn::Context context;
	IplImage *grayImage = NULL;
	
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
		
		// DepthGeneratorの作成
		xn::DepthGenerator depth;
		rc = context.FindExistingNode(XN_NODE_TYPE_DEPTH, depth);
		if (rc != XN_STATUS_OK) {
			throw std::runtime_error(xnGetStatusString(rc));
		}
		
		XnMapOutputMode mapMode;
		depth.GetMapOutputMode(mapMode);
		
		int width = mapMode.nXRes;
		int height = mapMode.nYRes;
		int numPixels = width * height;
		
		// グレースケール用ピクセルデータ
		unsigned char * grayPixels = new unsigned char[numPixels];
		xnOSMemSet(grayPixels, 0, numPixels * sizeof(unsigned char));
		
		// 最大深度
		const unsigned short int maxDepth = 8000;
				
		// 8ビット1チャンネルの画像データの作成
		grayImage = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
		
		while(1) {
			// ノードの更新を待つ
			context.WaitAndUpdateAll();
			
			// 深度の生データの取得
			xn::DepthMetaData depthMetaData;
			depth.GetMetaData(depthMetaData);
			const XnDepthPixel* depthPixels = depthMetaData.Data();		
			
			// グレースケールに変換
			for (int i=0; i<numPixels; i++) {
				grayPixels[i] = (int)(min(maxDepth, depthPixels[i])/(maxDepth/255));
			}
			// メモリコピー
			xnOSMemCopy(grayImage->imageData, grayPixels, numPixels);
			// 表示
			cvShowImage("Depth View", grayImage);
			
			// ESCもしくはqが押されたら終了させる
			char key = cvWaitKey(10);
			if (key == 27 || key == 'q') {
				break;
			}		
		}

	} catch (std::exception& ex) {
		std::cout << ex.what() << std::endl;
	}
	
	if (grayImage != NULL) {
		cvReleaseImage(&grayImage);	
	}
	context.Shutdown();
	
	return 0;
}
