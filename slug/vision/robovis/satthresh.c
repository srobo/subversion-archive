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

//#define CAMERA
//
#define DISPLAY

IplImage *allo_frame(CvSize framesize, unsigned char depth, unsigned char channels){
    IplImage *frame = cvCreateImage(framesize, depth, channels);
    if(frame == NULL){
        return NULL;
    }
    return frame;
}

int add_blob(CvSeq *cont, CvSize framesize, IplImage *out, IplImage *hsv, int minarea){
    static IplImage *blobmask = NULL; 
    CvScalar avghue, white, stddv;
    float area;
    CvRect outline;

    if(blobmask == NULL)
        blobmask = allo_frame(framesize, IPL_DEPTH_8U, 1);

    white = cvScalar(255, 255, 255, 255);
    
    area = fabs(cvContourArea(cont, CV_WHOLE_SEQ));
    if(area < minarea)
        return 0;
    
    printf("Area: %f\n", area);
    outline = cvBoundingRect(cont, 0);
    printf("%d %d %d %d %f\n", outline.x,
                                outline.width,
                                outline.y,
                                outline.height,
                                area);

    cvSetZero(blobmask);
    cvRectangle(blobmask, cvPoint(outline.x, outline.y),
                cvPoint(outline.x+outline.width, outline.y+outline.height),
                white, CV_FILLED, 8, 0);

    cvAvgSdv(hsv, &avghue, &stddv, blobmask);
    printf("Average values: %f %f %f %f\n", avghue.val[0],
                                            avghue.val[1],
                                            avghue.val[2],
                                            avghue.val[3]);
    printf("Deviations: %f %f %f %f\n", stddv.val[0],
                                        stddv.val[1],
                                        stddv.val[2],
                                        stddv.val[3]);
    printf("Sum of deviations: %f\n\n", stddv.val[0]+
                                        stddv.val[1]+
                                        stddv.val[2]+
                                        stddv.val[3]);

#ifdef DISPLAY
    cvAddS(out, avghue, out, blobmask);
#endif

    return 1;
}

int main(int argc, char **argv){
    IplImage *frame = NULL, *hsv, *hue, *sat, *val, *end, *endrgb,
             *satthresh;
    CvSize framesize;
    CvCapture *capture;
    IplConvKernel *k;

    CvMemStorage *contour_storage;
    CvSeq *cont, *child;
    int num_contours;
    double area;

    //No idea what this returns on fail.
#ifdef DISPLAY
    cvNamedWindow("testcam", CV_WINDOW_AUTOSIZE);
    cvNamedWindow("sats", CV_WINDOW_AUTOSIZE);
    cvNamedWindow("satthresh", CV_WINDOW_AUTOSIZE);
    cvNamedWindow("hue", CV_WINDOW_AUTOSIZE);
    cvNamedWindow("result", CV_WINDOW_AUTOSIZE);
#endif

#ifndef CAMERA
    //Get a frame to find the image size
    frame = cvLoadImage(argv[1], CV_LOAD_IMAGE_COLOR);
#else
    const unsigned int CAMWIDTH = 320;
    const unsigned int CAMHEIGHT = 240;
    capture = cvCaptureFromCAM(0);
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, CAMWIDTH);
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, CAMHEIGHT);
    frame = cvQueryFrame(capture);
#endif
    framesize = cvGetSize(frame);
    
    hsv = allo_frame(framesize, IPL_DEPTH_8U, 3);
    end = allo_frame(framesize, IPL_DEPTH_8U, 3);
    endrgb = allo_frame(framesize, IPL_DEPTH_8U, 3);
    hue = allo_frame(framesize, IPL_DEPTH_8U, 1);
    sat = allo_frame(framesize, IPL_DEPTH_8U, 1);
    val = allo_frame(framesize, IPL_DEPTH_8U, 1);
    satthresh = allo_frame(framesize, IPL_DEPTH_8U, 1);

#ifdef DISPLAY
    cvShowImage("testcam", frame);
#endif
    cvCvtColor(frame, hsv, CV_BGR2HSV);
    cvSplit(hsv, hue, sat, val, NULL);
#ifdef DISPLAY
    cvShowImage("sats", sat);
#endif

    k = cvCreateStructuringElementEx( 5, 5, 0, 0, CV_SHAPE_RECT, NULL);

    int param1 = 1, d = 1, minarea = 1, cutoff = 1;

    contour_storage = cvCreateMemStorage(0); //TODO: Look this up

    while (1){
#ifdef CAMERA
    frame = cvQueryFrame(capture);
#endif

#ifdef DISPLAY
        cvSetZero(end);
#endif
        printf("Enter edge diff, dilation, minarea and cutoff\n");
        if(scanf("%d %d %d %d", &param1, &d, &minarea, &cutoff) != 4)
            continue;

        //6 6 8 seem to be sensible
        cvAdaptiveThreshold(sat, satthresh, 255, CV_ADAPTIVE_THRESH_MEAN_C,
                            CV_THRESH_BINARY_INV, 5, param1);

#ifdef DISPLAY
        cvShowImage("hue", satthresh);
#endif

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

                    if(sum < 255 * cutoff)
                        data[x] = 0;
                }
        }

        cvDilate(satthresh, satthresh, NULL, d);

#ifdef DISPLAY
        cvShowImage("satthresh", satthresh);
#endif

        num_contours = cvFindContours(satthresh, contour_storage, &cont,
                       sizeof(CvContour), CV_RETR_TREE, CV_CHAIN_APPROX_NONE,
                       cvPoint(0,0));

        printf("%d contours found.\n", num_contours);
        
        for(; cont; cont = cont->h_next){
            printf("Poo\n");
            unsigned char bigchild = 0;

            area = fabs(cvContourArea(cont, CV_WHOLE_SEQ));
            if(area < minarea)
                continue;
            
            if(cont->v_next)
                for(child = cont->v_next; child; child = child->h_next)
                    if(add_blob(child, framesize, end, hsv, minarea))
                        bigchild = 1;

            if(!bigchild)
                add_blob(cont, framesize, end, hsv, minarea);
        }


#ifdef DISPLAY
        cvCvtColor(end, endrgb, CV_HSV2RGB);
        cvShowImage("result", end);
        cvWaitKey(200);
#endif
        cvSaveImage("out.jpg", frame);
    }
    return 0;
}
