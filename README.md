# OS64
Commodore 64 native emulator operating system for the x86 PC. Formerly called Emudore 64.

* Check out the project blog at: https://os64.blogspot.com/

WHAT:

This is a C64 emulator that I stripped down to run bare metal on a PC. Turn your blazing fast PC
into a Commodore 64 by booting up via CD-ROM or flash drive. Why?  Well, why not?
We have linux and android running on everything, so why not a C64?  We have taken a core OS, and mashed
it up with a C64 emulator as the kernel.  

STATUS:

Emulation is pretty good but the primary focus at the present is accessing the machine's harddrive, getting
proper keyboard, support, etc.

DEMO:

As a working demo, just burn the iso to a CD-ROM, and boot it up, or use something like rufus to convert
to a bootable flash drive.  You should quickly be seeing the ol' C64 screen. Attach an IDE ATA drive (primary
master), formatted to FAT32, and you should be able to load and save to drive 8.  This does not currently 
work with AHCI SATA drives.  The ESC key should take you to a screen which lets to manage the drive and other
things Im adding as a need arises.  This screen is subject to change a great deal as its primarily meant to be
used for testing.

BUILDING:
 * Code compiles for an x86 linux system using gcc 4.8.4
 * Will automatically initiate VirtualBox and start a VM called "emudore64" (see makefile)

CREDITS:
 * The OS portion was from the video series: https://www.youtube.com/watch?v=1rnA6wpF0o4&list=PLHh55M_Kq4OApWScZyPl5HhgsTJS9MZ6M&index=1
 * The emulator code is from: https://github.com/marioballano/emudore
 * All credit for emulation to Mario Ballano.  I just stuck stuff together to see if it would work.