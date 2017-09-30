# emudore64
Commodore 64 native emulator operating system for the x86 PC

WHAT:

This is a C64 emulator that I stripped down to run bare metal on a PC. Turn your blazing fast PC
into a Commodore 64 by booting up via CD-ROM or flash drive. Why?  Well, why not?
We have linux and android running on everything, so why not a C64?  We have taken a core OS, and mashed
it up with a C64 emulator as the kernel.  

STATUS:

There is so much wrong with this code that it isn't even funny.  But it works as a proof
of concept.  I would very much like to see this as a fully operational OS. If
you are good at emulation, C++, and OS development, please take a look.

DEMO:

As a working demo, just burn the iso to a CD-ROM, and boot it up, or use something like rufus to convert
to a bootable flash drive.  You should quickly be seeing the ol' C64 screen. You can't load/save (but there
is code to eventually talk to a harddrive). It's all a big hack really, but has some potential.

TO DO:
 * Proper keycode scanning - right now we are just injecting into the keyboard buffer
 * IO to load and save - this will undoubtbly be the biggest area of work
 * I'd like to include an ML monitor on the VGA 80x25 text mode screen as well (ESC key)
 * Scaling so that the border can be rendered. Right now the code works at 320x200 disallowing the border
 * Take advantage of the multitasking and other low level OS features to provide new & interesting capabilties
 * Continue to work with the emudore team to improve emulation
 * Consider a different core OS layer, depending on needs over time
 * Cross compiling for different architectures
 * QEMU and some real hardware wont switch properly to 80 col text mode when ESC is pressed.  Not sure why.

BUILDING:
 * Code compiles for an x86 linux system using gcc 4.8.4
 * Will automatically initiate VirtualBox and start a VM called "emudore64" (see makefile)

UPDATES:

9/21/17: Fixed some keyboard codes so that double quotes, home, and backspace work

9/23/17: Fixed color palette & more keycodes

9/24/17: Text mode screen can now be activated by pressing ESC key.  Will later use this for an ML monitor
         and other functions like loading D64 images, etc.  Appears to be some issues with this switch on various
         systems.

9/25/17: Keyboard code moved, start of ML monitor

CREDITS:
 * The OS portion was from the video series: https://www.youtube.com/watch?v=1rnA6wpF0o4&list=PLHh55M_Kq4OApWScZyPl5HhgsTJS9MZ6M&index=1
 * The emulator code is from: https://github.com/marioballano/emudore
 * All credit for emulation to Mario Ballano.  I just stuck stuff together to see if it would work.