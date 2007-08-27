#include <stdio.h>
#include "opencv/cv.h"
#include "opencv/highgui.h"

#define DEBUG 0
#define ERROR 1
#define DEBUGMODE

const unsigned char DEFAULTSATCUTOFF = 30;
const unsigned char MINMASS = 100;
const unsigned char MINHUEWIDTH = 2;
const unsigned char MINVAL = 50;
const unsigned int DEFAULTSATPEAK = 150;
const unsigned int MINSATPEAK = 15;
const unsigned char MINWHITEWIDTH = 2;
const unsigned char SATBINS = 100;
const unsigned int HUEBINS = 360;

typedef struct peak {
    unsigned int start;
    unsigned int end;
    unsigned int mass;
    struct peak *next;
} srPeak;

void srlog(char level, char *m){
    switch(level){
        case DEBUG:
#ifndef DEBUGMODE
            break;
#endif
        case ERROR:
            printf("%s\n", m);
    }
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
        printf("%f\n", bin_val);
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
            if(bin_val < thresh){
                peakstate = OUT;
                if(i-curpeakstart > MINHUEWIDTH){
                    tmp = (srPeak*) malloc(sizeof(srPeak));
                    //TODO: Check NULL
                    tmp->start = curpeakstart;
                    tmp->end = i;
                    tmp->mass = curpeakmass;
                    tmp->next = NULL;
                    if(head == NULL){
                        head = tmp;
                        tail = tmp;
                    } else
                        tail->next = tmp;
                }
                curpeakmass = 0;
            } else
                curpeakmass += bin_val;
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

int main(int argc, char **argv){
    CvCapture *capture = NULL;
    IplImage *frame = NULL, *hsv, *hue, *sat, *val, *dsthsv, *dstrgb, *satthresh, *recmask;
    CvSize framesize;
    CvHistogram *sathist, *huehist;
    int sathistbins[] = {SATBINS};
    int huehistbins[] = {HUEBINS};
    float s_ranges[] = {0, SATBINS};
    float h_ranges[] = {0, HUEBINS};
    float *s_ranges2[] = { s_ranges };
    float *h_ranges2[] = { h_ranges };
    
    unsigned char minsat;

    CvMemStorage *contour_storage;
    CvSeq *cont;
    CvRect outline;
    CvScalar avghue;
    int num_contours;
    double area;

#ifdef DEBUGMODE
    //No idea what this returns on fail.
    cvNamedWindow("testcam", CV_WINDOW_AUTOSIZE);
    cvNamedWindow("filled", CV_WINDOW_AUTOSIZE);
#endif

    srlog(DEBUG, "Opening camera");
    capture = cvCaptureFromCAM(1);
    if (capture == NULL){
        srlog(ERROR, "Failed to open camera");
        goto camfail;
    }

    //Get a frame to find the image size
    frame = cvQueryFrame(capture);
    if(frame == NULL){
        srlog(ERROR, "Failed to grab initial frame.");
        goto initgrabfail;
    }
 
    framesize = cvGetSize(frame);
    
    srlog(DEBUG, "Allocating scratchpads");
    hsv = cvCreateImage(framesize, IPL_DEPTH_8U, 3);
    hue = cvCreateImage(framesize, IPL_DEPTH_8U, 1);
    sat = cvCreateImage(framesize, IPL_DEPTH_8U, 1);
    val = cvCreateImage(framesize, IPL_DEPTH_8U, 1);
    recmask = cvCreateImage(framesize, IPL_DEPTH_8U, 1);
    satthresh = cvCreateImage(framesize, IPL_DEPTH_8U, 1);
    dsthsv = cvCreateImage(framesize, IPL_DEPTH_8U, 3);
    dstrgb = cvCreateImage(framesize, IPL_DEPTH_8U, 3);

    if((hsv == NULL) || (hue == NULL) || (sat == NULL) || (val == NULL) ||
        (dsthsv == NULL) || (dstrgb == NULL) || (satthresh == NULL)){
        srlog(ERROR, "Error allocating scratchpads.");
        goto allocatefail;
    }

    srlog(DEBUG, "Allocating histograms");
    sathist = cvCreateHist(1, sathistbins, CV_HIST_ARRAY, s_ranges2, 1);
    huehist = cvCreateHist(1, huehistbins, CV_HIST_ARRAY, h_ranges2, 1);

    if((sathist == NULL) || (huehist == NULL)){
        srlog(ERROR, "Could not allocate histograms");
        goto histallocatefail;
    }

    while (1){
        srlog(DEBUG, "Grabbing frame");
        frame = cvQueryFrame(capture);
        cvShowImage("testcam", frame);
        if(frame == NULL){
            srlog(ERROR, "Failed to grab frame.");
            goto grabfail;
        }
        
        srlog(DEBUG, "Converting to HSV");
        cvCvtColor(frame, hsv, CV_BGR2HSV);

        srlog(DEBUG, "Splitting into H, S and V");
        cvSplit(hsv, hue, sat, val, NULL);

        srlog(DEBUG, "Generating saturation histogram");
        cvCalcHist(&sat, sathist, 0, NULL);

        srlog(DEBUG, "Calculating minimum saturation.");
        minsat = get_min_sat(sathist);
#ifdef DEBUGMODE
        printf("Minsat = %d\n", minsat);
#endif
        
        srlog(DEBUG, "Generating mask of > minsat pixels");
        cvThreshold(sat, satthresh, minsat, 255, CV_THRESH_BINARY);

        contour_storage = cvCreateMemStorage(0); //TODO: Look this up

        num_contours = cvFindContours(satthresh, contour_storage, &cont,
                       sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE,
                       cvPoint(0,0));

        cvSetZero(dsthsv);
        for(; cont; cont = cont->h_next){
            //CvScalar color = CV_RGB( rand()&255, rand()&255, rand()&255 );
            area =  cvContourArea(cont, CV_WHOLE_SEQ);

            if(area < -MINMASS){
                outline = cvBoundingRect(cont, 0);
                cvSetZero(recmask);

                cvDrawContours(recmask, cont, cvScalarAll(255), cvScalarAll(255), 0, CV_FILLED, 8,
                        cvPoint(0,0));
                
                avghue = cvAvg(hue, recmask);
                printf("%f\n", avghue.val[0]*2);
                avghue.val[1] = 255;
                avghue.val[2] = 255;

                cvRectangle(dsthsv, cvPoint(outline.x, outline.y),
                                  cvPoint(outline.x+outline.width, outline.y+outline.height),
                                  avghue, CV_FILLED, 8, 0);
                cvCvtColor(dsthsv, dstrgb, CV_HSV2RGB); 
                srshow("filled", dstrgb);

            }
        }
        

        cvReleaseMemStorage(&contour_storage);
        
        srlog(DEBUG, "Saving frame to out.jpg");
        cvSaveImage("out.jpg", frame);
    }

grabfail:
histallocatefail:
allocatefail:
    //cvRelease scratchpads here
initgrabfail:
camfail:
    cvReleaseCapture(&capture);
    return 0;
}
