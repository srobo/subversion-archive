#!/bin/sh


while ! { dd if=/dev/sda2 of=/dev/null bs=1 count=1 ; }
do
sleep 1
done

mkdir /tmp/newroot
mount /dev/sda2 /tmp/newroot/
pivot_root /tmp/newroot/ /tmp/newroot/initrd/

echo "pivoted"

mount -a

umount /initrd/proc 
umount /initrd/dev/pts 
umount /initrd/tmp

killall syslogd klogd

echo "Remounting root read-only"
mount -o remount -o ro -o noatime /

#Source the startup script on the newly mounted root
exec /etc/ecssr.sh


