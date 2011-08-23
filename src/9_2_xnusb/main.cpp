#include <stdlib.h>
#include <stdexcept>
#include <iostream>
#include <XnCppWrapper.h>
#include <XnUSB.h>

#define VID_MICROSOFT 0x45e
#define PID_NUI_MOTOR 0x02b0

XN_USB_DEV_HANDLE dev;

void errorCheck(XnStatus rc) {
	if (rc != XN_STATUS_OK) {
		std::cout << xnGetStatusString(rc) << std::endl;
		throw std::runtime_error("Error");
	}
}

int main (int argc, char * const argv[]) {
	
	int angle;
	if (argc == 1) {
		angle = 0;
	} else {
		// Use program argument
		angle = atoi(argv[1]);
	}
	
	XnStatus rc = XN_STATUS_OK;
	
	rc = xnUSBInit();
	errorCheck(rc);
	
	rc = xnUSBOpenDevice(VID_MICROSOFT, PID_NUI_MOTOR, NULL, NULL, &dev);
	errorCheck(rc);
	
	XnUChar empty[0x1];
	angle = angle * 2;
		
	rc = xnUSBSendControl(dev, 
						  XN_USB_CONTROL_TYPE_VENDOR, 
						  0x31, 
						  (XnUInt16)angle, 
						  0x0, 
						  empty, 
						  0x0, 0);
	errorCheck(rc);
	
    rc = xnUSBCloseDevice(dev);
	errorCheck(rc);	
	return 0;
}
