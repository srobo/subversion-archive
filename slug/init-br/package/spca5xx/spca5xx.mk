##############################
#
# spca5xx
#
##############################

SPCA5XX_VERSION:=20061216
SPCA5XX_SOURCE:=gspcav1-$(SPCA5XX_VERSION).tar.gz
SPCA5XX_SITE := http://mxhaard.free.fr/spca50x/Download/
SPCA5XX_DIR := $(BUILD_DIR)/gspcav1-$(SPCA5XX_VERSION)
SPCA5XX_MODLOC := $(TARGET_DIR)/lib/modules/$(DOWNLOAD_LINUX_VERSION)/extra/gspca.ko

$(DL_DIR)/$(SPCA5XX_SOURCE):
	$(WGET) -P $(DL_DIR) $(SPCA5XX_SITE)/$(SPCA5XX_SOURCE)

$(SPCA5XX_DIR)/.source: $(DL_DIR)/$(SPCA5XX_SOURCE)
	$(ZCAT) $(DL_DIR)/$(SPCA5XX_SOURCE) | tar -C $(BUILD_DIR) $(TAR_OPTIONS) -
	touch $(SPCA5XX_DIR)/.source

$(SPCA5XX_DIR)/gspca.ko: $(SPCA5XX_DIR)/.source
	$(MAKE) -C $(LINUX_DIR) CROSS_COMPILE=$(KERNEL_CROSS) ARCH=$(LINUX_KARCH) M=$(SPCA5XX_DIR) modules

$(SPCA5XX_MODLOC): $(SPCA5XX_DIR)/gspca.ko
	echo DEPMOD=$(DEPMOD)
	$(MAKE) -C $(LINUX_DIR) CROSS_COMPILE=$(KERNEL_CROSS) ARCH=$(LINUX_KARCH) M=$(SPCA5XX_DIR) INSTALL_MOD_PATH=$(TARGET_DIR) modules_install
	$(DEPMOD) -b $(TARGET_DIR)/lib/modules/$(DOWNLOAD_LINUX_VERSION) -F $(LINUX_DIR)/System.map

#we need the kernel in order to build
spca5xx: linux $(SPCA5XX_MODLOC)

spca5xx-source: $(DL_DIR)/$(SPCA5XX_SOURCE)

ifeq ($(strip $(BR2_PACKAGE_SPCA5XX)),y)
TARGETS+=spca5xx
endif

