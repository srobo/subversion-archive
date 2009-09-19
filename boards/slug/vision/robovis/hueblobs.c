#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <cv.h>
#include <highgui.h>

#include "visfunc.h"

#define DEBUG 0
#define ERROR 1

#define IN_FILENAME "in.jpg"
#define OUT_FILENAME "out.jpg"

const unsigned int MINMASS = 200;
const unsigned int MAXMASS = 2000;

const unsigned int CAMWIDTH = 320;
const unsigned int CAMHEIGHT = 240;

IplImage *frame = NULL, *hsv, *hue, *sat, *val;

int USEFILE = 0;
int DEBUGOUTPUT = 0;
int DEBUGDISPLAY = 0;

//grab the command line options and set them where appropriate
void
get_command_line_opts(int argc, char **argv)
{
	for(int i = 1; i < argc; i++) {
		// Skip argv[0] - this is the program name
		if (strcmp(argv[i], "-debug") == 0) {
			DEBUGOUTPUT = 1;
		} else if (strcmp(argv[i], "-display") == 0) {
			DEBUGDISPLAY = 1;
		}
	}
	return;
}

/* Wait for a newline on stdin */
char
*wait_trigger(void)
{
	char *req_tag;

	req_tag = (char*)malloc(129);

	req_tag = fgets(req_tag, 128, stdin);
	if (req_tag == NULL)	/*EOF*/
		exit(0);

	return req_tag;
}

void
srlog(char level, const char *m)
{
	if(DEBUGOUTPUT) {
		struct tm * tm;
		struct timeval tv;
		struct timezone tz;

		gettimeofday(&tv, &tz);
		tm = localtime(&tv.tv_sec);
		fprintf(stderr, "%02d:%02d:%02d.%d", tm->tm_hour, tm->tm_min,
						tm->tm_sec, (int) tv.tv_usec);

		switch(level){
			case DEBUG:
				fprintf(stderr, " - DEBUG - %s\n", m);
				break;
			case ERROR:
				fprintf(stderr, " - ERROR - %s\n", m);
		}
	} else if(level == ERROR) {
		fprintf(stderr, "%s\n", m);
	}
}

void
srshow(char *window, IplImage *frame)
{
	if(DEBUGOUTPUT) {
		cvShowImage(window, frame);
		cvWaitKey(0);
	}
}

CvCapture
*get_camera()
{
	srlog(DEBUG, "Opening camera");
	CvCapture *capture = cvCaptureFromCAM(0);
	if (capture == NULL){
		srlog(ERROR, "Failed to open camera");
		//TODO: Exit here?
		return NULL;
	}

	//Setting to CAMWIDTH, CAMHEIGHT
	cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, CAMWIDTH);
	cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, CAMHEIGHT);
	return capture;
}

IplImage
*get_frame(CvCapture *capture)
{
	IplImage *frame = cvQueryFrame(capture);
	/*MY EYES THE BURNING AND THE PAIN AND THE PAAAIIINNN
	 * Highgui currently buffers 4 frames before handing them to us; this
	 * means that any picture we take is increadibly delayed. If you use the
	 * highgui binaries from srobo.org/~jmorse/highgui.tgz this is patched
	 * to be only one frame. To overcome this last frame, retrieve two.
	 */
	frame = cvQueryFrame(capture);
	if(frame == NULL){
		srlog(ERROR, "Failed to grab initial frame.");
		//TODO: Exit here?
		return NULL;
	}
	return frame;
}

IplImage
*allo_frame(CvSize framesize, unsigned char depth, unsigned char channels)
{
	IplImage *frame = cvCreateImage(framesize, depth, channels);
	if(frame == NULL){
		srlog(ERROR, "Failed to allocate scratchpad.");
		//TODO: Exit here?
		return NULL;
	}
	return frame;
}

void
Hoo(int event, int x, int y, int flags, void *param)
{
	unsigned char *data;
	CvSize size;
	int step, c;
	cvGetRawData(val, &data, &step, &size);
	c = data[y*step+x];
	printf("Val %d,%d - %d\n", x, y, c);
}

void
Goo(int event, int x, int y, int flags, void* param)
{
	unsigned char *data;
	CvSize size;
	int step, c;
	cvGetRawData(sat, &data, &step, &size);
	c = data[y*step+x];
	printf("Sat %d,%d - %d\n", x, y, c);
}

void
Foo(int event, int x, int y, int flags, void* param)
{
	unsigned char *data;
	CvSize size;
	int step, c;
	cvGetRawData(hue, &data, &step, &size);
	c = data[y*step+x];
	printf("Hue %d,%d - %d\n", x, y, c);
}

int
main(int argc, char **argv)
{
	get_command_line_opts(argc, argv);

	IplImage *dsthsv, *dstrgb;
	CvSize framesize;
	IplConvKernel *k;

	struct blob_position *blobs;
	char *req_tag = NULL;
	int i, w, h;
	CvCapture *capture = NULL;

	if(DEBUGDISPLAY) {
		//No idea what this returns on fail.
		cvNamedWindow("testcam", CV_WINDOW_AUTOSIZE);
		cvNamedWindow("val", CV_WINDOW_AUTOSIZE);
		cvSetMouseCallback("val", Hoo, val);
		cvNamedWindow("sat", CV_WINDOW_AUTOSIZE);
		cvSetMouseCallback("sat", Goo, sat);
		cvNamedWindow("hue", CV_WINDOW_AUTOSIZE);
		cvSetMouseCallback("hue", Foo, hue);
	}

	//Get a frame to find the image size
	if(USEFILE) {
		frame = cvLoadImage(IN_FILENAME, CV_LOAD_IMAGE_COLOR);
	} else {
		capture = get_camera();
		frame = get_frame(capture);
	}

	framesize = cvGetSize(frame);
	if(DEBUGOUTPUT) { //print the framesize if debug output requested
		printf("Framesize %dx%d.\n", framesize.width, framesize.height);
	}

	srlog(DEBUG, "Allocating scratchpads");
	hsv = allo_frame(framesize, IPL_DEPTH_8U, 3);
	hue = allo_frame(framesize, IPL_DEPTH_8U, 1);
	sat = allo_frame(framesize, IPL_DEPTH_8U, 1);
	val = allo_frame(framesize, IPL_DEPTH_8U, 1);
	dsthsv = allo_frame(framesize, IPL_DEPTH_8U, 3);
	dstrgb = allo_frame(framesize, IPL_DEPTH_8U, 3);

	k = cvCreateStructuringElementEx( 5, 5, 0, 0, CV_SHAPE_RECT, NULL);

	srlog(DEBUG, "Beginning looping");
	while (1){
		srlog(DEBUG, "Press enter to grab a frame:");

		if(!USEFILE && !DEBUGDISPLAY) {
			req_tag = wait_trigger();
		}

		srlog(DEBUG, "Grabbing frame");
		if(USEFILE) {
			frame = cvLoadImage(IN_FILENAME, CV_LOAD_IMAGE_COLOR);
		} else {
			frame = get_frame(capture);
		}

		srlog(DEBUG, "Converting to HSV");
		cvCvtColor(frame, hsv, CV_BGR2HSV);

		srlog(DEBUG, "Splitting into H, S and V");
		cvSplit(hsv, hue, sat, val, NULL);

		if(DEBUGDISPLAY) {
			cvShowImage("sat", sat);
		}

		if(DEBUGDISPLAY) {
			cvShowImage("hue", hue);
		}

		blobs = vis_find_blobs_through_scanlines(hue, sat, val);
		for (i = 0; ; i++) {
			if (blobs[i].x1 == 0 && blobs[i].x2 == 0)
				break;

			cvRectangle(frame, cvPoint(blobs[i].x1, blobs[i].y1),
					cvPoint(blobs[i].x2, blobs[i].y2),
					cvScalar(255, 0, 0), 1);
			w = blobs[i].x2 - blobs[i].x1;
			h = blobs[i].y2 - blobs[i].y1;
			printf("%d,%d,%d,%d,%d,%d\n", blobs[i].x1, blobs[i].y1,
					w, h, w*h, blobs[i].colour);
		}

		if(DEBUGDISPLAY) {
			cvShowImage("testcam", frame);
		}

		if (req_tag) {
			fputs(req_tag, stdout);
			free(req_tag);
		}

		fputs("BLOBS\n", stdout);
		fflush(stdout);

		srlog(DEBUG, "Saving frame to out.jpg");
		cvSaveImage(OUT_FILENAME, frame);
		if (USEFILE) {
			cvReleaseImage(&frame);
		}

		if (DEBUGDISPLAY)
			cvWaitKey(100);

	}	//end while loop
	return 0;
}
