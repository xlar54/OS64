
#ifndef __MYOS__DRIVERS__PIT_H
#define __MYOS__DRIVERS__PIT_H

#include <lib/stdint.h>
#include <hardwarecommunication/port.h>
#include <drivers/driver.h>
#include <hardwarecommunication/interrupts.h>

namespace myos
{
    namespace drivers
    {
    
        class PITEventHandler
        {
        public:
            PITEventHandler();

            virtual void OnActivate();
            virtual void OnTick();
        };
        
        
        class PITDriver : public myos::hardwarecommunication::InterruptHandler, public Driver
        {
            
            myos::hardwarecommunication::Port8Bit commandport;
	    myos::hardwarecommunication::Port8Bit channel0dataport;
	    myos::hardwarecommunication::Port8Bit channel1dataport;
	    myos::hardwarecommunication::Port8Bit channel2dataport;
	    
            void outb(uint16_t port, uint8_t value);
	      uint8_t inb(uint16_t port);
uint32_t ticker;
            PITEventHandler* handler;
        public:
            PITDriver(myos::hardwarecommunication::InterruptManager* manager, PITEventHandler* handler);
            ~PITDriver();
            virtual uint32_t HandleInterrupt(uint32_t esp);
            virtual void Activate();
        };

    }
}
    
#endif
