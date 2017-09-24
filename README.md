# emudore64
Commodore 64 native emulator for the x86 PC

This is a C64 emulator that I stripped down to run bare metal on a PC.  The core OS came
from another source.

Turn your blazing fast PC into a Commodore 64 by booting up with a CD-ROM.
Why?  Well, why not.  We have android running on everything, so why not a C64?

There is so much wrong with this code that it isn't even funny.  But it works as a proof
of concept.  I would very much like to see this as a fully operational OS. If
you are good at emulation, C++, and OS development, please take a look.

As a working demo, just burn the iso to a CD-ROM, and boot it up.  You should quickly
be seeing the ol' C64 screen.  Colors are completely fabricated and do not work.  No real effort
at a proper keyboard matrix - I'm just writing directly to the keyboard buffer.  Timing is terrible.
You can't load/save (but there is code to talk to a harddrive). It's all a 
big hack really.  But a lot of fun.


The OS portion was from the video series: 
https://www.youtube.com/watch?v=1rnA6wpF0o4&list=PLHh55M_Kq4OApWScZyPl5HhgsTJS9MZ6M&index=1

The emulator code is from:
https://github.com/marioballano/emudore

All credit for emulation to Mario Ballano.  I just stuck stuff together to see if it would work.


TO DO:
 * Proper keycode scanning - right now we are just injecting into the keyboard buffer
 * IO to load and save - this will undoubtbly be the biggest area of work
 * I'd like to include a monitor on the VGA 80x25 text mode screen as well (ESC key)
 * Scaling so that the border can be rendered. Right now the code works at 320x200 disallowing the border
 * Take advantage of the multitasking and other low level OS features to provide new & interesting capabilties
 * Continue to work with the emudore team to improve emulation
 
Code compiles for an x86 system using gcc 4.8.4. 

9/21/17: Fixed some keyboard codes so that double quotes, home, and backspace work

9/23/17: Fixed color palette & more keycodes

9/24/17: Text mode screen can now be activated by pressing ESC key.  Will later use this for an ML monitor
         and other functions like loading D64 images, etc.
