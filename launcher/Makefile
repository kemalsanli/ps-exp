# launch_pskit payload — flat binary, OpenOrbis toolchain
#
# Output: ../launch_pskit.bin (sibling of goldhen.bin in ps-exp/)
#
# Requires OO_PS4_TOOLCHAIN to point at the OpenOrbis install.
# Built the same way as other scene payloads: PIC code, single segment,
# objcopy -O binary to strip ELF headers. No CRT, no stdlib — everything
# resolves at runtime via args->sceKernelDlsym.

ifndef OO_PS4_TOOLCHAIN
    $(error OO_PS4_TOOLCHAIN is undefined)
endif

CC      := clang
LD      := ld.lld
OBJCOPY := llvm-objcopy

export PATH := $(PATH):$(OO_PS4_TOOLCHAIN)/bin/linux

CFLAGS := -target x86_64-scei-ps4-elf \
          -isysroot $(OO_PS4_TOOLCHAIN) \
          -isystem $(OO_PS4_TOOLCHAIN)/include \
          -Isrc \
          -Wall -Wextra -Os \
          -ffreestanding -fno-stack-protector \
          -fno-exceptions -fno-unwind-tables -fno-asynchronous-unwind-tables \
          -fno-plt -fpic \
          -mno-sse

LDFLAGS := -m elf_x86_64 -pie --no-dynamic-linker --nostdlib \
           -T src/linker.x

OUT_DIR := ..
OUT_BIN := $(OUT_DIR)/launch_pskit.bin
OBJS    := src/main.o

.PHONY: all clean

all: $(OUT_BIN)

$(OUT_BIN): launch_pskit.elf
	$(OBJCOPY) -O binary $< $@
	@ls -la $@

launch_pskit.elf: $(OBJS) src/linker.x
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

src/main.o: src/main.c src/payload.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f src/*.o launch_pskit.elf $(OUT_BIN)
