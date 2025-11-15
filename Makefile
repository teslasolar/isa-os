# ════════════════════════════════════════════════════════════════════════
# ISA-OS Build System (Makefile)
# Builds bootloader, kernel, and creates bootable image
# ════════════════════════════════════════════════════════════════════════

# ── Tools ──
ASM = nasm
CC = gcc
LD = ld
DD = dd
QEMU = qemu-system-i386

# ── Directories ──
BOOT_DIR = boot
KERNEL_DIR = kernel
BUILD_DIR = build
USERSPACE_DIR = userspace

# ── Flags ──
ASMFLAGS_BOOT = -f bin
ASMFLAGS_KERNEL = -f elf32
CFLAGS = -m32 -c -ffreestanding -fno-pie -fno-stack-protector \
         -mno-red-zone -mno-mmx -mno-sse -nostdlib -O2 -Wall -Wextra
LDFLAGS = -m elf_i386 -T $(KERNEL_DIR)/linker.ld

# ── Source Files ──
BOOT_ASM = $(BOOT_DIR)/boot.asm
KERNEL_ASM = $(KERNEL_DIR)/core/kernel.asm
KERNEL_C = $(KERNEL_DIR)/core/kernel.c \
           $(KERNEL_DIR)/core/module.c \
           $(KERNEL_DIR)/core/config.c \
           $(KERNEL_DIR)/core/isaf.c \
           $(KERNEL_DIR)/core/audit.c \
           $(KERNEL_DIR)/drivers/modbus.c \
           $(KERNEL_DIR)/drivers/profinet.c \
           $(KERNEL_DIR)/drivers/ethercat.c \
           $(KERNEL_DIR)/drivers/can_bus.c \
           $(KERNEL_DIR)/drivers/spi.c \
           $(KERNEL_DIR)/drivers/i2c.c \
           $(KERNEL_DIR)/services/opc_ua.c \
           $(KERNEL_DIR)/services/mqtt.c

# ── Object Files ──
KERNEL_ASM_OBJ = $(BUILD_DIR)/kernel_asm.o
KERNEL_C_OBJ = $(patsubst $(KERNEL_DIR)/%.c,$(BUILD_DIR)/%.o,$(KERNEL_C))

# ── Output Files ──
BOOT_BIN = $(BUILD_DIR)/boot.bin
KERNEL_BIN = $(BUILD_DIR)/kernel.bin
IMAGE = $(BUILD_DIR)/isa-os.img

# ── Targets ──
.PHONY: all clean run debug dirs

all: dirs $(IMAGE)

dirs:
	@mkdir -p $(BUILD_DIR)/core
	@mkdir -p $(BUILD_DIR)/drivers
	@mkdir -p $(BUILD_DIR)/services

# ── Bootloader ──
$(BOOT_BIN): $(BOOT_ASM)
	@echo "[ASM] Building bootloader..."
	$(ASM) $(ASMFLAGS_BOOT) $< -o $@

# ── Kernel Assembly ──
$(KERNEL_ASM_OBJ): $(KERNEL_ASM)
	@echo "[ASM] Assembling kernel..."
	$(ASM) $(ASMFLAGS_KERNEL) $< -o $@

# ── Kernel C Sources ──
$(BUILD_DIR)/core/%.o: $(KERNEL_DIR)/core/%.c
	@echo "[CC]  Compiling $<..."
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/drivers/%.o: $(KERNEL_DIR)/drivers/%.c
	@echo "[CC]  Compiling $<..."
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/services/%.o: $(KERNEL_DIR)/services/%.c
	@echo "[CC]  Compiling $<..."
	$(CC) $(CFLAGS) $< -o $@

# ── Link Kernel ──
$(KERNEL_BIN): $(KERNEL_ASM_OBJ) $(KERNEL_C_OBJ)
	@echo "[LD]  Linking kernel..."
	$(LD) $(LDFLAGS) $^ -o $@

# ── Create Bootable Image ──
$(IMAGE): $(BOOT_BIN) $(KERNEL_BIN)
	@echo "[IMG] Creating bootable image..."
	$(DD) if=/dev/zero of=$@ bs=512 count=2880 status=none
	$(DD) if=$(BOOT_BIN) of=$@ conv=notrunc status=none
	$(DD) if=$(KERNEL_BIN) of=$@ seek=1 conv=notrunc status=none
	@echo ""
	@echo "════════════════════════════════════════════════════════════════"
	@echo "  ISA-OS build complete!"
	@echo "  Image: $(IMAGE)"
	@echo "  Size:  $$(du -h $(IMAGE) | cut -f1)"
	@echo "════════════════════════════════════════════════════════════════"
	@echo ""
	@echo "Run with: make run"
	@echo "Debug with: make debug"
	@echo ""

# ── Run in QEMU ──
run: $(IMAGE)
	@echo "[RUN] Starting ISA-OS in QEMU..."
	$(QEMU) -drive file=$(IMAGE),format=raw -serial stdio

# ── Debug in QEMU (wait for GDB) ──
debug: $(IMAGE)
	@echo "[DBG] Starting ISA-OS in debug mode (GDB on :1234)..."
	$(QEMU) -drive file=$(IMAGE),format=raw -serial stdio -s -S

# ── Clean Build Artifacts ──
clean:
	@echo "[CLN] Cleaning build artifacts..."
	rm -rf $(BUILD_DIR)

# ── Help ──
help:
	@echo "ISA-OS Build System"
	@echo "==================="
	@echo ""
	@echo "Targets:"
	@echo "  all      - Build bootloader, kernel, and image (default)"
	@echo "  run      - Run ISA-OS in QEMU"
	@echo "  debug    - Run ISA-OS in QEMU with GDB server"
	@echo "  clean    - Remove build artifacts"
	@echo "  help     - Show this help"
	@echo ""
	@echo "Requirements:"
	@echo "  - nasm (assembler)"
	@echo "  - gcc (C compiler with 32-bit support)"
	@echo "  - ld (linker)"
	@echo "  - qemu-system-i386 (emulator)"
	@echo ""
