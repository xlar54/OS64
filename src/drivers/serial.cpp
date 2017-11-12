
#include <drivers/serial.h>

using namespace myos::drivers;
using namespace myos::hardwarecommunication;


    SerialEventHandler::SerialEventHandler()
    {
    }
    
    void SerialEventHandler::OnActivate()
    {
    }
    
    void SerialEventHandler::OnSend(uint8_t c)
    {
    }
    
    void SerialEventHandler::OnReceive(uint8_t c)
    {
    }



    SerialDriver::SerialDriver(InterruptManager* manager, SerialEventHandler* handler)
    : InterruptHandler(manager, 0x24),
    dataReg(SERIAL_PORT_A),
    enblInterruptReg(SERIAL_PORT_A + 1),
    idCtrlReg(SERIAL_PORT_A + 2),
    lineCtrlReg(SERIAL_PORT_A + 3),
    modemCtrlReg(SERIAL_PORT_A + 4),
    lineStatusReg(SERIAL_PORT_A + 5),
    modemStatusReg(SERIAL_PORT_A + 6),
    scratchReg(SERIAL_PORT_A + 7)
    {
        this->handler = handler;
    }

    SerialDriver::~SerialDriver()
    {
    }
    
    void SerialDriver::Activate()
    {
        if(handler != 0)
            handler->OnActivate();
        
	enblInterruptReg.Write(0x00);    	// Disable all interrupts
	lineCtrlReg.Write(0x80);    		// Enable DLAB (set baud rate divisor)
	dataReg.Write(0x03);    		// Set divisor to 3 (lo byte) 38400 baud
	enblInterruptReg.Write(0x00);    	//                 (hi byte)
	lineCtrlReg.Write(0x03);    		// 8 bits, no parity, one stop bit
	idCtrlReg.Write(0xC7);    		// Enable FIFO, clear them, with 14-byte threshold
	modemCtrlReg.Write(0x0B);    		// IRQs enabled, RTS/DSR set
	  
	
	enblInterruptReg.Write(0x01);		// enable interrupts
    }
    
    void SerialDriver::Send(uint8_t c)
    {
      dataReg.Write(c);
    }
    
    int SerialDriver::PortIsBusy()
    {
      return (lineStatusReg.Read() & 0x20) == 0;
    }
    
    uint32_t SerialDriver::HandleInterrupt(uint32_t esp)
    {
        uint8_t b = dataReg.Read();
        //if (!(b & 0x24))
        //    return esp;

        handler->OnReceive(b);
        
        //printf("%c",b);
        
        return esp;
    }
