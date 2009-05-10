#include "cv.h"
#include "highgui.h"
#include "Blob.h"
#include "BlobLibraryConfiguration.h"
#include "BlobResult.h"
#include "BlobExtraction.h"
#include <stdio.h>
#include <ctype.h>

//These are betweeo 0 and 255
#define MIN_SAT 65 
#define MIN_VAL 50

//Must be less than 30!!!
#define HUE_STD 5

//The 360 value / 2
#define COL_RED 0
#define COL_BLUE 120
#define COL_GREEN 60

#define NO_COLOURS 2

typedef struct {
    unsigned char colour;
    unsigned char r, g, b;
} colour;

const colour colours[] = {
    {COL_RED,0,0,255},
    {COL_GREEN,255,0,0}};

IplImage *image = 0, *hsv = 0, *hue = 0, *hsv_temp = 0;
IplImage *frame = 0, *withblob = 0;
CvHistogram *hist = 0;

CvCapture *capture = 0;

int main(int argc, char **argv){
    int t, c, i;
    CvSize framesize;
    CBlobResult *blobs;
    CBlob *blob, fat;
    CBlobGetXCenter getxc;
    CBlobGetYCenter getyc;
    CBlobGetMajorAxisLength  getl;

    capture = cvCaptureFromCAM(-1);

    cvNamedWindow("Input", 1);
    cvNamedWindow("Hue", 1);
    cvNamedWindow("Output", 1);

    while(1){
        frame = cvQueryFrame(capture);

        if (!hsv)
        {
            framesize = cvGetSize(frame);
            hsv_temp = cvCreateImage(framesize, 8, 3);
            hsv = cvCreateImage(framesize, 8, 3);
            hue = cvCreateImage(framesize, 8, 1);
            withblob = cvCreateImage(framesize, 8, 3);
        }
        cvCvtColor(frame, hsv_temp, CV_BGR2HSV);
        
        cvCopy(frame, withblob);

        cvAddS(hsv_temp, cvScalar(30, 0, 0), hsv);

        //For each colour in colours, highlight
        for(c=0;c<NO_COLOURS;c++){
            cvInRangeS(hsv, cvScalar(colours[c].colour+30-HUE_STD, MIN_SAT, MIN_VAL),
                            cvScalar(colours[c].colour+30+HUE_STD, 255, 255), hue); 
            
            //the number here is the threshhold. all values are 255 already...
            blobs = new CBlobResult( hue, NULL, 200, true);

            blobs->Filter(*blobs, B_INCLUDE, CBlobGetArea(), B_GREATER, 200);

            for(i=0;i<blobs->GetNumBlobs();i++){
                blob = blobs->GetBlob(i);
                if (not (blob->MinX() == 0 &&
                         blob->MinY() == 0 &&
                         blob->MaxX() == framesize.width &&
                         blob->MaxY() == framesize.height)){
                    
                    cvCircle(withblob, cvPoint((int) getxc(*blob), 
                                               (int) getyc(*blob)),
                             (int) getl(*blob), CV_RGB(colours[c].r,
                                                       colours[c].g,
                                                       colours[c].b));
                }
            }
            blobs->ClearBlobs();
            delete blobs;
        }

        cvShowImage("Input", frame);
        cvShowImage("Hue", hue);
        cvShowImage("Output", withblob);

        t = cvWaitKey(10);
    }

    cvReleaseCapture( &capture);
    cvDestroyWindow("Hue");
    cvDestroyWindow("Input");
    cvDestroyWindow("Output");

    return 0;
}
