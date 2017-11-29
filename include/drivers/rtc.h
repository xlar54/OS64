#ifndef __MYOS__DRIVERS__RTC_H
#define __MYOS__DRIVERS__RTC_H

#include <lib/stdint.h>
#include <hardwarecommunication/port.h>
#include <drivers/driver.h>
#include <hardwarecommunication/interrupts.h>

struct datetime {
 
  unsigned char second;
  unsigned char minute;
  unsigned char hour;
  unsigned char day;
  unsigned char month;
  uint16_t year;
  unsigned char century;
};

namespace myos
{
    namespace drivers
    {
	enum {
	cmos_address = 0x70,
	cmos_data    = 0x71
	};
      
        class RTCDriver
        {
	  private:
	      myos::hardwarecommunication::Port8Bit indexPort;
	      myos::hardwarecommunication::Port8Bit readwritePort;
            
	      int century_register;                                // Set by ACPI table parsing code if possible
 
	      unsigned char second;
	      unsigned char minute;
	      unsigned char hour;
	      unsigned char day;
	      unsigned char month;
	      uint16_t year;
	      
	      void outb(uint16_t port, uint8_t value);
	      uint8_t inb(uint16_t port);
	      
	  public:
	    RTCDriver();
	    ~RTCDriver();
	    void GetRTC(struct datetime* curDateTime);
	    int get_update_in_progress_flag();
	    unsigned char get_RTC_register(int reg);
	};
    }
}    
#endif
