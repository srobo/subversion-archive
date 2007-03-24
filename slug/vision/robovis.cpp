#include <Python.h>
#include "cv.h"
#include "highgui.h"
#include "Blob.h"
#include "BlobLibraryConfiguration.h"
#include "BlobResult.h"
#include "BlobExtraction.h"
#include <ctype.h>

//These are between 0 and 255
#define MIN_SAT 65 
#define MIN_VAL 50

//Must be less than 30!!!
#define HUE_STD 5

//The 360 value / 2
#define COL_RED 0
#define COL_BLUE 120
#define COL_GREEN 60

#define MIN_SIZE 200

IplImage *image = 0, *hsv = 0, *hue = 0, *hsv_temp = 0;
IplImage *frame = 0, *withblob = 0;
CvHistogram *hist = 0;

CvCapture *capture = 0;
CvSize framesize;
CBlobGetXCenter getxc;
CBlobGetYCenter getyc;
CBlobGetMajorAxisLength  getl;

PyObject *robovis_capture(PyObject *self, PyObject *args);
PyObject *colours, *self;

static PyMethodDef robovisMethods[] = {
    {"capture", robovis_capture, METH_VARARGS, "Grab a frame and find a list of blobs"},
    {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC initrobovis(void){
    self = Py_InitModule3("robovis", robovisMethods, "StudentRobotics vision routines");

    colours = Py_BuildValue("[]");
    
    PyModule_AddObject(self, "colours", colours);

    capture = cvCaptureFromCAM(-1);
    frame = cvQueryFrame(capture);
    
    framesize = cvGetSize(frame);
    hsv_temp = cvCreateImage(framesize, 8, 3);
    hsv = cvCreateImage(framesize, 8, 3);
    hue = cvCreateImage(framesize, 8, 1);
    withblob = cvCreateImage(framesize, 8, 3);
}

PyObject * robovis_capture(PyObject *self, PyObject *args){
    int c, i, curhue;
    CBlobResult *blobs;
    CBlob *blob;
    
    PyObject *result = Py_BuildValue("[]");

    frame = cvQueryFrame(capture);

    cvCvtColor(frame, hsv_temp, CV_BGR2HSV);
    
    cvCopy(frame, withblob);

    cvAddS(hsv_temp, cvScalar(30, 0, 0), hsv);

    //For each colour in colours, highlight
    for(c=0;c<PyList_Size(colours);c++){
        curhue = (char) PyInt_AsLong(PyList_GetItem(colours, c));
        cvInRangeS(hsv, cvScalar(curhue+30-HUE_STD, MIN_SAT, MIN_VAL),
                        cvScalar(curhue+30+HUE_STD, 255, 255), hue); 
        
        //the number here is the threshhold. all values are 255 already...
        blobs = new CBlobResult( hue, NULL, 200, true);

        blobs->Filter(*blobs, B_INCLUDE, CBlobGetArea(), B_GREATER, MIN_SIZE);

        for(i=0;i<blobs->GetNumBlobs();i++){
            blob = blobs->GetBlob(i);
            if (not (blob->MinX() == 0 &&
                        blob->MinY() == 0 &&
                        blob->MaxX() == framesize.width &&
                        blob->MaxY() == framesize.height)){
                
                //Add this to a list
                PyObject *pyblob = Py_BuildValue("(iiii)",
                                                 (int) curhue,
                                                 (int) getxc(*blob),
                                                 (int) getyc(*blob),
                                                 (int) getl(*blob));
                PyList_Append(result, pyblob);
                Py_DECREF(pyblob);
            }
        }
        blobs->ClearBlobs();
        delete blobs;
    }
    return result;
}

void end(){
    cvReleaseCapture( &capture);
}
