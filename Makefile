# Toolchain
CC      = i686-elf-gcc
AS      = i686-elf-gcc
LD      = i686-elf-gcc

# Directories
CODEDIRS  := . OS
INCDIRS   := -I OS
SRC       := $(wildcard OS/*.c)
OBJ       := $(patsubst OS/%.c, bin/%.o, $(SRC))
OUT       := bin
ISO       := isodir

# Files
ASFILE      := OS/boot.s
ASOUT       := $(OUT)/boot.o
LINKERFILE  := OS/linker.ld
BINARY      := $(OUT)/myos.bin
CONFIG      := OS/grub.cfg
IMGBIN      := $(ISO)/boot/myos.bin
IMGCNF      := $(ISO)/boot/grub/grub.cfg
IMGOUT      := $(OUT)/myos.iso

# Flags
CFLAGS     := -std=gnu99 -ffreestanding -O2 -Wall -Wextra -g $(INCDIRS)
LDFLAGS    := -T $(LINKERFILE) -ffreestanding -O2 -nostdlib -lgcc

# Targets
all: $(BINARY)

# Link all objects into kernel binary
$(BINARY): $(LINKERFILE) $(ASOUT) $(OBJ)
	$(LD) -o $@ $(ASOUT) $(OBJ) $(LDFLAGS)

# Assemble boot.s
$(ASOUT): $(ASFILE)
	$(AS) -x assembler-with-cpp -c $< -o $@ $(CFLAGS)

# Compile all C sources in OS/ to bin/*.o
bin/%.o: OS/%.c | $(OUT)
	$(CC) -c $< -o $@ $(CFLAGS)

# Create output directory if it doesn't exist
$(OUT):
	mkdir -p $(OUT)

# ISO creation
$(IMGOUT): $(BINARY)
	mkdir -p $(ISO)/boot/grub
	cp $(BINARY) $(IMGBIN)
	cp $(CONFIG) $(IMGCNF)
	grub-mkrescue -o $@ $(ISO)

.PHONY: iso run clean jproto

iso: $(IMGOUT)

run: $(IMGOUT)
	qemu-system-i386 -m 512M -cdrom $(IMGOUT)

jproto:
	cd C-Tools/jproto && cb jproto.c && cp ./jproto ../../

clean:
	rm -rf $(OUT)/*
	rm -rf $(IMGOUT) $(ISO)
