CC=/tmp/trb105-buildroot/ecssr/slug/init-br/build_armeb/staging_dir/bin/armeb-linux-gcc
pwr-test-read: pwr-test-read.c
	$(CC) -o pwr-test-read pwr-test-read.c
	scp -P 4002 pwr-test-read root@localhost:/root/

pwr-test-write: pwr-test-write.c
	$(CC) -o pwr-test-write pwr-test-write.c
	scp -P 4002 pwr-test-write root@localhost:/root/
