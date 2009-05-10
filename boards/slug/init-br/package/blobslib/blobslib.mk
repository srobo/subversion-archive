##############################
# blobslib
##############################

BLOBSLIB_REV :=  261
BLOBSLIB_REPO := svn+ssh://rds204@svn.ugforge.ecs.soton.ac.uk/projects/ecssr/slug/vision/cvblobslib_v6
BLOBSLIB_DIR := $(BUILD_DIR)/blobslib-$(BLOBSLIB_REV)

$(DL_DIR)/blobslib-$(BLOBSLIB_REV):
	svn co -r $(BLOBSLIB_REV) $(BLOBSLIB_REPO) $@

$(BLOBSLIB_DIR)/.source: $(DL_DIR)/blobslib-$(BLOBSLIB_REV)
	cp -r $< $(BLOBSLIB_DIR)
	touch $@

$(BLOBSLIB_DIR)/libblob.a: $(BLOBSLIB_DIR)/.source 
	$(MAKE) CXX=$(TARGET_CXX) CC=$(TARGET_CC) \
		PKG_CONFIG_PATH="$(STAGING_DIR)/usr/local/lib/pkgconfig" \
		PKG_CONFIG_PREFIX="$(STAGING_DIR)" \
		PATH="$(BASE_DIR)/package/opencv:$$PATH" -C $(BLOBSLIB_DIR)

$(STAGING_DIR)/usr/local/lib/libblob.a: $(BLOBSLIB_DIR)/libblob.a
	mkdir -p $(STAGING_DIR)/usr/lib
	cp $< $@

$(STAGING_DIR)/usr/include/Blob.h: $(BLOBSLIB_DIR)/libblob.a
	mkdir -p $(STAGING_DIR)/usr/include
	cp $(BLOBSLIB_DIR)/*.h $(STAGING_DIR)/usr/include

$(STAGING_DIR)/usr/local/lib/pkgconfig/blobslib.pc: $(BLOBSLIB_DIR)/.source
	mkdir -p $(STAGING_DIR)/usr/local/lib/pkgconfig
	cp $(BLOBSLIB_DIR)/pkg/blobslib.pc $@

blobslib: opencv $(STAGING_DIR)/usr/local/lib/libblob.a $(STAGING_DIR)/usr/include/Blob.h $(STAGING_DIR)/usr/local/lib/pkgconfig/blobslib.pc

ifeq ($(strip $(BR2_PACKAGE_BLOBSLIB)),y)
TARGETS+=blobslib
endif

