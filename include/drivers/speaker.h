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
            
	  public:
	      SpeakerDriver();
	      ~SpeakerDriver();

	    void Sound(unsigned int);
	    void Nosound();
	};
    }
}    
#endif
