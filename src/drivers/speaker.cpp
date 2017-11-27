#include <drivers/speaker.h>

using namespace myos::drivers;
using namespace myos::hardwarecommunication;

SpeakerDriver::SpeakerDriver()
:control(0x43),
data(0x42)
{
}

SpeakerDriver::~SpeakerDriver()
{
}

void SpeakerDriver::Sound(unsigned int frequency)
{
 unsigned char status;
 frequency = 1193180 / frequency;
	
 control.Write(0xB6);
 data.Write((unsigned char)frequency);
 data.Write((unsigned char)(frequency >> 0x08));

 status = inb(0x61);
 if(status != (status|0x03)) outb(0x61, status|3) ;
}

void SpeakerDriver::Nosound()
{
 outb(0x61 , inb(0x61) & 0xFC );
}

// Basic port Functions
void SpeakerDriver::outb(uint16_t port, uint8_t value) 
{
  //("a" puts value in eax, "dN" puts port in edx or uses 1-byte constant.)
  asm volatile ("outb %0, %1" :: "a" (value), "dN" (port));
}

uint8_t SpeakerDriver::inb(uint16_t port)
{
    uint8_t result;
    asm volatile("inb %1, %0" : "=a" (result) : "Nd" (port));
    return result;
}