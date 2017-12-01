
#include <drivers/pit.h>

using namespace myos::drivers;
using namespace myos::hardwarecommunication;


    PITEventHandler::PITEventHandler()
    {
    }
    
    void PITEventHandler::OnActivate()
    {
    }
    
    void PITEventHandler::OnTick()
    {
    }


    PITDriver::PITDriver(InterruptManager* manager, PITEventHandler* handler)
    : InterruptHandler(manager, 0x20),
    commandport(0x43),
    channel0dataport(0x40),
    channel1dataport(0x41),
    channel2dataport(0x42)
    {
        this->handler = handler;
    }

    PITDriver::~PITDriver()
    {
    }
    
    void PITDriver::Activate()
    {
      uint16_t Hz = 1000; // 60hz frequency
      
        if(handler != 0)
            handler->OnActivate();
        
	//uint32_t divisor = 1193180 / 1000; //18.222; //frequency;
	uint16_t divisor = (7159090 + 6/2) / (6 * Hz);
	
        commandport.Write(0x36);
        
	// Divisor has to be sent byte-wise, so split here into upper/lower bytes.
	uint8_t l = (uint8_t)(divisor & 0xFF);
	uint8_t h = (uint8_t)( (divisor>>8) & 0xFF );

	// Send the frequency divisor.
	outb(0x40, l);
	outb(0x40, h);
	     
    }
    
    uint32_t PITDriver::HandleInterrupt(uint32_t esp)
    {
      //printf("tick...");
      handler->OnTick();

      return esp;
    }

    // Basic port Functions
    void PITDriver::outb(uint16_t port, uint8_t value) 
    {
      //("a" puts value in eax, "dN" puts port in edx or uses 1-byte constant.)
      asm volatile ("outb %0, %1" :: "a" (value), "dN" (port));
    }

    uint8_t PITDriver::inb(uint16_t port)
    {
	uint8_t result;
	asm volatile("inb %1, %0" : "=a" (result) : "Nd" (port));
	return result;
    }