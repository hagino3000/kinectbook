#include <stdexcept>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <XnCppWrapper.h>

// 設定ファイルのパス 自分の環境にあわせて書き変えてください
const char* CONFIG_XML_PATH = "/Users/takashi/dev/workspace/kinectbook/apps/recorder_sample/config.xml";
// 保存ファイル名
const char* RECORD_FILE_PATH = "KinectRecorderXXXX2.oni";

void check(XnStatus rc) {
	if (rc != XN_STATUS_OK) {
		throw std::runtime_error(xnGetStatusString(rc));
	}	
}

int main (int argc, char * const argv[]) {

	xn::Context context;
	IplImage *video = NULL;

	try {
		xn::EnumerationErrors ctxErrors;
		XnStatus rc = context.InitFromXmlFile(CONFIG_XML_PATH, 
											  &ctxErrors);
		if (rc != XN_STATUS_OK) {
			XnChar strError[1024];
			ctxErrors.ToString(strError, 1024);
			throw std::runtime_error(strError);
		}
		
		std::cout << "Create Generators" << std::endl;
		xn::ImageGenerator image;
		check(context.FindExistingNode(XN_NODE_TYPE_IMAGE, image));		
		xn::DepthGenerator depth;
		check(context.FindExistingNode(XN_NODE_TYPE_DEPTH, depth));

		// 表示用の画像データを作成
		XnMapOutputMode mapMode;
        image.GetMapOutputMode(mapMode);
        video = cvCreateImage(cvSize(mapMode.nXRes, mapMode.nYRes), 
								 IPL_DEPTH_8U, 3);
		
		// レコーダーの作成
		std::cout << "Setup Recorder" << std::endl;
		xn::Recorder recorder;
		check(recorder.Create(context));
		
		// 保存設定
		check(recorder.SetDestination(XN_RECORD_MEDIUM_FILE, 
									  RECORD_FILE_PATH));
		
		// 深度、ビデオカメラ入力を保存対象として記録開始
		check(recorder.AddNodeToRecording(image, XN_CODEC_NULL));
		check(recorder.AddNodeToRecording(depth, XN_CODEC_NULL));
		
		std::cout << "Start Recording...." << std::endl;
		check(recorder.Record());

		while(1) {
			context.WaitAndUpdateAll();

			// ビデオカメラ画像を表示
            xn::ImageMetaData imageMetaData;
            image.GetMetaData(imageMetaData);
            xnOSMemCopy(video->imageData, 
						imageMetaData.RGB24Data(), 
						video->imageSize);
            cvCvtColor(video, video, CV_RGB2BGR);
            cvShowImage("Video View", video);
            
            // ESCもしくはqが押されたら終了させる
            char key = cvWaitKey(10);
            if (key == 27 || key == 'q') {
                break;
            }        
			
		}
		
	} catch (std::exception& ex) {
		std::cout << ex.what() << std::endl;
	}
	if (video != NULL) cvReleaseImage(&video);
	context.Shutdown();
	return 0;
}

