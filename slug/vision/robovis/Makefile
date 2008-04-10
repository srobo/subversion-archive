CBFLAGS = `pkg-config --cflags blobslib`
LDBFLAGS = `pkg-config --libs blobslib`
CFLAGS ?= `pkg-config --cflags opencv` -Wall
LDFLAGS += `pkg-config --libs opencv`

PY_CFLAGS ?= /usr/include/python2.4

robovis.so: robovis.o
	$(CXX) -shared $< $(LDBFLAGS) -o $@

robovis.o: robovis.cpp
	$(CXX) -ggdb $(CBFLAGS) -o $@ -fPIC -c $< $(PY_CFLAGS)

catcam: catcam.cpp
	$(CXX) -ggdb $(CBFLAGS) -o $@ $< $(LDBFLAGS)

dispcam: dispcam.cpp
	$(CXX) -ggdb  -o dispcam $(CBFLAGS) dispcam.cpp $(LDBFLAGS)

hueblobs: hueblobs.c
	$(CC) -ggdb -o $@ $(CFLAGS) $< $(LDFLAGS)

getblobs: getblobs.c
	$(CC) -ggdb -o $@ $(CFLAGS) $< $(LDFLAGS)

testcam: testcam.c
	$(CC) -ggdb -o $@ $(CFLAGS) $< $(LDFLAGS)

satthresh: satthresh.c
	$(CC) -ggdb -o $@ $(CFLAGS) $< $(LDFLAGS)

testhough: testhough.c
	$(CC) -ggdb -o $@ $(CFLAGS) $< $(LDFLAGS)

.PHONY: clean

clean:
	-rm -f robovis.o robovis.so
