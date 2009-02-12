#!/bin/bash

EXPECTED_ARGS=3
E_BADARGS=65



if [ $# -lt $EXPECTED_ARGS ]
then
  echo "Usage: `basename $0` [4/5][file.brd] [outputprefix] <xshift  yshift>"
  echo "output files in gerbers/outputprefixfile.{gbr,drd}}"
  exit $E_BADARGS
fi


if [ $1 -eq "5" ]
then
    egl="/opt/eagle-5.2.0/bin/eagle" # path to eagle 5 binary
    echo 'using eagle 5'

elif [ $1 -eq "4" ]
then
    egl="/opt/eagle/bin/eagle1" # path to eagle 4 binary
    echo 'using eagle 4'
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

$egl -X -dGERBER_RS274X  $of1 -o$3-tstop.gbr $2 tstop
$egl -X -dGERBER_RS274X  $of1 -o$3-bstop.gbr $2 bstop
    

$egl -X -dGERBER_RS274X  $of1 -o$3-tsilk.gbr $2 tplace tname
$egl -X -dGERBER_RS274X  $of1 -o$3-bsilk.gbr $2 bplace bname



rm $3-top.gpi
rm $3-bottom.gpi
rm $3-dim.gpi
rm $3-drill.dri
rm $3-tstop.gpi
rm $3-bstop.gpi
rm $3-tsilk.gpi
rm $3-bsilk.gpi

