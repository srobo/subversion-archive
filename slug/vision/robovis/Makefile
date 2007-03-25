CFLAGS = `pkg-config --cflags blobslib`
LDFLAGS = `pkg-config --libs blobslib`
PY_CFLAGS ?= /usr/include/python2.4

robovis.so: robovis.o
	$(CXX) -shared $< $(LDFLAGS) -o $@

robovis.o: robovis.cpp
	$(CXX) -ggdb $(CFLAGS) -o $@ -fPIC -c $< $(PY_CFLAGS)

catcam: catcam.cpp
	$(CXX) -ggdb $(CFLAGS) -o $@ $< $(LDFLAGS)

dispcam: dispcam.cpp
	$(CXX) -ggdb  -o dispcam $(CFLAGS) dispcam.cpp $(LDFLAGS)

.PHONY: clean

clean:
	-rm -f robovis.o robovis.so
