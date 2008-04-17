##############################
# xbout
##############################

XBOUT_REVISION := 1339
XBOUT_SVN := http://svn.studentrobotics.org/slug/xbout/trunk
XBOUT_SOURCE := $(DL_DIR)/xbout-$(XBOUT_REVISION)
XBOUT_DIR := $(BUILD_DIR)/xbout-$(XBOUT_REVISION)

$(DL_DIR)/xbout-$(XBOUT_REVISION)/.exported:
	svn export -r $(XBOUT_REVISION) $(XBOUT_SVN) $(XBOUT_SOURCE)
	touch $(XBOUT_SOURCE)/.exported

$(XBOUT_DIR)/.source: $(DL_DIR)/xbout-$(XBOUT_REVISION)/.exported
	cp -r $(DL_DIR)/xbout-$(XBOUT_REVISION) $(XBOUT_DIR)
	touch $(XBOUT_DIR)/.source

$(XBOUT_DIR)/xbout: $(XBOUT_DIR)/.source
	( export PKG_CONFIG_LIBDIR=$(STAGING_DIR)/usr/local/lib/pkgconfig ; \
	  export PKG_CONFIG_ARGS="--define-variable=prefix=$(STAGING_DIR)/usr/local" ; \
	  echo `pkg-config $$PKG_CONFIG_ARGS --cflags libxb` ; \
	 CFLAGS="-g" LDFLAGS="-L$(STAGING_DIR)/usr/lib" $(MAKE) CC=$(TARGET_CC) -C $(XBOUT_DIR) ; )

xbout: libxb $(XBOUT_DIR)/xbout

xbout-clean:
	$(MAKE) -C $(XBOUT_DIR) clean
	rm $(TARGET_DIR)/usr/bin/xbd

xbout-dirclean:
	rm -rf $(XBOUT_DIR)

ifeq ($(strip $(BR2_PACKAGE_XBOUT)),y)
TARGETS+=xbout
endif

