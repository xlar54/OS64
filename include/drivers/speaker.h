#ifndef __MYOS__DRIVERS__SPEAKER_H
#define __MYOS__DRIVERS__SPEAKER_H

#include <lib/stdint.h>
#include <hardwarecommunication/port.h>
#include <drivers/driver.h>
#include <hardwarecommunication/interrupts.h>


namespace myos
{
    namespace drivers
    {
        class SpeakerDriver
        {
	  private:
	      myos::hardwarecommunication::Port8Bit control;
	      myos::hardwarecommunication::Port8Bit data;
            
	      void outb(uint16_t port, uint8_t value);
	      uint8_t inb(uint16_t port);
	      
	  public:
	      SpeakerDriver();
	      ~SpeakerDriver();

	    void Sound(unsigned int);
	    void Nosound();
	};
    }
}    
#endif
