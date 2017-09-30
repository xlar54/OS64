#ifndef EMUDORE_MON_H
#define EMUDORE_MON_H

#include <lib/stdint.h>
#include <c64/monitor.h>
#include <c64/io.h>
#include <c64/cpu.h>
#include <c64/memory.h>
#include <lib/string.h>

Monitor::Monitor()
{
}

Monitor::~Monitor()
{
}

void Monitor::Run()
{
  setTextModeVGA(0);
  vga_cursorOn = 1;
  
  printf("Emudore 64 - ML Monitor:\n\n");
  printf("====================================================\n");
  printf("Commands:\n");
  printf("M - Memory Display\n");
  printf("====================================================\n");
  
  prompt();
  
}

void Monitor::OnKeyDown(uint8_t c)
{
  switch(c)
  {
    case 0x0E:
    {
      if (bufPtr > 0)
      {
	bufPtr--;
	buf[bufPtr] = '\0';
	
	char* foo = " ";
	foo[0] = c;
	printf(foo);
      }
      break;
    }
    case '\n':
    {
      buf[bufPtr] == '\0';
      process_cmd();
      
      bufPtr = 0;
      buf[bufPtr] = '\0';
      prompt();
      break;
    }
    default:
      if(bufPtr<80)
      {
	buf[bufPtr++] = c;
	char* foo = " ";
	foo[0] = c;
	printf(foo);
      }
      break;
  }
}

void Monitor::prompt()
{
  printf("\n>");
}

void Monitor::process_cmd()
{
 
  if(!strcmp(buf, "M 0400"))
  {
    printf("\n\n0400: "); 
    
    printf("%02X", mem_->read_byte(1024));
    printf("-");
    printf("%02X", mem_->read_byte(1025));
    printf("-");
    printf("%02X", mem_->read_byte(1026));
    printf("-");
    printf("%02X", mem_->read_byte(1027));
    printf("-");
    printf("%02X", mem_->read_byte(1028));
    printf("-");
    printf("%02X", mem_->read_byte(1029));
    printf("-");
    printf("%02X", mem_->read_byte(1030));
    printf("-");
    printf("%02X", mem_->read_byte(1031));
    printf("-");
    printf("%02X", mem_->read_byte(1032));
    printf("-");
  }
}

#endif