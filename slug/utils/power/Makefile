
CC=/tmp/2trb-buildroot/slug/init-br/build_armeb/staging_dir/bin/armeb-linux-gcc
#CC=/tmp/trb105-buildroot/ecssr/slug/init-br/build_armeb/staging_dir/bin/armeb-linux-gcc
all: pwr-test-read pwr-test-write

pwr-test-read: pwr-test-read.c
	$(CC) -o pwr-test-read pwr-test-read.c
	scp pwr-test-read slug:~/

pwr-test-write: pwr-test-write.c
	$(CC) -o pwr-test-write pwr-test-write.c
	scp pwr-test-write slug:~/

.PHONY: clean

clean:
	-rm -f pwr-test-{read,write}
