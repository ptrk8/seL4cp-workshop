SDK_DIR = "sdk"

# Temporary build directory for hello world example.
BUILD_DIR_CHECK = "hello_world_build"


.PHONY: clean
clean:
	rm -rf \
		$(BUILD_DIR_CHECK)

# Checks that the environment has been set up correctly.
.PHONY: check
check:
	mkdir $(BUILD_DIR_CHECK)
	make \
		-C $(SDK_DIR)/board/qemu_arm_virt/example/hello/ \
		BUILD_DIR=$(PWD)/$(BUILD_DIR_CHECK) \
		SEL4CP_SDK=$(PWD)/$(SDK_DIR) \
		SEL4CP_BOARD=qemu_arm_virt \
		SEL4CP_CONFIG=debug

.PHONY: build
build: check
	# # Untar the the SDK.
	# tar xvf sdk.tar.gz



