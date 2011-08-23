#include <iostream>
#include <XnCppWrapper.h>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

// 設定ファイルのパス 自分の環境にあわせて書き変えてください
const char* CONFIG_XML_PATH = "/Users/takashi/dev/workspace/kinectbook/apps/openni_skeleton/config.xml";

IplImage *rgbImage = NULL;
CvFont calibFont, jointFont;

void errorCheck(XnStatus status) {
	if (status != XN_STATUS_OK) {
		throw std::runtime_error(xnGetStatusString(status));
	}	
}

// キャリブレーションステータス
typedef enum {
	JUST_GENERATED,
	USER_DETECTED,
	USER_LOST,
	POSE_DETECTED,
	POSE_LOST,
	CALIB_STARTED,
	CALIB_FAILED,
	CALIB_SUCCEEDED
} CALIBRATION_STATUS;

// コールバック関数とやりとりするデータ
struct CalibrationCookie {
	xn::UserGenerator * user;
	xn::DepthGenerator * depth;
	CALIBRATION_STATUS lastStatus;
	XnChar * pose;
};

// 新たにユーザーを検出した時のコールバック
void XN_CALLBACK_TYPE NewUser(xn::UserGenerator& generator, XnUserID userId, void* pCookie) {
	// ポーズ検出を開始する
	CalibrationCookie* cookie = (CalibrationCookie*)pCookie;
	cookie->lastStatus = USER_DETECTED;
	cookie->user->GetPoseDetectionCap().StartPoseDetection(cookie->pose, userId);
}

// ユーザーの消失を検知した時のコールバック
void XN_CALLBACK_TYPE LostUser(xn::UserGenerator& generator, XnUserID userId, void* pCookie) {
	CalibrationCookie* cookie = (CalibrationCookie*)pCookie;
	cookie->lastStatus = USER_LOST;
}

// ポーズを検出した時のコールバック
void XN_CALLBACK_TYPE DetectedPose(xn::PoseDetectionCapability& capability, 
								   const XnChar* strPose, XnUserID userId, void* pCookie) {
	CalibrationCookie* cookie = (CalibrationCookie*)pCookie;
	// ポーズ検出を中止してキャリブレーションを開始する
	cookie->user->GetPoseDetectionCap().StopPoseDetection(userId);
	cookie->user->GetSkeletonCap().RequestCalibration(userId, TRUE);
	cookie->lastStatus = POSE_DETECTED;
}

// ポーズを消失した時のコールバック
void XN_CALLBACK_TYPE LostPose(xn::PoseDetectionCapability& capability, 
								   const XnChar* strPose, XnUserID userId, void* pCookie) {
	CalibrationCookie* cookie = (CalibrationCookie*)pCookie;
	// ポーズ検出を再開する
	cookie->user->GetPoseDetectionCap().StartPoseDetection(cookie->pose, userId);
	cookie->lastStatus = POSE_LOST;
}

// キャリブレーションを開始した時のコールバック
void XN_CALLBACK_TYPE StartCalibration(xn::SkeletonCapability& capability, XnUserID userId, void* pCookie) {
	CalibrationCookie* cookie = (CalibrationCookie*)pCookie;
	cookie->lastStatus = CALIB_STARTED;
}

// キャリブレーションが終了(成功 or 失敗)した時のコールバック
void XN_CALLBACK_TYPE EndCalibration(xn::SkeletonCapability& capability, XnUserID userId, XnBool bSuccess, void* pCookie) {
	CalibrationCookie* cookie = (CalibrationCookie*)pCookie;
	if (bSuccess) {
		cookie->user->GetSkeletonCap().StartTracking(userId);
		cookie->lastStatus = CALIB_SUCCEEDED;
	} else {
		// 失敗、ポーズ検出からやりなおす
		cookie->user->GetPoseDetectionCap().StartPoseDetection(cookie->pose, userId);
		cookie->lastStatus = CALIB_FAILED;
	}
}

// ジョイント情報を描画する
void drawJointPoint(xn::DepthGenerator& depth, xn::SkeletonCapability& capability, XnSkeletonJoint joint, std::string name, XnUserID user) {
	XnSkeletonJointPosition pos;
	// ジョイント座標の取得
	capability.GetSkeletonJointPosition(user, joint, pos);
	XnPoint3D pReal[1] = {pos.position};
	XnPoint3D pProjective[1];
	// 世界座標系から表示座標系に変換した座標を取得
	depth.ConvertRealWorldToProjective(1, pReal, pProjective);

	// ジョイントの位置に円を描画
	cvCircle(rgbImage, cvPoint(pProjective[0].X, pProjective[0].Y), 6, CV_RGB(255,255,255), 2, CV_AA, 0);
	// ジョイントの名前を描画
	cvPutText(rgbImage, name.c_str(), cvPoint(pProjective[0].X+10, pProjective[0].Y+10), &jointFont, CV_RGB(255, 230, 50));
}

int main (int argc, char* const argv[]) {
	
	xn::Context context;
	
    cvInitFont(&jointFont, CV_FONT_HERSHEY_SIMPLEX , 0.5, 0.5, 0.0, 1, CV_AA);	
	cvInitFont(&calibFont, CV_FONT_HERSHEY_SIMPLEX , 1, 1, 0.0, 1, CV_AA);	
	
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
		
		// cookieの作成
		XnChar pose[20] = "";
		CalibrationCookie cookie = {&user, &depth, JUST_GENERATED, pose};
		
		// コールバックの登録
		XnCallbackHandle userCallbacks, poseCallbacks, calibCallbacks;
		user.RegisterUserCallbacks(NewUser, LostUser, &cookie, userCallbacks);
		
		// PoseCapabilityの取得
		xn::PoseDetectionCapability poseCap = user.GetPoseDetectionCap();
		poseCap.RegisterToPoseCallbacks(&DetectedPose, &LostPose, &cookie, poseCallbacks);

		// SkeletonCapabilityの取得
		xn::SkeletonCapability skeletonCap = user.GetSkeletonCap();
		skeletonCap.GetCalibrationPose(pose);
		skeletonCap.RegisterCalibrationCallbacks(&StartCalibration, &EndCalibration, &cookie, calibCallbacks);
		skeletonCap.SetSkeletonProfile(XN_SKEL_PROFILE_ALL);

		
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
			
			XnUserID allUsers[20];
			XnUInt16 nUsers = 20;
			user.GetUsers(allUsers, nUsers);
			for (int i = 0; i < nUsers; i++) {
				
				// キャリブレーションに成功しているかどうか
				if (skeletonCap.IsTracking(allUsers[i])) {
					
					// 各ジョイント情報の描画
					XnUserID u = allUsers[i];
					drawJointPoint(depth, skeletonCap, XN_SKEL_HEAD, "HEAD", u);
					drawJointPoint(depth, skeletonCap, XN_SKEL_NECK, "NECK", u);
					drawJointPoint(depth, skeletonCap, XN_SKEL_TORSO, "TORSO", u);
					drawJointPoint(depth, skeletonCap, XN_SKEL_LEFT_ELBOW, "LEFT ELBOW", u);
					drawJointPoint(depth, skeletonCap, XN_SKEL_RIGHT_ELBOW, "RIGHT ELBOW", u);
					drawJointPoint(depth, skeletonCap, XN_SKEL_LEFT_HIP, "LEFT HIP", u);
					drawJointPoint(depth, skeletonCap, XN_SKEL_RIGHT_HIP, "RIGHT HIP", u);
					drawJointPoint(depth, skeletonCap, XN_SKEL_LEFT_KNEE, "LEFT KNEE", u);
					drawJointPoint(depth, skeletonCap, XN_SKEL_RIGHT_KNEE, "RIGHT KNEE", u);
					drawJointPoint(depth, skeletonCap, XN_SKEL_LEFT_HAND, "LEFT HAND", u);
					drawJointPoint(depth, skeletonCap, XN_SKEL_RIGHT_HAND, "RIGHT HAND", u);
					drawJointPoint(depth, skeletonCap, XN_SKEL_LEFT_SHOULDER, "SHOULDER", u);
					drawJointPoint(depth, skeletonCap, XN_SKEL_RIGHT_SHOULDER, "SHOULDER", u);
					drawJointPoint(depth, skeletonCap, XN_SKEL_LEFT_FOOT, "LEFT FOOT", u);
					drawJointPoint(depth, skeletonCap, XN_SKEL_RIGHT_FOOT, "RIGHT FOOT", u);
					
					// 最初の一人だけ処理したら抜ける
					break;
				}
			}

			// 現在のキャリブレーション状況を表示
			std::string strStatus;
			CALIBRATION_STATUS status = cookie.lastStatus;
			if (status == JUST_GENERATED) {
				strStatus = "Just Generated";
			} else if (status == USER_DETECTED) {
				strStatus = "User Detected";
			} else if (status == USER_LOST) {
				strStatus = "User Lost";
			} else if (status == POSE_DETECTED) {
				strStatus = "Pose Detected";
			} else if (status == POSE_LOST) {
				strStatus = "Pose Lost";
			} else if (status == CALIB_STARTED) {
				strStatus = "Calibration Started";
			} else if (status == CALIB_SUCCEEDED) {
				strStatus = "Tracking Started";
			} else {
				strStatus = "Calibration Failed";
			}
			cvPutText(rgbImage, strStatus.c_str(), cvPoint(10, 40), &calibFont, CV_RGB(255, 255, 255));
						
			// 表示
			cvShowImage("User View", rgbImage);
					
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
