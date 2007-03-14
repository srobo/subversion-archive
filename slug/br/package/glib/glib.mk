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
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="-L$(TARGET_DIR)/usr/local/lib -L$(TARGET_DIR)/usr/lib" \
		./configure \
		--target=$(GNU_TARGET_NAME) \
		--host=$(GNU_TARGET_NAME) \
		--build=$(GNU_HOST_NAME) \
		--with-libiconv \
		--cache-file=config.cache \
	);
	touch $@

glib: libiconv pkgconfig libintl $(GLIB_DIR)/.configured

glib-source: $(DL_DIR)/$(GLIB_SOURCE)

ifeq ($(strip $(BR2_PACKAGE_GLIB)),y)
TARGETS+=glib
endif