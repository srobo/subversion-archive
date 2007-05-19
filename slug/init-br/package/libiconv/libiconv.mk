##############################
# libiconv
##############################

ICONV_VERSION := 1.11
ICONV_SOURCE := libiconv-$(ICONV_VERSION).tar.gz
ICONV_SITE := http://ftp.gnu.org/pub/gnu/libiconv
ICONV_DIR := $(BUILD_DIR)/libiconv-$(ICONV_VERSION)

$(DL_DIR)/$(ICONV_SOURCE):
	$(WGET) -P $(DL_DIR) $(ICONV_SITE)/$(ICONV_SOURCE)

$(ICONV_DIR)/.source: $(DL_DIR)/$(ICONV_SOURCE)
	tar -xz -C $(BUILD_DIR) -f $(DL_DIR)/$(ICONV_SOURCE)
	touch $(ICONV_DIR)/.source

$(ICONV_DIR)/.configured: $(ICONV_DIR)/.source
	(cd $(ICONV_DIR); rm -rf config.cache ; \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS)" \
		./configure \
		--target=$(GNU_TARGET_NAME) \
		--host=$(GNU_TARGET_NAME) \
		--build=$(GNU_HOST_NAME) \
	);
	touch $@

$(ICONV_DIR)/src/iconv_no_i18n: $(ICONV_DIR)/.configured
	$(MAKE) CC=$(TARGET_CC) -C $(ICONV_DIR)

$(STAGING_DIR)/usr/local/include/iconv.h: $(ICONV_DIR)/src/iconv_no_i18n
	$(MAKE) CC=$(TARGET_CC) DESTDIR=$(STAGING_DIR) -C $(ICONV_DIR) install

$(TARGET_DIR)/usr/local/bin/iconv: $(STAGING_DIR)/usr/local/include/iconv.h
	mkdir -p $(TARGET_DIR)/usr/local/lib $(TARGET_DIR)/usr/local/bin
	cp $(STAGING_DIR)/usr/local/lib/libiconv* $(STAGING_DIR)/usr/local/lib/preloadable_libiconv* $(TARGET_DIR)/usr/local/lib
	cp $(STAGING_DIR)/usr/local/bin/iconv $(TARGET_DIR)/usr/local/bin

libiconv: $(TARGET_DIR)/usr/local/bin/iconv

ifeq ($(strip $(BR2_PACKAGE_LIBICONV)),y)
TARGETS+=libiconv
endif
