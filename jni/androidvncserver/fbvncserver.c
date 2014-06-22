#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>             /* For makedev() */
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <assert.h>
#include <errno.h>
/* libvncserver */
#include "rfb/rfb.h"
#include "rfb/keysym.h"

#include "fb.h" 

#define FB_DEVICE "/dev/graphics/fb0"
static char KBD_DEVICE[256] = "/dev/input/event3";
static char TOUCH_DEVICE[256] = "/dev/input/event1";
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

	printf("%15s : %x\n", "current color", fbmmap[(1280*600) + 640]);


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

    lseek(fb_temp, active_buffer_offset, SEEK_SET);
    read_size = read(fb_temp, raw, raw_size);
  
  	if (read_size < 0)
  		printf("%15s : %d\n", "buffer size", read_size);

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

	/* TODO: This assumes scrinfo.bits_per_pixel is 16. */
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

	vncscr->kbdAddEvent = keyevent;
	vncscr->ptrAddEvent = ptrevent;
	vncscr->serverFormat.trueColour = TRUE; 
	vncscr->depth = vncscr->serverFormat.depth = 24;

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

static void update_screen(void)
{
	// printf("updateing\n");
	unsigned int *f, *c, *r;
	int x, y;

	varblock.min_i = varblock.min_j = 9999;
	varblock.max_i = varblock.max_j = -1;

	free(fbmmap);
	fbmmap = private_fb(&fb);

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

static void init_kbd()
{
	if((kbdfd = open(KBD_DEVICE, O_RDWR)) == -1)
	{
		printf("cannot open kbd device %s\n", KBD_DEVICE);
		exit(EXIT_FAILURE);
	}
}

static void cleanup_kbd()
{
	if(kbdfd != -1)
	{
		close(kbdfd);
	}
}

static void init_touch()
{
    struct input_absinfo info;
    if((touchfd = open(TOUCH_DEVICE, O_RDWR)) == -1)
    {
        printf("cannot open touch device %s\n", TOUCH_DEVICE);
        exit(EXIT_FAILURE);
    }
    // Get the Range of X and Y
    if(ioctl(touchfd, EVIOCGABS(ABS_X), &info)) {
        printf("cannot get ABS_X info, %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    xmin = info.minimum;
    xmax = info.maximum;
    if(ioctl(touchfd, EVIOCGABS(ABS_Y), &info)) {
        printf("cannot get ABS_Y, %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    ymin = info.minimum;
    ymax = info.maximum;

}

static void cleanup_touch()
{
	if(touchfd != -1)
	{
		close(touchfd);
	}
}

/*****************************************************************************/
void injectKeyEvent(uint16_t code, uint16_t value)
{
    struct input_event ev;
    memset(&ev, 0, sizeof(ev));
    gettimeofday(&ev.time,0);
    ev.type = EV_KEY;
    ev.code = code;
    ev.value = value;
    if(write(kbdfd, &ev, sizeof(ev)) < 0)
    {
        printf("write event failed, %s\n", strerror(errno));
    }

    printf("injectKey (%d, %d)\n", code , value);    
}

static int keysym2scancode(rfbBool down, rfbKeySym key, rfbClientPtr cl)
{
    int scancode = 0;

    int code = (int)key;
    if (code>='0' && code<='9') {
        scancode = (code & 0xF) - 1;
        if (scancode<0) scancode += 10;
        scancode += KEY_1;
    } else if (code>=0xFF50 && code<=0xFF58) {
        static const uint16_t map[] =
             {  KEY_HOME, KEY_LEFT, KEY_UP, KEY_RIGHT, KEY_DOWN,
                KEY_SOFT1, KEY_SOFT2, KEY_END, 0 };
        scancode = map[code & 0xF];
    } else if (code>=0xFFE1 && code<=0xFFEE) {
        static const uint16_t map[] =
             {  KEY_LEFTSHIFT, KEY_LEFTSHIFT,
                KEY_COMPOSE, KEY_COMPOSE,
                KEY_LEFTSHIFT, KEY_LEFTSHIFT,
                0,0,
                KEY_LEFTALT, KEY_RIGHTALT,
                0, 0, 0, 0 };
        scancode = map[code & 0xF];
    } else if ((code>='A' && code<='Z') || (code>='a' && code<='z')) {
        static const uint16_t map[] = {
                KEY_A, KEY_B, KEY_C, KEY_D, KEY_E,
                KEY_F, KEY_G, KEY_H, KEY_I, KEY_J,
                KEY_K, KEY_L, KEY_M, KEY_N, KEY_O,
                KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T,
                KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z };
        scancode = map[(code & 0x5F) - 'A'];
    } else {
        switch (code) {
            case 0x0003:    scancode = KEY_CENTER;      break;
            case 0x0020:    scancode = KEY_SPACE;       break;
            case 0x0023:    scancode = KEY_SHARP;       break;
            case 0x0033:    scancode = KEY_SHARP;       break;
            case 0x002C:    scancode = KEY_COMMA;       break;
            case 0x003C:    scancode = KEY_COMMA;       break;
            case 0x002E:    scancode = KEY_DOT;         break;
            case 0x003E:    scancode = KEY_DOT;         break;
            case 0x002F:    scancode = KEY_SLASH;       break;
            case 0x003F:    scancode = KEY_SLASH;       break;
            case 0x0032:    scancode = KEY_EMAIL;       break;
            case 0x0040:    scancode = KEY_EMAIL;       break;
            case 0xFF08:    scancode = KEY_BACKSPACE;   break;
            case 0xFF1B:    scancode = KEY_BACK;        break;
            case 0xFF09:    scancode = KEY_TAB;         break;
            case 0xFF0D:    scancode = KEY_ENTER;       break;
            case 0x002A:    scancode = KEY_STAR;        break;
            case 0xFFBE:    scancode = KEY_F1;        break; // F1
            case 0xFFBF:    scancode = KEY_F2;         break; // F2
            case 0xFFC0:    scancode = KEY_F3;        break; // F3
            case 0xFFC5:    scancode = KEY_F4;       break; // F8
            case 0xFFC8:    rfbShutdownServer(cl->screen,TRUE);       break; // F11            
        }
    }

    return scancode;
}

static void keyevent(rfbBool down, rfbKeySym key, rfbClientPtr cl)
{
	int scancode;

	printf("Got keysym: %04x (down=%d)\n", (unsigned int)key, (int)down);

	if ((scancode = keysym2scancode(down, key, cl)))
	{
		injectKeyEvent(scancode, down);
	}
}

void injectTouchEvent(int down, int x, int y)
{
    struct input_event ev;
    
    // Calculate the final x and y
    x = xmin + (x * (xmax - xmin)) / (vinfo.xres);
    y = ymin + (y * (ymax - ymin)) / (vinfo.yres);
    
    memset(&ev, 0, sizeof(ev));

    // Then send a BTN_TOUCH
    gettimeofday(&ev.time,0);
    ev.type = EV_KEY;
    ev.code = BTN_TOUCH;
    ev.value = down;
    if(write(touchfd, &ev, sizeof(ev)) < 0)
    {
        printf("write event failed, %s\n", strerror(errno));
    }

    // Then send the X
    gettimeofday(&ev.time,0);
    ev.type = EV_ABS;
    ev.code = ABS_X;
    ev.value = x;
    if(write(touchfd, &ev, sizeof(ev)) < 0)
    {
        printf("write event failed, %s\n", strerror(errno));
    }

    // Then send the Y
    gettimeofday(&ev.time,0);
    ev.type = EV_ABS;
    ev.code = ABS_Y;
    ev.value = y;
    if(write(touchfd, &ev, sizeof(ev)) < 0)
    {
        printf("write event failed, %s\n", strerror(errno));
    }

    // Finally send the SYN
    gettimeofday(&ev.time,0);
    ev.type = EV_SYN;
    ev.code = 0;
    ev.value = 0;
    if(write(touchfd, &ev, sizeof(ev)) < 0)
    {
        printf("write event failed, %s\n", strerror(errno));
    }

    printf("injectTouchEvent (x=%d, y=%d, down=%d)\n", x , y, down);    
}

static void ptrevent(int buttonMask, int x, int y, rfbClientPtr cl)
{
	/* Indicates either pointer movement or a pointer button press or release. The pointer is
now at (x-position, y-position), and the current state of buttons 1 to 8 are represented
by bits 0 to 7 of button-mask respectively, 0 meaning up, 1 meaning down (pressed).
On a conventional mouse, buttons 1, 2 and 3 correspond to the left, middle and right
buttons on the mouse. On a wheel mouse, each step of the wheel upwards is represented
by a press and release of button 4, and each step downwards is represented by
a press and release of button 5. 
  From: http://www.vislab.usyd.edu.au/blogs/index.php/2009/05/22/an-headerless-indexed-protocol-for-input-1?blog=61 */
	
	//printf("Got ptrevent: %04x (x=%d, y=%d)\n", buttonMask, x, y);
	if(buttonMask & 1) {
		// Simulate left mouse event as touch event
		injectTouchEvent(1, x, y);
		injectTouchEvent(0, x, y);
	} 
}

int main(int argc, char **argv)
{
	printf("\n\n======= oPsai VNCServer Daemon initiating =======\n\n");
	if(argc > 1){
		int i=1;
		while(i < argc){
			if(*argv[i] == '-'){
				switch(*(argv[i] + 1)){
					case 'k':
						i++;
						strcpy(KBD_DEVICE, argv[i]);
						break;
					case 't':
						i++;
						strcpy(TOUCH_DEVICE, argv[i]);
						break;
				}
			}
			i++;
		}
	}

	printf("Initializing framebuffer device " FB_DEVICE "...\n");
	init_fb();
	printf("Initializing keyboard device %s ...\n", KBD_DEVICE);
	init_kbd();
	printf("Initializing touch device %s ...\n", TOUCH_DEVICE);
	init_touch();

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
	cleanup_kdb();
	cleanup_touch();
}
