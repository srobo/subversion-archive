#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include "opencv/cv.h"
#include "opencv/highgui.h"

#define DEBUG 0
#define ERROR 1

//#define USEFILE
#define FILENAME "out.jpg"

//#define DEBUGMODE
//#define DEBUGDISPLAY

#define ADAPTIVESATTHRESH

const unsigned int MINMASS = 300;
const unsigned int MAXMASS = 2000;

const unsigned int CAMWIDTH = 320;
const unsigned int CAMHEIGHT = 240;

const unsigned int SATEDGE = 6;
const unsigned int DILATE = 2;
const unsigned int CUTOFF = 2;

/* Wait for a newline on stdin */
void wait_trigger(void)
{
	int r = 0;

	while( r != '\n' ) 
	{
		r = fgetc(stdin);		
		if( r == EOF )
            exit(0);
	}
}

void srlog(char level, char *m){
#ifdef DEBUGMODE
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
#endif
#ifndef DEBUGMODE
    if(level == ERROR)
        printf("%s\n", m);
#endif
}

void srshow(char *window, IplImage *frame){
#ifdef DEBUGMODE
    cvShowImage(window, frame);
    cvWaitKey(0);
#endif
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
int add_blob(CvSeq *cont, CvSize framesize, IplImage *out, IplImage *hue, int minarea){
    static IplImage *blobmask = NULL; 
    CvScalar avghue, white;
    float area;
    CvRect outline;
    float colour = 0;

    if(blobmask == NULL)
        blobmask = allo_frame(framesize, IPL_DEPTH_8U, 1);

    white = cvScalar(255, 255, 255, 255);
    
    area = fabs(cvContourArea(cont, CV_WHOLE_SEQ));
    if(area < minarea)
        return 0;
    
    outline = cvBoundingRect(cont, 0);
    cvSetZero(blobmask);
    cvRectangle(blobmask, cvPoint(outline.x, outline.y),
                cvPoint(outline.x+outline.width, outline.y+outline.height),
                white, CV_FILLED, 8, 0);

    avghue = cvAvg(hue, blobmask);

    colour = avghue.val[0];

    printf("%d %d %d %d %f %f\n", outline.x,
                                outline.y,
                                outline.width,
                                outline.height,
                                area, colour);

#ifdef DEBUGDISPLAY
    cvAddS(out, avghue, out, blobmask);
#endif
 
    return 1;

}

int main(int argc, char **argv){
#ifndef USEFILE
    CvCapture *capture = NULL;
#endif
    IplImage *frame = NULL, *hsv, *hue, *sat, *val,
             *satthresh;
    IplImage *dsthsv, *dstrgb;
    CvSize framesize;
    IplConvKernel *k;
    
    CvMemStorage *contour_storage;
    CvSeq *cont, *child;
    int num_contours;
    double area;

#ifdef DEBUGDISPLAY
    //No idea what this returns on fail.
    cvNamedWindow("testcam", CV_WINDOW_AUTOSIZE);
    cvNamedWindow("satthresh", CV_WINDOW_AUTOSIZE);
    cvNamedWindow("sat", CV_WINDOW_AUTOSIZE);
    cvNamedWindow("satdil", CV_WINDOW_AUTOSIZE);
    cvNamedWindow("filled", CV_WINDOW_AUTOSIZE);
#endif

//Get a frame to find the image size
#ifdef USEFILE
    frame = cvLoadImage(FILENAME, CV_LOAD_IMAGE_COLOR);
#else
    capture = get_camera();
    frame = get_frame(capture);
#endif

    framesize = cvGetSize(frame);
#ifdef DEBUGMODE
    printf("Framesize %dx%d.\n", framesize.width, framesize.height);
#endif
    
    srlog(DEBUG, "Allocating scratchpads");
    hsv = allo_frame(framesize, IPL_DEPTH_8U, 3);
    hue = allo_frame(framesize, IPL_DEPTH_8U, 1);
    sat = allo_frame(framesize, IPL_DEPTH_8U, 1);
    val = allo_frame(framesize, IPL_DEPTH_8U, 1);
    satthresh = allo_frame(framesize, IPL_DEPTH_8U, 1);
    dsthsv = allo_frame(framesize, IPL_DEPTH_8U, 3);
    dstrgb = allo_frame(framesize, IPL_DEPTH_8U, 3);


    k = cvCreateStructuringElementEx( 5, 5, 0, 0, CV_SHAPE_RECT, NULL);

    while (1){
#ifndef USEFILE
    #ifndef DEBUGDISPLAY
	    wait_trigger();
    #endif
#endif
        contour_storage = cvCreateMemStorage(0); //TODO: Look this up
        
        srlog(DEBUG, "Grabbing frame");
#ifdef USEFILE
        frame = cvLoadImage(FILENAME, CV_LOAD_IMAGE_COLOR);
#else
        frame = get_frame(capture);
#endif

#ifdef DEBUGDISPLAY
        cvShowImage("testcam", frame);
#endif
        srlog(DEBUG, "Converting to HSV");
        cvCvtColor(frame, hsv, CV_BGR2HSV);

        srlog(DEBUG, "Splitting into H, S and V");
        cvSplit(hsv, hue, sat, val, NULL);

        {
            FILE *f = fopen("face.txt", "wt");
            unsigned char *data;
            CvSize size;
            int x, y, step;
            cvGetRawData(hue, &data, &step, &size);

            for(y = 0; y < size.height; y++, data += step)
                for(x = 0; x < size.width; x++){
                    fprintf(f, "%d\n", data[y*step+x]);
                }
            fclose(f);
        }


#ifdef DEBUGDISPLAY
        cvShowImage("sat", sat);
#endif
        cvAdaptiveThreshold(sat, satthresh, 255,
                CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY_INV, 5, SATEDGE);

        cvShowImage("satthresh", satthresh);

        cvSetZero(dsthsv);

        {
            unsigned char *data;
            CvSize size;
            int x, y, z, i, sum, step;
            cvGetRawData(satthresh, &data, &step, &size);

            for(y = 0; y < size.height; y++, data += step)
                for(x = 0; x < size.width; x++){
                    sum = 0;
                    for(i = 0; i < 9; i++)
                        if( ((y > 0) || (i > 2)) &&
                            ((y < size.height-1) || (i < 7)) &&
                            ((x > 0) || (i%3 > 0)) &&
                            ((x < size.width-1) || (i%3 < 2))){

                                z = i%3 - 1 + step*(i/3) - step;
                                sum += data[x + z];
                        }

                    if(sum < 255 * CUTOFF)
                        data[x] = 0;
                }
        }

        cvDilate(satthresh, satthresh, NULL, DILATE);

#ifdef DEBUGDISPLAY
        cvShowImage("satdil", satthresh);
#endif

        srlog(DEBUG, "Finding contours");
        num_contours = cvFindContours(satthresh, contour_storage, &cont,
                    sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE,
                    cvPoint(0,0));

        srlog(DEBUG, "Looping through contours");
        for(; cont; cont = cont->h_next){
            unsigned char bigchild = 0;

            area = fabs(cvContourArea(cont, CV_WHOLE_SEQ));
            if(area < MINMASS)
                continue;
            
            if(cont->v_next)
                for(child = cont->v_next; child; child = child->h_next)
                    if(add_blob(child, framesize, dsthsv, hue, MINMASS))
                        bigchild = 1;

            if(!bigchild)
                add_blob(cont, framesize, dsthsv, hue, MINMASS);
        }

        fputs("BLOBS\n", stdout);
        fflush(stdout);

#ifdef DEBUGDISPLAY
        cvCvtColor(dsthsv, dstrgb, CV_HSV2RGB);        
        srshow("filled", dsthsv);
#endif

        cvReleaseMemStorage(&contour_storage);
#ifdef DEBUGMODE
        srlog(DEBUG, "Saving frame to out.jpg");
#endif
        cvSaveImage("out.jpg", frame);
    }
    return 0;
}
