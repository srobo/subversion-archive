############################################################
#
# flashb
#
############################################################
FLASHB_REVISION := 2272
FLASHB_SVN := http://svn.srobo.org/slug/utils/flashb/trunk

ifneq ($(strip $(subst ",, $(BR2_PACKAGE_FLASHB_IS_DEV))),y)
#"
FLASHB_SOURCE := $(DL_DIR)/flashb-$(FLASHB_REVISION)
FLASHB_DIR := $(BUILD_DIR)/flashb-$(FLASHB_REVISION)

$(FLASHB_SOURCE)/.exported:
	svn export -r $(FLASHB_REVISION) $(FLASHB_SVN) $(FLASHB_SOURCE)
	touch $(FLASHB_SOURCE)/.exported

$(FLASHB_DIR)/.source: $(FLASHB_SOURCE)/.exported
	cp -r $(FLASHB_SOURCE) $(FLASHB_DIR)
	touch $(FLASHB_DIR)/.source
else
FLASHB_DIR :=$(strip $(subst ",, $(BR2_PACKAGE_FLASHB_DEV_PATH)))
#"

$(FLASHB_DIR)/.source:
#	echo ***** WARNING: Building flashb from development tree ******
	touch $(FLASHB_DIR)/.source
endif

$(FLASHB_DIR)/flashb: $(FLASHB_DIR)/.source
	( export PKG_CONFIG_LIBDIR=$(STAGING_DIR)/usr/local/lib/pkgconfig ; \
	  export PKG_CONFIG_ARGS="--define-variable=prefix=$(STAGING_DIR)/usr/local" ; \
	  LDFLAGS="-L $(STAGING_DIR)/usr/local/lib -L $(STAGING_DIR)/usr/lib" CFLAGS="-I $(STAGING_DIR)/usr/include" $(MAKE) CC=$(TARGET_CC) -C $(FLASHB_DIR) ; )

$(TARGET_DIR)/usr/bin/flashb: $(FLASHB_DIR)/flashb
	cp -a $(FLASHB_DIR)/flashb $(TARGET_DIR)/usr/bin/

flashb: glib libelf $(TARGET_DIR)/usr/bin/flashb

ifeq ($(strip $(BR2_PACKAGE_FLASHB)),y)
TARGETS+=flashb
endif
