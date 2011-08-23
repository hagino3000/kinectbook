#include <iostream>
#include <XnCppWrapper.h>
#include <XnLog.h>

int main (int argc, char * const argv[]) {
	
	// Logシステムの初期化
	xnLogInitSystem();
	// ファイルにログを出力しない
	xnLogSetFileOutput(false);
	// コンソール出力をONにする
	xnLogSetConsoleOutput(true);
	// ログレベルの閾値をINFOにセット
	xnLogSetSeverityFilter(XN_LOG_INFO);
	// 全てを表示
	xnLogSetMaskState("ALL", true);
	// Line InfoをONに
	xnLogSetLineInfo(true);

	xnLogVerbose(XN_MASK_LOG, "Message is : %s", "AAAA");	
	xnLogInfo(XN_MASK_LOG, "Message is %s", "BBBB");
	xnLogWarning(XN_MASK_LOG, "Message is %s", "CCCC");
	xnLogError(XN_MASK_LOG, "Message is %s", "DDDD");
	
	return 0;
}
