# Toolchain
CC      := arm-none-eabi-gcc
OBJCOPY := arm-none-eabi-objcopy

# Flags
CPU     := arm1176jzf-s
CFLAGS  := -mcpu=$(CPU) -fpic -ffreestanding -std=gnu99 -O2 -Wall -Wextra
ASFLAGS := -mcpu=$(CPU) -fpic -ffreestanding
LDFLAGS := -T linker.ld -ffreestanding -O2 -nostdlib -lgcc

# Directories
SRC_DIR := src
OUT_DIR := out

# Auto-discover sources
C_SRCS  := $(shell find $(SRC_DIR) -name '*.c')
S_SRCS  := $(shell find $(SRC_DIR) -name 'bootpi2.S')

# Derive object files (mirror src layout into out/)
C_OBJS  := $(patsubst $(SRC_DIR)/%.c,  $(OUT_DIR)/%.o, $(C_SRCS))
S_OBJS  := $(patsubst $(SRC_DIR)/%.S,  $(OUT_DIR)/%.o, $(S_SRCS))
OBJS    := $(S_OBJS) $(C_OBJS)   # boot .S first so entry point lands at front

TARGET_ELF := $(OUT_DIR)/kernel.elf
TARGET_IMG := $(OUT_DIR)/kernel.img

# ── Default target ────────────────────────────────────────────────────────────
.PHONY: all
all: $(TARGET_IMG)

# ── Image ─────────────────────────────────────────────────────────────────────
$(TARGET_IMG): $(TARGET_ELF)
	$(OBJCOPY) $< -O binary $@

# ── Link ──────────────────────────────────────────────────────────────────────
$(TARGET_ELF): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

# ── Compile C ─────────────────────────────────────────────────────────────────
$(OUT_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# ── Assemble S ────────────────────────────────────────────────────────────────
$(OUT_DIR)/%.o: $(SRC_DIR)/%.S
	@mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) -c $< -o $@

# ── Run in QEMU ───────────────────────────────────────────────────────────────
.PHONY: run
run: $(TARGET_IMG)
	qemu-system-arm -M raspi2b -serial stdio \
	    -device loader,file=$(TARGET_IMG),addr=0x8000,cpu-num=0

# ── Debug (GDB stub) ──────────────────────────────────────────────────────────
.PHONY: debug
debug: $(TARGET_IMG)
	qemu-system-arm -M raspi2b -serial stdio \
	    -device loader,file=$(TARGET_IMG),addr=0x8000,cpu-num=0 \
	    -s -S

# ── Clean ─────────────────────────────────────────────────────────────────────
.PHONY: clean
clean:
	rm -rf $(OUT_DIR)/*