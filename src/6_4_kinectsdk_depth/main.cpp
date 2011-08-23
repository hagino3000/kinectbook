#include <iostream>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

// Windows.hがMSR_NuiApi.hの前に必要
#include <Windows.h>
#include <MSR_NuiApi.h>

int main() {

    IplImage * image = NULL, * tmpImage = NULL;
    HRESULT hr;

    try {
        // デバイスの初期化
        hr = NuiInitialize(NUI_INITIALIZE_FLAG_USES_DEPTH);
        if (FAILED(hr)) {
            throw std::runtime_error("Failed open the Kinect");
        }

        // 深度画像ストリームのオープン
        HANDLE nextImageEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        HANDLE nextFrameHandle = NULL;
        hr = NuiImageStreamOpen(NUI_IMAGE_TYPE_DEPTH,
                                NUI_IMAGE_RESOLUTION_640x480, 
                                NULL, 2, nextImageEvent, 
                                &nextFrameHandle);
        if (FAILED(hr)) {
            throw std::runtime_error("Failed at opening stream");
        }

        // OpenCVの画像データの作成
        int width = 640, height = 480, numPixels = width * height;
        image = cvCreateImage(cvSize(width, height), 
                              IPL_DEPTH_8U, 1);
        tmpImage = cvCreateImage(cvSize(width, height), 
                                 IPL_DEPTH_8U, 1);

        // グレースケールピクセルデータ
        unsigned char * grayPixels = new unsigned char[numPixels];
        memset(grayPixels, 0, numPixels * sizeof(unsigned char));

        while(1) {

            // 深度画像が更新されるのを待つ
            WaitForSingleObject(nextImageEvent, INFINITE);

            // フレームの取得
            CONST NUI_IMAGE_FRAME *imageFrame = 0;
            hr = NuiImageStreamGetNextFrame(nextFrameHandle, 0, 
                                            &imageFrame);
            if (FAILED(hr)) {
                std::cout << "Failed getting frame" << std::endl;
                break;
            }

            NuiImageBuffer * pTexture = imageFrame->pFrameTexture;
            KINECT_LOCKED_RECT LockedRect;
            pTexture->LockRect( 0, &LockedRect, NULL, 0 );

            if( LockedRect.Pitch != 0 ){
                BYTE * pBuffer = (BYTE*) LockedRect.pBits;
                // 取得できる値は80から4000
                // 4000が255になる様な8bitグレースケールに変換する
                USHORT * pBufferRun = (USHORT*) pBuffer;
                for(int i = 0; i < numPixels; i++) {
                    pBufferRun++;
                    grayPixels[i] = (unsigned short)(*pBufferRun*255/4000);
                }

                // メモリコピー
                memcpy(tmpImage->imageData, grayPixels, 
                       tmpImage->widthStep * tmpImage->height);
                // 左右反転
                cvFlip(tmpImage, image, 1);
                // 表示
                cvShowImage("Depth View", image);
            } else {
               // NOP
            }
            // フレームバッファの開放
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
    if (tmpImage != NULL) {
        cvReleaseImage(&tmpImage);
    }
    return 0;
}
