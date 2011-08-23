#include <iostream>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

int main (int argc, char* const argv[]) {
    
    // フォントの初期化
    CvFont dfont;
    cvInitFont(&dfont, CV_FONT_HERSHEY_SIMPLEX , 1.5, 1.5, 0.0, 1, CV_AA);    
    
    // 表示用の画像データの作成
    IplImage * rgbImage = cvCreateImage(cvSize(640, 480), IPL_DEPTH_8U, 3);
    
    int count = 0;
    while(1) {
        
        // 画像をゼロクリア
        cvZero(rgbImage);
        
        // 現在のカウントを文字列で描画
        count++;
        stringstream ss;
        ss << "Count: " << count;
        cvPutText(rgbImage, ss.str().c_str(), cvPoint(10, 40), &dfont, 
                  CV_RGB(255, 255, 255));
        
        // 四角形の描画
        cvRectangle(rgbImage, cvPoint(100, 100), cvPoint(200, 200), 
                    CV_RGB(255, 255, 0), 3, CV_AA);
        
        // 円の描画
        cvCircle(rgbImage, cvPoint(320, 240), 100, 
                 CV_RGB(0, 255, 255), 5, CV_AA);
        
        // 線分の描画
        cvLine(rgbImage, cvPoint(50, 400), cvPoint(500, 10), 
               CV_RGB(255, 0, 255), 4, CV_AA);
        
        // 画像の表示
        cvShowImage("Rect and Label", rgbImage);
        
        // ESCもしくはqが押されたら終了させる
        char key = cvWaitKey(10);
        if (key == 27 || key == 'q') {
            break;
        }
    }
    
    if (rgbImage != NULL) {
        cvReleaseImage(&rgbImage);    
    }
    
    return 0;
}
