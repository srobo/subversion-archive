##############################
# libxb
##############################

LIBXB_REVISION := 991
LIBXB_SVN := http://svn.studentrobotics.org/xbee/trunk
LIBXB_SOURCE := $(DL_DIR)/libxb-$(LIBXB_REVISION)
LIBXB_DIR := $(BUILD_DIR)/libxb-$(LIBXB_REVISION)


$(DL_DIR)/libxb-$(LIBXB_REVISION)/.exported:
	svn export -r $(LIBXB_REVISION) $(LIBXB_SVN) $(LIBXB_SOURCE)
	touch $(LIBXB_SOURCE)/.exported

$(LIBXB_DIR)/.source: $(DL_DIR)/libxb-$(LIBXB_REVISION)/.exported
	cp -r $(DL_DIR)/libxb-$(LIBXB_REVISION) $(LIBXB_DIR)
	touch $(LIBXB_DIR)/.source

$(LIBXB_DIR)/xbd/xbd: $(LIBXB_DIR)/.source
# Set the pkg-config variables
	PKG_CONFIG_ARGS="--define-variable=libdir=$(STAGING_DIR)/usr/local/lib \
	--define-variable=includedir=$(STAGING_DIR)/usr/local/include" \
	PKG_CONFIG_PATH=$(STAGING_DIR)/usr/local/lib/pkgconfig:$(STAGING_DIR)/lib/pkgconfig \
	LDFLAGS="-L$(STAGING_DIR)/usr/lib" make CC=$(TARGET_CC) -C $(LIBXB_DIR)

$(TARGET_DIR)/usr/bin/xbd: $(LIBXB_DIR)/xbd/xbd
	cp $< $@

libxb: glib $(TARGET_DIR)/usr/bin/xbd

libxb-clean:
	$(MAKE) -C $(LIBXB_DIR) clean
	rm $(TARGET_DIR)/usr/bin/xbd

libxb-dirclean:
	rm -rf $(LIBXB_DIR)

ifeq ($(strip $(BR2_PACKAGE_LIBXB)),y)
TARGETS+=libxb
endif

