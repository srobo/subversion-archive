##############################
# opencv
##############################

OPENCV_DATE := 2007-09-26
#2007-03-24
OPENCV_REPOS := :pserver:anonymous@opencvlibrary.cvs.sourceforge.net:/cvsroot/opencvlibrary
OPENCV_DIR := $(BUILD_DIR)/opencv-$(OPENCV_DATE)

$(DL_DIR)/opencv-$(OPENCV_DATE):
	( cd $(DL_DIR); \
	  cvs -d$(OPENCV_REPOS) co -D $(OPENCV_DATE) opencv; \
	  mv opencv opencv-$(OPENCV_DATE) )

$(DL_DIR)/opencv-$(OPENCV_DATE).tar.bz2: $(DL_DIR)/opencv-$(OPENCV_DATE)
	tar -cjf $@ $<

$(OPENCV_DIR)/.source: $(DL_DIR)/opencv-$(OPENCV_DATE).tar.bz2
	cp -r $(DL_DIR)/opencv-$(OPENCV_DATE) $(OPENCV_DIR)
	touch $(OPENCV_DIR)/.source

$(OPENCV_DIR)/.configured: $(OPENCV_DIR)/.source
	#Hack to remove the libpng dependency
	sed -i -e 's/IMAGELIBS="-lpng12 $$IMAGELIBS"//' $(OPENCV_DIR)/configure
	sed -i -e 's/LIBS="-lpng12 $$LIBS"//' $(OPENCV_DIR)/configure
	sed -i -e 's/have_png=yes/have_png=no/' $(OPENCV_DIR)/configure
	sed -i -e 's/#define HAVE_PNG//' $(OPENCV_DIR)/configure
	( cd $(OPENCV_DIR); \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS) " \
		./configure \
		--target=$(GNU_TARGET_NAME) \
		--host=$(GNU_TARGET_NAME) \
		--build=$(GNU_HOST_NAME) \
		--with-v4l --without-python --without-quicktime --with-swig --without-libpng --without-gtk --disable-apps -C $(OPENCV_DIR)/config.cache );
	touch $@

$(OPENCV_DIR)/cv/src/.libs/libcv.so.2.0.0: $(OPENCV_DIR)/.configured
	$(MAKE) CFLAGS="$(TARGET_CFLAGS)" CC=$(TARGET_CC) -C $(OPENCV_DIR) 

$(STAGING_DIR)/usr/local/lib/libcv.so.2.0.0: $(OPENCV_DIR)/cv/src/.libs/libcv.so.2.0.0
	$(MAKE) DESTDIR=$(STAGING_DIR) -C $(OPENCV_DIR) install

$(TARGET_DIR)/usr/local/lib/libcv.so.2.0.0: $(STAGING_DIR)/usr/local/lib/libcv.so.2.0.0
	mkdir -p $(TARGET_DIR)/usr/local/lib
	cp -a $(STAGING_DIR)/usr/local/lib/libcv* $(TARGET_DIR)/usr/local/lib
	cp -a $(STAGING_DIR)/usr/local/lib/libcx* $(TARGET_DIR)/usr/local/lib
	cp -a $(STAGING_DIR)/usr/local/lib/libml* $(TARGET_DIR)/usr/local/lib
	cp -a $(STAGING_DIR)/usr/local/lib/libhighgui* $(TARGET_DIR)/usr/local/lib

opencv: libpng jpeg $(TARGET_DIR)/usr/local/lib/libcv.so.2.0.0

ifeq ($(strip $(BR2_PACKAGE_OPENCV)),y)
TARGETS+=opencv
endif