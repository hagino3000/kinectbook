#include <libfreenect_sync.h>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

// 11bit深度データをセンチに直す
// @see http://openkinect.org/wiki/Imaging_Information
inline float rawToCentimeters(unsigned short raw) {
	return 100/(-0.00307 * raw + 3.33);
}

int main (int argc, char * const argv[]) {
	
	// IplImageの作成
	IplImage *grayImage = cvCreateImage(cvSize(640, 480), IPL_DEPTH_8U, 1);
	IplImage *binaryImage1 = cvCreateImage(cvSize(640, 480), IPL_DEPTH_8U, 1);
	IplImage *binaryImage2 = cvCreateImage(cvSize(640, 480), IPL_DEPTH_8U, 1);
	IplImage *binaryImage3 = cvCreateImage(cvSize(640, 480), IPL_DEPTH_8U, 1);

	int numPixels = 640 * 480;
	short *depth;
	uint32_t timestamp;	

	while (1) {
		// 深度データの取得
		freenect_sync_get_depth((void**)&depth, &timestamp, 0, FREENECT_DEPTH_11BIT);
		
		// 8ビットに変換
		for (int i=0; i<numPixels; i++) {
			float centi = rawToCentimeters(depth[i]);
			grayImage->imageData[i] = (short)(min(centi*255/600, 255.0f));
		}
		
		// グレースケールを二値化
		cvThreshold(grayImage, binaryImage1, 50, 255, CV_THRESH_BINARY);
		cvThreshold(grayImage, binaryImage2, 65, 255, CV_THRESH_BINARY_INV);
		cvAnd(binaryImage1, binaryImage2, binaryImage3);
		
		
		// 表示
		cvShowImage("Depth View", grayImage);			
		cvShowImage("Binary View", binaryImage3);
		
		// ESCもしくはqが押されたら終了させる
		char key = cvWaitKey(10);
		if (key == 27 || key == 'q') {
			break;
		}			
	}
	if (grayImage != NULL) cvReleaseImage(&grayImage);
	if (binaryImage1 != NULL) cvReleaseImage(&binaryImage1);
	if (binaryImage2 != NULL) cvReleaseImage(&binaryImage2);
	if (binaryImage3 != NULL) cvReleaseImage(&binaryImage3);
	
	return 0;
}
