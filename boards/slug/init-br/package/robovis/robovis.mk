##############################
# robovis
##############################

ROBOVIS_REV := 270
ROBOVIS_REPO := svn+ssh://rds204@svn.ugforge.ecs.soton.ac.uk/projects/ecssr/slug/vision/robovis
ROBOVIS_DIR := $(BUILD_DIR)/robovis-$(ROBOVIS_REV)

$(DL_DIR)/robovis-$(ROBOVIS_REV):
	svn co -r $(ROBOVIS_REV) $(ROBOVIS_REPO) $@

$(ROBOVIS_DIR)/.source: $(DL_DIR)/robovis-$(ROBOVIS_REV)
	cp -r $< $(ROBOVIS_DIR)
	touch $@

$(ROBOVIS_DIR)/robovis.so: $(ROBOVIS_DIR)/.source
	$(MAKE) CXX="$(TARGET_CXX)" \
		PATH="$(BASE_DIR)/package/opencv:$$PATH" \
		PKG_CONFIG_PATH="$(STAGING_DIR)/usr/local/lib/pkgconfig:$(STAGING_DIR)/usr/local/lib/pkgconfig" \
		PKG_CONFIG_ALLOW_SYSTEM_CFLAGS=1 \
		PKG_CONFIG_PREFIX="$(STAGING_DIR)" \
		PY_CFLAGS="-I$(STAGING_DIR)/usr/include/python-2.4" \
		-C $(ROBOVIS_DIR)

$(TARGET_DIR)/usr/lib/robovis.so: $(ROBOVIS_DIR)/robovis.so
	mkdir -p $(TARGET_DIR)/usr/lib
	cp $< $@

$(STAGING_DIR)/usr/lib/robovis.so: $(ROBOVIS_DIR)/robovis.so
	mkdir -p $(STAGING_DIR)/usr/lib
	cp $< $@

$(ROBOVIS_DIR)/catcam: $(ROBOVIS_DIR)/.source
	$(MAKE) CXX="$(TARGET_CXX)" \
		PATH="$(BASE_DIR)/package/opencv:$$PATH" \
		PKG_CONFIG_PATH="$(STAGING_DIR)/usr/local/lib/pkgconfig:$(STAGING_DIR)/usr/local/lib/pkgconfig" \
		PKG_CONFIG_ALLOW_SYSTEM_CFLAGS=1 \
		PKG_CONFIG_PREFIX="$(STAGING_DIR)" \
		PY_CFLAGS="-I$(STAGING_DIR)/usr/include/python-2.4" \
		-C $(ROBOVIS_DIR) catcam


robovis: blobslib $(TARGET_DIR)/usr/lib/robovis.so $(STAGING_DIR)/usr/lib/robovis.so $(ROBOVIS_DIR)/catcam

ifeq ($(strip $(BR2_PACKAGE_ROBOVIS)),y)
TARGETS+=robovis
endif

