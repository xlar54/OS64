#include <drivers/rtc.h>

using namespace myos::drivers;
using namespace myos::hardwarecommunication;

#define CURRENT_YEAR 2017

RTCDriver::RTCDriver()
:indexPort(0x70),
readwritePort(0x71)
{
  century_register = 0x32;
}

RTCDriver::~RTCDriver()
{
}

void RTCDriver::GetRTC(struct datetime* curDateTime)
{
  struct datetime currentDateTime;
  
    unsigned char century;
    unsigned char last_second;
    unsigned char last_minute;
    unsigned char last_hour;
    unsigned char last_day;
    unsigned char last_month;
    unsigned char last_year;
    unsigned char last_century;
    unsigned char registerB;

    // Note: This uses the "read registers until you get the same values twice in a row" technique
    //       to avoid getting dodgy/inconsistent values due to RTC updates

    while (get_update_in_progress_flag());                // Make sure an update isn't in progress
    
    curDateTime->second = get_RTC_register(0x00);
    curDateTime->minute = get_RTC_register(0x02);
    curDateTime->hour = get_RTC_register(0x04);
    curDateTime->day = get_RTC_register(0x07);
    curDateTime->month = get_RTC_register(0x08);
    curDateTime->year = get_RTC_register(0x09);
    
    if(century_register != 0) {
	  curDateTime->century = get_RTC_register(century_register);
    }

    do {
      last_second = curDateTime->second;
      last_minute = curDateTime->minute;
      last_hour = curDateTime->hour;
      last_day = curDateTime->day;
      last_month = curDateTime->month;
      last_year = curDateTime->year;
      last_century = curDateTime->century;

      while (get_update_in_progress_flag());           // Make sure an update isn't in progress
      
      curDateTime->second = get_RTC_register(0x00);
      curDateTime->minute = get_RTC_register(0x02);
      curDateTime->hour = get_RTC_register(0x04);
      curDateTime->day = get_RTC_register(0x07);
      curDateTime->month = get_RTC_register(0x08);
      curDateTime->year = get_RTC_register(0x09);
      
      if(century_register != 0) {
	    curDateTime->century = get_RTC_register(century_register);
      }
      
    } while( (last_second != curDateTime->second) || (last_minute != curDateTime->minute) ||
	      (last_hour != curDateTime->hour) || (last_day != curDateTime->day) ||
	      (last_month != curDateTime->month) || (last_year != curDateTime->year) ||
	      (last_century != curDateTime->century) );

    registerB = get_RTC_register(0x0B);

    // Convert BCD to binary values if necessary
    if (!(registerB & 0x04)) {
	  curDateTime->second = (curDateTime->second & 0x0F) + ((curDateTime->second / 16) * 10);
	  curDateTime->minute = (curDateTime->minute & 0x0F) + ((curDateTime->minute / 16) * 10);
	  curDateTime->hour = ( (curDateTime->hour & 0x0F) + (((curDateTime->hour & 0x70) / 16) * 10) ) | (curDateTime->hour & 0x80);
	  curDateTime->day = (curDateTime->day & 0x0F) + ((curDateTime->day / 16) * 10);
	  curDateTime->month = (curDateTime->month & 0x0F) + ((curDateTime->month / 16) * 10);
	  curDateTime->year = (curDateTime->year & 0x0F) + ((curDateTime->year / 16) * 10);
	  if(century_register != 0) {
		curDateTime->century = (curDateTime->century & 0x0F) + ((curDateTime->century / 16) * 10);
	  }
    }

    // Convert 12 hour clock to 24 hour clock if necessary
    if (!(registerB & 0x02) && (curDateTime->hour & 0x80)) {
	  curDateTime->hour = ((curDateTime->hour & 0x7F) + 12) % 24;
    }

    // Calculate the full (4-digit) year
    if(century_register != 0) {
	  curDateTime->year += curDateTime->century * 100;
    } else {
      curDateTime->year += 2000;
	  //curDateTime->year += (CURRENT_YEAR / 100) * 100;
	  //if(curDateTime->year < CURRENT_YEAR) curDateTime->year += 100;
    }
}

int RTCDriver::get_update_in_progress_flag() {
      outb(cmos_address, 0x0A);
      return (inb(cmos_data) & 0x80);
}
 
unsigned char RTCDriver::get_RTC_register(int reg) {
      outb(cmos_address, reg);
      return inb(cmos_data);
}

// Basic port Functions
void RTCDriver::outb(uint16_t port, uint8_t value) 
{
  //("a" puts value in eax, "dN" puts port in edx or uses 1-byte constant.)
  asm volatile ("outb %0, %1" :: "a" (value), "dN" (port));
}

uint8_t RTCDriver::inb(uint16_t port)
{
    uint8_t result;
    asm volatile("inb %1, %0" : "=a" (result) : "Nd" (port));
    return result;
}