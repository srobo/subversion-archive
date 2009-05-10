###############################################################################
#
# ixp4xx-microcode
#
################################################################################

IXP4XX_SOURCE=IPL_ixp400NpeLibraryWithCrypto-2_4.zip
IXP4XX_SITE=http://downloads.openwrt.org/sources
IXP4XX_DIR=$(BUILD_DIR)/ixp400_xscale_sw

# PKG_NAME:=ixp4xx-microcode
# PKG_VERSION:=2.4
# PKG_RELEASE:=2

# PKG_SOURCE:=IPL_ixp400NpeLibraryWithCrypto-2_4.zip
# PKG_SOURCE_URL:=http://downloads.openwrt.org/sources
# PKG_MD5SUM:=dd5f6482e625ecb334469958bcd54b37

$(DL_DIR)/$(IXP4XX_SOURCE):
	$(WGET) -P $(DL_DIR) $(IXP4XX_SITE)/$(IXP4XX_SOURCE)

$(IXP4XX_DIR)/.source: $(DL_DIR)/$(IXP4XX_SOURCE)
	unzip -d $(BUILD_DIR) $(DL_DIR)/$(IXP4XX_SOURCE)
	mv $(IXP4XX_DIR)/src/npeDl/*.c $(IXP4XX_DIR)/
	touch $(IXP4XX_DIR)/.source;

$(IXP4XX_DIR)/.prepared: $(IXP4XX_DIR)/.source
	cp package/ixp4xx-microcode/src/* $(IXP4XX_DIR)
	touch $(IXP4XX_DIR)/.prepared

$(IXP4XX_DIR)/NPE-{A,B,C}: $(IXP4XX_DIR)/.prepared
	( cd $(IXP4XX_DIR); \
		$(HOSTCC) -Wall -I$(STAGING_DIR)/include IxNpeMicrocode.c -o IxNpeMicrocode; \
		./IxNpeMicrocode \
	)

$(TARGET_DIR)/lib/firmware/NPE-{A,B,C}: $(IXP4XX_DIR)/NPE-{A,B,C}
	mkdir -p $(TARGET_DIR)/lib/firmware/
	cp $(IXP4XX_DIR)/NPE-{A,B,C} $(TARGET_DIR)/lib/firmware/

ixp4xx-microcode: uclibc $(TARGET_DIR)/lib/firmware/NPE-{A,B,C}

ixp4xx-source: $(DL_DIR)/$(IXP4XX_SOURCE)

#############################################################
#
# Toplevel Makefile options
#
#############################################################
ifeq ($(strip $(BR2_PACKAGE_IXP4XX_MICROCODE)),y)
TARGETS+=ixp4xx-microcode
endif

