#############################################################
#
# file
#
#############################################################
FILE_VER:=4.17
FILE_SOURCE:=file-$(FILE_VER).tar.gz
FILE_SITE:=ftp://ftp.astron.com/pub/file
FILE_DIR1:=$(TOOL_BUILD_DIR)/file-$(FILE_VER)
FILE_DIR2:=$(BUILD_DIR)/file-$(FILE_VER)
FILE_CAT:=$(ZCAT)
FILE_BINARY:=src/file
FILE_TARGET_BINARY:=usr/bin/file

$(DL_DIR)/$(FILE_SOURCE):
	 $(WGET) -P $(DL_DIR) $(FILE_SITE)/$(FILE_SOURCE)

file-source: $(DL_DIR)/$(FILE_SOURCE)


#############################################################
#
# build file for use on the host system
#
#############################################################
$(FILE_DIR1)/.unpacked: $(DL_DIR)/$(FILE_SOURCE)
	$(FILE_CAT) $(DL_DIR)/$(FILE_SOURCE) | tar -C $(TOOL_BUILD_DIR) $(TAR_OPTIONS) -
	touch $(FILE_DIR1)/.unpacked

$(FILE_DIR1)/.configured: $(FILE_DIR1)/.unpacked
	(cd $(FILE_DIR1); rm -rf config.cache; \
		./configure \
		--prefix=$(FILE_DIR1)/install \
	);
	touch $(FILE_DIR1)/.configured

$(TOOL_BUILD_DIR)/bin/file: $(FILE_DIR1)/.configured
	$(MAKE) -C $(FILE_DIR1) install
	ln -sf $(FILE_DIR1)/install/bin/file $(TOOL_BUILD_DIR)/bin/file

host-file: $(TOOL_BUILD_DIR)/bin/file

host-file-clean:
	$(MAKE) -C $(FILE_DIR1) clean

host-file-dirclean:
	rm -rf $(FILE_DIR1)

#############################################################
#
# build file for use on the target system
#
#############################################################
file-unpacked: $(FILE_DIR2)/.unpacked
$(FILE_DIR2)/.unpacked: $(DL_DIR)/$(FILE_SOURCE)
	$(FILE_CAT) $(DL_DIR)/$(FILE_SOURCE) | tar -C $(BUILD_DIR) $(TAR_OPTIONS) -
	toolchain/patch-kernel.sh $(FILE_DIR2) package/file/ file\*.patch
	touch $(FILE_DIR2)/.unpacked

$(FILE_DIR2)/.configured: $(FILE_DIR2)/.unpacked
	(cd $(FILE_DIR2); rm -rf config.cache; \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS)" \
		./configure \
		--target=$(GNU_TARGET_NAME) \
		--host=$(GNU_TARGET_NAME) \
		--build=$(GNU_HOST_NAME) \
		--prefix=/usr \
		--exec-prefix=/usr \
		--bindir=/usr/bin \
		--sbindir=/usr/sbin \
		--libexecdir=/usr/lib \
		--sysconfdir=/etc \
		--datadir=/usr/share/misc \
		--localstatedir=/var \
		--mandir=/usr/man \
		--infodir=/usr/info \
		$(DISABLE_NLS) \
		$(DISABLE_LARGEFILE) \
		--enable-static \
		--disable-fsect-man5 \
	);
	touch $(FILE_DIR2)/.configured

$(FILE_DIR2)/$(FILE_BINARY): $(FILE_DIR2)/.configured $(TOOL_BUILD_DIR)/bin/file
	$(MAKE) $(TARGET_CONFIGURE_OPTS) LDFLAGS="-static" -C $(FILE_DIR2)

$(TARGET_DIR)/$(FILE_TARGET_BINARY): $(FILE_DIR2)/$(FILE_BINARY)
	$(MAKE) $(TARGET_CONFIGURE_OPTS) DESTDIR=$(TARGET_DIR) -C $(FILE_DIR2) install
	-($(STRIP) $(TARGET_DIR)/usr/lib/libmagic.so.*.* > /dev/null 2>&1)
	rm -rf $(TARGET_DIR)/share/locale $(TARGET_DIR)/usr/info \
		$(TARGET_DIR)/usr/man $(TARGET_DIR)/usr/share/doc

file: zlib uclibc $(TARGET_DIR)/$(FILE_TARGET_BINARY)

file-clean:
	$(MAKE) DESTDIR=$(TARGET_DIR) CC=$(TARGET_CC) -C $(FILE_DIR2) uninstall
	-$(MAKE) -C $(FILE_DIR2) clean

file-dirclean:
	rm -rf $(FILE_DIR2)

#############################################################
#
# Toplevel Makefile options
#
#############################################################
ifeq ($(strip $(BR2_PACKAGE_FILE)),y)
TARGETS+=file
endif
