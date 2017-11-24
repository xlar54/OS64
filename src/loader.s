.set MBALIGN,  1<<0              ; // align loaded modules on page boundaries
.set MEMINFO,  1<<1              ; // provide memory map
.set VIDMOD,   1<<2              ; // information about the video mode table must be available to the kernel.
.set MAGIC, 0x1badb002		 ; // 'magic number' lets bootloader find the header
.set FLAGS, MBALIGN | MEMINFO | VIDMOD ; //(1<<0 | 1<<1)
.set CHECKSUM, -(MAGIC + FLAGS)	 ; // checksum of above, to prove we are multiboot

.section .multiboot
    .long MAGIC
    .long FLAGS
    .long CHECKSUM
    .long 0                  ; //12    u32    header_addr    if flags[16] is set
    .long 0                  ; //16    u32    load_addr    if flags[16] is set
    .long 0                  ; //20    u32    load_end_addr    if flags[16] is set
    .long 0                  ; //24    u32    bss_end_addr    if flags[16] is set
    .long 0                  ; //28    u32    entry_addr    if flags[16] is set
    .long 0                  ; //32    u32    mode_type    if flags[2] is set      1 = text mode      0=gfx mode
    .long 640                ; //36    u32    width    if flags[2] is set         80 = columns
    .long 480                ; //40    u32    height    if flags[2] is set        24 = rows 
    .long 8                 ; //44    u32    depth    if flags[2] is set          0 = for text mode

.section .text
.extern kernelMain
.extern callConstructors
.global loader


loader:
    mov $kernel_stack, %esp
    call callConstructors
    push %eax
    push %ebx
    call kernelMain


_stop:
    cli
    hlt
    jmp _stop


.section .bss
.space 2*1024*1024; # 2 MiB
kernel_stack:

