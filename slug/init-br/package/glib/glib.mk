##############################
# glib
##############################

GLIB_VERSION_MAJOR := 2.12
GLIB_VERSION_MINOR := 0
GLIB_VERSION := $(GLIB_VERSION_MAJOR).$(GLIB_VERSION_MINOR)
GLIB_SOURCE := glib-$(GLIB_VERSION).tar.bz2
GLIB_SITE := ftp://ftp.gtk.org/pub/glib/$(GLIB_VERSION_MAJOR)
GLIB_DIR := $(BUILD_DIR)/glib-$(GLIB_VERSION)

$(DL_DIR)/$(GLIB_SOURCE):
	$(WGET) -P $(DL_DIR) $(GLIB_SITE)/$(GLIB_SOURCE)

$(GLIB_DIR)/.source: $(DL_DIR)/$(GLIB_SOURCE)
	tar -xj -C $(BUILD_DIR) -f $(DL_DIR)/$(GLIB_SOURCE)
	touch $(GLIB_DIR)/.source

$(GLIB_DIR)/.configured: $(GLIB_DIR)/.source
	rm -f $(GLIB_DIR)/config.cache
	cp package/glib/config.cache $(GLIB_DIR)
	(cd $(GLIB_DIR); \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS) -I$(STAGING_DIR)/usr/local/include" \
		LDFLAGS="-L$(STAGING_DIR)/usr/local/lib -L$(STAGING_DIR)/usr/lib" \
		./configure \
		--target=$(GNU_TARGET_NAME) \
		--host=$(GNU_TARGET_NAME) \
		--build=$(GNU_HOST_NAME) \
		--with-libiconv \
		--cache-file=config.cache \
	);
	touch $@

$(GLIB_DIR)/glib/.libs/libglib-2.0.so: $(GLIB_DIR)/.configured
	$(MAKE) CC=$(TARGET_CC) -C $(GLIB_DIR) LDFLAGS="-L$(STAGING_DIR)/usr/local/lib -L$(STAGING_DIR)/usr/lib" CFLAGS="-I$(STAGING_DIR)/usr/local/include"

$(STAGING_DIR)/usr/local/lib/libglib-2.0.so.0.1200.0: $(GLIB_DIR)/glib/.libs/libglib-2.0.so 
	$(MAKE) DESTDIR=$(STAGING_DIR) -C $(GLIB_DIR) install

$(TARGET_DIR)/usr/local/lib/libglib-2.0.so.0.1200.0: $(STAGING_DIR)/usr/local/lib/libglib-2.0.so.0.1200.0
	mkdir -p $(TARGET_DIR)/usr/local/lib
	cp $(STAGING_DIR)/usr/local/lib/libglib* $(TARGET_DIR)/usr/local/lib

	mkdir -p $(TARGET_DIR)/usr/local/share/locale
	cp -r $(STAGING_DIR)/usr/local/share/locale/* $(TARGET_DIR)/usr/local/share/locale


glib: libiconv pkgconfig libintl $(TARGET_DIR)/usr/local/lib/libglib-2.0.so.0.1200.0

glib-source: $(DL_DIR)/$(GLIB_SOURCE)

ifeq ($(strip $(BR2_PACKAGE_GLIB)),y)
TARGETS+=glib
endif