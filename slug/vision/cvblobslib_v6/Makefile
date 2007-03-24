#
# Makefile for blobs library for OpenCV Version 4 and its examples
#

CFLAGS += `pkg-config --cflags opencv ` -I. 
LDFLAGS += `pkg-config --libs opencv ` -L. -lblob 

CPPFILES= Blob.cpp\
	BlobResult.cpp \
	BlobExtraction.cpp

.SUFFIXES: .cpp.o
.cpp.o:
	$(CXX) $(CFLAGS) -c $*.cpp

libblob.a: $(CPPFILES:.cpp=.o)
	ar ru libblob.a $(CPPFILES:.cpp=.o) 
	ranlib libblob.a
	@echo Cleaning objects...
	rm -f $(CPPFILES:.cpp=.o)

all: clean

clean:
	@echo Cleaning...
	rm -f $(CFILES:.cpp=.o)
	rm -f libblob.a

