#!/bin/bash


EXPECTED_ARGS=1
E_BADARGS=65

if [ $# -ne $EXPECTED_ARGS ]
then
  echo "Usage: `basename $0` <file.brd>"
  echo "output files in gerbers/"

  exit $E_BADARGS
fi

mkdir -p gerbers
eagle -X -dGERBER_RS274X -ogerbers/top.gbr $1 top pad via
eagle -X -dGERBER_RS274X -ogerbers/bottom.gbr $1 bottom pad via
eagle -X -dGERBER_RS274X -ogerbers/dim.gbr $1 dimension
eagle -X -dexcellon -ogerbers/drill.drd PWR1.brd drills