#include <libfreenect_sync.h>
#include <stream.h>

// 11bit深度データをセンチに直す
// @see http://openkinect.org/wiki/Imaging_Information
inline float rawToCentimeters(unsigned short raw) {
	return 100/(-0.00307 * raw + 3.33);
}

int main (int argc, char * const argv[]) {
	
	short *depth;
	uint32_t timestamp;	

	while (1) {
		// 深度データの取得
		freenect_sync_get_depth((void**)&depth, &timestamp, 0, FREENECT_DEPTH_11BIT);
		
		// RAWデータをセンチメートルに変換
		int centerDepth = rawToCentimeters(depth[240*320]);
		
		std::cout << "Center Depth:" << centerDepth << "cm" << std::endl;
		
		sleep(1);
	}
	return 0;
}
