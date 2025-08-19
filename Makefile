ISO_NAME   = ChadOS.iso
BUILD_DIR  = build
SRC_DIR    = src
KERNEL_DIR = kernel

CFLAGS  = -ffreestanding -Wall -Wextra -O0 -m64 -fno-pie -fno-stack-protector -nostdlib -nostartfiles
LDFLAGS = -n -T $(KERNEL_DIR)/linker.ld

# Recursive C files in src/
SRC_C_FILES    := $(shell find $(SRC_DIR) -type f -name '*.c')
SRC_OBJ_FILES  := $(patsubst $(SRC_DIR)/%,$(BUILD_DIR)/%,$(SRC_C_FILES:.c=.o))

# Kernel objects
KERNEL_OBJ_FILES := $(BUILD_DIR)/kernel.o $(BUILD_DIR)/boot.o

# Every object to link
OBJ_FILES := $(SRC_OBJ_FILES) $(KERNEL_OBJ_FILES)

# Default
.PHONY: all
all: iso

# Kernel rules
$(BUILD_DIR)/kernel.o: $(KERNEL_DIR)/kernel.c
	@mkdir -p $(dir $@)
	x86_64-elf-gcc $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/boot.o: $(KERNEL_DIR)/boot.s
	@mkdir -p $(dir $@)
	nasm -f elf64 $< -o $@

# src/ -> build/
# Create target directory (dir $@) before compiling
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	x86_64-elf-gcc $(CFLAGS) -c $< -o $@

# --- Linking ---
$(BUILD_DIR)/kernel.bin: $(OBJ_FILES)
	@mkdir -p $(dir $@)
	x86_64-elf-ld $(LDFLAGS) -o $@ $^

# --- ISO ---
.PHONY: iso
iso: $(BUILD_DIR)/kernel.bin
	@mkdir -p isodir/boot/grub
	cp $< isodir/boot/kernel.bin
	cp boot/grub/grub.cfg isodir/boot/grub/grub.cfg
	grub-mkrescue -o $(ISO_NAME) isodir

# --- Run ---
.PHONY: run
run:
	qemu-system-x86_64 -cdrom $(ISO_NAME) -serial stdio

# --- Clean ---
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) isodir $(ISO_NAME)

.PHONY: full
full: clean iso run
