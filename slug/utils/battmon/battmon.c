#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>

#include "i2c-dev.h"

#define GETV 2
#define GETI 3
#define ADDRESS 0x3f

/* Reads a word from the I2C slave */
uint16_t readword(int fd, uint8_t cmd);

int main (int argc, char **argv)
{



  int fd = 0;

  /* Attempt to open I2C device (/dev/i2c-0) */
  fd = open ("/dev/i2c-0", O_RDWR);
  if (fd == -1)
    {
      fprintf (stderr, "Failed to open I2C device: %m\n");
      exit (0);
    }

  /* Set the slave address of type I2C_SLAVE */
  fprintf (stdout, "Initialising to address: %x\n", ADDRESS);
  if (ioctl(fd, I2C_SLAVE, ADDRESS) < 0)
    {
      fprintf (stderr, "Failed to set slave address: %m\n");
      exit (0);      
    }

  /* Attempt to enable PEC */
  if (ioctl(fd, I2C_PEC, 1) < 0)
    {
      fprintf (stderr, "Failed to enable PEC\n");
      exit (0);
    }

  struct timeval tp;		/* Timeval pointer - containing data since epoch */
  void *tpz; 			/* Unspecified Pointer - behaviour undefined by time.h */
  long int epoch = 0;		/* Used to define starting epoch */
  long int currenttime = 0;		/* Used to define the current time */

  uint16_t current;		/* The relative 16bit Current Value */
  uint16_t voltage;		/* The relative 16bit Voltage Value */
  
  /* Set the epoch value */
  gettimeofday (&tp, tpz);
  epoch = tp.tv_sec;  

  /* Begin Current and Voltage Reading */
  while (1)
    {
      /* Read current and voltage from I2C */
      voltage = readword (fd, GETV);
      current = readword (fd, GETI);      
      
      /* Get the current time */
      gettimeofday (&tp, tpz);
      currenttime = tp.tv_sec;

      /* Output the current time relative to epoch */
      fprintf (stderr, "Voltage: %u, Time: %d\n", voltage, currenttime - epoch);
      fprintf (stderr, "Current: %u, Time: %d\n", current, currenttime - epoch);
      sleep (1);
    }


  return 0;

}


/* Read a word from the I2C */
/* GETV - Gets a value for the voltage */
/* GETI - Gets a value for the current */
uint16_t readword(int fd, uint8_t cmd)
{
  signed int tmp = -1;

  /* Attempt to read from I2C */
  while (tmp < 1)
    {
      tmp = i2c_smbus_read_word_data( fd, cmd );
      if (tmp < 1)
	fprintf (stderr, "Error Reading from I2C Bus: %d\n", cmd);
    }

  return tmp;
}
