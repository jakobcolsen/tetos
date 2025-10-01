# TetOS Makefile
# My desktop runs Windows, my latop Ubuntu, so this Makefile tries to be cross-platform.
# It assumes you have a RISC-V toolchain installed and in your PATH.

# ===== Toolchain =====
CROSS   = riscv64-unknown-elf-
CC      = $(CROSS)gcc
OBJCOPY = $(CROSS)objcopy
OBJDUMP = $(CROSS)objdump
READELF = $(CROSS)readelf
NM      = $(CROSS)nm

# ===== Project layout (run make from repo root) =====
SRCDIRS = os/src os/src/boot os/src/kernel os/src/drivers os/src/lib os/src/fdt
INCDIRS = os/include .

# ===== Output =====
TARGET   = tetos
BUILDDIR = build

# ===== RISC-V flags (no F/D, soft-float ABI) =====
RISCV_ISA = -march=rv64imac
RISCV_ABI = -mabi=lp64

CFLAGS  = -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -O2 \
          -mcmodel=medany $(RISCV_ISA) $(RISCV_ABI) \
          $(foreach d,$(INCDIRS),-I$(d))
# Prefer boot linker if present
LINKER  := $(firstword $(wildcard os/src/boot/linker.ld linker.ld))
LDFLAGS = -T $(LINKER) -nostdlib -Wl,-Map=$(TARGET).map \
          -mcmodel=medany $(RISCV_ISA) $(RISCV_ABI)

# ===== Windows / Unix portability helpers =====
ifeq ($(OS),Windows_NT)
  # Path fix (not strictly needed for GCC, but for shell cmds)
  FIXPATH = $(subst /,\,$1)
  # mkdir that only runs when needed and never errors if exists
  MKDIR_P = if not exist "$(call FIXPATH,$(dir $@))" mkdir "$(call FIXPATH,$(dir $@))"
  RM_RDIR = rmdir /S /Q
  RM_FILE = del /Q
  NULL    = 2> NUL
else
  FIXPATH = $1
  MKDIR_P = mkdir -p $(dir $@)
  RM_RDIR = rm -rf
  RM_FILE = rm -f
  NULL    =
endif

# ===== Source discovery (portable, no external 'find') =====
CFILES := $(foreach d,$(SRCDIRS),$(wildcard $(d)/*.c))
ASMS   := $(foreach d,$(SRCDIRS),$(wildcard $(d)/*.[sS]))

# Build object paths mirroring source tree under build/
OBJS = \
  $(patsubst %.c,$(BUILDDIR)/%.o,$(CFILES)) \
  $(patsubst %.S,$(BUILDDIR)/%.o,$(filter %.S,$(ASMS))) \
  $(patsubst %.s,$(BUILDDIR)/%.o,$(filter %.s,$(ASMS)))

# ===== Sanity guards =====
ifeq ($(strip $(LINKER)),)
  $(error No linker script found (looked for os/src/boot/linker.ld or ./linker.ld))
endif
ifeq ($(strip $(CFILES) $(ASMS)),)
  $(error No source files found in $(SRCDIRS). Put .c/.s/.S in os/src/... )
endif

# ===== Default =====
all: $(TARGET).elf $(TARGET).bin

# ===== Compile rules (Windows-safe mkdir) =====
$(BUILDDIR)/%.o: %.c
	@$(MKDIR_P)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/%.o: %.S
	@$(MKDIR_P)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/%.o: %.s
	@$(MKDIR_P)
	$(CC) $(CFLAGS) -c $< -o $@

# ===== Link & tools =====
$(TARGET).elf: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

dump: $(TARGET).elf
	$(OBJDUMP) -D $< > $(TARGET).lst
	@echo "Disassembly written to $(TARGET).lst"

list:
	@echo SRCDIRS: $(SRCDIRS)
	@echo INCDIRS: $(INCDIRS)
	@echo LINKER : $(LINKER)
	@echo CFILES : $(CFILES)
	@echo ASMS   : $(ASMS)
	@echo OBJS   : $(OBJS)

run: $(TARGET).elf
	qemu-system-riscv64 -machine virt -nographic -bios none -kernel $(TARGET).elf

trace: $(TARGET).elf
	qemu-system-riscv64 -machine virt -nographic -serial stdio -bios none -kernel $(TARGET).elf -d guest_errors,unimp,mmu

clean:
ifeq ($(OS),Windows_NT)
	-$(RM_RDIR) $(call FIXPATH,$(BUILDDIR)) $(NULL)
	-$(RM_FILE) $(call FIXPATH,$(TARGET).elf) $(NULL) || exit 0
	-$(RM_FILE) $(call FIXPATH,$(TARGET).bin) $(NULL) || exit 0
	-$(RM_FILE) $(call FIXPATH,$(TARGET).map) $(NULL) || exit 0
	-$(RM_FILE) $(call FIXPATH,$(TARGET).lst) $(NULL) || exit 0
else
	-$(RM_RDIR) $(BUILDDIR)
	-$(RM_FILE) $(TARGET).elf $(TARGET).bin $(TARGET).map $(TARGET).lst
endif

.PHONY: all clean dump run run256 trace list
