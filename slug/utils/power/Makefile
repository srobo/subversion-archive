CC=armeb-linux-gcc
motor-test: pwr-test.c
	$(CC) -o pwr-test pwr-test.c
	cp pwr-test ~/public_html/
	chmod a+r ~/public_html/pwr-test
	#scp -P 3000 pwr-test root@localhost:/root/
