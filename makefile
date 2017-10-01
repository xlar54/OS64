
GCCPARAMS = -m32 -Iinclude -fno-use-cxa-atexit -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fno-leading-underscore -Wno-write-strings
ASPARAMS = --32
LDPARAMS = -melf_i386

objects = obj/loader.o \
	  obj/memorymanagement.o \
	  obj/lib/stdio.o \
	  obj/lib/vga.o \
	  obj/lib/string.o \
	  obj/lib/stdlib.o \
          obj/gdt.o \
          obj/drivers/driver.o \
          obj/hardwarecommunication/port.o \
          obj/hardwarecommunication/interruptstubs.o \
          obj/hardwarecommunication/interrupts.o \
          obj/syscalls.o \
          obj/multitasking.o \
          obj/hardwarecommunication/pci.o \
          obj/drivers/keyboard.o \
          obj/drivers/mouse.o \
          obj/drivers/ata.o \
          obj/filesystem/fat.o \
          obj/c64/c64.o \
          obj/c64/cia1.o \
          obj/c64/cia2.o \
          obj/c64/cpu.o \
          obj/c64/io.o \
          obj/c64/memory.o \
          obj/c64/vic.o \
          obj/c64/monitor.o \
          obj/kernel.o


run: emu64kernel.iso
	(killall VirtualBox && sleep 1) || true
	VirtualBox --startvm 'emudore64' &

obj/%.o: src/%.cpp
	mkdir -p $(@D)
	gcc $(GCCPARAMS) -c -o $@ $<

obj/%.o: src/%.s
	mkdir -p $(@D)
	as $(ASPARAMS) -o $@ $<

emu64kernel.bin: linker.ld $(objects)
	ld $(LDPARAMS) -T $< -o $@ $(objects)

emu64kernel.iso: emu64kernel.bin
	mkdir iso
	mkdir iso/boot
	mkdir iso/boot/grub
	cp emu64kernel.bin iso/boot/emu64kernel.bin
	echo 'set timeout=0'                       > iso/boot/grub/grub.cfg
	echo 'set default=0'                       >> iso/boot/grub/grub.cfg
	echo ''                                    >> iso/boot/grub/grub.cfg
	echo 'menuentry "Emudore 64" {'            >> iso/boot/grub/grub.cfg
	echo '  multiboot /boot/emu64kernel.bin'   >> iso/boot/grub/grub.cfg
	echo '  boot'                              >> iso/boot/grub/grub.cfg
	echo '}'                                   >> iso/boot/grub/grub.cfg
	grub-mkrescue --output=emudore64boot.iso iso
	rm -rf iso

install: emu64kernel.bin
	sudo cp $< /boot/emu64kernel.bin

.PHONY: clean
clean:
	rm -rf obj emu64kernel.bin emudore64boot.iso
