#include <Python.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include "i2c-dev.h"

static int fd;

static PyObject* c2py_test( PyObject *self, 
			    PyObject *args );

static PyMethodDef c2pyMethods[] = {
	{"test",c2py_test,METH_VARARGS,"A test function"},
	{NULL,NULL,0,NULL}
};

PyMODINIT_FUNC
initc2py(void)
{
	(void) Py_InitModule("c2py", c2pyMethods);

	fd = open("/dev/i2c-0", O_RDWR);
	if( fd < 0 )
		fprintf(stderr,"Failed to open i2c device: %m\n");
}

static PyObject* c2py_write_quick( PyObject *self, 
				   PyObject *args )
{
	if (!PyArg_ParseTuple(args,"s",&command))
		return NULL;

	
}


static PyObject* c2py_test( PyObject *self, 
			    PyObject *args )
{
	const char *command;
	int sts;

	if (!PyArg_ParseTuple(args,"s",&command))
		return NULL;

	sts = system(command);
	return Py_BuildValue("i",sts);
}
