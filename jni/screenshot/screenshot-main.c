#include "fb2png.h"
#include "log.h"

// for native asset manager
#include <sys/types.h>
#include <assert.h>


//fb test
#include <linux/fb.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>

/* Let's take a picture of framebuffer*/
int fbfd = -1;
unsigned int *fbmmap;

char framebuffer_device[256] = "/dev/graphics/fb0";

struct fb_var_screeninfo scrinfo;
struct fb_fix_screeninfo fscrinfo;
#define DEFAULT_SAVE_PATH "/sdcard/fbdump.png"
// #define DEFAULT_SAVE_PATH "/data/local/fbdump.png"
#define L(...) LOGV( __VA_ARGS__)

//E/AndroidRuntime(24842): java.lang.UnsatisfiedLinkError: Native method not found: org.onaips.vnc.ScreenshotManager.takeScreenShot:()V
//void org.onaips.vnc.ScreenshotManager.takeScreenShot()
/*void Java_org_onaips_vnc_ScreenshotManager_takeScreenShot(JNIEnv* env, jobject obj) {
	fbmmap = MAP_FAILED;




	if ((fbfd = open(framebuffer_device, O_RDWR)) == -1) {
		L("Cannot open fb device %s\n", framebuffer_device);
		return -1;
	}


	if (ioctl(fbfd, FBIOGET_FSCREENINFO, &fscrinfo) != 0) {
		L("ioctl error\n");
		return -1;
	}

	unsigned char *raw;
	raw = malloc(fscrinfo.line_length * scrinfo.yres);

	if (!raw) {
	  L("*********** raw malloc error :( ");
	} else
	  L("*********** raw malloc no error :)  ");

	free(raw);

	struct timeval  tv;
	gettimeofday(&tv, NULL);

	double time_in_mill =
	         (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000 ; // convert tv_sec & tv_usec to millisecond

	char *path;
	path = DEFAULT_SAVE_PATH;
	int ret = fb2png(path);
	if (!ret)
	  printf("***********Image saved to %s\n", path);

} */


int main(int argc, char **argv)
{
	L("Entering screenshot-main ");

	//fbmmap = MAP_FAILED;
	L("Initializing fbmmap to MAP_FAILED");

	L("Opening fb");
	if ((fbfd = open(framebuffer_device, O_RDWR)) == -1) {
		L("Cannot open fb device %s\n", framebuffer_device);
		return -1;
	}
	L("Opened fb");

	if (ioctl(fbfd, FBIOGET_FSCREENINFO, &fscrinfo) != 0) {
		L("ioctl error\n");
		return -1;
	}

	unsigned char *raw;
	raw = malloc(fscrinfo.line_length * scrinfo.yres);

	if (!raw) {
	  L("*********** raw malloc error :( ");
	} else
	  L("*********** raw malloc no error :)  ");

	free(raw);


	char *path;
	path = DEFAULT_SAVE_PATH;
	int ret = fb2png(path);
	if (!ret)
	  printf("***********Image saved to %s\n", path);
	return 0;
}
