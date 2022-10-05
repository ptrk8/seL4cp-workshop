# If you would like to choose a different path to the SDK, you can pass it as an
# argument.
ifdef SEL4CP_SDK
	SEL4CP_SDK := $(SEL4CP_SDK)
endif

SEL4CP_SDK := ../sel4cp_workshop_sdk

# In case the default compiler triple doesn't work for you or your package manager
# only has aarch64-none-elf or something, you can specifiy the toolchain.
ifdef TOOLCHAIN
	TOOLCHAIN := $(TOOLCHAIN)
endif

SEL4CP_BOARD := qemu_arm_virt
SEL4CP_CONFIG := debug
BUILD_DIR := build

TOOLCHAIN := aarch64-linux-gnu

CPU := cortex-a53

CC := $(TOOLCHAIN)-gcc
LD := $(TOOLCHAIN)-ld
AS := $(TOOLCHAIN)-as
SEL4CP_TOOL ?= $(SEL4CP_SDK)/bin/sel4cp

SERIAL_SERVER_OBJS := serial_server.o
CLIENT_OBJS := client.o
WORDLE_SERVER_OBJS := wordle_server.o

BOARD_DIR := $(SEL4CP_SDK)/board/$(SEL4CP_BOARD)/$(SEL4CP_CONFIG)

IMAGES_PART_1 := serial_server.elf
IMAGES_PART_2 := serial_server.elf client.elf
IMAGES_PART_3 := serial_server.elf client.elf wordle_server.elf
CFLAGS := -mcpu=$(CPU) -mstrict-align -nostdlib -ffreestanding -g3 -O3 -Wall -Wno-array-bounds -Wno-unused-variable -Wno-unused-function -Werror -I$(BOARD_DIR)/include -Iinclude
LDFLAGS := -L$(BOARD_DIR)/lib
LIBS := -lsel4cp -Tsel4cp.ld

IMAGE_FILE_PART_1 = $(BUILD_DIR)/wordle_part_one.img
IMAGE_FILE_PART_2 = $(BUILD_DIR)/wordle_part_two.img
IMAGE_FILE_PART_3 = $(BUILD_DIR)/wordle_part_three.img
IMAGE_FILE = $(BUILD_DIR)/wordle.img
REPORT_FILE = $(BUILD_DIR)/report.txt

all: directories $(IMAGE_FILE)

directories:
	$(info $(shell mkdir -p $(BUILD_DIR)))

run: $(IMAGE_FILE)
	qemu-system-aarch64 -machine virt -cpu $(CPU) -serial mon:stdio -device loader,file=$(IMAGE_FILE),addr=0x70000000,cpu-num=0 -m size=1G -nographic

part1: directories $(BUILD_DIR)/serial_server.elf $(IMAGE_FILE_PART_1)
part2: directories $(BUILD_DIR)/client.elf $(IMAGE_FILE_PART_2)
part3: directories $(BUILD_DIR)/wordle_server.elf $(IMAGE_FILE_PART_3)

$(BUILD_DIR)/%.o: %.c Makefile
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile
	$(AS) -g3 -mcpu=$(CPU) $< -o $@

$(BUILD_DIR)/serial_server.elf: $(addprefix $(BUILD_DIR)/, $(SERIAL_SERVER_OBJS))
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@

$(BUILD_DIR)/client.elf: $(addprefix $(BUILD_DIR)/, $(CLIENT_OBJS))
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@

$(BUILD_DIR)/wordle_server.elf: $(addprefix $(BUILD_DIR)/, $(WORDLE_SERVER_OBJS))
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@

$(IMAGE_FILE_PART_1): $(addprefix $(BUILD_DIR)/, $(IMAGES_PART_1)) wordle.system
	$(SEL4CP_TOOL) wordle.system --search-path $(BUILD_DIR) --board $(SEL4CP_BOARD) --config $(SEL4CP_CONFIG) -o $(IMAGE_FILE) -r $(REPORT_FILE)

$(IMAGE_FILE_PART_2): $(addprefix $(BUILD_DIR)/, $(IMAGES_PART_2)) wordle.system
	$(SEL4CP_TOOL) wordle.system --search-path $(BUILD_DIR) --board $(SEL4CP_BOARD) --config $(SEL4CP_CONFIG) -o $(IMAGE_FILE) -r $(REPORT_FILE)

$(IMAGE_FILE_PART_3): $(addprefix $(BUILD_DIR)/, $(IMAGES_PART_3)) wordle.system
	$(SEL4CP_TOOL) wordle.system --search-path $(BUILD_DIR) --board $(SEL4CP_BOARD) --config $(SEL4CP_CONFIG) -o $(IMAGE_FILE) -r $(REPORT_FILE)
