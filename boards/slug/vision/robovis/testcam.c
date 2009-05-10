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

#define DEBUGMODE
#define DEBUGDISPLAY

#define ADAPTIVESATTHRESH

const unsigned int MINMASS = 300;
const unsigned int MAXMASS = 2000;

const unsigned int CAMWIDTH = 320;
const unsigned int CAMHEIGHT = 240;

const unsigned int SATEDGE = 6;
const unsigned int DILATE = 2;
const unsigned int CUTOFF = 2;

typedef struct peak {
    unsigned int start;
    unsigned int end;
    unsigned int mass;
    struct peak *next;
} srPeak;

typedef struct blob {
    unsigned int centrex, centrey;
    unsigned int mass;
    unsigned int colour;
} srBlob;

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

void srprinthist(CvHistogram *hist, unsigned int bins){
    float bin_val, min, max;
    int minpos, maxpos;
    unsigned int i;

#ifndef DEBUGMODE
    return;
#endif

    cvGetMinMaxHistValue(hist, &min, &max, &minpos, &maxpos);
    printf("Maximum is %f.\n", max);
    max = 80/max; //Turn max into a scaling constant
    
    for(i=0;i<bins;i++){
        bin_val = cvQueryHistValue_1D(hist, i);
        printf("%d - %f\n", i, bin_val);
    }
    printf("Histogram printed\n");
}

float get_hue_cut_off(CvHistogram *hist, unsigned int bins){
    float counter = 0;
    unsigned int i;
    for(i=0;i<bins;i++)
        counter += cvQueryHistValue_1D(hist, i);
    return counter / bins;
}

srPeak* findpeaks(CvHistogram *hist, unsigned int bins,
                       float thresh){
    unsigned int i, curpeakstart = 0, curpeakmass = 0;
    float bin_val;
    enum state {OUT, IN} peakstate;
    srPeak *head = NULL, *tail = NULL, *tmp;

    peakstate = OUT;

    for(i=0; i<bins; i++){
        bin_val = cvQueryHistValue_1D(hist, i);

        if(peakstate == OUT){
            if(bin_val > thresh){
                peakstate = IN;
                curpeakstart = i;
                curpeakmass += bin_val;
            }
        }else
            if((bin_val < thresh) || (i == bins-1)){

                peakstate = OUT;
                if(i-curpeakstart > MINHUEWIDTH){
                    tmp = (srPeak*) malloc(sizeof(srPeak));
                    if(tmp == NULL){
                        srlog(ERROR, "Could not allocate memory for a peak record.");
                        return NULL;
                    }
                    tmp->start = curpeakstart;
                    tmp->end = i;
                    tmp->mass = curpeakmass;
                    tmp->next = NULL;
                    if(head == NULL){
                        head = tmp;
                        tail = tmp;
                    } else {
                        tail->next = tmp;
                        tail = tmp;
                    }
                }
                curpeakmass = 0;
            } else {
                if((i-curpeakstart) > 5){
                    tmp = (srPeak*) malloc(sizeof(srPeak));
                    if(tmp == NULL){
                        srlog(ERROR, "Could not allocate memory for a peak record.");
                        return NULL;
                    }
                    tmp->start = curpeakstart;
                    tmp->end = i;
                    tmp->mass = curpeakmass;
                    tmp->next = NULL;
                    if(head == NULL){
                        head = tmp;
                        tail = tmp;
                    } else {
                        tail->next = tmp;
                        tail = tmp;
                    }
                    peakstate = IN;
                    curpeakstart = i;
                    curpeakmass += bin_val;
                }
                curpeakmass += bin_val;
        }
    }
    //Is there a peak continuing off the end?
    if(peakstate == IN){
        //Should it be wrapped with the one at the start
        if(head->start == 0){
            //Merge them
            head->start = curpeakstart;
            head->mass += curpeakmass;
        } else {
            //Finish a peak off
            if(i-curpeakstart > MINHUEWIDTH){
                tmp = (srPeak*) malloc(sizeof(srPeak));
                if(tmp == NULL){
                    srlog(ERROR, "Could not allocate memory for a peak record.");
                    return NULL;
                }
                tmp->start = curpeakstart;
                tmp->end = i;
                tmp->mass = curpeakmass;
                tmp->next = NULL;
                if(head == NULL){
                    head = tmp;
                    tail = tmp;
                } else {
                    tail->next = tmp;
                    tail = tmp;
                }
            }

        }
    }
    return head;
}

void freepeaks(srPeak* peaks){
    srPeak* tmp;
    while(peaks != NULL){
        tmp = peaks->next;
        free(peaks);
        peaks = tmp;
    }
}

unsigned char get_min_sat(CvHistogram *sathist){
    /*Saturations below this value are white.
    Go from 0 (fully saturated) right, until the count has gone above then
    below DEFAULTSATPEAK - if that was wider than MINWHITEWIDTH then presume
    that was the white peak.
    If don't find anything, return DEFAULTSATCUTOFF*/
    float bin_val;
    unsigned char i, whitepeakstart = 0;

    for(i=MINSATPEAK;i<SATBINS;i++){
        bin_val = cvQueryHistValue_1D(sathist, i);

        if(whitepeakstart==0){
            if(bin_val > DEFAULTSATPEAK)
                whitepeakstart = 1;
        } else
            if((bin_val < DEFAULTSATPEAK) && 
                (i>whitepeakstart + MINWHITEWIDTH))
                return i;
    }
    return DEFAULTSATCUTOFF;
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

CvHistogram *allo_hist(unsigned char dimensions, int *bins, float **ranges){
    CvHistogram *hist = cvCreateHist(dimensions, bins, CV_HIST_ARRAY, ranges, 1);

    if(hist == NULL){
        srlog(ERROR, "Could not allocate histogram");
        //TODO: Exit here?
        return NULL;
    }
    return hist;
}


int main(int argc, char **argv){
#ifndef USEFILE
    CvCapture *capture = NULL;
#endif
    IplImage *frame = NULL, *hsv, *hue, *sat, *val,
             *satthresh, *huemask, *hue2mask, *comthresh, *tmpmask;
#ifdef DEBUGDISPLAY
    IplImage *dsthsv, *dstrgb;
    CvScalar avghue;
#endif
    CvRect outline;
    CvSize framesize;
    CvHistogram *sathist, *huehist;
    int sathistbins[] = {SATBINS};
    int huehistbins[] = {HUEBINS};
    float s_ranges[] = {0, SATBINS};
    float h_ranges[] = {0, HUEBINS};
    float *s_ranges2[] = { s_ranges };
    float *h_ranges2[] = { h_ranges };
    
    unsigned char minsat;
    srPeak *headpeak, *tmppeak;
    unsigned int minhue;

    CvMemStorage *contour_storage;
    CvSeq *cont;
    int num_contours;
    double area;

    srBlob curblob;

#ifdef DEBUGDISPLAY
    //No idea what this returns on fail.
    cvNamedWindow("testcam", CV_WINDOW_AUTOSIZE);
    cvNamedWindow("satthresh", CV_WINDOW_AUTOSIZE);
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
    huemask = allo_frame(framesize, IPL_DEPTH_8U, 1);
    satthresh = allo_frame(framesize, IPL_DEPTH_8U, 1);
    comthresh = allo_frame(framesize, IPL_DEPTH_8U, 1);
    tmpmask = allo_frame(framesize, IPL_DEPTH_8U, 1);
    hue2mask = allo_frame(framesize, IPL_DEPTH_8U, 1);
#ifdef DEBUGDISPLAY
    dsthsv = allo_frame(framesize, IPL_DEPTH_8U, 3);
    dstrgb = allo_frame(framesize, IPL_DEPTH_8U, 3);
#endif

    srlog(DEBUG, "Allocating histograms");
    sathist = allo_hist(1, sathistbins, s_ranges2);
    huehist = allo_hist(1, huehistbins, h_ranges2);

    while (1){
#ifndef USEFILE
    #ifndef DEBUGDISPLAY
	    wait_trigger();
    #endif
#endif
        
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

        srlog(DEBUG, "Generating saturation histogram");
        cvCalcHist(&sat, sathist, 0, NULL);

        srlog(DEBUG, "Calculating minimum saturation.");
        minsat = get_min_sat(sathist);
#ifdef DEBUGMODE
        printf("Minimum saturation of blobs = %d\n", minsat);
#endif
        
        srlog(DEBUG, "Generating mask of > minsat pixels");
#ifdef ADAPTIVESATTHRESH
        cvAdaptiveThreshold(sat, satthresh, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY_INV, 5, SATEDGE);
#else
        cvThreshold(sat, satthresh, minsat, 255, CV_THRESH_BINARY);
#endif
        //huemask used temporarily
        cvDilate(satthresh, huemask, NULL, DILATE);
        cvErode(huemask, satthresh, NULL, ERODE);

#ifdef DEBUGDISPLAY
        cvShowImage("satthresh", satthresh);
#endif

        srlog(DEBUG, "Generating hue histogram");
        cvCalcHist(&hue, huehist, 0, satthresh);

        srlog(DEBUG, "Finding minimum hue peak value");
        minhue = get_hue_cut_off(huehist, HUEBINS);
#ifdef DEBUGMODE
        printf("Minimum hue weighting in histogram = %d.\n", minhue);
#endif
        
        srlog(DEBUG, "Finding hue peaks");
        headpeak = findpeaks(huehist, HUEBINS, minhue);

#ifdef DEBUGDISPLAY
        cvSetZero(dsthsv);
#endif
        contour_storage = cvCreateMemStorage(0); //TODO: Look this up

        tmppeak = headpeak;

        while(tmppeak){
#ifdef DEBUGMODE
            printf("Peak found from hue %d to %d, Mass %d.\n",
                    tmppeak->start, tmppeak->end, tmppeak->mass);
#endif
            srlog(DEBUG, "Masking out peak");
            if(tmppeak->start < tmppeak->end)
                cvInRangeS(hue, cvScalarAll(tmppeak->start), cvScalarAll(tmppeak->end), huemask);
            else {
                cvInRangeS(hue, cvScalarAll(tmppeak->start), cvScalarAll(255), huemask);
                cvInRangeS(hue, cvScalarAll(0), cvScalarAll(tmppeak->end), hue2mask);
                cvOr(huemask, hue2mask, tmpmask, NULL);
            }
            srlog(DEBUG, "Combining huemask and satthresh.");
            cvAnd(huemask, satthresh, comthresh, NULL);

            srlog(DEBUG, "Finding contours");
            num_contours = cvFindContours(comthresh, contour_storage, &cont,
                       sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE,
                       cvPoint(0,0));

            srlog(DEBUG, "Looping through contours");
            for(; cont; cont = cont->h_next){
                area = abs(cvContourArea(cont, CV_WHOLE_SEQ));

                if(area > MINMASS && area < MAXMASS){

                    outline = cvBoundingRect(cont, 0);
                    
                    curblob.centrex = outline.x + (outline.width)/2;
                    curblob.centrey = outline.y + (outline.height)/2;
                    if(curblob.centrex < CAMWIDTH && curblob.centrey < CAMHEIGHT){
                        curblob.mass = area;
                        curblob.colour = (tmppeak->end + tmppeak->start) / 2;

                        fprintf(stdout, "%d,%d,%d,%d,%d,%d\n",    outline.x,
                                                                    outline.y,
                                                                    outline.width,
                                                                    outline.height,
                                                                    curblob.mass,
                                                                    curblob.colour);
#ifdef DEBUGDISPLAY
                        srlog(DEBUG, "Drawing the contour");
                        
                        avghue = cvScalar(((tmppeak->start)+(tmppeak->end))/2, 255, 255, 0);

                        cvRectangle(dsthsv, cvPoint(outline.x, outline.y),
                                        cvPoint(outline.x+outline.width, outline.y+outline.height),
                                        avghue, CV_FILLED, 8, 0);
#endif
                    }
                                        
                }
            }
            tmppeak = tmppeak->next;
        }

        fputs("BLOBS\n", stdout);
        fflush(stdout);

        srlog(DEBUG, "Freeing peaks");
        freepeaks(headpeak);
        
#ifdef DEBUGDISPLAY
        cvCvtColor(dsthsv, dstrgb, CV_HSV2RGB);        
        srshow("filled", dstrgb);
#endif

        cvReleaseMemStorage(&contour_storage);
#ifdef DEBUGMODE
        srlog(DEBUG, "Saving frame to out.jpg");
#endif
        cvSaveImage("out.jpg", frame);
    }
    return 0;
}
