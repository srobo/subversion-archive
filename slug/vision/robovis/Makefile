dispcam: dispcam.cpp
	g++ -ggdb `pkg-config --cflags opencv` -o dispcam dispcam.cpp `pkg-config --libs opencv` -I ./cvblobslib_v6 cvblobslib_v6/libblob.a

robovis.o: robovis.cpp
	g++ -ggdb `pkg-config --cflags opencv` \
	-o robovis.o \
	-fPIC \
	-c robovis.cpp \
	-I ../cvblobslib_v6 \
	-I /usr/include/python2.4

robovis.so: robovis.o
	g++ -shared robovis.o ../cvblobslib_v6/libblob.a `pkg-config --libs opencv` -o robovis.so

clean:
	rm robovis.o robovis.so
