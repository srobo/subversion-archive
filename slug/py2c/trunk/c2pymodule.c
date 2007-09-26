#include <Python.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include "i2c-dev.h"

static int fd;

static PyObject* c2py_smbuswrite( PyObject *self, 
			    PyObject *args );

static PyObject* c2py_smbusread( PyObject *self, 
			    PyObject *args );

static PyObject *I2CError;

static PyMethodDef c2pyMethods[] = {
	{"write",c2py_smbuswrite,METH_VARARGS,"Write to the SMBus"},
	{"read",c2py_smbusread,METH_VARARGS,"Read from the SMBus"},
    {NULL,NULL,0,NULL}
};

PyMODINIT_FUNC
initc2py(void)
{
	PyObject *m;
    m = Py_InitModule("c2py", c2pyMethods);

    I2CError = PyErr_NewException("c2py.I2CError", NULL, NULL);
    Py_INCREF(I2CError);
    PyModule_AddObject(m, "I2CError", I2CError);

	fd = open("/dev/i2c-0", O_RDWR);
	if( fd < 0 ){
        //Don't need to incref the ioerror object
        PyErr_SetString(PyExc_IOError, "Could not open /dev/i2c-0.\n");
        return;
    }

    if( ioctl( fd, I2C_PEC, 1) < 0)
    {
        PyErr_SetString(I2CError, "Could not enable checksumming on i2c.\n");
        return;
    }
}

static PyObject* c2py_smbuswrite( PyObject *self, 
				   PyObject *args )
{
    unsigned char address, command; //Address of device, command for device
    int size, width, result;        //Size to send, width of buffer, result
    const char *data = NULL;                     //Data buffer

	if (!PyArg_ParseTuple(args,"BBis#", &address, &command, &size, &data, &width)){
        PyErr_SetString(PyExc_TypeError, "Pass Address, ReadWrite, Command, Size, Data.");
	    return NULL;
    }

    if(size > width){
        PyErr_SetString(PyExc_RuntimeError, "Not enough data provided.");
        return NULL;
    }

    union i2c_smbus_data datablock;

    memcpy(datablock.block, data, width);

    if(i2c_smbus_access(fd, I2C_SMBUS_WRITE, command, size, &datablock)){
        PyErr_SetString(I2CError, "Error writing to bus");
        return NULL;
    }

    I2CError = PyErr_NewException("I2C Error", NULL, NULL);

    Py_INCREF(Py_None);

    return Py_None;
}

static PyObject* c2py_smbusread( PyObject *self, 
				   PyObject *args )
{
    unsigned char address, command; //Address of device, command for device
    int size,  result;        //Size to send, width of buffer, result
    PyObject *retstr = NULL;

	if (!PyArg_ParseTuple(args,"BBi", &address, &command, &size)){
        PyErr_SetString(PyExc_TypeError, "Pass Address, ReadWrite, Command, Size, Data.");
	    return NULL;
    }

    union i2c_smbus_data datablock;

    if(i2c_smbus_access(fd, I2C_SMBUS_READ, command, size, &datablock)){
        PyErr_SetString(I2CError, "Error reading from bus");
        return NULL;
    }

    return Py_BuildValue("s#", datablock.block, size);
}
