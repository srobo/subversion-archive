##############################
# libxb
##############################

LIBXB_REVISION := 33
LIBXB_SVN := https://libxb.svn.sourceforge.net/svnroot/libxb/pc/trunk
LIBXB_SOURCE := $(DL_DIR)/libxb-$(LIBXB_REVISION)
LIBXB_DIR := $(BUILD_DIR)/libxb-$(LIBXB_REVISION)


$(DL_DIR)/libxb-$(LIBXB_REVISION)/.exported:
	svn export -r $(LIBXB_REVISION) $(LIBXB_SVN) $(LIBXB_SOURCE)
	touch $(LIBXB_SOURCE)/.exported

$(LIBXB_DIR)/.source: $(DL_DIR)/libxb-$(LIBXB_REVISION)/.exported
	cp -r $(DL_DIR)/libxb-$(LIBXB_REVISION) $(LIBXB_DIR)
	touch $(LIBXB_DIR)/.source

$(LIBXB_DIR)/xbd/xbd: $(LIBXB_DIR)/.source
	( export PKG_CONFIG_LIBDIR=$(STAGING_DIR)/usr/local/lib/pkgconfig ; \
	  export PKG_CONFIG_ARGS="--define-variable=prefix=$(STAGING_DIR)/usr/local" ; \
	  LDFLAGS="-L$(STAGING_DIR)/usr/lib" $(MAKE) CC=$(TARGET_CC) -C $(LIBXB_DIR) ; )

$(TARGET_DIR)/usr/bin/xbd: $(LIBXB_DIR)/xbd/xbd
	cp $< $@

$(STAGING_DIR)/usr/local/lib/libxbee.so: $(LIBXB_DIR)/xbd/xbd
	cp -P $(LIBXB_DIR)/libxbee/libxbee.so* $(STAGING_DIR)/usr/local/lib

$(STAGING_DIR)/usr/local/include/libxb/xbee-conn.h: $(LIBXB_DIR)/xbd/xbd
	mkdir -p $(STAGING_DIR)/usr/local/include/libxb
	cp -P $(LIBXB_DIR)/common/*.h $(STAGING_DIR)/usr/local/include/libxb
	cp -P $(LIBXB_DIR)/libxbee/*.h $(STAGING_DIR)/usr/local/include/libxb
	cp $(LIBXB_DIR)/libxbee/libxb.pc $(STAGING_DIR)/usr/local/lib/pkgconfig

libxb: glib $(TARGET_DIR)/usr/bin/xbd $(STAGING_DIR)/usr/local/lib/libxbee.so \
	$(STAGING_DIR)/usr/local/include/libxb/xbee-conn.h

libxb-clean:
	$(MAKE) -C $(LIBXB_DIR) clean
	rm $(TARGET_DIR)/usr/bin/xbd

libxb-dirclean:
	rm -rf $(LIBXB_DIR)

ifeq ($(strip $(BR2_PACKAGE_LIBXB)),y)
TARGETS+=libxb
endif

