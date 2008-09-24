#!/bin/bash

EXPECTED_ARGS=3
E_BADARGS=65



if [ $# -lt $EXPECTED_ARGS ]
then
  echo "Usage: `basename $0` [4/5][file.brd] <xshift  yshift>"
  echo "output files in gerbers/"
  exit $E_BADARGS
fi


if [ $1 -eq "5" ]
then
    egl="/opt/eagle-5.2.0/bin/eagle"
    echo 'using egl 5'

elif [ $1 -eq "4" ]
then
    egl="/opt/eagle/bin/eagle"
    echo 'using egl 5'
else
    echo "error second argument must be 4 or 5 indicating eagle version to use"
    exit 
fi



if [ $# -gt $EXPECTED_ARGS ]
then
    of1="-x$4 -y$5"
else
    of1=""
fi
echo $of1
echo $egl

$egl -X -dGERBER_RS274X  $of1 -o$3-top.gbr $2 top pad via
$egl -X -dGERBER_RS274X  $of1 -o$3-bottom.gbr $2 bottom pad via
$egl -X -dGERBER_RS274X  $of1 -o$3-dim.gbr $2 dimension
$egl -X -dexcellon  $of1 -o$3-drill.drd $2 drills
    

#eagle -X -dGERBER_RS274X -x3 -y4 -o$2-topshift.gbr $1 top pad via

