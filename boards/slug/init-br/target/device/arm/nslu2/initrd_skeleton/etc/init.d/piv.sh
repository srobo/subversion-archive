#!/bin/sh
# Find the root disk and pivot to it 

# The challenge here is to find the USB key which is the SR root disk.
# The SR root disk is identified by the file "sr-root" in it's root.

disk_exists()
{
    DEV=$1
    dd if=$DEV of=/dev/null bs=1 count=1
}

check_disk()
{
    DEV=$1
    if { disk_exists $DEV ; }
	then
	# Try mounting it
	TD=`mktemp -d /tmp/init_check.XXXXXX`

	if ! { mount $DEV $TD ; }
	    then
	    umount $TD
	    rm -rf $TD
	    false
	    return
	fi

	if [ -f ${TD}/sr-root ]
	    then
	    # Pass the mountpoint through to the caller
	    echo $TD
	    true
	    return
	fi

	# It's not the root 
	umount $TD
	rm -rf $TD
	false
	return
    fi

    false
    return
}

# Scan around possible root disks
tmp_mount=""
root_dev=""
while [[ "$tmp_mount" == "" ]]
do
    for m in a b c d
    do
	! { disk_exists /dev/sd${m} ; } && continue

	for n in 1 2 3 4
	do
	    if { mnt=`check_disk /dev/sd${m}${n}` ; }
	    then
	        # Found the root
		tmp_mount=$mnt
		root_dev="/dev/sd${m}${n}"
		break
	    fi
	done

	[[ $tmp_mount != "" ]] && break
    done

    # Throttle this a little!
    usleep 200000
done

# Now pivot :-D
pivot_root ${tmp_mount} ${tmp_mount}/initrd/
echo "Pivoted to $root_dev"

mount -a

umount /initrd/proc 
umount /initrd/dev/pts 
umount /initrd/tmp

killall syslogd klogd

echo "Remounting root read-only"
mount -o remount -o ro -o noatime /

#Source the startup script on the newly mounted root
exec /etc/ecssr.sh


