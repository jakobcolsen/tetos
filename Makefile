# TetOS Makefile
# My desktop runs Windows, my latop Ubuntu, so this Makefile tries to be cross-platform.
# It assumes you have a RISC-V toolchain installed and in your PATH.

# ===== Toolchain =====
CROSS   = riscv64-unknown-elf-
CC      = $(CROSS)gcc
OBJCOPY = $(CROSS)objcopy
OBJDUMP = $(CROSS)objdump

# ===== Cross-platform shell utils =====
ifeq ($(OS),Windows_NT)
  MKDIR = mkdir
  RMDIR = rmdir /S /Q
  FIXPATH = $(subst /,\,$1)
else
  MKDIR = mkdir -p
  RMDIR = rm -rf
  FIXPATH = $1
endif

# Works whether you run from repo root (with os/…) or inside os/.
SRCDIRS_CANDIDATES := os/src src . \
                      os/src/boot os/src/kernel os/src/drivers os/src/lib os/src/fdt \
                      src/boot src/kernel src/drivers src/lib src/fdt
INCDIRS_CANDIDATES := os/include include .

# Only keep those that actually exist
SRCDIRS := $(foreach d,$(SRCDIRS_CANDIDATES),$(if $(wildcard $(d)),$(d),))
INCDIRS := $(foreach d,$(INCDIRS_CANDIDATES),$(if $(wildcard $(d)),$(d),))

# Collect sources
CFILES  := $(foreach d,$(SRCDIRS),$(wildcard $(d)/*.c))
ASM     := $(foreach d,$(SRCDIRS),$(wildcard $(d)/*.s))

# Prefer linker in boot/, else fallback to ./linker.ld
LINKER  := $(firstword \
            $(foreach d,$(SRCDIRS),$(wildcard $(d:/src/%=/boot)/linker.ld)) \
            $(wildcard linker.ld))

# ===== Output =====
TARGET   = tetos
BUILDDIR = build

# Objects mirror source tree under build/
OBJS = $(patsubst %.c,$(BUILDDIR)/%.o,$(CFILES)) \
       $(patsubst %.s,$(BUILDDIR)/%.o,$(ASM))

# ===== Flags =====
INCLUDES = $(foreach d,$(INCDIRS),-I$(d))
RISCV_ISA  = -march=rv64imac
RISCV_ABI  = -mabi=lp64

CFLAGS  = -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -O2 \
          -mcmodel=medany $(RISCV_ISA) $(RISCV_ABI) \
          $(INCLUDES)

LDFLAGS = -T $(LINKER) -nostdlib -mcmodel=medany $(RISCV_ISA) $(RISCV_ABI)

ifeq ($(strip $(LINKER)),)
  $(error No linker script found (looked for */boot/linker.ld or ./linker.ld))
endif
ifeq ($(strip $(CFILES) $(ASM)),)
  $(error No source files found in $(SRCDIRS). Put .c/.S in src/… or os/src/…)
endif

# ===== Default =====
all: $(TARGET).elf $(TARGET).bin

# ===== Compile rules (Windows-safe mkdir) =====
$(BUILDDIR)/%.o: %.c
	@$(MKDIR) $(call FIXPATH,$(dir $@))
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/%.o: %.s
	@$(MKDIR) $(call FIXPATH,$(dir $@))
	$(CC) $(CFLAGS) -c $< -o $@

# ===== Link / tools =====
$(TARGET).elf: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

dump: $(TARGET).elf
	$(OBJDUMP) -D $< > $(TARGET).lst
	@echo "Disassembly written to $(TARGET).lst"

run: $(TARGET).elf
	qemu-system-riscv64 -machine virt -nographic -bios none -kernel $(TARGET).elf

list:
	@echo SRCDIRS: $(SRCDIRS)
	@echo INCDIRS: $(INCDIRS)
	@echo LINKER : $(LINKER)
	@echo CFILES : $(CFILES)
	@echo ASM    : $(ASM)
	@echo OBJS   : $(OBJS)

clean:
ifeq ($(OS),Windows_NT)
	-$(RMDIR) $(call FIXPATH,$(BUILDDIR)) 2> NUL
	-del /Q $(call FIXPATH,$(TARGET).elf) 2> NUL || exit 0
	-del /Q $(call FIXPATH,$(TARGET).bin) 2> NUL || exit 0
	-del /Q $(call FIXPATH,$(TARGET).map) 2> NUL || exit 0
else
	-$(RMDIR) $(call FIXPATH,$(BUILDDIR))
	-rm -f $(TARGET).elf $(TARGET).bin $(TARGET).map
endif

.PHONY: all clean dump run list