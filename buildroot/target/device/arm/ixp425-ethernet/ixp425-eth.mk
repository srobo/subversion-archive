##############################
# ixp425 ethernet driver
##############################

IXP425_ETH_SOURCE:=
IXP425_DIR := $(BUILD_DIR)/ixp425-eth
IXP425_PATCH_DIR := $(BASE_DIR)/target/device/arm/ixp425-ethernet/patches

IXP425_LIBOSAL := $(IXP425_DIR)/ixp_osal/lib/ixp425/linux/linuxbe/libosal.a
IXP425_KMOD := $(IXP425_DIR)/ixp400_xscale_sw/lib/linuxbe/ixp400.ko 
IXP425_KMOD_LOC := $(TARGET_DIR)/lib/modules/$(DOWNLOAD_LINUX_VERSION)/extra/ixp400.ko
IXP425_COMPONENTS := "qmgr npeMh npeDl ethAcc ethDB ethMii featureCtrl osServices oslinux"


$(IXP425_DIR)/.source:
	#unzip the files
	mkdir -p $(IXP425_DIR)
	unzip -q -d $(IXP425_DIR) $(DL_DIR)/BSD_ixp400AccessLibrary-2_1.zip
	unzip -q -d $(IXP425_DIR) $(DL_DIR)/IPL_ixp400NpeLibrary-2_1.zip

	touch $(IXP425_DIR)/.source

$(IXP425_DIR)/.patched: $(IXP425_DIR)/.source
	(cd $(IXP425_DIR); QUILT_PATCHES=$(IXP425_PATCH_DIR) $(QUILT) push -a)
	touch $(IXP425_DIR)/.patched

$(IXP425_KMOD): $(IXP425_DIR)/.patched 
	$(MAKE) -C $(IXP425_DIR)/ixp_osal IX_TARGET=linuxbe CC=$(TARGET_CC) LD=$(KERNEL_CROSS)ld AR=$(KERNEL_CROSS)ar IX_DEVICE=ixp425 LINUX_SRC=$(LINUX_DIR) LINUX_CROSS_COMPILE=$(KERNEL_CROSS) libosal module
	$(MAKE) -C $(IXP425_DIR)/ixp400_xscale_sw AR=$(KERNEL_CROSS)ar IX_TARGET=linuxbe IX_XSCALE_SW=$(IXP425_DIR)/ixp400_xscale_sw linuxbe_COMPONENTS=$(IXP425_COMPONENTS) linuxbe_CODELETS_COMPONENTS="" IX_DEVICE=ixp425 IX_INCLUDE_MICROCODE=1 IX_UTOPIAMODE=0 IX_MPHYSINGLEPORT=1 LINUX_SRC=$(LINUX_DIR) LINUX_CROSS_COMPILE=$(KERNEL_CROSS) OSAL_DIR=$(IXP425_DIR)/ixp_osal OSAL_IMAGE=$(IXP425_DIR)/ixp_osal/lib/ixp425/linux/linuxbe/libosal.a OSAL_MODULE=$(IXP425_DIR)/ixp_osal/lib/ixp425/linux/linuxbe/ixp_osal.o lib/linuxbe/ixp400.o

$(IXP425_KMOD_LOC): $(IXP425_KMOD)
	$(MAKE) -C $(LINUX_DIR) CROSS_COMPILE=$(KERNEL_CROSS) ARCH=$(LINUX_KARCH) INSTALL_MOD_PATH=$(TARGET_DIR) M=$(IXP425_DIR)/ixp400_xscale_sw/lib/linuxbe modules_install
	$(DEPMOD) -b $(TARGET_DIR)/lib/modules/$(DOWNLOAD_LINUX_VERSION) -F $(LINUX_DIR)/System.map

ixp425_eth: linux $(IXP425_KMOD_LOC)

ixp425_eth-unpack: $(IXP425_DIR)/.source
