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

static PyObject* c2py_smbusreadblock_data( PyObject *self, 
					   PyObject *args );

static PyObject* c2py_powerread_data( PyObject *self, 
				      PyObject *args );

static PyObject* c2py_powerwrite_data( PyObject *self, 
				       PyObject *args );

static PyObject* c2py_motor_reg_read( PyObject *self, 
				      PyObject *args );

static PyObject* c2py_motor_reg_write( PyObject *self, 
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
	{"powerread",c2py_powerread_data,METH_VARARGS,"Read data block from power board with gumsense format"},
	{"powerwrite",c2py_powerwrite_data,METH_VARARGS,"Write data block from power board with gumsense format"},
	{"motor_reg_read", c2py_motor_reg_read, METH_VARARGS, "Read register in motor board format."},
	{"motor_reg_write", c2py_motor_reg_write, METH_VARARGS, "Write register in motor board format."},
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
		printf ("inited!");
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
		return NULL;
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
		return NULL;
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
		return NULL;
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
		return NULL;
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
	PyObject *retval;

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
		return NULL;
	}
    
	if(i2c_smbus_write_byte(fd, command) < 0){
		PyErr_SetString(I2CError, "Error sending block command");
		return NULL;
	}

	c = crc8(address<<1);
	c = crc8(c ^ command);
	c = crc8(c ^ ((address<<1)|1));

	buf = malloc(nobytes+1);

	if(buf == NULL){
		PyErr_SetString(I2CError, "Error allocating I2C receive buffer.");
		return NULL;
	}

	if( read( fd, buf, nobytes+1 ) < nobytes+1 ){
		free(buf);
		PyErr_SetString(I2CError, "Error reading from I2C.");
		return NULL;
	}

	for( i=0; i<nobytes; i++ )
		c = crc8( c ^ buf[i] );

	if( c != buf[nobytes] ){
		printf("%2.2X %2.2X\n", c, buf[nobytes]);
		free(buf);
		PyErr_SetString(I2CError, "Checksum error.");
		return NULL;
	}

	retval = Py_BuildValue("s#", buf, nobytes-1);
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
		return NULL;
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
		return NULL;
	}
    
	if(i2c_smbus_access(fd, I2C_SMBUS_READ, command, I2C_SMBUS_WORD_DATA, &datablock)){
		PyErr_SetString(I2CError, "Error reading from bus");
		return NULL;
	}

	return Py_BuildValue("i", datablock.word & 0x0FFFF);
}

static PyObject* c2py_powerread_data( PyObject *self, 
				      PyObject *args )
{
	const uint8_t BUFLEN = 30; /* see gumsense docs for explanation */
	uint8_t address, command;
	uint8_t *buf = NULL;
	PyObject *retval;
	int32_t len,r;
	uint8_t checksum, i;

	if (!PyArg_ParseTuple(args,"BB", &address, &command)){
		return NULL;
	}

	if(ioctl( fd, I2C_SLAVE, address)){
		PyErr_SetString(I2CError, "Error setting I2C address.");
		return NULL;
	}

	/* pec off */
	if( ioctl( fd, I2C_PEC, 0) < 0)
	{
		PyErr_SetString(I2CError, "Could not configure checksumming on i2c.\n");
		goto error0;
	}

	len = i2c_smbus_read_byte_data( fd, command );

	if( len < 0 ) {
		printf("%m");
		PyErr_SetString(I2CError, "Error reading from bus");
		goto error0;
	}

	/* Prevent buffer overflows */
	if (!((len+3) < BUFLEN ))
	{
		PyErr_SetString(I2CError, "Error");
		goto error0;
	}

	buf = malloc(len +3 );
	r = read(fd, buf, len + 3 );

	if( r < 0 ) {
		PyErr_SetString(I2CError, "Error");
		goto error0;
	}

	if( r != len + 3 ) {
		PyErr_SetString(I2CError, "Error");
		goto error0;
	}

	/* Generate the checksum: */
	checksum = crc8( (address<<1) | 1 );
	for( i=0; i<len+2; i++ )
		checksum = crc8( checksum ^ (buf)[i] );

	if( (buf)[r-1] != checksum ) {
		PyErr_SetString(I2CError, "checksum Error");
		goto error0;		
	}

	if( (buf)[1] != command ) {
		PyErr_SetString(I2CError, "comand val Error");
		goto error0;		
	}

	/* pec on */
	if( ioctl( fd, I2C_PEC, 1) < 0)
	{
		PyErr_SetString(I2CError, "Could not configure checksumming on i2c.\n");
		goto error0;
	}

	retval = PyList_New(len);
	
	for (i=0;i<len;i++)
	{
/* 		printf("%hhu buf",buf[i+2]); */
/* 		printf("%li casted",((long)buf[i+2])); */		
		PyList_SetItem(retval, i , PyInt_FromLong((long)buf[i+2]) );
	}

	free(buf);

	return retval;

error0:
	ioctl( fd, I2C_PEC, 1);
	free(buf);
       	return NULL;

}

static PyObject* c2py_powerwrite_data( PyObject *self, 
				       PyObject *args )
{
	unsigned char address, command, usepec=1;
	PyObject* data;	    /* waiting for a list */
	PyObject* list_int;	    /* waiting for a int from a list location */

	uint8_t *buf;
	uint8_t buf_len=0;
	uint8_t i;

	/* usepec is optional */
	if (!PyArg_ParseTuple(args,"BBO|B", &address, &command, &data, &usepec)){
		return NULL;
	}

	/* this is just to prove a theory */
	if (! PyList_Check(data))
	{
		PyErr_SetString(PyExc_TypeError, "Object passed was not a list");
		return NULL;
	}
		
	buf_len = PyList_Size(data);

	buf = malloc(buf_len);
	
	for(i=0;i<buf_len;i++)
	{
		list_int = PyList_GetItem(data, i );
		
		if (!PyInt_Check(list_int))
		{
			PyErr_SetString(PyExc_TypeError, "Object passed was not an int");
		}
		
		buf[i]=(uint8_t)PyInt_AsUnsignedLongMask(list_int);
	}

	if(ioctl( fd, I2C_SLAVE, address)){
		PyErr_SetString(I2CError, "Error setting I2C address.");
		return NULL;
	}

	if( ioctl( fd, I2C_PEC, usepec) < 0)
	{
		PyErr_SetString(I2CError, "Could not configure checksumming on i2c.\n");
		return NULL;
	}
    
	if( i2c_smbus_write_block_data(fd,command,buf_len,buf) != 0 )
	{
		PyErr_SetString(I2CError, "Error writing to bus");
		return NULL;
	}
	
	free(buf);

	Py_INCREF(Py_None);
    
	return Py_None;
}

static PyObject* c2py_motor_reg_read( PyObject *self, 
				      PyObject *args )
{
	unsigned char address, reg, len;
 	int r;
	uint8_t checksum, i;
	PyObject *retval;

	/* TODO: Grab reg num and data */
	if (!PyArg_ParseTuple(args,"BBB", &address, &reg, &len))
		return NULL;
	uint8_t b[len + 1];

	/* Turn PEC off for this -- we calculate it ourselves */
	if( ioctl( fd, I2C_PEC, 0) < 0) {
		PyErr_SetString(I2CError, "Could not disable PEC.");
		return NULL;
	}

	if(ioctl( fd, I2C_SLAVE, address)) {
		PyErr_SetString(I2CError, "Error setting I2C address.");
		return NULL;
	}

	/* Send the command */
	r = i2c_smbus_write_byte(fd, reg);
	if( r < 0 ) {
		PyErr_SetString( I2CError, "c2py_motor_read_reg: Write command failed." );
		return NULL;
	}

	/* Read the data and checksum */
	r = read( fd, b, len + 1 );
	if( r < len + 1 ) {
		PyErr_SetString( I2CError, "Failed to read correct number of bytes from register." );
		return NULL;
	}

	checksum = crc8( address << 1 );
	checksum = crc8( checksum ^ reg );
	checksum = crc8( checksum ^ ((address << 1) | 1) );

	for( i=0; i<len; i++ )
		checksum = crc8( checksum ^ b[i] );

	if( checksum != b[len] ) {
		PyErr_SetString( I2CError, "Checksum failed." );
		return NULL;
	}

	retval = PyList_New(len);
	if( retval == NULL ) {
		PyErr_SetString( I2CError, "Couldn't create list." );
		return NULL;
	}

	for( i=0; i<len; i++ )
		PyList_SetItem( retval, i, PyInt_FromLong((long)b[i]) );

	return retval;
}

static PyObject* c2py_motor_reg_write( PyObject *self, 
				       PyObject *args )
{
	unsigned char address, reg, len;
	int w;
	uint8_t checksum, i;
	PyObject *data;

	/* TODO: Grab reg num and data */
	if (!PyArg_ParseTuple(args,"BBO", &address, &reg, &data))
		return NULL;

	if( !PyList_Check(data) ) {
		PyErr_SetString( PyExc_TypeError, "Data must be a list of unsigned integers" );
		return NULL;
	}

	len = PyList_Size(data);
	uint8_t lbuf[len + 2];

	/* Turn PEC off for this -- we calculate it ourselves */
	if( ioctl( fd, I2C_PEC, 0) < 0) {
		PyErr_SetString(I2CError, "Could not disable PEC.");
		return NULL;
	}

	if(ioctl( fd, I2C_SLAVE, address)) {
		PyErr_SetString(I2CError, "Error setting I2C address.");
		return NULL;
	}

	lbuf[0] = reg;
	for( i=0; i<len; i++ ) {
		PyObject *li = PyList_GetItem(data, i);

		if( !PyInt_Check(li) ) {
			PyErr_SetString( PyExc_TypeError, "Data list must only contain unsigned integers." );
			return NULL;
		}

		lbuf[i+1] = (uint8_t)PyInt_AsUnsignedLongMask(li);
	}

	/* Calculate the PEC */
	checksum = crc8( address<<1 );
	for( i=0; i<len+1; i++ )
		checksum = crc8( checksum ^ lbuf[i] );

	lbuf[len+1] = checksum;

	w = write( fd, lbuf, len+2 );

	if( w != len + 2 ) {
		PyErr_SetString( I2CError, "Couldn't write to I2C bus.");
		return NULL;
	}

	Py_INCREF(Py_None);
    
	return Py_None;
}
