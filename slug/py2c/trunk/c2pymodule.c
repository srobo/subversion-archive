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
    {"powerwrite",c2py_powerwrite_data,METH_VARARGS,"Write  data block from power board with gumsense format"},
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

	fd = open("/dev/i2c-1", O_RDWR);
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
        return;
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




static PyObject* c2py_powerread_data( PyObject *self, 
				      PyObject *args )
{
	uint8_t address, command, nobytes;
	uint8_t *buf, i=0, c=0;
	PyObject *retval;

	if (!PyArg_ParseTuple(args,"BBB", &address, &command, &nobytes)){
		return NULL;
	}
}



/* int32_t sr_read( int fd, uint8_t reg, uint8_t *buf ) */
/* { */
/* 	/\* The Origins of this code are stolen from Robert Spanton's Gumsense firmware forge.ecs.soton.ac.uk *\/ */
/* 	int len, r; */
/* 	uint8_t checksum, i; */

/* 	pecoff(fd); /\* need to disable pec so can do 2 actions consecutively - see gumsense readme *\/ */

	
/* 	/\* We have to hack around the fact that the i2c adapter doesn't */
/* 	   support i2c block read. *\/ */
/* 	/\*  First, do a read to get the length byte. *\/ */
/* 	/\* This read byte operation also sets the command that we're doing */
/* 	   within the powerboard *\/ */

/* 	/\* Set the command and grab the length *\/ */
/* 	len = i2c_smbus_read_byte_data( fd, reg ); */

/* 	if( len < 0 ) { */
/* 		fprintf( stderr, "Failed to read register %hhu length\n", reg ); */
/* 		fprintf( stderr, "length is %d", len ); */
/* 		goto error0; */
/* 	} */

/* 	/\* Prevent buffer overflows *\/ */
/* 	assert( (len+3) < BUFLEN ); */

/* 	r = read(fd, buf, len + 3 ); */
	
/* 	if( r < 0 ) { */
/* 		fprintf( stderr, "Failed to read register %hhu\n", reg ); */
/* 		goto error0; */
/* 	} */

/* 	if( r != len + 3 ) { */
/* 		fprintf( stderr, "Failed to read all of register %hhu\n", reg ); */
/* 		goto error0; */
/* 	} */

/* 	/\* Generate the checksum: *\/ */
/* 	checksum = crc8( (ADDRESS<<1) | 1 ); */
/* 	for( i=0; i<len+2; i++ ) */
/* 		checksum = crc8( checksum ^ (buf)[i] ); */

/* 	if( (buf)[r-1] != checksum ) { */
/* 		if( 1 ) */
/* 			fprintf( stderr, "Incorrect checksum reading register %hhu\n", reg ); */
/* 		printf( "Checksums: received = 0x%2.2hhx, calculated = 0x%2.2hhx\n", */
/* 			(buf)[len+2], */
/* 			checksum ); */
/* 		/\* Checksum's incorrect *\/ */
/* 		goto error0; */
/* 	} */

/* 	if( (buf)[1] != reg ) { */
/* 		fprintf( stderr, "Incorrect register read %hhu\n", reg ); */

/* 		/\* Incorrect command read back *\/ */
/* 		goto error0; */
/* 	} */

/* 	if( 0 )	{		/\* read buffer debug - set to 1 to gat buffer printout *\/ */
/* 		uint8_t i; */
/* 		printf( "Read %i bytes from register %hhu:\n", len, reg ); */
/* 		for( i=0; i<len+2; i++ ) */
/* 			printf( "%hhX: %hhX\n", i, (buf)[i] ); */
/* 	} */

/* 	pecon(fd); */

/* 	return len+1; */

/* error0: */
/* 	pecon(fd); */
/* 	return -1; */
/* } */



static PyObject* c2py_powerwrite_data( PyObject *self, 
				       PyObject *args )
{

	unsigned char address, command, usepec=1;
	PyObject* data;	    /* waiting for a list */
	PyObject* list_int;	    /* waiting for a int from a list location */

	uint8_t *buf;
	uint8_t buf_len=0;
	uint8_t i;


/* 	union i2c_smbus_data datablock; */
	/* usepec is optional */
	if (!PyArg_ParseTuple(args,"BBO|B", &address, &command, &data, &usepec)){
		return NULL;
	}

	/* this is just to prove a theory */
	if (PyList_Check(data))
		printf("pyobject was a list !");
	else
	{
		PyErr_SetString(I2CError, "Object passed was not a list");
		printf("list is not a list!");
	}
		
	buf_len = PyList_Size(data);

	buf = malloc(buf_len);
	
	for(i=0;i<buf_len;i++)
	{
		list_int = PyList_GetItem(data, i );
		
		if (PyInt_Check(list_int))
			printf("int is an int - good");
		else
		{
			PyErr_SetString(I2CError, "Object passed was not an int");
			printf("int is not an int!");
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
		return;
	}
    
/*     datablock.word = data; */

	if( i2c_smbus_write_block_data(fd,command,buf_len,buf) != 0 )
	{
		PyErr_SetString(I2CError, "Error writing to bus");
		return NULL;
	}
	
	free(buf);

	Py_INCREF(Py_None);
    
	return Py_None;

}






/* int32_t sr_write( int fd, uint8_t command, uint8_t len, uint8_t *buf ){ */
/* 	int retval=0; */

/* 	retval = i2c_smbus_write_block_data(fd,command,len,buf); */

/* 	if (retval != 0) */
/* 	{ */
/* 		fprintf( stderr, "Block Write failed:  %m\n" ); */
/* 	} */
/* 	return retval; */
/* } */
