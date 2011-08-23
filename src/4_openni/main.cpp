#include <iostream>
#include <XnCppWrapper.h>
#include <XnVSessionManager.h>

// 設定ファイルのパス 自分の環境にあわせて書き変えてください
const char* CONFIG_XML_PATH = "/Users/takashi/dev/lib/forKinect/Nite-1.3.1.4/Data/Sample-Tracking.xml";

int main (int argc, char * const argv[]) {
	
	// OpenNIコンテキストの作成
	xn::Context context;
	xn::EnumerationErrors ctxErrors;
	XnStatus rc = context.InitFromXmlFile(CONFIG_XML_PATH, &ctxErrors);
	if (rc != XN_STATUS_OK) {
		XnChar strError[1024];
		ctxErrors.ToString(strError, 1024);
		std::cout << strError << std::endl;
		return 1;
	}
	
	// NITEのセッション初期化
	XnVSessionManager sessionManager;
	rc = sessionManager.Initialize(&context, "Wave,Click", "RaiseHand");
	if (rc != XN_STATUS_OK) {
		std::cout << "Error at initialize NITE session" << std::endl;
		return 1;
	}
	
	// ライセンス情報の表示
	xnPrintRegisteredLicenses();
	
	std::cout << "OK!! OpenNI context and NITE session initialized" << std::endl;
	
	return 0;
}
