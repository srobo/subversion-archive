##############################
# Generates an initrd
##############################

INITRD_DIR:=$(BUILD_DIR)/initrd
INITRD_TARGET:=initrd.cramfs

$(INITRD_DIR): 
	cp -r $(INITRD_SKELETON) $(INITRD_DIR)

$(INITRD_TARGET): $(INITRD_DIR) busybox-initrd
	-@find $(INITRD_DIR) -type f -perm +111 | xargs $(STRIP) 2>/dev/null || true;
	@rm -rf $(INITRD_DIR)/usr/man
	@rm -rf $(INITRD_DIR)/usr/info
	-/sbin/ldconfig -r $(INITRD_DIR) 2>/dev/null

	# Use fakeroot to pretend all target binaries are owned by root
	rm -f $(STAGING_DIR)/_initrd_fakeroot.$(notdir $(INITRD_TARGET))
	touch $(STAGING_DIR)/.initrd_fakeroot.00000
	cat $(STAGING_DIR)/.initrd_fakeroot* > $(STAGING_DIR)/_initrd_fakeroot.$(notdir $(INITRD_TARGET))
	echo "chown -R root:root $(TARGET_DIR)" >> $(STAGING_DIR)/_initrd_fakeroot.$(notdir $(INITRD_TARGET))

	# Use fakeroot to pretend to create all needed device nodes
	echo "$(STAGING_DIR)/bin/makedevs -d $(TARGET_DEVICE_TABLE) $(INITRD_DIR)" \
		>> $(STAGING_DIR)/_initrd_fakeroot.$(notdir $(INITRD_TARGET))

	# Use fakeroot so mkcramfs believes the previous fakery
	echo "$(CRAMFS_DIR)/mkcramfs -q $(CRAMFS_ENDIANNESS) " \
		"$(INITRD_DIR) $(INITRD_TARGET)" >> $(STAGING_DIR)/_initrd_fakeroot.$(notdir $(INITRD_TARGET))
	chmod a+x $(STAGING_DIR)/_initrd_fakeroot.$(notdir $(INITRD_TARGET))
	$(STAGING_DIR)/usr/bin/fakeroot -- $(STAGING_DIR)/_initrd_fakeroot.$(notdir $(INITRD_TARGET))
	-@rm -f $(STAGING_DIR)/_initrd_fakeroot.$(notdir $(INITRD_TARGET))


initrd: cramfs $(INITRD_TARGET)

ifeq ($(strip $(BR2_TARGET_INITRD)),y)
TARGETS+=initrd
endif