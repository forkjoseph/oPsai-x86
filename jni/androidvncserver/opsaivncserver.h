#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>             /* For makedev() */
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/syscall.h>

#include <fcntl.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <netinet/in.h>
#include <net/if.h>

#include <assert.h>
#include <errno.h>

/* libvncserver */
#include "rfb/rfb.h"
#include "rfb/keysym.h"

/* framebuffer and input */
#include "fb.h"
#include "input.h"

/* Multithreading */
#include <pthread.h>

/* Eureka magic ;> */
#define PIXEL_FB_TO_RFB(p,r,g,b) (((p>>r)<<16)&0x00ffffff)|((((p>>g))<<8)&0x00ffffff)|(((p>>b)&0x00ffffff))
#define TRUE_VAL 0xff00ff
#define GREEN_VAL 0x00ff00
#define NUM_FB 0x000003 // seq = seq & NUM_FB -> moduolus 3
#define FB_DEVICE "/dev/graphics/fb0"
#define VNC_PORT 5901 //default VNC port is 5900 but we use 5901
#define PROCESS_TIME 100000

static rfbScreenInfoPtr vncscr;
struct fb fb;
static int fb_des;
ssize_t read_size;
unsigned char *raw;
unsigned int bytespp;
unsigned int raw_size;
unsigned int raw_line_length;
static int xmin, xmax;
static int ymin, ymax;

static unsigned int *fbmmap = MAP_FAILED;
static unsigned int *vncbuf;
static unsigned int *fbbuf;
static struct fb_fix_screeninfo finfo;
static struct fb_var_screeninfo vinfo;
static int kbdfd = -1;
static int touchfd = -1;

static struct varblock_t
{
	int min_i;
	int min_j;
	int max_i;
	int max_j;
	int r_offset;
	int g_offset;
	int b_offset;
	int rfb_xres;
	int rfb_maxy;
} varblock;

/* traditional */
static void keyevent(rfbBool down, rfbKeySym key, rfbClientPtr cl);
static void ptrevent(int buttonMask, int x, int y, rfbClientPtr cl);
static void init_private_fb(struct fb* fb);
static void* private_fb(struct fb* fb);
static void dump_fb(const struct fb* fb);
static int get_fb_format(const struct fb *fb);
static char* find_fb_format(const struct fb *fb);

typedef struct framebuffer;

/*
 *  multithreading framebuffer struct
 *  Use pread(int fildes, void *buf, size_t nbyte, off_t offset);
 *  1280 x 800 => x * y
 *  Suppose we use 4 threads to read, each take 0-199, 200-399, 400-599, 600-799
 */
struct framebuffer {
	int seq; // 0 - 3 inclusive
	void *buf;
	size_t size;
	off_t offset;
};

pthread_t *thrs;
static pthread_mutex_t frame_mutex;
static pthread_cond_t frame_cond;

void private_fb_create();
void* private_fb_read(int);















