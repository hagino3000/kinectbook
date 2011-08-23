#include <stdlib.h>
#include <iostream>
#include <stdexcept>
#include <libusb-1.0/libusb.h>

// チルトモーターの製造元ID, 製品ID
#define VID_MICROSOFT 0x45e
#define PID_NUI_MOTOR 0x02b0

libusb_device_handle *dev;

int main (int argc, char * const argv[]) {
	
	int angle;
	if (argc == 1) {
		angle = 0;
	} else {
		// プログラム引数を角度とする
		angle = atoi(argv[1]);
	}
	
	std::cout << "Start libusb init" << std::endl;
	
	int rt;
	libusb_context *ctx = NULL;
	libusb_device **devs;
	
	try {
		rt = libusb_init(&ctx);
		
		// USB接続しているデバイス一覧の取得
		ssize_t cnt = libusb_get_device_list (ctx, &devs);		
		if (cnt < 0) {
			throw std::runtime_error("No devices on USB interface");
		}
		
		for (int i = 0; i < cnt; i++) {
			struct libusb_device_descriptor desc;
			const int r = libusb_get_device_descriptor (devs[i], &desc);
			if (r < 0) {
				std::cout << "Cannot get device descriptor" << std::endl;
				continue;
			}
			
			printf("Device: %i VID: %i PID: %i\n", i, desc.idVendor, desc.idProduct);
			
			// 製造元ID, 製品IDが一致する物を探す
			if (desc.idVendor == VID_MICROSOFT && 
				desc.idProduct == PID_NUI_MOTOR) {
				
				// 最初に見つかったKinectモーターをOpen
				if ((libusb_open (devs[i], &dev) != 0) || (dev == 0)) {
					throw std::runtime_error("Cannot open the NUI motor");
				}
				printf("Openning device No. %d\n", i);
				libusb_claim_interface (dev, 0);
				break;
			}
		}
		
		// デバイスリストの解放
		libusb_free_device_list(devs, 1);
		
		// メッセージの送信
		// @see http://openkinect.org/wiki/Protocol_Documentation
		uint8_t empty[0x1];		
		angle = angle * 2;
		const int ret = libusb_control_transfer(dev, 0x40, 0x31, (uint16_t)angle, 
												0x0, empty, 0x0, 0);
		if (ret != 0) {
			throw std::runtime_error("Failed to set tilt angle.");
		}
		
	} catch (std::exception& ex) {
		std::cout << ex.what() << std::endl;
	}
	
	if (ctx != NULL) {
		libusb_exit(ctx);
	}
    return 0;
}
