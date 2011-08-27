#include <iostream>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

// Windows.hがMSR_NuiApi.hの前に必要
#include <Windows.h>
#include <MSR_NuiApi.h>

int main() {

	IplImage * image = NULL;
	HRESULT hr;

	try {
		// デバイスの初期化
		hr = NuiInitialize(NUI_INITIALIZE_FLAG_USES_COLOR);
		if (FAILED(hr)) {
			throw std::runtime_error("Failed at initializing Kinect");
		}

		// ビデオカメラストリームのオープン
		HANDLE nextImageEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		HANDLE nextFrameHandle = NULL;
		hr = NuiImageStreamOpen(NUI_IMAGE_TYPE_COLOR, 
			                    NUI_IMAGE_RESOLUTION_640x480, 
								NULL, 2, nextImageEvent, &nextFrameHandle);
		if (FAILED(hr)) {
			throw std::runtime_error("Failed at opening image stream");
		}

		// OpenCVの画像データの作成
		DWORD width = 0, height = 0;
		NuiImageResolutionToSize(NUI_IMAGE_RESOLUTION_640x480, width, height);

		image = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 4);
		if (image == 0) {
			throw std::runtime_error("Failed at creating image");
		}

		while(1) {

			// ビデオカメラ画像が更新されるのを待つ
			WaitForSingleObject(nextImageEvent, INFINITE);

			// フレームの取得
			CONST NUI_IMAGE_FRAME *imageFrame = 0;
			hr = NuiImageStreamGetNextFrame(nextFrameHandle, 0, &imageFrame);
			if (FAILED(hr)) {
				std::cout << "Failed at getting next frame data" << std::endl;
				break;
			}

			NuiImageBuffer * pTexture = imageFrame->pFrameTexture;
			KINECT_LOCKED_RECT LockedRect;
			pTexture->LockRect( 0, &LockedRect, NULL, 0 );

			if( LockedRect.Pitch != 0 ){
				BYTE * pBuffer = (BYTE*) LockedRect.pBits;
				// メモリコピー
				memcpy( image ->imageData, pBuffer, image->widthStep * image->height);
				// 表示
				cvShowImage("Video View", image);
			} else {
			   // NOP
			}

			// フレームバッファの解放
			NuiImageStreamReleaseFrame(nextFrameHandle, imageFrame);

			// ESCもしくはqが押されたら終了させる
			char key = cvWaitKey(10);
			if (key == 27 || key == 'q') {
				break;
			}		
		}

	} catch(std::exception &ex) {
		std::cout << ex.what() << std::endl;
	}

	// デバイスのクローズ
	NuiShutdown();

	if (image != NULL) {
		cvReleaseImage(&image);
	}
	return 0;
}