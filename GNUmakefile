# Nuke built-in rules and variables.
override MAKEFLAGS += -rR
override IMAGE_NAME := tellurium
override LIMINE_DIR := ../limine
override KERNEL_DIR := kernel
override LIBGUI_DIR := libGUI
override APPS_DIR := applications
override IMG_DIR := imgBin

.PHONY: all
all: $(IMAGE_NAME).iso

.PHONY: all-hdd
all-hdd: $(IMAGE_NAME).hdd

.PHONY: run
run: $(IMAGE_NAME).iso
	qemu-system-x86_64 -M q35 -m 2G -cdrom $(IMAGE_NAME).iso -boot d

.PHONY: $(KERNEL_DIR) $(APPS_DIR)
kernel:
	$(MAKE) -C $(LIBGUI_DIR)
	$(MAKE) -C $(KERNEL_DIR)
	$(MAKE) -C $(APPS_DIR)
	$(MAKE) -C $(IMG_DIR)

$(IMAGE_NAME).iso: $(KERNEL_DIR)
	rm -rf iso_root
	mkdir -p iso_root
	cp $(KERNEL_DIR)/kernel.elf \
		$(APPS_DIR)/userspace.tar \
		$(IMG_DIR)/images.tar \
		limine.cfg \
        $(LIMINE_DIR)/limine.sys \
        $(LIMINE_DIR)/limine-cd.bin \
        $(LIMINE_DIR)/limine-cd-efi.bin \
        iso_root/
	xorriso -as mkisofs -b limine-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot limine-cd-efi.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		iso_root -o $(IMAGE_NAME).iso
	$(LIMINE_DIR)/limine-deploy $(IMAGE_NAME).iso
	rm -rf iso_root

.PHONY: clean
clean:
	rm -rf iso_root $(IMAGE_NAME).iso
	rm -f *.log
	$(MAKE) -C $(KERNEL_DIR) clean
	$(MAKE) -C $(APPS_DIR) clean
	$(MAKE) -C $(LIBGUI_DIR) clean
	$(MAKE) -C $(IMG_DIR) clean