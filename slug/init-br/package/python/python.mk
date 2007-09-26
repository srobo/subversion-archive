#############################################################
#
# python
#
#############################################################
PYTHON_VERSION=2.4.2
PYTHON_SOURCE:=Python-$(PYTHON_VERSION).tar.bz2
PYTHON_SITE:=http://python.org/ftp/python/$(PYTHON_VERSION)
PYTHON_DIR:=$(BUILD_DIR)/Python-$(PYTHON_VERSION)
PYTHON_CAT:=$(BZCAT)
PYTHON_BINARY:=python
PYTHON_TARGET_BINARY:=usr/bin/python

$(DL_DIR)/$(PYTHON_SOURCE):
	 $(WGET) -P $(DL_DIR) $(PYTHON_SITE)/$(PYTHON_SOURCE)

python-source: $(DL_DIR)/$(PYTHON_SOURCE)

$(PYTHON_DIR)/.unpacked: $(DL_DIR)/$(PYTHON_SOURCE)
	$(PYTHON_CAT) $(DL_DIR)/$(PYTHON_SOURCE) | tar -C $(BUILD_DIR) $(TAR_OPTIONS) -
	touch $(PYTHON_DIR)/.unpacked

$(PYTHON_DIR)/.patched: $(PYTHON_DIR)/.unpacked
	toolchain/patch-kernel.sh $(PYTHON_DIR) package/python/ python\*.patch
	touch $(PYTHON_DIR)/.patched

$(PYTHON_DIR)/.hostpython: $(PYTHON_DIR)/.patched
	(cd $(PYTHON_DIR); rm -rf config.cache; \
		OPT="-O1" \
		./configure \
		--with-cxx=no \
		--with-system-ffi \
		$(DISABLE_NLS); \
		make python Parser/pygen; \
		mv python hostpython; \
		mv Parser/pgen Parser/hostpgen; \
		make distclean \
	);
	touch $(PYTHON_DIR)/.hostpython

ifeq ($(strip $(BR2_PACKAGE_PYTHON_SHARED_LIB)),y)
PYTHON_CONF_EXTRA := --enable-shared
else
PYTHON_CONF_EXTRA := 
endif

$(PYTHON_DIR)/.configured: $(PYTHON_DIR)/.hostpython
	(cd $(PYTHON_DIR); rm -rf config.cache; \
		$(TARGET_CONFIGURE_OPTS) \
		OPT="$(TARGET_OPTIMIZATION)" \
		./configure \
		--target=$(GNU_TARGET_NAME) \
		--host=$(GNU_TARGET_NAME) \
		--build=$(GNU_HOST_NAME) \
		--prefix=/usr \
		--sysconfdir=/etc \
		--with-cxx=no \
		$(PYTHON_CONF_EXTRA)	\
		$(DISABLE_NLS) \
	);
	touch $(PYTHON_DIR)/.configured

$(PYTHON_DIR)/$(PYTHON_BINARY): $(PYTHON_DIR)/.configured
	export PYTHON_DISABLE_SSL=1
	$(MAKE) CC=$(TARGET_CC) -C $(PYTHON_DIR) DESTDIR=$(TARGET_DIR) \
		PYTHON_DISABLE_MODULES="readline pyexpat dbm gdbm bsddb _curses _curses_panel _tkinter" \
		HOSTPYTHON=./hostpython HOSTPGEN=./Parser/hostpgen

$(TARGET_DIR)/$(PYTHON_TARGET_BINARY): $(PYTHON_DIR)/$(PYTHON_BINARY)
	export PYTHON_DISABLE_SSL=1
	LD_LIBRARY_PATH=$(STAGING_DIR)/lib
	$(MAKE) CC=$(TARGET_CC) -C $(PYTHON_DIR) install \
		DESTDIR=$(TARGET_DIR) CROSS_COMPILE=yes \
		PYTHON_DISABLE_MODULES="readline pyexpat dbm gdbm bsddb _curses _curses_panel _tkinter" \
		HOSTPYTHON=./hostpython HOSTPGEN=./Parser/hostpgen
	rm $(TARGET_DIR)/usr/bin/python?.?
	rm $(TARGET_DIR)/usr/bin/idle
	rm $(TARGET_DIR)/usr/bin/pydoc
	find $(TARGET_DIR)/usr/lib/ -name '*.pyc' -o -name '*.pyo' -exec rm {} \;
	rm -rf $(TARGET_DIR)/share/locale $(TARGET_DIR)/usr/info \
		$(TARGET_DIR)/usr/man $(TARGET_DIR)/usr/share/doc \
		$(TARGET_DIR)/usr/lib/python*/test

#Install the python header files
$(STAGING_DIR)/usr/include/python-2.4/Python.h: $(TARGET_DIR)/$(PYTHON_TARGET_BINARY)
	mkdir -p $(STAGING_DIR)/usr/include/python-2.4
	cp $(TARGET_DIR)/usr/include/python2.4/*.h $(STAGING_DIR)/usr/include/python-2.4

#Install the python library
$(STAGING_DIR)/usr/lib/libpython2.4.so: $(PYTHON_DIR)/$(PYTHON_BINARY)
	mkdir -p $(STAGING_IR)/usr/lib
	cp $(PYTHON_DIR)/libpython2.4* $(STAGING_DIR)/usr/lib

python: uclibc $(TARGET_DIR)/$(PYTHON_TARGET_BINARY) $(STAGING_DIR)/usr/include/python-2.4/Python.h $(STAGING_DIR)/usr/lib/libpython2.4.so

python-clean:
	-$(MAKE) -C $(PYTHON_DIR) distclean
	rm $(PYTHON_DIR)/.configured

python-dirclean:
	rm -rf $(PYTHON_DIR)
#############################################################
#
# Toplevel Makefile options
#
#############################################################
ifeq ($(strip $(BR2_PACKAGE_PYTHON)),y)
TARGETS+=python
endif
