##############################
#
# spcaview
#
##############################

SPCAVIEW_VERSION:=20061208
SPCAVIEW_SOURCE:=spcaview-$(SPCAVIEW_VERSION).tar.gz
SPCAVIEW_SITE := http://mxhaard.free.fr/spca50x/Download
SPCAVIEW_DIR := $(BUILD_DIR)/spcaview-$(SPCAVIEW_VERSION)
SPCAVIEW_BINARIES := spcacat spcaview spcaserv
SPCAVIEW_BINLOCS := $(addprefix $(SPCAVIEW_DIR)/,$(SPCAVIEW_BINARIES))
SPCAVIEW_BIN_INSTALLED := $(addprefix $(TARGET_DIR)/bin/,$(SPCAVIEW_BINARIES))
SPCAVIEW_PATCH_DIR := $(BASE_DIR)/package/spcaview/patches

$(DL_DIR)/$(SPCAVIEW_SOURCE):
	$(WGET) -P $(DL_DIR) $(SPCAVIEW_SITE)/$(SPCAVIEW_SOURCE)

$(SPCAVIEW_DIR)/.source: $(DL_DIR)/$(SPCAVIEW_SOURCE)
	$(ZCAT) $(DL_DIR)/$(SPCAVIEW_SOURCE) | tar -C $(BUILD_DIR) $(TAR_OPTIONS) -
	touch $(SPCAVIEW_DIR)/.source

$(SPCAVIEW_DIR)/.patched: $(SPCAVIEW_DIR)/.source
	echo QUILT=$(QUILT)
	(cd $(SPCAVIEW_DIR); QUILT_PATCHES=$(SPCAVIEW_PATCH_DIR) $(QUILT) push -a)
	touch $(SPCAVIEW_DIR)/.patched

$(SPCAVIEW_BINLOCS): $(SPCAVIEW_DIR)/.patched
	$(MAKE) -C $(SPCAVIEW_DIR) CC=$(TARGET_CC) BIN=$(TARGET_DIR)/usr/bin SDL_CONFIG="sh $(SDL_DIR)/sdl-config" 

$(SPCAVIEW_BIN_INSTALLED): $(SPCAVIEW_BINLOCS)
	cp $(SPCAVIEW_BINLOCS) $(TARGET_DIR)/bin

#we need the kernel in order to build
spcaview: sdl $(SPCAVIEW_BIN_INSTALLED)

spcaview-source: $(DL_DIR)/$(SPCAVIEW_SOURCE)

ifeq ($(strip $(BR2_PACKAGE_SPCAVIEW)),y)
TARGETS+=spcaview
endif

