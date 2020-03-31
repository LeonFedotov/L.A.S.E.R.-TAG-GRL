#ifndef _V4L_UTILS
#define _V4L_UTILS

#include "ofConstants.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <time.h>
//#include <asm/types.h>          /* for videodev2.h */
#include <linux/videodev.h>

#define HAVE_V4L	1

/* V4L structures */
static struct video_capability capability;
static struct video_window     window;
static struct video_picture    imageProperties; 
static struct video_mbuf       mbuf;
static struct video_mmap       vmmap;

static int 					deviceHandle;    
static int 					frameIndex;    
static bool					bFirstFrame;
static unsigned char 			*bigbuf;	
static unsigned char 		*pixels;

/* V4L functions */
bool openV4L_device(const char *device_name);
void initialiseV4L_device(const char *device_name);
bool initV4L(int width, int height,const char *devname);
bool queryV4L_imageProperties(void);
bool setV4L_palette(int palette, int depth);
bool setV4L_imageProperties(void);
bool setV4L_videoSize(int width, int height);
bool getFrameV4L(unsigned char ** pixels);
void closeV4L(void);
bool mmapV4L(void);
int getV4L_Height();
int getV4L_Width();


static void errno_exit (const char *s)
{
	printf ("%s error %d, %s\n",s, errno, strerror (errno));
	exit (EXIT_FAILURE);
}

static int xioctl (int deviceHandle, int request, void *arg)
{
	int r;

	do r = ioctl (deviceHandle, request, arg);
	while (-1 == r && EINTR == errno);

	return r;
}

bool openV4L_device(const char *device_name)
{
	struct stat st; 

	if (stat (device_name, &st) == -1) {
			printf("V4L : Cannot identify '%s': %d, %s\n", device_name, errno, strerror (errno));
			exit (EXIT_FAILURE);
	}

	if (!S_ISCHR (st.st_mode)) {
			printf("V4L : %s is no device\n", device_name);
			exit (EXIT_FAILURE);
	}

	deviceHandle = open(device_name, O_RDWR | O_NONBLOCK, 0);

	if (deviceHandle == -1) {
		printf("V4L : Cannot open '%s': %d, %s\n", device_name, errno, strerror (errno));
		close(deviceHandle);
		exit (EXIT_FAILURE);
	}
	
	return true;
}

bool queryV4L_imageProperties(void) {

	memset(&imageProperties, 0, sizeof(struct video_picture));	// clear v4l image properties
	
	if (xioctl(deviceHandle, VIDIOCGPICT, &imageProperties) == -1) {
		printf("V4L : Unable to determine image properties\n");
		errno_exit ("VIDIOCGPICT");
	}			

	printf("V4L : Brightness\t= %u\n",imageProperties.brightness);
	printf("V4L : Hue\t\t= %u\n",imageProperties.hue);	
	printf("V4L : Colour\t\t= %u\n",imageProperties.colour);
	printf("V4L : Depth\t\t= %u bits\n",imageProperties.depth);
	printf("V4L : Palette\t\t= ");
	
	switch(imageProperties.palette) {
		case VIDEO_PALETTE_GREY: // 1
			printf("VIDEO_PALETTE_GREY\n");
			break;
		case VIDEO_PALETTE_HI240: // 2
			printf("VIDEO_PALETTE_HI240\n");
			break;
		case VIDEO_PALETTE_RGB565:  // 3
			printf("VIDEO_PALETTE_RGB565\n");
			break;	
		case VIDEO_PALETTE_RGB24: // 4
			printf("VIDEO_PALETTE_RGB24\n");
			break;			
		case VIDEO_PALETTE_RGB32: // 5
			printf("VIDEO_PALETTE_RGB32\n");
			break;
		case VIDEO_PALETTE_RGB555:	// 6
			printf("VIDEO_PALETTE_RGB555\n");
			break;	
		case VIDEO_PALETTE_YUV422: // 7
			printf("VIDEO_PALETTE_YUV422\n");
			break;		
		case VIDEO_PALETTE_YUYV: // 8
			printf("VIDEO_PALETTE_YUYV\n");
			break;	
		case VIDEO_PALETTE_UYVY: // 9
			printf("VIDEO_PALETTE_UYVY\n");
			break;			
		case VIDEO_PALETTE_YUV420: // 10
			printf("VIDEO_PALETTE_YUV420\n");
			break;		
		case VIDEO_PALETTE_YUV411: // 11
			printf("VIDEO_PALETTE_YUV411\n");
			break;	
		case VIDEO_PALETTE_RAW:	// 12
			printf("VIDEO_PALETTE_RAW\n");
			break;	
		case VIDEO_PALETTE_YUV422P:	// 13
			printf("VIDEO_PALETTE_YUV422P\n");
			break;
		case VIDEO_PALETTE_YUV411P:	// 14
			printf("VIDEO_PALETTE_YUV411P\n");
			break;	
		case  VIDEO_PALETTE_YUV420P: // 15		
			printf("VIDEO_PALETTE_YUV420P\n");
			break;
		case  VIDEO_PALETTE_YUV410P: // 16		
			printf("VIDEO_PALETTE_YUV410P\n");
			break;			
		default:
			printf(" Couldn't read palette\n");
			break;
	}
	return true;
}

bool setV4L_palette(int palette, int depth) {
	imageProperties.palette = palette;
	imageProperties.depth = depth;
  
	if (xioctl(deviceHandle, VIDIOCSPICT, &imageProperties) < 0) {
		printf("V4L : Failed to set image properties : %d, %s\n", errno, strerror (errno));
		return false;
	}
	if ((imageProperties.palette == palette) && (imageProperties.depth == depth)) {
		return true;
	}
	return false;
}

bool setV4L_imageProperties(void) {

	if (setV4L_palette(VIDEO_PALETTE_RGB24, 24)) {
		printf("V4L : Changed current palette to VIDEO_PALETTE_RGB24\n");
	}
	//else if setV4L_palette(VIDEO_PALETTE_YUV420P, 16)) {
		//printf("V4L : Changed current palette to VIDEO_PALETTE_YUV420P\n");
	//}
	else {
		printf("V4L : ERROR : Unable to change to a suitable palette\n");
		return false;
	}
	return true;
}

void initialiseV4L_device(const char *device_name) {

	if (xioctl(deviceHandle, VIDIOCGCAP, &capability) == -1) {
		if (errno == EINVAL) {
			printf("V4L : %s is no V4L device\n",device_name);
			exit (EXIT_FAILURE);
		} else {
			errno_exit ("VIDIOC_QUERYCAP");
		}
	}

	if (!(capability.type & VID_TYPE_CAPTURE)) {
		printf("V4L : %s is no video capture device\n", device_name);
		exit (EXIT_FAILURE);
	} else {
		printf("V4L : Name = '%s'\n",capability.name);	
		printf("V4L : Dimensions (%i x %i) - (%i x %i)\n", capability.minwidth, capability.minheight, capability.maxwidth,capability.maxheight);	
		printf("V4L : Capability :\n");
		if (capability.type & VID_TYPE_CAPTURE      ) printf(" - CAPTURE\n");
		if (capability.type & VID_TYPE_TUNER        ) printf(" - TUNER\n");
		if (capability.type & VID_TYPE_TELETEXT     ) printf(" - TELETEXT\n");
		if (capability.type & VID_TYPE_OVERLAY      ) printf(" - OVERLAY\n");
		if (capability.type & VID_TYPE_CHROMAKEY    ) printf(" - CHROMAKEY\n");
		if (capability.type & VID_TYPE_CLIPPING     ) printf(" - CLIPPING\n");
		if (capability.type & VID_TYPE_FRAMERAM     ) printf(" - FRAMERAM\n");
		if (capability.type & VID_TYPE_SCALES       ) printf(" - SCALES\n");
		if (capability.type & VID_TYPE_MONOCHROME   ) printf(" - MONOCHROME\n");
		if (capability.type & VID_TYPE_SUBCAPTURE   ) printf(" - SUBCAPTURE\n");
	}

	queryV4L_imageProperties();
	setV4L_imageProperties();
}

void writePPM(unsigned char *data, char *filename, int width, int height) {
  int num;
  int size = width * height * 3;
  FILE *fp = fopen(filename, "w");

  if (!fp) {printf("V4L : cannot open file for writing!\n");}

  fprintf(fp, "P6\n%d %d\n%d\n", width, height, 255);
  num = fwrite((void *) data, 1, (size_t) size, fp);

  if (num != size) {printf("V4L : cannot write image data to file\n");}

  fclose(fp);
}

void swap_rgb24(unsigned char *mem, int n)
{
  unsigned char c;
  unsigned char *p = mem;
  int i = n;
  while(--i) {
    c = p[0];
    p[0] = p[2];
    p[2] = c;
    p += 3;
  }
}

bool getFrameV4L(unsigned char ** _pixels) {

	if (bFirstFrame) { 
		bFirstFrame = false;
		/* Queue all available buffers for capture */
		for (frameIndex = 0;frameIndex < (mbuf.frames);++frameIndex) {
			vmmap.frame  = frameIndex;
			vmmap.width  = window.width;
			vmmap.height = window.height;
			vmmap.format = imageProperties.palette;
			
			if (xioctl(deviceHandle, VIDIOCMCAPTURE, &vmmap) == -1) {
				printf("V4L : ERROR: Could not make initial capture\n");
				return false;
			}
		}   
		/* reset frame index*/
		frameIndex = 0;   		
	}   
	 
	/* Sync to video */
	if (xioctl(deviceHandle, VIDIOCSYNC, &frameIndex) == -1) {
		printf("V4L : ERROR: VIDIOCSYNC failed. %s\n", strerror(errno));
	}
 
	int imagesize = window.width*window.height*(imageProperties.depth/8);
	
	/* Save current frame */
	switch(imageProperties.palette) {
		case VIDEO_PALETTE_RGB24:
	  	{
			memcpy((unsigned char*)pixels, (unsigned char*)(bigbuf + mbuf.offsets[frameIndex]),imagesize);
			swap_rgb24(pixels, window.width * window.height);
		}
		break;
		//case VIDEO_PALETTE_YUV420:
			// convert yuv420_to_rgb24
		//break;
	  default:
		printf("V4L : ERROR: Cannot convert from palette %d to RGB\n",imageProperties.palette);
		return false;
	}	
	
	vmmap.frame  = frameIndex;
	vmmap.width  = window.width;
	vmmap.height = window.height;
	vmmap.format = imageProperties.palette;

	/* Capture video */
	if (xioctl (deviceHandle, VIDIOCMCAPTURE, &vmmap) == -1) {
		printf("V4L : ERROR: Could not capture...\n");
		return false;
	}

	++frameIndex;
	if (frameIndex == mbuf.frames) {
		frameIndex = 0;
	}

	/* pass the pixel array back */
	*_pixels = pixels;	
	
	return true;
}

bool setV4L_videoSize(int w, int h) {
  
	if (!(capability.type & VID_TYPE_CAPTURE)) return false;
	
	if (xioctl(deviceHandle, VIDIOCGWIN, &window) < 0) {
		printf("V4L : Could not get video size : %d, %s\n", errno, strerror (errno));
		return false;
	}
		
	if (w > capability.maxwidth) w = capability.maxwidth;
	if (h > capability.maxheight) h = capability.maxheight;

	window.width = w;
	window.height= h;
	window.x = window.y = window.chromakey = window.flags = 0;

	if (xioctl(deviceHandle, VIDIOCSWIN, &window) < 0) {
		printf("V4L : ERROR : Could not set video size : %d, %s\n", errno, strerror (errno));
		return false;
	}
	/* Get the video capabilities to check that the set worked */
	if (xioctl(deviceHandle, VIDIOCGWIN, &window) < 0) {
		printf("V4L : Could not get video size : %d, %s\n", errno, strerror (errno));
		return false;
	}	
	printf("V4L : Video size changed to width=%i height=%i\n",window.width,window.height);
	
	/* Allocate space for grabbing RGB data */
	if (pixels != NULL) {
		free(pixels);
	} 	
	pixels = (unsigned char *) malloc(w * h *(imageProperties.depth/8)*sizeof(unsigned char));

	return true;
}

bool mmapV4L(void) {

	if (xioctl(deviceHandle, VIDIOCGMBUF, &mbuf) < 0) {
		printf("V4L : Could not use mmap : %d, %s\n", errno, strerror (errno));
	} 				
    
	/* Get memory mapped io */
	/* create a large buffer that can hold up to the maximum frames at the maximum resolution*/
	bigbuf = (unsigned char*) mmap(0, mbuf.size, PROT_READ | PROT_WRITE, MAP_SHARED, deviceHandle, 0);

	if (bigbuf == MAP_FAILED) { 
		printf("V4L : Could not use mmap buffer.\n");
		closeV4L();
	}
     
	return true;
}

void closeV4L(void) {
	close(deviceHandle);		// Close device
    munmap(bigbuf, mbuf.size); 	// Un-mmap memory
    if(pixels != NULL) free(pixels); // Free user frame data

}

bool initV4L(int width, int height, const char *devname) {
	bFirstFrame	= true;
	pixels = NULL;
	bigbuf = NULL;
			
	openV4L_device(devname);
	initialiseV4L_device(devname);
	setV4L_videoSize(width,height);		

	mmapV4L();
	
	return true;
}

int getV4L_Height(void) {
	if(window.height)
		return window.height;	
}

int getV4L_Width(void) {
	if(window.width)
		return window.width;	
}

#endif

