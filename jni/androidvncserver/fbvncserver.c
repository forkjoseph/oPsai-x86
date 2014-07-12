#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>             /* For makedev() */
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <netinet/in.h>
#include <net/if.h>

/* Multithreading */
#include <pthread.h>

#include <assert.h>
#include <errno.h>

/* libvncserver */
#include "rfb/rfb.h"
#include "rfb/keysym.h"

/* framebuffer and input */
#include "fb.h" 
#include "input.h"

#define FB_DEVICE "/dev/graphics/fb0"
static struct fb_var_screeninfo scrinfo;
static struct fb_fix_screeninfo finfo;
static int fbfd = -1;
static int kbdfd = -1;
static int touchfd = -1;
static unsigned int *fbmmap = MAP_FAILED;
static unsigned int *vncbuf;
static unsigned int *fbbuf;

/* Android already has 5900 bound natively. */
#define VNC_PORT 5901
static rfbScreenInfoPtr vncscr;

static int xmin, xmax;
static int ymin, ymax;

/* No idea, just copied from fbvncserver as part of the frame differerencing
 * algorithm.  I will probably be later rewriting all of this. */
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

/*****************************************************************************/

static void keyevent(rfbBool down, rfbKeySym key, rfbClientPtr cl);
static void ptrevent(int buttonMask, int x, int y, rfbClientPtr cl);

/*****************************************************************************/

static void init_private_fb(struct fb* fb);
static void* private_fb(struct fb* fb);
static void dump_fb(const struct fb* fb);
static int get_fb_format(const struct fb *fb);
static char* find_fb_format(const struct fb *fb);
struct fb fb;
struct fb_var_screeninfo vinfo;
unsigned char *raw; 
unsigned int bytespp;
unsigned int raw_size;
unsigned int raw_line_length;
ssize_t read_size;
int fb_temp;
#define PROCESS_TIME 100000
/*****************************************************************************/

static void init_fb(void)
{
	size_t pixels;
	size_t bytespp;

	pixels = vinfo.xres * vinfo.yres;
	bytespp = vinfo.bits_per_pixel / 8;

	printf("Initializing frame buffer...\n");

	init_private_fb (&fb);
	fbmmap = private_fb(&fb);

	printf("%15s : %p\n", "fbmmap addr", fbmmap);
	printf("%15s : %x\n", "fbmmap 1s color", fbmmap[0]);

	printf("%15s : %x\n", "current color", fbmmap[(1280*600) + 640]);

 //    fbmmap = mmap (NULL, raw_size, PROT_READ | PROT_WRITE, MAP_SHARED , fb_temp,0);
 //    int i,j;
	// for (i=0; i < 400 ; i++ ) {
	// 	for(j = 0; j < 1280; j++){
	// 		*(fbmmap+(i*1280) + j) = 0xff0000ff;
	// 	}
	// }
	// printf("%15s : %p\n", "fbmmap addr", fbmmap);
	// printf("%15s : %x\n", "current color", fbmmap[0]);

	if (fbmmap == MAP_FAILED)
	{
		printf("mmap failed\n");
		exit(EXIT_FAILURE);
	}
	printf("\n");
}



static void init_private_fb(struct fb* fb) {
	if ((fb_temp = open(FB_DEVICE, O_RDWR)) < 0 ) 
    	return;

    if (ioctl(fb_temp, FBIOGET_FSCREENINFO, &finfo) != 0)
	{
		printf("ioctl error from finfo\n");
		// close(fb_temp);
		exit(EXIT_FAILURE);
	}

	if (ioctl(fb_temp, FBIOGET_VSCREENINFO, &vinfo) != 0 )
	{
		printf("ioctl error\n");
		// close(fb_temp);
		exit(EXIT_FAILURE);
	}

	bytespp = vinfo.bits_per_pixel / 8;
	raw_line_length = finfo.line_length;
	raw_size = vinfo.yres * raw_line_length;

	fb->bpp = vinfo.bits_per_pixel;
    fb->size = vinfo.xres * vinfo.yres * bytespp;
    fb->width = vinfo.xres;
    fb->height = vinfo.yres;
    fb->red_offset = vinfo.red.offset;
    fb->red_length = vinfo.red.length;
    fb->green_offset = vinfo.green.offset;
    fb->green_length = vinfo.green.length;
    fb->blue_offset = vinfo.blue.offset;
    fb->blue_length = vinfo.blue.length;
    fb->alpha_offset = vinfo.transp.offset;
    fb->alpha_length = vinfo.transp.length;

    dump_fb(fb);
    printf("%15s : %u\n", "bytespp", bytespp);
    printf("%15s : %u\n", "raw size", raw_size);
}

static void* private_fb( struct fb *fb){
    raw = malloc(raw_size);
    if (!raw) {
    	printf("raw: memory error\n");
    	// close(fb_temp);
    	return NULL;
    }
	unsigned int active_buffer_offset = 0;
 	int num_buffers = 0;

    lseek(fb_temp, 0, SEEK_SET);
    read_size = read(fb_temp, raw, raw_size); // this makes so slow
//  	if (read_size < 0)
//  		printf("%15s : %d\n", "buffer size", read_size);

    if (read_size < 0 || (unsigned)read_size != raw_size) {
    	printf("read_size: read error\n");
    	// close(fb_temp);
	    free(raw);
		exit(EXIT_FAILURE);
	}
    fb->data = raw;

    return fb->data;
}
// freeze thing -> should close framebuffer and reopen it 


static void init_fb_server(int argc, char **argv)
{
	printf("\nInitializing server...\n");

	/* Allocate the VNC server buffer to be managed (not manipulated) by 
	 * libvncserver. */
	vncbuf = calloc(vinfo.xres * vinfo.yres, vinfo.bits_per_pixel / 2);
	assert(vncbuf != NULL);

	/* Allocate the comparison buffer for detecting drawing updates from frame
	 * to frame. */
	fbbuf = calloc(vinfo.xres * vinfo.yres, vinfo.bits_per_pixel / 2);
	assert(fbbuf != NULL);

	/* A pixel is one dot on the screen. The number of bytes in a pixel will depend 
	on the number of samples in that pixel and the number of bits in each sample.
	 A sample represents one of the primary colors in a color model. 
	 The RGB color model uses red, green, and blue samples respectively. 
	 Suppose you wanted to use 16-bit RGB color: 
	 You would have three samples per pixel (one for each primary color), 
	 five bits per sample (the quotient of 16 RGB bits divided by three samples),
	 and two bytes per pixel (the smallest multiple of eight bits in which the 16-bit pixel will fit). 
	 If you wanted 32-bit RGB color, you would have three samples per pixel again, 
	 eight bits per sample (since that's how 32-bit color is defined), 
	 and four bytes per pixel (the smallest multiple of eight bits in which the 32-bit pixel will fit.*/

	vncscr = rfbGetScreen(&argc, argv, vinfo.xres, vinfo.yres, 8, 4, 4);
	assert(vncscr != NULL);

	/*
					int bitsPerSample,int samplesPerPixel, int bytesPerPixel	 
	16 bit Gray     2 bytes per pixel 1 sample  per pixel 16 bits per sample
	24 bit RGB      3 bytes per pixel 3 samples per pixel  8 bits per sample
	48 bit RGB      6 bytes per pixel 3 samples per pixel 16 bits per sample
	from http://stuff.mit.edu/afs/sipb/project/scanner/bin/html/vuesc13.htm
	*/

	vncscr->desktopName = "oPsai";
	vncscr->frameBuffer = (char *)vncbuf;
	vncscr->alwaysShared = TRUE;
	vncscr->httpDir = NULL;
	vncscr->port = VNC_PORT;

	vncscr->kbdAddEvent = keyEvent;
	vncscr->ptrAddEvent = ptrEvent;
	vncscr->serverFormat.trueColour = TRUE; 
	vncscr->depth = vncscr->serverFormat.depth = 24;
	vncscr->alwaysShared = TRUE;
	vncscr->handleEventsEagerly = TRUE;

	printf("%15s : %d\n", "depth", vncscr->serverFormat.depth);
	printf("%15s : %d\n", "bitsPerPixel", vncscr->serverFormat.bitsPerPixel);
	printf("%15s : %d\n", "redMax", vncscr->serverFormat.redMax);
	printf("%15s : %d\n", "greenMax",vncscr->serverFormat.greenMax );
	printf("%15s : %d\n", "blueMax", vncscr->serverFormat.blueMax);
	printf("%15s : %d\n", "redShift", vncscr->serverFormat.redShift);
	printf("%15s : %d\n", "greenShift", vncscr->serverFormat.greenShift);
	printf("%15s : %d\n", "blueShift", vncscr->serverFormat.blueShift);

	rfbInitServer(vncscr);
	rfbMarkRectAsModified(vncscr, 0, 0, vinfo.xres, vinfo.yres);

	varblock.r_offset = vinfo.red.offset + vinfo.red.length - 8;
	varblock.g_offset = vinfo.green.offset + vinfo.green.length - 8;
	varblock.b_offset = vinfo.blue.offset + vinfo.blue.length - 8;
	varblock.rfb_xres = vinfo.yres;
	varblock.rfb_maxy = vinfo.xres;
}

/* Eureka magic ;> */
#define PIXEL_FB_TO_RFB(p,r,g,b) (((p>>r)<<16)&0x00ffffff)|((((p>>g))<<8)&0x00ffffff)|(((p>>b)&0x00ffffff))
#define TRUE_VAL 0xff00ff
#define GREEN_VAL 0x00ff00
#include <sys/time.h>

static void update_screen(void)
{
	// printf("updateing\n");
	unsigned int *f, *c, *r;
	int x, y;

	varblock.min_i = varblock.min_j = 9999;
	varblock.max_i = varblock.max_j = -1;

	struct timeval stop, start;
	gettimeofday(&start, NULL);


	free(fbmmap);
	fbmmap = private_fb(&fb); // apprx 0.1s 

	gettimeofday(&stop, NULL);
	// printf("private_fb time: %lu\n", stop.tv_usec - start.tv_usec); 

	f = (unsigned int *)fbmmap;        /* -> framebuffer         */
	c = (unsigned int *)fbbuf;         /* -> compare framebuffer */
	r = (unsigned int *)vncbuf;        /* -> remote framebuffer  */
	// printf("f: %p   c: %p   r: %p\n", f,c,r);

	for (y = 0; y < vinfo.yres; y++)
	{
		/* faster ? increase compression level -_- */
		for (x = 0; x < vinfo.xres; x ++)
		{
			unsigned int pixel = *f;

			if (pixel != *c)
			{
				*c = pixel;
				*r = PIXEL_FB_TO_RFB(pixel, varblock.r_offset, varblock.g_offset, varblock.b_offset);
				*r = ((*r & GREEN_VAL) | (((*r & TRUE_VAL) <<16) | ((*r & TRUE_VAL) >> 16))); 
				if (x < varblock.min_i)
					varblock.min_i = x;
				else
				{
					if (x > varblock.max_i)
						varblock.max_i = x;

					if (y > varblock.max_j)
						varblock.max_j = y;
					else if (y < varblock.min_j)
						varblock.min_j = y;
				}
			}

			f++, c++;
			r++;
		}
	}

	if (varblock.min_i < 9999)
	{
		if (varblock.max_i < 0)
			varblock.max_i = varblock.min_i;

		if (varblock.max_j < 0)
			varblock.max_j = varblock.min_j;

		/*fprintf(stderr, "Dirty page: %dx%d+%d+%d...\n",
		  (varblock.max_i+2) - varblock.min_i, (varblock.max_j+1) - varblock.min_j,
		  varblock.min_i, varblock.min_j);*/

		rfbMarkRectAsModified(vncscr, varblock.min_i, varblock.min_j,varblock.max_i + 2, varblock.max_j + 1);

		rfbProcessEvents(vncscr, PROCESS_TIME);
	}
}

static void dump_fb(const struct fb* fb)
{
    printf("%15s : %d\n", "bpp", fb->bpp);
    printf("%15s : %d\n", "size", fb->size);
    printf("%15s : %d\n", "width", fb->width);
    printf("%15s : %d\n", "height", fb->height);
    char *c;
    c =  find_fb_format(fb);
	printf("%15s : %s\n", "fb foramt", c);
    printf("%s %s : %d %d %d %d\n", c, "offset",
            fb->alpha_offset, fb->red_offset,
            fb->green_offset, fb->blue_offset);
    printf("%s %s : %d %d %d %d\n", c, "length",
            fb->alpha_length, fb->red_length,
            fb->green_length, fb->blue_length);
}

static char* find_fb_format(const struct fb *fb) {
	int x; // for future use:) 
	// x = get_fb_format(fb);
	char *c;
	// switch(x) {
	// 	case 0 : 
	// 		c = "unknown";
	// 		break;
	// 	case 1:
	// 		c = "RGB565";
	// 		break;
	// 	case 2:
	// 		c = "ARGB8888";
	// 		break;
	// 	case 3:
	// 		c = "RGBA8888";
	// 		break;
	// 	case 4:
	// 		c = "ABGR8888";
	// 		break;
	// 	case 5:
	// 		c = "BGRA8888";
	// 		break;
	// }
	//just for galaxy tab 3
	c = "ARGB8888";
	return c;
}

static int get_fb_format(const struct fb *fb)
{
    int ao = fb->alpha_offset;
    int ro = fb->red_offset;
    int go = fb->green_offset;
    int bo = fb->blue_offset;

#define FB_FORMAT_UNKNOWN   0
#define FB_FORMAT_RGB565    1
#define FB_FORMAT_ARGB8888  2
#define FB_FORMAT_RGBA8888  3
#define FB_FORMAT_ABGR8888  4
#define FB_FORMAT_BGRA8888  5
#define FB_FORMAT_RGBX8888  FB_FORMAT_RGBA8888

    /* TODO: use offset */
    if (fb->bpp == 16)
        return FB_FORMAT_RGB565;

    /* TODO: validate */
    if (ao == 0 && ro == 8)
        return FB_FORMAT_ARGB8888;

    if (ao == 0 && ro == 24 && go == 16 && bo == 8)
        return FB_FORMAT_RGBX8888;

    if (ao == 0 && bo == 8)
        return FB_FORMAT_ABGR8888;

    if (ro == 0)
        return FB_FORMAT_RGBA8888;

    if (bo == 0)
        return FB_FORMAT_BGRA8888;

    /* fallback */
    return FB_FORMAT_UNKNOWN;
}


/*****************************************************************************/
static void cleanup_fb(void)
{
	if(fbfd != -1)
	{
		close(fbfd);
	}
}

int pid;
int main(int argc, char **argv)
{
	printf("\n======= oPsai VNCServer Daemon initiating =======\n");
	pid = getpid();
	printf("%5s: %d\n", "PID", pid);

	printf("Initializing framebuffer device " FB_DEVICE "...\n");
	init_fb();


	initInput();

	init_fb_server(argc, argv);

	while (1)
	{
		while (vncscr->clientHead == NULL)
 			rfbProcessEvents(vncscr, 100000);
		rfbProcessEvents(vncscr, PROCESS_TIME);
		update_screen();
	}

	printf("Cleaning up...\n");
	cleanup_fb();
}
