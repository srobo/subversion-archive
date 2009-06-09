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

#define DEBUG 0
#define ERROR 1

#define FILENAME "out.jpg"

/* NB: if you use USEFILE, disable the call to cvSaveFile near the end of	 */
/* this program. Otherwise, it repeatedly opens and saves out.jpg, causing a  */
/* mass of jpeg compression roundings.*/

#define ADAPTIVESATTHRESH

const unsigned int MINMASS = 200;
const unsigned int MAXMASS = 2000;

const unsigned int CAMWIDTH = 320;
const unsigned int CAMHEIGHT = 240;

const unsigned int SATEDGE = 6;
const unsigned int DILATE = 2;
const unsigned int CUTOFF = 2;

IplImage *frame = NULL, *hsv, *hue, *sat, *val,
			*satthresh, *huethresh, *huemasked;

unsigned int huebins[4][2] = {{1, 20},	//red
							{21, 38},	//yellow
							{39, 78},	//green
							{100, 149}};	//blue

int USEFILE = 0;
int DEBUGOUTPUT = 0;
int DEBUGDISPLAY = 0;

//grab the command line options and set them where appropriate
void get_command_line_opts(int argc, char **argv){
	for(int i = 1; i < argc; i++) { 	// Skip argv[0] - this is the program name
		if (strcmp(argv[i], "-debug") == 0) {
			DEBUGOUTPUT = 1;
		} else if (strcmp(argv[i], "-display") == 0) {
			DEBUGDISPLAY = 1;
		}
	}
	return;
}

/* Wait for a newline on stdin */
char *wait_trigger(void)
{
	char *req_tag;
	int r = 0;

	req_tag = (char*)malloc(129);

	req_tag = fgets(req_tag, 128, stdin);
	if (req_tag == NULL)	/*EOF*/
		exit(0);

	return req_tag;
}

void srlog(char level, const char *m){
	if(DEBUGOUTPUT) {
		struct tm * tm;
		struct timeval tv;
		struct timezone tz;

		gettimeofday(&tv, &tz);
		tm = localtime(&tv.tv_sec);
		printf("%02d:%02d:%02d.%d", tm->tm_hour, tm->tm_min, tm->tm_sec, (int) tv.tv_usec);

		switch(level){
			case DEBUG:
				printf(" - DEBUG - %s\n", m);
				break;
			case ERROR:
				printf(" - ERROR - %s\n", m);
		}
	} else if(level == ERROR) {
		printf("%s\n", m);
	}
}

void srshow(char *window, IplImage *frame){
	if(DEBUGOUTPUT) {
		cvShowImage(window, frame);
		cvWaitKey(0);
	}
}

CvCapture *get_camera(){
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

IplImage *get_frame(CvCapture *capture){
	IplImage *frame = cvQueryFrame(capture);
	/*MY EYES THE BURNING AND THE PAIN AND THE PAAAIIINNN
	 * Highgui currently buffers 4 frames before handing them to us; this
	 * means that any picture we take is increadibly delayed. If you use the
	 * highgui binaries from srobo.org/~jmorse/highgui.tgz this is patched to
	 * be only one frame. To overcome this last frame, retrieve two.
	 */
	frame = cvQueryFrame(capture);
	if(frame == NULL){
		srlog(ERROR, "Failed to grab initial frame.");
		//TODO: Exit here?
		return NULL;
	}
	return frame;
}

IplImage *allo_frame(CvSize framesize, unsigned char depth, unsigned char channels){
	IplImage *frame = cvCreateImage(framesize, depth, channels);
	if(frame == NULL){
		srlog(ERROR, "Failed to allocate scratchpad.");
		//TODO: Exit here?
		return NULL;
	}
	return frame;
}
int add_blob(CvSeq *cont, CvSize framesize, IplImage *out, int colour, int minarea, IplImage *huemask){
	static IplImage *blobmask = NULL;
	CvScalar avghue, white;
	float area;
	CvRect outline;
	int count;

	if(blobmask == NULL)
		blobmask = allo_frame(framesize, IPL_DEPTH_8U, 1);

	white = cvScalar(255, 255, 255, 255);

	outline = cvBoundingRect(cont, 0);
	cvSetZero(blobmask);
	cvRectangle(blobmask, cvPoint(outline.x, outline.y),
				cvPoint(outline.x+outline.width, outline.y+outline.height),
				white, CV_FILLED, 8, 0);

	cvAnd(huemask, blobmask, blobmask, NULL);
	count = cvCountNonZero( blobmask );
	if(count < minarea)
		return 0;
	if(DEBUGOUTPUT) { //print the count if debug output requested
		printf("%d\n", count);
	}

	avghue = cvScalarAll(colour*20+50);

	printf("%d,%d,%d,%d,%d,%d\n", outline.x,
								outline.y,
								outline.width,
								outline.height,
								count, colour);

	if(DEBUGDISPLAY) {
		cvAddS(out, avghue, out, blobmask);
	}

	return 1;

}

void Hoo(int event, int x, int y, int flags, void *param){
	unsigned char *data;
	CvSize size;
	int step, c;
	cvGetRawData(val, &data, &step, &size);
	c = data[y*step+x];
	printf("Val %d,%d - %d\n", x, y, c);
}

void Goo(int event, int x, int y, int flags, void* param){
	unsigned char *data;
	CvSize size;
	int step, c;
	cvGetRawData(sat, &data, &step, &size);
	c = data[y*step+x];
	printf("Sat %d,%d - %d\n", x, y, c);
}

void Foo(int event, int x, int y, int flags, void* param){
	unsigned char *data;
	CvSize size;
	int step, c;
	cvGetRawData(hue, &data, &step, &size);
	c = data[y*step+x];
	printf("Hue %d,%d - %d\n", x, y, c);
}

int main(int argc, char **argv){
	get_command_line_opts(argc, argv);

	IplImage *dsthsv, *dstrgb, *huemask_backup, *red_second_step;
	CvSize framesize;
	IplConvKernel *k;

	CvMemStorage *contour_storage;
	CvSeq *cont;
	char *req_tag = NULL;
	int num_contours, i;
	double area;
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
		cvNamedWindow("r", CV_WINDOW_AUTOSIZE);
		cvNamedWindow("g", CV_WINDOW_AUTOSIZE);
		cvNamedWindow("b", CV_WINDOW_AUTOSIZE);
		cvNamedWindow("y", CV_WINDOW_AUTOSIZE);
	}

	//Get a frame to find the image size
	if(USEFILE) {
		frame = cvLoadImage(FILENAME, CV_LOAD_IMAGE_COLOR);
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
	huemasked = allo_frame(framesize, IPL_DEPTH_8U, 1);
	huethresh = allo_frame(framesize, IPL_DEPTH_8U, 1);
	hue = allo_frame(framesize, IPL_DEPTH_8U, 1);
	sat = allo_frame(framesize, IPL_DEPTH_8U, 1);
	val = allo_frame(framesize, IPL_DEPTH_8U, 1);
	satthresh = allo_frame(framesize, IPL_DEPTH_8U, 1);
	dsthsv = allo_frame(framesize, IPL_DEPTH_8U, 3);
	dstrgb = allo_frame(framesize, IPL_DEPTH_8U, 3);
	huemask_backup = allo_frame(framesize, IPL_DEPTH_8U, 1);
	red_second_step = allo_frame(framesize, IPL_DEPTH_8U, 1);

	k = cvCreateStructuringElementEx( 5, 5, 0, 0, CV_SHAPE_RECT, NULL);

	srlog(DEBUG, "Beginning looping");
	while (1){
		srlog(DEBUG, "Beginning new loop - press enter to grab a frame:");

		if(!USEFILE && !DEBUGDISPLAY) {
			req_tag = wait_trigger();
		}

		srlog(DEBUG, "Allocating contour stroage");
		contour_storage = cvCreateMemStorage(0); //TODO: Look this up

		srlog(DEBUG, "Grabbing frame");
		if(USEFILE) {
			frame = cvLoadImage(FILENAME, CV_LOAD_IMAGE_COLOR);
		} else {
			frame = get_frame(capture);
		}

		if(DEBUGDISPLAY) {
			cvShowImage("testcam", frame);
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


			cvInRangeS(sat, cvScalarAll(80),
							cvScalarAll(256),
							satthresh);

			cvAnd(satthresh, hue, hue, NULL);

		/* To stop black lines on the floor and other robots being visible,
		 * clober anything with a variance under 60. This is a measure of
		 * where the colour lies on a greyscale, so lower is black, higher
		 * is whiter. At 60, we risk also removing parts of objects that are
		 * far away, most notably the corners of the arena. However, some
		 * reduction in their size is better than being distracted by the
		 * lines and / or other robots
		 */

		/* re-use satthresh */
		cvInRangeS(val, cvScalarAll(60), cvScalarAll(256), satthresh);

		cvAnd(satthresh, hue, hue, NULL);

		for(i = 0; i<4; i++){
			if(DEBUGOUTPUT) {
				printf("Looking for %d %d\n", huebins[i][0], huebins[i][1]);
			}


			cvInRangeS(hue, cvScalarAll(huebins[i][0]),
							cvScalarAll(huebins[i][1]),
							huemasked);

		if (i == 0) {
		/* An unpleasent side-effect of red being... red, i that it
		 * wraps around the hue address space. There is also purplish
		 * red in the 160+ range, which should be included too
		 */

		cvInRangeS(hue, cvScalarAll(150), cvScalarAll(185),
			red_second_step);

		cvOr(red_second_step, huemasked, huemasked, NULL);
	}

			if(DEBUGDISPLAY) {
				if (i == 0) cvShowImage("r", huemasked);
				if (i == 1) cvShowImage("y", huemasked);
				if (i == 2) cvShowImage("g", huemasked);
				if (i == 3) cvShowImage("b", huemasked);
			}

		cvCopy(huemasked, huemask_backup, NULL);
		//cvFindContours writes over the original

			num_contours = cvFindContours(huemasked, contour_storage, &cont,
						sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE,
						cvPoint(0,0));

			srlog(DEBUG, "Looping through contours");
			for(; cont; cont = cont->h_next){
				area = fabs(cvContourArea(cont, CV_WHOLE_SEQ));
				if(area < MINMASS)
					continue;

				add_blob(cont, framesize, dsthsv, i, MINMASS, huemask_backup);
			}
		}

		if (req_tag) {
			fputs(req_tag, stdout);
			free(req_tag);
		}

		fputs("BLOBS\n", stdout);
		fflush(stdout);

		cvReleaseMemStorage(&contour_storage);
		srlog(DEBUG, "Saving frame to out.jpg");

		cvSaveImage("out.jpg", frame);
	}	//end while loop
	return 0;
}
