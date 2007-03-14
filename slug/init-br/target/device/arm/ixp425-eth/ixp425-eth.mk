##############################
# IXP425 Ethernet Driver
##############################

IXP425_ETH_SOURCE := ixp400linuxethernetdriverpatch-1_2.zip
IXP425_ETH_URL := ftp://download.intel.com/design/network/swsup/$(IXP425_ETH_SOURCE)
IXP425_ETH_PATCH_DIR := $(BASE_DIR)/target/device/arm/ixp425-eth/patches
IXP425_ETH_DIR := $(BUILD_DIR)/ixp425-eth
IXP425_ETH_KMOD := $(IXP425_ETH_DIR)/ixp425_eth.ko
IXP425_ETH_KMOD_INSTALL := $(TARGET_DIR)/lib/modules/$(DOWNLOAD_LINUX_VERSION)/extra/ixp400_eth.ko

$(DL_DIR)/$(IXP425_ETH_SOURCE):
	wget -P $(DL_DIR) $(IXP425_ETH_URL)

$(IXP425_ETH_DIR)/.unpacked: $(DL_DIR)/$(IXP425_ETH_SOURCE)
	unzip -q -d $(IXP425_ETH_DIR) $(DL_DIR)/$(IXP425_ETH_SOURCE)
	touch $(IXP425_ETH_DIR)/.unpacked

$(IXP425_ETH_DIR)/.patched: $(IXP425_ETH_DIR)/.unpacked
	(cd $(IXP425_ETH_DIR); QUILT_PATCHES=$(IXP425_ETH_PATCH_DIR) $(QUILT) push -a)
	touch $(IXP425_ETH_DIR)/.patched

$(IXP425_ETH_DIR)/Module.symvers: $(IXP425_ETH_DIR)/.unpacked
	cp $(IXP425_OSAL_MODULE_SYMVER) $(IXP425_ETH_DIR)/Module.symvers

$(IXP425_ETH_KMOD): $(IXP425_ETH_DIR)/.patched $(IXP425_ETH_DIR)/Module.symvers
	$(MAKE) -C $(IXP425_ETH_DIR) ARCH=$(LINUX_KARCH) LINUX_CROSS_COMPILE=$(KERNEL_CROSS) IX_TARGET=linuxbe IXP4XX_CSR_DIR=$(IXP425_OSAL_HEADER_DIR) linuxbe_KERNEL_DIR=$(LINUX_DIR) IX_XSCALE_SW=$(IXP425_OSAL_DIR)/ixp400_xscale_sw LINUX_CROSS_COMPILE=$(KERNEL_CROSS) 

$(IXP425_ETH_KMOD_INSTALL): $(IXP425_ETH_KMOD)
	$(MAKE) -C $(LINUX_DIR) CROSS_COMPILE=$(KERNEL_CROSS) ARCH=$(LINUX_KARCH) INSTALL_MOD_PATH=$(TARGET_DIR) M=$(IXP425_ETH_DIR) modules_install
	$(DEPMOD) -b $(TARGET_DIR)/lib/modules/$(DOWNLOAD_LINUX_VERSION) -F $(LINUX_DIR)/System.map

ixp425_eth: ixp425_osal $(IXP425_ETH_KMOD_INSTALL)




