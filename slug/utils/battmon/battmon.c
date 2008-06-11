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
uint16_t readword(int fd, uint8_t cmd, FILE* write_file);

int main (int argc, char **argv)
{

  int fd = 0;

  int len; 			/* Defines the number of seconds to execute for */
  
  if (argc < 2)
    {
      fprintf (stderr, "Incorrect number of arguments:\n\n");
      fprintf (stderr, "./battmon <number of seconds to execute>\n");
      exit (0);
    }
  else
    len = atoi (argv[1]);

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

  int i = 0; 			/* Loop variable */

  FILE *write_file = NULL;	/* Output file pointer */

  /* Attempt to open output file */
  if ((write_file = fopen ("output", "w")) == NULL)
    {
      fprintf (stderr, "Error opening file!");
      exit (0);
    }

  /* Set the epoch value */
  gettimeofday (&tp, tpz);
  epoch = tp.tv_sec;  

  /* Begin Current and Voltage Reading */
  while (1)
    {
      fprintf (stderr, "Current iteration: %d\n", i);
      
      /* Read current and voltage from I2C */
      voltage = readword (fd, GETV, write_file);
      current = readword (fd, GETI, write_file);      
      
      /* Get the current time */
      gettimeofday (&tp, tpz);
      currenttime = tp.tv_sec;

      /* Output the current time relative to epoch */
      fprintf (write_file, "%u, %u, %d\n", voltage, current, (currenttime - epoch));
      sleep (1);
      i++;
    }

  fclose (write_file);
  return 0;

}


/* Read a word from the I2C */
/* GETV - Gets a value for the voltage */
/* GETI - Gets a value for the current */
uint16_t readword(int fd, uint8_t cmd, FILE *write_file)
{
  signed int tmp = -1;
  int i = 0;

  /* Attempt to read from I2C */
  while (tmp < 1)
    {
      tmp = i2c_smbus_read_word_data( fd, cmd );
      if (tmp < 1)
	{
	  while (i<5 && tmp < 1)
	    {
	      fprintf (stderr, "Error Reading from I2C Bus: %d\n", cmd);
	      tmp = i2c_smbus_read_word_data(fd, cmd);
	      i++;
	    }
	  if (tmp < 1)
	    {
	      fprintf (stderr, "Power Lost to Power Board\n");
	      fclose (write_file);
	      exit (0);
	    }
	  else
	    return tmp;
	}
    }
  return tmp;
}
