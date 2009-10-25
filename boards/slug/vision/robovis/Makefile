CFLAGS += -Wall -Wextra -Werror

CBFLAGS = `pkg-config --cflags blobslib`
LDBFLAGS = `pkg-config --libs blobslib`
OPENCV_CFLAGS += `pkg-config --cflags opencv` -Wall
OPENCV_LDFLAGS += `pkg-config --libs opencv`

# Python 2.4 doesn't support pkg-config; bodge this to your own include path
PY_CFLAGS += -I/usr/include/python2.4

hueblobs: hueblobs.c visfunc.o
	$(CXX) -ggdb -o $@ $(OPENCV_CFLAGS) $< $(OPENCV_LDFLAGS) $(CFLAGS) visfunc.o

visfunc.o: visfunc.cpp
	$(CXX) -ggdb $< $(OPENCV_CFLAGS) $(CFLAGS) -c -o $@ -fPIC

.PHONY: clean

clean:
	-rm -f hueblobs visfunc.o
