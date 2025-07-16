CC=i686-elf-
CODEDIRS=. OS 
INCDIRS=-I OS 
OUT = bin
ISO = isodir

ASFILE=OS/boot.s
ASOUT=$(OUT)/boot.o 
KERNELFILE=OS/kernel.c
KERNELOUT=$(OUT)/kernel.o
LINKERFILE=OS/linker.ld 
BINARY=$(OUT)/myos.bin
CONFIG=OS/grub.cfg

IMGBIN=$(ISO)/boot/myos.bin
IMGCNF=$(ISO)/boot/grub/grub.cfg
IMGOUT=$(OUT)/myos.iso

CFLAGS=-std=gnu99 -ffreestanding -O2 -Wall -Wextra -g $(INCDIRS)
LINKFLAGS=-ffreestanding -O2 -nostdlib $(ASOUT) $(KERNELOUT) -lgcc

all : $(BINARY)

$(BINARY) : $(LINKERFILE) $(ASOUT) $(KERNELOUT)
	$(CC)gcc -T $(LINKERFILE) -o $(BINARY) $(LINKFLAGS)

$(ASOUT) : $(ASFILE)
	$(CC)gcc -x assembler-with-cpp -c $(ASFILE) -o $(ASOUT) $(CFLAGS)
# 	$(CC)as $(ASFILE) -o $(ASOUT)

$(KERNELOUT) : $(KERNELFILE)
	$(CC)gcc -c $(KERNELFILE) -o $(KERNELOUT) $(CFLAGS)

$(IMGOUT) : $(BINARY)
	cp $(BINARY) $(IMGBIN)
	cp $(CONFIG) $(IMGCNF)
	grub-mkrescue -o $(IMGOUT) $(ISO) 

.PHONY : iso 
iso : $(IMGOUT)

run : $(IMGOUT) 
	qemu-system-i386 -m 512M -cdrom $(IMGOUT)

clean :
	rm -rf $(ASOUT) $(KERNELOUT) $(BINARY)
	rm -rf $(IMGOUT) $(IMGBIN)