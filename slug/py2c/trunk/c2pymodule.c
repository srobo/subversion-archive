#include <Python.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <stdint.h>
#include "i2c-dev.h"
#include "smbus_pec.h"

static int fd;

static PyObject* c2py_smbuswritebyte( PyObject *self, 
			    PyObject *args );

static PyObject* c2py_smbusreadbyte( PyObject *self, 
			    PyObject *args );

static PyObject* c2py_smbuswritebyte_data( PyObject *self, 
			    PyObject *args );

static PyObject* c2py_smbusreadbyte_data( PyObject *self, 
			    PyObject *args );

static PyObject* c2py_smbuswriteword_data( PyObject *self, 
			    PyObject *args );

static PyObject* c2py_smbusreadword_data( PyObject *self, 
			    PyObject *args );

static PyObject *I2CError;

static PyMethodDef c2pyMethods[] = {
	{"writebyte",c2py_smbuswritebyte,METH_VARARGS,"Write a byte to the SMBus"},
	{"readbyte",c2py_smbusreadbyte,METH_VARARGS,"Read a byte from the SMBus"},
    {"readblockdata",c2py_smbusreadblock_data,METH_VARARGS,"Read a block from the SMBus with a command"},
    {"readbytedata",c2py_smbusreadbyte_data,METH_VARARGS,"Read a byte from the SMBus with a command"},
    {"writebytedata",c2py_smbuswritebyte_data,METH_VARARGS,"Write a byte to the SMBus with a command"},
    {"readworddata",c2py_smbusreadword_data,METH_VARARGS,"Read a word from the SMBus with a command"},
    {"writeworddata",c2py_smbuswriteword_data,METH_VARARGS,"Write a word to the SMBus with a command"},
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

    
}

static PyObject* c2py_smbuswritebyte( PyObject *self, 
				   PyObject *args )
{
    unsigned char address;                 //Address of device
    unsigned char data, usepec = 1;        //Data

	if (!PyArg_ParseTuple(args,"BB|B", &address, &data, &usepec)){
	    return NULL;
    }

    if(ioctl( fd, I2C_SLAVE, address)){
        PyErr_SetString(I2CError, "Error setting I2C address.");
        return NULL;
    }
    
    if( ioctl( fd, I2C_PEC, usepec) < 0)
    {
        PyErr_SetString(I2CError, "Could not configure checksumming on i2c.\n");
        return;
    }

    if(i2c_smbus_access(fd, I2C_SMBUS_WRITE, data, I2C_SMBUS_BYTE, NULL) < 0){
        PyErr_SetString(I2CError, "Error writing to bus");
        return NULL;
    }

    Py_INCREF(Py_None);

    return Py_None;
}

static PyObject* c2py_smbusreadbyte( PyObject *self, 
				   PyObject *args )
{
    unsigned char address, usepec = 1; //Address of device
    union i2c_smbus_data datablock;
    PyObject *retstr = NULL;

	if (!PyArg_ParseTuple(args,"B|B", &address, &usepec)){
	    return NULL;
    }
    
    if(ioctl( fd, I2C_SLAVE, address)){
        PyErr_SetString(I2CError, "Error setting I2C address.");
        return NULL;
    }

    if( ioctl( fd, I2C_PEC, usepec) < 0)
    {
        PyErr_SetString(I2CError, "Could not configure checksumming on i2c.\n");
        return;
    }

    if(i2c_smbus_access(fd, I2C_SMBUS_READ, 0, I2C_SMBUS_WRITE, &datablock)){
        PyErr_SetString(I2CError, "Error reading from bus");
        return NULL;
    }

    return Py_BuildValue("i", 0x0FF & datablock.byte);
}

static PyObject* c2py_smbuswritebyte_data( PyObject *self, 
				   PyObject *args )
{
    unsigned char address, command, usepec = 1, data;
    union i2c_smbus_data datablock;

	if (!PyArg_ParseTuple(args,"BBB|B", &address, &command, &data, &usepec)){
	    return NULL;
    }

    if(ioctl( fd, I2C_SLAVE, address)){
        PyErr_SetString(I2CError, "Error setting I2C address.");
        return NULL;
    }

    if( ioctl( fd, I2C_PEC, usepec) < 0)
    {
        PyErr_SetString(I2CError, "Could not configure checksumming on i2c.\n");
        return;
    }
    
    datablock.byte = data;

    if(i2c_smbus_access(fd, I2C_SMBUS_WRITE, command, I2C_SMBUS_BYTE_DATA, &datablock) < 0){
        PyErr_SetString(I2CError, "Error writing to bus");
        return NULL;
    }

    Py_INCREF(Py_None);

    return Py_None;
}
static PyObject* c2py_smbusreadbyte_data( PyObject *self, 
				   PyObject *args )
{
    unsigned char address, command, usepec = 1;
    union i2c_smbus_data datablock;

	if (!PyArg_ParseTuple(args,"BB|B", &address, &command, &usepec)){
	    return NULL;
    }
    
    if(ioctl( fd, I2C_SLAVE, address)){
        PyErr_SetString(I2CError, "Error setting I2C address.");
        return NULL;
    }

    if( ioctl( fd, I2C_PEC, usepec) < 0)
    {
        PyErr_SetString(I2CError, "Could not configure checksumming on i2c.\n");
        return;
    }
    
    if(i2c_smbus_access(fd, I2C_SMBUS_READ, command, I2C_SMBUS_BYTE_DATA, &datablock)){
        PyErr_SetString(I2CError, "Error reading from bus");
        return NULL;
    }

    return Py_BuildValue("i", datablock.byte & 0x0FF);
}

static PyObject* c2py_smbusreadblock_data( PyObject *self, 
				   PyObject *args )
{
    uint8_t address, command, nobytes;
    uint8_t *buf, i=0, c=0;
    union i2c_smbus_data datablock;

	if (!PyArg_ParseTuple(args,"BBB", &address, &command, &nobytes)){
	    return NULL;
    }
    
    if(ioctl( fd, I2C_SLAVE, address)){
        PyErr_SetString(I2CError, "Error setting I2C address.");
        return NULL;
    }

    if( ioctl( fd, I2C_PEC, 0) < 0)
    {
        PyErr_SetString(I2CError, "Could not disable checksumming on i2c.\n");
        return;
    }
    
    if(i2c_smbus_write_byte(fd, command)){
        PyErr_SetString(I2CError, "Error sending block command");
        return NULL;
    }

	c = crc8(address<<1);
	c = crc8(c ^ command);
	c = crc8(c ^ ((address<<1)|1));

    buf = malloc(nobytes+1);

	if( read( fd, buf, nobytes+1 ) < nobytes+1 ){
        PyErr_SetString(I2CError, "Error allocating I2C receive buffer.");
        return NULL;
    }

	for( i=0; i<nobytes; i++ )
		c = crc8( c ^ buf[i] );

	if( c != buf[nobytes] ){
        free(buf);
        PyErr_SetString(I2CError, "Checksum error.");
        return NULL;
    }

    retval = Py_BuildValue("is#", datablock.byte & 0x0FF, buf, nobytes-1);
    free(buf);
    return retval;
}

static PyObject* c2py_smbuswriteword_data( PyObject *self, 
				   PyObject *args )
{
    unsigned char address, command, usepec = 1;
    uint16_t data;
    union i2c_smbus_data datablock;

	if (!PyArg_ParseTuple(args,"BBH|B", &address, &command, &data, &usepec)){
	    return NULL;
    }

    if(ioctl( fd, I2C_SLAVE, address)){
        PyErr_SetString(I2CError, "Error setting I2C address.");
        return NULL;
    }

    if( ioctl( fd, I2C_PEC, usepec) < 0)
    {
        PyErr_SetString(I2CError, "Could not configure checksumming on i2c.\n");
        return;
    }
    
    datablock.word = data;

    if(i2c_smbus_access(fd, I2C_SMBUS_WRITE, command, I2C_SMBUS_WORD_DATA, &datablock) < 0){
        PyErr_SetString(I2CError, "Error writing to bus");
        return NULL;
    }

    Py_INCREF(Py_None);

    return Py_None;
}
static PyObject* c2py_smbusreadword_data( PyObject *self, 
				   PyObject *args )
{
    unsigned char address, command, usepec = 1;
    union i2c_smbus_data datablock;

	if (!PyArg_ParseTuple(args,"BB|B", &address, &command, &usepec)){
	    return NULL;
    }
    
    if(ioctl( fd, I2C_SLAVE, address)){
        PyErr_SetString(I2CError, "Error setting I2C address.");
        return NULL;
    }

    if( ioctl( fd, I2C_PEC, usepec) < 0)
    {
        PyErr_SetString(I2CError, "Could not configure checksumming on i2c.\n");
        return;
    }
    
    if(i2c_smbus_access(fd, I2C_SMBUS_READ, command, I2C_SMBUS_WORD_DATA, &datablock)){
        PyErr_SetString(I2CError, "Error reading from bus");
        return NULL;
    }

    return Py_BuildValue("i", datablock.word & 0x0FFFF);
}

