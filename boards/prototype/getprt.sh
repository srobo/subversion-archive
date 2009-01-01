#!/bin/bash
if [ -f $2 ]
then
    rm $2
fi

echo "export partlist $2;" > munch.scr
eagle5 $1 -S munch.scr &
while [ ! -f $2 ]
do
    sleep 0.1
done
killall eagle5
rm munch.scr