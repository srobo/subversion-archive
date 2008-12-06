##############################
# Generates an initrd
##############################

INITRD_TARGET:=initrd.cramfs

INITRD_SKELETON := target/device/arm/nslu2/initrd_skeleton

$(INITRD_DIR): 
	mkdir -p $(INITRD_DIR)

$(INITRD_DIR)/.full: $(INITRD_DIR)
	cp -r $(INITRD_SKELETON)/* $(INITRD_DIR)
	touch $(INITRD_DIR)/.full

initrd-stuff: $(INITRD_DIR)/.full gcc-target-libs-initrd busybox-initrd uclibc-initrd

$(INITRD_TARGET): initrd-stuff
	-@find $(INITRD_DIR) -type f -perm +111 | xargs $(STRIP) 2>/dev/null || true;
	@rm -rf $(INITRD_DIR)/usr/man
	@rm -rf $(INITRD_DIR)/usr/info
	-/sbin/ldconfig -r $(INITRD_DIR) 2>/dev/null

	rm -f $(STAGING_DIR)/_initrd_fakeroot.$(notdir $(INITRD_TARGET))

	# Use fakeroot to pretend all target binaries are owned by root
	echo "chown -R root:root $(INITRD_DIR)" >> $(STAGING_DIR)/_initrd_fakeroot.$(notdir $(INITRD_TARGET))

	# Use fakeroot to pretend to create all needed device nodes
	echo "$(STAGING_DIR)/bin/makedevs -d $(TARGET_DEVICE_TABLE) $(INITRD_DIR)" \
		>> $(STAGING_DIR)/_initrd_fakeroot.$(notdir $(INITRD_TARGET))

	# Use fakeroot so mkcramfs believes the previous fakery
	echo "$(CRAMFS_DIR)/mkcramfs -q $(CRAMFS_ENDIANNESS) " \
		"$(INITRD_DIR) $(INITRD_TARGET)" >> $(STAGING_DIR)/_initrd_fakeroot.$(notdir $(INITRD_TARGET))

	chmod a+x $(STAGING_DIR)/_initrd_fakeroot.$(notdir $(INITRD_TARGET))
	$(STAGING_DIR)/usr/bin/fakeroot -- $(STAGING_DIR)/_initrd_fakeroot.$(notdir $(INITRD_TARGET))
	-@rm -f $(STAGING_DIR)/_initrd_fakeroot.$(notdir $(INITRD_TARGET))


initrd: host-fakeroot cramfs $(INITRD_TARGET)

ifeq ($(strip $(BR2_TARGET_INITRD)),y)
TARGETS+=initrd
endif