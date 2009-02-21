#
# Makefile for blobs library for OpenCV Version 4 and its examples
#

.if !defined(DESTDIR)
  INST_MSG= @echo "Specify DESTDIR= on the command line (ie, /usr, /usr/local, /usr/pkg...)"; false
.else
  INST_MSG=@echo "Installing to $(DESTDIR)..."
.endif

CFLAGS= `pkg-config --cflags opencv gtk+-2.0` -I. 
LDFLAGS= `pkg-config --libs opencv gtk+-2.0` -L. -lblob 
CXX=g++

CPPFILES= \
	Blob.cpp\
	BlobResult.cpp \
	BlobExtraction.cpp

.SUFFIXES: .cpp.o
.cpp.o:	; echo 'Compiling $*.cpp' ; $(CXX) $(CFLAGS) -c $*.cpp

libblob.a: $(CPPFILES:.cpp=.o)
	ar ru libblob.a $(CPPFILES:.cpp=.o) 2> /dev/null
	ranlib libblob.a
#
#	@echo Build tests, examples and tools...
#	$(CXX) -g blobdemo.cpp $(LDFLAGS) $(CFLAGS) -o blobdemo
#	$(CXX) -g blobdemo2.cpp $(LDFLAGS) $(CFLAGS) -o blobdemo2
#
#	@echo Cleaning objects...
#	rm -f $(CPPFILES:.cpp=.o)

all: libblob.a

clean:
	@echo Cleaning...
	rm -f $(CFILES:.cpp=.o)
	rm -f libblob.a
	rm -f blobslib.pc

.PHONY: blobslib.pc
blobslib.pc:
	printf "prefix=$(DESTDIR)\nName: Blob library\nDescription: Image processing library to find blobs of jam\nVersion: 1.0\nRequres: opencv\nCflags: -I$$%s/include\nLibs: -L$$%s/lib -lblob\n" "{prefix}" "{prefix}" > blobslib.pc


install: libblob.a blobslib.pc
	$(INST_MSG)
	cp libblob.a $(DESTDIR)/lib
	cp Blob.h $(DESTDIR)/include
	cp blobslib.pc $(PKG_CONFIG_PATH)/blobslib.pc
