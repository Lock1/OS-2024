OBJECTS       = src/kernel.o src/gdt.o src/kernel-entrypoint.o src/framebuffer.o \
				src/cpu/portio.o src/cpu/interrupt.o src/cpu/intsetup.o src/cpu/idt.o \
				src/keyboard.o src/disk.o src/fat32.o src/stdlib/string.o src/paging.o \
				src/textio.o src/process.o src/scheduler.o src/context-switch.o

# Compiler & linker
ASM           = nasm
LIN           = ld
CC            = gcc

# Directory
SOURCE_FOLDER = src
OUTPUT_FOLDER = bin
ISO_NAME      = OS2024
DISK_NAME     = storage

# Flags
WARNING_CFLAG = -Wall -Wextra -Werror
DEBUG_CFLAG   = -fshort-wchar -g
STRIP_CFLAG   = -nostdlib -fno-stack-protector -nostartfiles -nodefaultlibs -ffreestanding
CFLAGS        = $(DEBUG_CFLAG) $(WARNING_CFLAG) $(STRIP_CFLAG) -m32 -c -I$(SOURCE_FOLDER)
AFLAGS        = -f elf32 -g -F dwarf
LFLAGS        = -T $(SOURCE_FOLDER)/linker.ld -melf_i386


run: all
	@qemu-system-i386 -s -S -drive file=$(OUTPUT_FOLDER)/$(DISK_NAME).bin,format=raw,if=ide,index=0,media=disk -cdrom $(OUTPUT_FOLDER)/$(ISO_NAME).iso
all: build
build: iso
clean:
	rm -rf *.o $(OUTPUT_FOLDER)/*.iso $(OUTPUT_FOLDER)/kernel ./**/*.o

disk:
	@qemu-img create -f raw $(OUTPUT_FOLDER)/$(DISK_NAME).bin 4M

kernel: $(OBJECTS)
	@$(LIN) $(LFLAGS) $(OBJECTS) -o $(OUTPUT_FOLDER)/kernel
	@echo Linking object files and generate elf32...
	@rm -f *.o

inserter:
	@$(CC) -Wno-builtin-declaration-mismatch -g -I$(SOURCE_FOLDER) \
		$(SOURCE_FOLDER)/stdlib/string.c \
		$(SOURCE_FOLDER)/fat32.c \
		$(SOURCE_FOLDER)/external/external-inserter.c \
		-o $(OUTPUT_FOLDER)/inserter

user-shell:
	@$(ASM) $(AFLAGS) $(SOURCE_FOLDER)/external/crt0.s -o crt0.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/external/user-program/user-shell.c -o user-shell.o
	@$(LIN) -T $(SOURCE_FOLDER)/external/user-linker.ld -melf_i386 --oformat=binary \
		crt0.o user-shell.o -o $(OUTPUT_FOLDER)/shell
	@$(LIN) -T $(SOURCE_FOLDER)/external/user-linker.ld -melf_i386 --oformat=elf32-i386 \
		crt0.o user-shell.o -o $(OUTPUT_FOLDER)/shell_elf
	@echo Linking object shell object files and generate ELF32 for debugging...
	@echo Linking object shell object files and generate flat binary...
	@size --target=binary $(OUTPUT_FOLDER)/shell
	@rm -f *.o

insert-shell: inserter user-shell
	@echo Inserting shell into root directory...
	@cd $(OUTPUT_FOLDER); ./inserter shell 2 $(DISK_NAME).bin

user-clock:
	@$(ASM) $(AFLAGS) $(SOURCE_FOLDER)/external/crt0.s -o crt0.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/external/user-program/clock.c -o clock.o
	@$(LIN) -T $(SOURCE_FOLDER)/external/user-linker.ld -melf_i386 --oformat=binary \
		crt0.o clock.o -o $(OUTPUT_FOLDER)/clock
	@$(LIN) -T $(SOURCE_FOLDER)/external/user-linker.ld -melf_i386 --oformat=elf32-i386 \
		crt0.o clock.o -o $(OUTPUT_FOLDER)/clock_elf
	@echo Linking object clock object files and generate ELF32 for debugging...
	@echo Linking object clock object files and generate flat binary...
	@size --target=binary $(OUTPUT_FOLDER)/clock
	@rm -f *.o

insert-clock: inserter user-clock
	@echo Inserting clock into root directory...
	@cd $(OUTPUT_FOLDER); ./inserter clock 2 $(DISK_NAME).bin

iso: kernel
	@mkdir -p $(OUTPUT_FOLDER)/iso/boot/grub
	@cp $(OUTPUT_FOLDER)/kernel     $(OUTPUT_FOLDER)/iso/boot/
	@cp other/grub1                 $(OUTPUT_FOLDER)/iso/boot/grub/
	@cp $(SOURCE_FOLDER)/menu.lst   $(OUTPUT_FOLDER)/iso/boot/grub/
	@genisoimage -R                   \
		-b boot/grub/grub1         \
		-no-emul-boot              \
		-boot-load-size 4          \
		-A os                      \
		-input-charset utf8        \
		-quiet                     \
		-boot-info-table           \
		-o $(OUTPUT_FOLDER)/OS2024.iso \
		$(OUTPUT_FOLDER)/iso
	@rm -r $(OUTPUT_FOLDER)/iso/

%.o: %.c
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $< -o $@
	@echo Compiling $@...

%.o: %.s
	@mkdir -p $(@D)
	@$(ASM) $(AFLAGS) $< -o $@
	@echo Compiling $@...