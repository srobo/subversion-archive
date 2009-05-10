#include <Python.h>
#include "structmember.h"
#include "cv.h"
#include "highgui.h"
#include "Blob.h"
#include "BlobLibraryConfiguration.h"
#include "BlobResult.h"
#include "BlobExtraction.h"
#include <ctype.h>

IplImage *hue = 0, *hsv_temp = 0;
IplImage *frame = 0;

CvCapture *capture = 0;
CvSize framesize;
CBlobGetXCenter getxc;
CBlobGetYCenter getyc;
CBlobGetArea  geta;
CBlobGetMinX getminx;
CBlobGetMinY getminy;
CBlobGetMaxX getmaxx;
CBlobGetMaxY getmaxy;

typedef struct {
    PyObject_HEAD
    PyObject *hue, *cx, *cy, *area;
    PyObject *MinX, *MinY, *MaxX, *MaxY;
} robovis_BlobObject;

static void robovis_BlobObject_dealloc(robovis_BlobObject *self){
    Py_XDECREF(self->hue);
    Py_XDECREF(self->cx);
    Py_XDECREF(self->cy);
    Py_XDECREF(self->area);
    Py_XDECREF(self->MinX);
    Py_XDECREF(self->MinY);
    Py_XDECREF(self->MaxX);
    Py_XDECREF(self->MaxY);

    self->ob_type->tp_free((PyObject*)self);
}

static PyObject *robovis_BlobObject_new(PyTypeObject *type, PyObject *args, PyObject *kwds){
    robovis_BlobObject *self;
    self = (robovis_BlobObject *)type->tp_alloc(type, 0);
    if (self != NULL) {
        self->hue = PyInt_FromLong((long) 0);
        self->cx = PyInt_FromLong((long) 0);
        self->cy = PyInt_FromLong((long) 0);
        self->area = PyInt_FromLong((long) 0);
        self->MinX = PyInt_FromLong((long) 0);
        self->MinY = PyInt_FromLong((long) 0);
        self->MaxX = PyInt_FromLong((long) 0);
        self->MaxY = PyInt_FromLong((long) 0);
    }

    return (PyObject *)self;
}

static PyMemberDef robovis_BlobObject_members[] = {
    {"hue", T_OBJECT_EX, offsetof(robovis_BlobObject, hue), 0, "Colour HUE (0 < hue < 180"},
    {"cx", T_OBJECT_EX, offsetof(robovis_BlobObject, cx), 0, "Blob center x"},
    {"cy", T_OBJECT_EX, offsetof(robovis_BlobObject, cy), 0, "Blob center y"},
    {"area", T_OBJECT_EX, offsetof(robovis_BlobObject, area), 0, "Blob area"},
    {"MinX", T_OBJECT_EX, offsetof(robovis_BlobObject, MinX), 0, "Blob top position"},
    {"MinY", T_OBJECT_EX, offsetof(robovis_BlobObject, MinY), 0, "Blob left position"},
    {"MaxX", T_OBJECT_EX, offsetof(robovis_BlobObject, MaxX), 0, "Blob bottom position"},
    {"MaxY", T_OBJECT_EX, offsetof(robovis_BlobObject, MaxY), 0, "Blob right position"},
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
PyObject * robovis_set_min_val(PyObject *self, PyObject *args);
PyObject * robovis_set_min_sat(PyObject *self, PyObject *args);
PyObject * robovis_set_hue_thresh(PyObject *self, PyObject *args);
PyObject * robovis_set_min_size(PyObject *self, PyObject *args);

PyObject *colours, *self;
char min_sat = 65, min_val = 50, hue_thresh = 5;
int min_size = 200;

static PyMethodDef robovisMethods[] = {
    {"capture", robovis_capture, METH_VARARGS, "Grab a frame and find a list of blobs"},
    {"setminsat", robovis_set_min_sat, METH_VARARGS, "Set the minimum saturation"},
    {"setminval", robovis_set_min_val, METH_VARARGS, "Set the minimum pixel value"},
    {"sethuethresh", robovis_set_hue_thresh, METH_VARARGS, "Set the hue threshold"},
    {"setminsize", robovis_set_min_size, METH_VARARGS, "Set the minimum blob size"},
    {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC initrobovis(void){
    printf("Initializing.\n");
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

PyObject * robovis_set_min_val(PyObject *self, PyObject *args){
    int ok, tmp;
    ok = PyArg_ParseTuple(args, "i", &tmp);
    if (!ok)
        return NULL;
    if (tmp > 0 && tmp < 255){
        min_val = (char) tmp;
        Py_INCREF(Py_None);
        return Py_None;
    }

    PyErr_SetString(PyExc_TypeError, "Minimum value out of range. Acceptable range 0<min_sat<255.");
    return NULL;
}
PyObject * robovis_set_min_size(PyObject *self, PyObject *args){
    int ok, tmp;
    ok = PyArg_ParseTuple(args, "i", &tmp);
    if (!ok)
        return NULL;
    if (tmp > 0 && tmp < (framesize.width*framesize.height)){
        min_sat = tmp;
        Py_INCREF(Py_None);
        return Py_None;
    }

    PyErr_SetString(PyExc_TypeError, "Minimum size out of range. Must be greater than 0 and less than total pixels in the frame.");
    return NULL;
}
PyObject * robovis_set_hue_thresh(PyObject *self, PyObject *args){
    int ok, tmp;
    ok = PyArg_ParseTuple(args, "i", &tmp);
    if (!ok)
        return NULL;
    if (tmp > 0 && tmp < 30){
        hue_thresh = (char)tmp;
        Py_INCREF(Py_None);
        return Py_None;
    }

    PyErr_SetString(PyExc_TypeError, "Hue threshold out of range. Acceptable range 0<min_sat<30.");
    return NULL;
}
PyObject * robovis_set_min_sat(PyObject *self, PyObject *args){
    int ok, tmp;
    ok = PyArg_ParseTuple(args, "i", &tmp);
    if (!ok)
        return NULL;
    if (tmp > 0 && tmp < 255){
        min_sat = tmp;
        Py_INCREF(Py_None);
        return Py_None;
    }

    PyErr_SetString(PyExc_TypeError, "Minimum saturation out of range. Acceptable range 0<min_sat<255.");
    return NULL;
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
        if(curhue < 0 || curhue > 180)
            continue;
        
        cvInRangeS(frame, cvScalar(curhue+30-hue_thresh, min_sat, min_val),
                        cvScalar(curhue+30+hue_thresh, 255, 255), hue); 
        
        //the number here is the threshhold. all values are 255 already...
        blobs = new CBlobResult( hue, NULL, 200, true);

        blobs->Filter(*blobs, B_INCLUDE, CBlobGetArea(), B_GREATER, min_size);

        for(i=0;i<blobs->GetNumBlobs();i++){
            blob = blobs->GetBlob(i);
            if (not (blob->MinX() == 0 &&
                        blob->MinY() == 0 &&
                        blob->MaxX() == framesize.width &&
                        blob->MaxY() == framesize.height)){
                
                //Add this to a list
                PyObject *pyblob = robovis_BlobObject_new(&robovis_BlobType, NULL, NULL);

                //Put in the values
                tmp = ((robovis_BlobObject *)pyblob)->hue;
                ((robovis_BlobObject *)pyblob)->hue = PyInt_FromLong((int) curhue);
                Py_XDECREF(tmp);

                tmp = ((robovis_BlobObject *)pyblob)->cx;
                ((robovis_BlobObject *)pyblob)->cx = PyInt_FromLong((int) getxc(*blob));
                Py_XDECREF(tmp);
                
                tmp = ((robovis_BlobObject *)pyblob)->cy;
                ((robovis_BlobObject *)pyblob)->cy = PyInt_FromLong((int) getyc(*blob));
                Py_XDECREF(tmp);

                tmp = ((robovis_BlobObject *)pyblob)->area;
                ((robovis_BlobObject *)pyblob)->area = PyInt_FromLong((int) geta(*blob));
                Py_XDECREF(tmp);

                tmp = ((robovis_BlobObject *)pyblob)->MinX;
                ((robovis_BlobObject *)pyblob)->MinX = PyInt_FromLong((int) getminx(*blob));
                Py_XDECREF(tmp);

                tmp = ((robovis_BlobObject *)pyblob)->MinY;
                ((robovis_BlobObject *)pyblob)->MinY = PyInt_FromLong((int) getminy(*blob));
                Py_XDECREF(tmp);

                tmp = ((robovis_BlobObject *)pyblob)->MaxX;
                ((robovis_BlobObject *)pyblob)->MaxX = PyInt_FromLong((int) getmaxx(*blob));
                Py_XDECREF(tmp);

                tmp = ((robovis_BlobObject *)pyblob)->MaxY;
                ((robovis_BlobObject *)pyblob)->MaxY = PyInt_FromLong((int) getmaxy(*blob));
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
