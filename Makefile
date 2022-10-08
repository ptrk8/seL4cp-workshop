SDK_DIR = "$(PWD)/sdk"
WORKSHOP_DIR = "$(PWD)/workshop"

# Temporary build directory for hello world example.
BUILD_DIR_CHECK = "hello_world_build"

.PHONY: clean
clean:
	rm -rf \
		$(BUILD_DIR_CHECK)

# Checks that the environment has been set up correctly.
.PHONY: check
check:
	# Creates a directory for building into.
	mkdir -p $(BUILD_DIR_CHECK)
	# Create bootable image in build directory.
	make \
		-C $(SDK_DIR)/board/qemu_arm_virt/example/hello/ \
		BUILD_DIR=$(PWD)/$(BUILD_DIR_CHECK) \
		SEL4CP_SDK=$(PWD)/$(SDK_DIR) \
		SEL4CP_BOARD=qemu_arm_virt \
		SEL4CP_CONFIG=debug
	# Run the bootable image in QEMU.
	qemu-system-aarch64 \
		-machine virt \
		-cpu cortex-a53 \
		-serial mon:stdio \
		-device loader,file=hello_world_build/loader.img,addr=0x70000000,cpu-num=0 \
		-m size=1G \
		-display none

# Run the `part1` make command in the workshop/ directory.
.PHONY: build-part1
build-part1:
	$(MAKE) -C $(WORKSHOP_DIR) \
		part1 \
		SEL4CP_SDK=$(SDK_DIR)

# Run `part1`.
.PHONY: run-part1
run-part1: build-part1 run

# Run the `part2` make command in the workshop/ directory.
.PHONY: build-part2
build-part2:
	$(MAKE) -C $(WORKSHOP_DIR) \
		part2 \
		SEL4CP_SDK=$(SDK_DIR)

# Run `part2`.
.PHONY: run-part2
run-part2: build-part2 run

# Run the `part3` make command in the workshop/ directory.
.PHONY: build-part3
build-part3:
	$(MAKE) -C $(WORKSHOP_DIR) \
		part3 \
		SEL4CP_SDK=$(SDK_DIR)

# Run `part3`.
.PHONY: run-part3
run-part3: build-part3 run

# Runs built seL4cp image.
.PHONY: run
run:
	$(MAKE) -C $(WORKSHOP_DIR) \
		run



