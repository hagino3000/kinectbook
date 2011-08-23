#include <iostream>

// Windows.hがMSR_NuiApi.hの前に必要
#include <Windows.h>
#include <MSR_NuiApi.h>

int main() {

    HRESULT hr;

    // デバイスの初期化
    hr = NuiInitialize(NUI_INITIALIZE_FLAG_USES_DEPTH);
    if (FAILED(hr)) {
        std::cout << "Error at initialize Kinect" << std::endl;
        return 1;
    }

    std::cout << "Open the Kinect succeeded." << std::endl;

    // デバイスのクローズ
    NuiShutdown();
    return 0;
}