#include <stdexcept>
#include <iostream>
#include <XnCppWrapper.h>

void errorCheck(XnStatus status, const char *msg) {
	if (status != XN_STATUS_OK) {
		std::cout << msg << std::endl;
		throw std::runtime_error(xnGetStatusString(status));
	}	
}

int main (int argc, char * const argv[]) {
	
	XnStatus rc = XN_STATUS_OK;
	xn::Context context;
	
	try {
		// OpenNIコンテキストの作成
		rc = context.Init();
		errorCheck(rc, "Failed at Initializing context");
		
		// DepthGeneratorの作成
		xn::DepthGenerator depth;
		rc = depth.Create(context);
		errorCheck(rc, "Failed at creating DepthGenerator");
		
		XnMapOutputMode mapMode;
		mapMode.nXRes = XN_VGA_X_RES;
		mapMode.nYRes = XN_VGA_Y_RES;
		mapMode.nFPS = 30;
		
		depth.SetMapOutputMode(mapMode);
		
		rc = context.StartGeneratingAll();
		errorCheck(rc, "Failed at StartGeneratingAll()");
		
		int roopCount = 0;
		while(roopCount++ < 200) {
			// ノードの更新を待つ
			context.WaitAndUpdateAll();
			
			// ノードからデータを取得
			xn::DepthMetaData depthMetaData;
			depth.GetMetaData(depthMetaData);
			const XnDepthPixel* depthPixels = depthMetaData.Data();
			
			// なんらかの処理を行なう
			std::cout << "Center Depth:" << depthPixels[240*320] << std::endl;			
		}

	} catch (std::exception& ex) {
		std::cout << ex.what() << std::endl;
	}
	
	context.Shutdown();
	return 0;
}
