#############################################################
#
# Linux kernel targets
#
# Note:  If you have any patches to apply, create the directory
# sources/kernel-patches and put your patches in there and number
# them in the order you wish to apply them...  i.e.
#
#   sources/kernel-patches/001-my-special-stuff.bz2
#   sources/kernel-patches/003-gcc-Os.bz2
#   sources/kernel-patches/004_no-warnings.bz2
#   sources/kernel-patches/030-lowlatency-mini.bz2
#   sources/kernel-patches/031-lowlatency-fixes-5.bz2
#   sources/kernel-patches/099-shutup.bz2
#   etc...
#
# these patches will all be applied by the patch-kernel.sh
# script (which will also abort the build if it finds rejects)
#  -Erik
#
# File based on gumstix buildroot linux.mk (r1199)
#  -Rob 11/12/2006
#
#############################################################
ifneq ($(filter $(TARGETS),linux),)

# Version of Linux to download and then apply patches to
DOWNLOAD_LINUX_VERSION=2.6.27
# Version of Linux AFTER patches
LINUX_VERSION=$(DOWNLOAD_LINUX_VERSION)nslu2

LINUX_FORMAT=zImage

LINUX_KARCH:=$(shell echo $(ARCH) | sed -e 's/i[3-9]86/i386/' \
	-e 's/mipsel/mips/' \
	-e 's/powerpc/ppc/' \
	-e 's/sh[234]/sh/' \
	-e 's/armeb/arm/' \
	)
LINUX_BINLOC=arch/$(LINUX_KARCH)/boot/$(LINUX_FORMAT)

LINUX_DIR=$(BUILD_DIR)/linux-$(LINUX_VERSION)
#LINUX_HEADERS_DIR=$(LINUX_DIR)
LINUX_SOURCE=linux-$(DOWNLOAD_LINUX_VERSION).tar.bz2
LINUX_SITE=http://www.uk.kernel.org/pub/linux/kernel/v2.6
LINUX_KCONFIG=$(BASE_DIR)/target/device/arm/nslu2/linux.config
LINUX_PATCH_DIR=$(BASE_DIR)/target/device/arm/nslu2/kernel-patches
LINUX_KERNEL=$(TARGET_DIR)/boot/zImage
# Used by pcmcia-cs and others
LINUX_SOURCE_DIR=$(LINUX_DIR)
LINUX_UIMAGE=arch/arm/boot/uImage
pathsearch = $(firstword $(wildcard $(addsuffix /$(1),$(subst :, ,/sbin))))
DEPMOD_TMP := $(call pathsearch,depmod)
DEPMOD=$(BUSYBOX_DIR)/examples/depmod.pl


# ifneq "$(DEPMOD_TMP)" "/sbin/depmod"
# DEPMOD=$(BUSYBOX_DIR)/examples/depmod.pl
# echo DEPMOD1=$(DEPMOD)
# else
# DEPMOD=/sbin/depmod
# echo DEPMOD2=$(DEPMOD)
# endif

#ifneq  ($(DEPMOD),/sbin/depmod)
#echo No depmod on path, using busybox variant
#DEPMOD=$(BUSYBOX_DIR)/examples/depmod.pl
#endif

$(DL_DIR)/$(LINUX_SOURCE):
	-mkdir -p $(DL_DIR)
	$(WGET) -P $(DL_DIR) $(LINUX_SITE)/$(LINUX_SOURCE)

$(LINUX_DIR)/.unpacked: $(DL_DIR)/$(LINUX_SOURCE)
	-mkdir -p $(TOOL_BUILD_DIR)
	-mkdir -p $(BUILD_DIR)
	bzcat $(DL_DIR)/$(LINUX_SOURCE) | tar -C $(BUILD_DIR) -xf -

ifneq ($(DOWNLOAD_LINUX_VERSION),$(LINUX_VERSION))
	# Rename the dir from the downloaded version to the AFTER patch version	
	-rm -rf $(BUILD_DIR)/linux-$(LINUX_VERSION)
	mv -f $(BUILD_DIR)/linux-$(DOWNLOAD_LINUX_VERSION) $(BUILD_DIR)/linux-$(LINUX_VERSION)
endif

	#Apply the patches
	(cd $(LINUX_DIR); QUILT_PATCHES=$(LINUX_PATCH_DIR) $(QUILT) push -a)

	touch $(LINUX_DIR)/.unpacked
	touch $(LINUX_DIR)/.patched

$(LINUX_KCONFIG):
	@if [ ! -f "$(LINUX_KCONFIG)" ] ; then \
		echo ""; \
		echo "You should create a .config for your kernel"; \
		echo "and install it as $(LINUX_KCONFIG)"; \
		echo ""; \
		sleep 5; \
	fi;

kernel-menuconfig: $(LINUX_DIR)/include/linux/autoconf.h $(BUILD_DIR)/linux/include/linux/autoconf.h
	$(MAKE) $(JLEVEL) -C $(LINUX_DIR) CROSS_COMPILE=$(KERNEL_CROSS) ARCH=$(LINUX_KARCH) menuconfig

kernel-gconfig: $(LINUX_DIR)/include/linux/autoconf.h $(BUILD_DIR)/linux/include/linux/autoconf.h
	$(MAKE) $(JLEVEL) -C $(LINUX_DIR) CROSS_COMPILE=$(KERNEL_CROSS) ARCH=$(LINUX_KARCH) gconfig

kernel-reconf: $(LINUX_DIR)/.config
	@( read -p "Do you want to use the serial port for console? [y/n] (If unsure, say no) " -n 1 yn ; \
	   echo ; \
	  if [[ "$$yn" == "y" ]] ; \
	  then sed -e 's#^CONFIG_CMDLINE=.*#CONFIG_CMDLINE=\"console=ttyS0,115200n8 root=/dev/ram0 rw rootfstype=cramfs initrd=0x01000000,0x8B000 mem=32M@00000000\"#' \
		   -e 's,# CONFIG_SERIAL_8250_CONSOLE is not set,CONFIG_SERIAL_8250_CONSOLE=y,' -i $(LINUX_DIR)/.config; \
	  else \
	   sed -e 's#^CONFIG_CMDLINE=.*#CONFIG_CMDLINE=\"root=/dev/ram0 rw rootfstype=cramfs initrd=0x01000000,0x8B000 mem=32M@00000000\"#' \
	       -e 's,CONFIG_SERIAL_8250_CONSOLE=y,# CONFIG_SERIAL_8250_CONSOLE is not set,' -i $(LINUX_DIR)/.config; \
	 fi ; )

$(LINUX_DIR)/.config $(LINUX_DIR)/include/linux/autoconf.h $(BUILD_DIR)/linux/include/linux/autoconf.h:  $(LINUX_DIR)/.unpacked $(LINUX_KCONFIG) 
	-cp $(LINUX_KCONFIG) $(LINUX_DIR)/.config
	$(MAKE) $(JLEVEL) -C $(LINUX_DIR) CROSS_COMPILE=$(KERNEL_CROSS) ARCH=$(LINUX_KARCH) silentoldconfig
	$(MAKE) $(JLEVEL) -C $(LINUX_DIR) CROSS_COMPILE=$(KERNEL_CROSS) ARCH=$(LINUX_KARCH) archprepare

$(LINUX_DIR)/$(LINUX_BINLOC): $(LINUX_DIR)/.config $(LINUX_DIR)/include/linux/autoconf.h
	$(MAKE) $(JLEVEL) -C $(LINUX_DIR) CROSS_COMPILE=$(KERNEL_CROSS) ARCH=$(LINUX_KARCH)

$(LINUX_KERNEL): $(LINUX_DIR)/$(LINUX_BINLOC) $(BUSYBOX_DIR)/.configured
	mkdir -p $(TARGET_DIR)/boot

	$(MAKE) $(JLEVEL) -C $(LINUX_DIR) DEPMOD="$(DEPMOD_TMP)" CROSS_COMPILE=$(KERNEL_CROSS) ARCH=$(LINUX_KARCH) INSTALL_MOD_PATH=$(TARGET_DIR) modules_install
	find $(TARGET_DIR)/lib/modules -type l -name build -exec rm {} \;
	find $(TARGET_DIR)/lib/modules -type l -name source -exec rm {} \;
	#cp -fpR --no-dereference $(LINUX_DIR)/$(LINUX_BINLOC) $(LINUX_KERNEL)
	touch $(LINUX_KERNEL)

$(TARGET_DIR)/lib/modules/latest: $(LINUX_KERNEL)
	( cd $(TARGET_DIR)/lib/modules; \
	  ln -sf $(DOWNLOAD_LINUX_VERSION) latest ; )

kernel-headers: $(LINUX_DIR)/include/linux/autoconf.h #$(STAGING_DIR)/include/linux/version.h 
	$(MAKE) -C $(LINUX_DIR) ARCH=$(LINUX_KARCH) HOSTCC="$(HOSTCC)" HOSTCFLAGS="$(HOSTCFLAGS)" \
		HOSTCXX="$(HOSTCXX)" INSTALL_HDR_PATH=$(LINUX_HEADERS_DIR) headers_install
	$(MAKE) -C $(LINUX_DIR) ARCH=$(LINUX_KARCH) HOSTCC="$(HOSTCC)" HOSTCFLAGS="$(HOSTCFLAGS)" \
		HOSTCXX="$(HOSTCXX)" INSTALL_HDR_PATH=$(STAGING_DIR)/$(REAL_GNU_TARGET_NAME) headers_install

linux: $(LINUX_KERNEL) $(TARGET_DIR)/lib/modules/latest

linux-source: $(DL_DIR)/$(LINUX_SOURCE)

# This has been renamed so we do _NOT_ by default run this on 'make clean'
linuxclean: clean
	rm -f $(LINUX_KERNEL) $(LINUX_DIR)/$(LINUX_UIMAGE)
	-$(MAKE) $(JLEVEL) -C $(LINUX_DIR) CROSS_COMPILE=$(KERNEL_CROSS) ARCH=$(LINUX_KARCH) clean

linux-dirclean:
	rm -rf $(LINUX_DIR)

endif