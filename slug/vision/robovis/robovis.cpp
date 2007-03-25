#include <Python.h>
#include "structmember.h"
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

IplImage *hue = 0, *hsv_temp = 0;
IplImage *frame = 0;

CvCapture *capture = 0;
CvSize framesize;
CBlobGetXCenter getxc;
CBlobGetYCenter getyc;
CBlobGetMajorAxisLength  getl;


typedef struct {
    PyObject_HEAD
    PyObject *hue, *cx, *cy;
} robovis_BlobObject;

static void robovis_BlobObject_dealloc(robovis_BlobObject *self){
    Py_XDECREF(self->hue);
    Py_XDECREF(self->cx);
    Py_XDECREF(self->cy);
    self->ob_type->tp_free((PyObject*)self);
}

static PyObject *robovis_BlobObject_new(PyTypeObject *type, PyObject *args, PyObject *kwds){
    robovis_BlobObject *self;
    self = (robovis_BlobObject *)type->tp_alloc(type, 0);
    if (self != NULL) {
        self->hue = PyInt_FromLong((long) 0);
        self->cx = PyInt_FromLong((long) 0);
        self->cy = PyInt_FromLong((long) 0);
    }

    return (PyObject *)self;
}

static PyMemberDef robovis_BlobObject_members[] = {
    {"hue", T_OBJECT_EX, offsetof(robovis_BlobObject, hue), 0, "Colour HUE (0 < hue < 180"},
    {"cx", T_OBJECT_EX, offsetof(robovis_BlobObject, cx), 0, "Blob center x"},
    {"cy", T_OBJECT_EX, offsetof(robovis_BlobObject, cy), 0, "Blob center y"},
    {NULL}
};

static PyMethodDef robovis_BlobObject_methods[] = {
    {NULL}
};

static PyTypeObject robovis_BlobType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "robovis.Blob",             /*tp_name*/
    sizeof(robovis_BlobObject), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)robovis_BlobObject_dealloc,    /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,        /*tp_flags*/
    "A Blob of colour",           /* tp_doc */
    0,                          /* tp_traverse */
    0,                          /* tp_clear */
    0,                          /* tp_richcompare */
    0,                          /* tp_weaklistoffset */
    0,                          /* tp_iter */
    0,                          /* tp_iternext */
    robovis_BlobObject_methods, /* tp_methods */
    robovis_BlobObject_members, /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    0,                         /* tp_init */
    0,                         /* tp_alloc */
    robovis_BlobObject_new,    /* tp_new */
};

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

    PyType_Ready(&robovis_BlobType);
    Py_INCREF(&robovis_BlobType); 
    PyModule_AddObject(self, "Blob", (PyObject *)&robovis_BlobType);

    capture = cvCaptureFromCAM(-1);
    frame = cvQueryFrame(capture);
    
    framesize = cvGetSize(frame);
    hsv_temp = cvCreateImage(framesize, 8, 3);
    hue = cvCreateImage(framesize, 8, 1);
}

PyObject * robovis_capture(PyObject *self, PyObject *args){
    int c, i, curhue;
    PyObject *tmp;
    CBlobResult *blobs;
    CBlob *blob;
    
    PyObject *result = Py_BuildValue("[]");

    frame = cvQueryFrame(capture);

    cvCvtColor(frame, hsv_temp, CV_BGR2HSV);
    
    cvAddS(hsv_temp, cvScalar(30, 0, 0), frame);

    //For each colour in colours, highlight
    for(c=0;c<PyList_Size(colours);c++){
        curhue = (char) PyInt_AsLong(PyList_GetItem(colours, c));
        cvInRangeS(frame, cvScalar(curhue+30-HUE_STD, MIN_SAT, MIN_VAL),
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
                PyObject *pyblob = robovis_BlobObject_new(&robovis_BlobType, NULL, NULL);
                tmp = ((robovis_BlobObject *)pyblob)->hue;
                ((robovis_BlobObject *)pyblob)->hue = PyInt_FromLong((int) curhue);
                Py_XDECREF(tmp);

                tmp = ((robovis_BlobObject *)pyblob)->cx;
                ((robovis_BlobObject *)pyblob)->cx = PyInt_FromLong((int) getxc(*blob));
                Py_XDECREF(tmp);
                
                tmp = ((robovis_BlobObject *)pyblob)->cy;
                ((robovis_BlobObject *)pyblob)->cy = PyInt_FromLong((int) getyc(*blob));
                Py_XDECREF(tmp);

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
