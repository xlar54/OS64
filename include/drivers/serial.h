#ifndef __MYOS__DRIVERS__SERIAL_H
#define __MYOS__DRIVERS__SERIAL_H

#include <lib/stdint.h>
#include <hardwarecommunication/port.h>
#include <drivers/driver.h>
#include <hardwarecommunication/interrupts.h>

#define SERIAL_PORT_A 0x3F8
#define SERIAL_PORT_B 0x2F8
#define SERIAL_PORT_C 0x3E8
#define SERIAL_PORT_D 0x2E8

namespace myos
{
    namespace drivers
    {
      class SerialEventHandler
        {
        public:
            SerialEventHandler();

            virtual void OnActivate();
            virtual void OnSend(uint8_t c);
            virtual void OnReceive(uint8_t c);
        };
        
        
        class SerialDriver : public myos::hardwarecommunication::InterruptHandler, public Driver
        {
            myos::hardwarecommunication::Port8Bit dataReg;
	    myos::hardwarecommunication::Port8Bit enblInterruptReg;
	    myos::hardwarecommunication::Port8Bit idCtrlReg;
	    myos::hardwarecommunication::Port8Bit lineCtrlReg;
	    myos::hardwarecommunication::Port8Bit modemCtrlReg;
	    myos::hardwarecommunication::Port8Bit lineStatusReg;
	    myos::hardwarecommunication::Port8Bit modemStatusReg;
	    myos::hardwarecommunication::Port8Bit scratchReg;
	    
            uint8_t buffer[3];
            uint8_t offset;
            uint8_t buttons;

            SerialEventHandler* handler;
        public:
            SerialDriver(myos::hardwarecommunication::InterruptManager* manager, SerialEventHandler* handler);
            ~SerialDriver();
            virtual uint32_t HandleInterrupt(uint32_t esp);
            virtual void Activate();
	    void Send(uint8_t c);
	    int PortIsBusy();
        };
    }
}    
#endif
