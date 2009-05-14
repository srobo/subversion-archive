CBFLAGS = `pkg-config --cflags blobslib`
LDBFLAGS = `pkg-config --libs blobslib`
OPENCV_CFLAGS += `pkg-config --cflags opencv` -Wall
OPENCV_LDFLAGS += `pkg-config --libs opencv`

# Python 2.4 doesn't support pkg-config; bodge this to your own include path
PY_CFLAGS += -I/usr/include/python2.4

robovis.so: robovis.o
	$(CXX) -shared $< $(LDBFLAGS) -o $@ 

robovis.o: robovis.cpp
	$(CXX) -ggdb $(CBFLAGS) $(CFLAGS) -o $@ -fPIC -c $< $(PY_CFLAGS)

catcam: catcam.cpp
	$(CXX) -ggdb $(CBFLAGS) -o $@ $< $(LDBFLAGS)

dispcam: dispcam.cpp
	$(CXX) -ggdb  -o dispcam $(CBFLAGS) dispcam.cpp $(LDBFLAGS)

hueblobs: hueblobs.c
	$(CXX) -ggdb -o $@ $(OPENCV_CFLAGS) $< $(OPENCV_LDFLAGS)

getblobs: getblobs.c
	$(CC) -ggdb -o $@ $(OPENCV_CFLAGS) $< $(OPENCV_LDFLAGS)

testcam: testcam.c
	$(CC) -ggdb -o $@ $(OPENCV_CFLAGS) $< $(OPENCV_LDFLAGS)

satthresh: satthresh.c
	$(CC) -ggdb -o $@ $(OPENCV_CFLAGS) $< $(OPENCV_LDFLAGS)

testhough: testhough.c
	$(CC) -ggdb -o $@ $(OPENCV_CFLAGS) $< $(OPENCV_LDFLAGS)

.PHONY: clean

clean:
	-rm -f robovis.o robovis.so
