# emudore64
Commodore 64 native emulator for the x86 PC

This is an C64 emulator that I stripped down to run bare metal on a PC.  The core OS came
from another source.

Turn your blazing fast PC into a Commodore 64 by booting up with a CD-ROM.
Why?  Well, why not.  We have android running on everything, so why not a C64?

There is so much wrong with this code that it isnt even funny.  But it works as a proof
of concept.  I desperately need help to get this working as a full operating system. If
you are good at emulation, C++, and OS development, please take a look.

As a working demo, just burn the iso to a CD-ROM, and boot it up.  You should quickly
be seeing the ol' C64 screen.  Left Shift is mapped to the quotation mark so you can
play around with basic.  Colors are completely fabricated and do not work.  No real effort
at a proper keyboard matrix - im just writing directly to the keyboard buffer.  Timing is terrible.
You cant load/save (but there is code to talk to a harddrive). Its all a 
big hack really.  But a lot of fun.

The OS portion was from the video series: 
https://www.youtube.com/watch?v=1rnA6wpF0o4&list=PLHh55M_Kq4OApWScZyPl5HhgsTJS9MZ6M&index=1

The emulator code is from:
https://github.com/marioballano/emudore

All credit for emulation to Mario Ballano.  I just stuck stuff together to see if it would work.