#!/bin/sh


while ! { dd if=/dev/sda of=/dev/null bs=1 count=1 ; }
do
sleep 1
done

while ! { dd if=/dev/sdb of=/dev/null bs=1 count=1 ; }
do
sleep 1
done

# Work out which disk it's from
mount /dev/sda /mnt/testa
mount /dev/sdb /mnt/testb

newroot="not found"
usrdev=

echo testa:
ls -al /mnt/testa

echo testb:
ls -al /mnt/testb

if [ -f /mnt/testa/sr-root ]
then
    # Root's on sda
newroot=/dev/sda
usrdev=/dev/sdb

umount /mnt/testb

elif [ -f /mnt/testb/sr-root ]
then
    # Root's on sdb
newroot=/dev/sdb
usrdev=/dev/sda

umount /mnt/testa
fi

mount
echo "newroot is $newroot"

mkdir /tmp/newroot
mount $newroot /tmp/newroot/
pivot_root /tmp/newroot/ /tmp/newroot/initrd/

echo "pivoted"

mount -a

umount /initrd/proc 
umount /initrd/dev/pts 
umount /initrd/tmp

killall syslogd klogd

mkdir -p /mnt/usr
mount $usrdev /mnt/usr

exec /etc/ecssr.sh


