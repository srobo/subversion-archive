#!/bin/sh


while ! { dd if=/dev/sda of=/dev/null bs=1 count=1 ; }
do
sleep 1
done

mkdir /tmp/newroot
mount /dev/sda /tmp/newroot/
pivot_root /tmp/newroot/ /tmp/newroot/initrd/

echo "pivoted"

mount -a

umount /initrd/proc 
umount /initrd/dev/pts 
umount /initrd/tmp

killall syslogd klogd

#Source the startup script on the newly mounted root
exec /etc/ecssr.sh


