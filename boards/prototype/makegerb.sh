#!/bin/bash


EXPECTED_ARGS=2
#EXTENDED_ARGS=3
E_BADARGS=65




if [ $# -lt $EXPECTED_ARGS ]
then
  echo "Usage: `basename $0` <file.brd>"
  echo "output files in gerbers/"
  exit $E_BADARGS
fi


if [ $# -gt $EXPECTED_ARGS ]
then
    of1="-x$3 -y$4"
else
    of1=""
fi
echo $of1


eagle -X -dGERBER_RS274X  $of1 -o$2-top.gbr $1 top pad via
eagle -X -dGERBER_RS274X  $of1 -o$2-bottom.gbr $1 bottom pad via
eagle -X -dGERBER_RS274X  $of1 -o$2-dim.gbr $1 dimension
eagle -X -dexcellon  $of1 -o$2-drill.drd $1 drills
    


#eagle -X -dGERBER_RS274X -x3 -y4 -o$2-topshift.gbr $1 top pad via

