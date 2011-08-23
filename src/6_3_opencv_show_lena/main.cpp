#include <iostream> 

#include <opencv2/opencv.hpp> 
#include <opencv2/highgui/highgui.hpp>

// lena画像の場所
const char* IMAGE_PATH = "/Users/takashi/Downloads/OpenCV-2.3.0/samples/cpp/lena.jpg";

int main (int argc, char * const argv[]) {
	
	IplImage *image = cvLoadImage(IMAGE_PATH);
	if (image == NULL) {
		std::cout << "Cannot load image file" << std::endl;
		return 1;
	}
	
	while(1) {
		// 表示
		cvShowImage("Show Lena", image);
			
		// ESCもしくはqが押されたら終了させる
		char key = cvWaitKey (10);
		if (key == 27 || key == 'q') {
			break;
		}
	}

	// リソースの解放
	cvReleaseImage(&image);
	
	return 0;
}
