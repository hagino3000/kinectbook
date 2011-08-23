#include <iostream> 
#include <stdexcept> 

#include <opencv2/opencv.hpp> 
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>

// ダウンロードしたファイルに含まれている学習データの場所 (自分の環境に合せて書き変えてください)
const char* CASCADE_NAME = "/Users/takashi/Downloads/OpenCV-2.3.0/data/haarcascades/haarcascade_frontalface_alt2.xml";

int main (int argc, char * const argv[]) {
	
	IplImage *frame = NULL;
	double height = 240;
	double width = 320;
	
	CvMemStorage* storage = NULL;
	CvHaarClassifierCascade* cascade = NULL;
	CvCapture* camera = NULL;

	try {
		storage = cvCreateMemStorage(0);
				
		// 検出器の作成
		cascade = (CvHaarClassifierCascade*)cvLoad(CASCADE_NAME, 0, 0, 0);
		if (!cascade) {
			throw std::runtime_error("Cannot create cascade");
		}
		
		// カメラデバイスの取得
		camera = cvCreateCameraCapture(0);
		if (!camera) {
			throw std::runtime_error("Camera device not found");
		}
		
		// 取得サイズの設定
		cvSetCaptureProperty(camera, CV_CAP_PROP_FRAME_WIDTH, width);
		cvSetCaptureProperty(camera, CV_CAP_PROP_FRAME_HEIGHT, height);
		
		while (1) {
			frame = cvQueryFrame (camera);
			// 学習データを使った検出
			CvSeq* faces = cvHaarDetectObjects(frame, cascade, storage,
											   1.1, 2, CV_HAAR_DO_CANNY_PRUNING,
											   cvSize (100, 100));
			
			
			for (int i = 0; i < faces->total; i++) {
				
				CvRect* faceRect = (CvRect*)cvGetSeqElem(faces, i);
				
				// 検出した位置に四角を描画
				cvRectangle(frame,
							cvPoint(faceRect->x, faceRect->y),
							cvPoint(faceRect->x + faceRect->width, 
									faceRect->y + faceRect->height),
							CV_RGB(255, 0 ,0),
							3, CV_AA);
			}
			
			// ビデオ画像と検出結果の表示
			cvShowImage("Face Tracker", frame);
			
			// ESCもしくはqが押されたら終了させる
			char key = cvWaitKey (10);
			if (key == 27 || key == 'q') {
				break;
			}
		}
	} catch (std::exception& ex) {
		std::cout << ex.what() << std::endl;
	}

	// リソースの解放
	cvReleaseMemStorage(&storage);
	cvReleaseCapture(&camera);
	cvReleaseHaarClassifierCascade(&cascade);
	
	return 0;
}
