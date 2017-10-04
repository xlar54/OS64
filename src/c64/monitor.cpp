#ifndef EMUDORE_MON_H
#define EMUDORE_MON_H

#include <lib/stdint.h>
#include <c64/monitor.h>
#include <c64/io.h>
#include <c64/cpu.h>
#include <c64/memory.h>
#include <lib/string.h>
#include <lib/stdlib.h>

Monitor::Monitor()
{
  bufPtr = 0;
}

Monitor::~Monitor()
{
}

void Monitor::Run()
{
  printf("Emudore 64 - ML Monitor:\n\n");
  printf("====================================================\n");
  help();
  
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
    case '?':
    {
      help();
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

void Monitor::help()
{
  printf("Commands:\n");
  printf("M - Memory Display\n");
  printf("R - Register Display\n");
  printf("$ - Directory (ATA FAT32 Harddisk Master 0)\n");
  printf("ESC - Return to system\n");
  printf("====================================================\n");
  printf("Built in ML monitor activated via SYS 36864\n");
}

void Monitor::prompt()
{
  for(int x=0;x<80;x++)
    buf[x] = 0;
  
  bufPtr = 0;
  
  printf("\n>");
}

void Monitor::process_cmd()
{
  char cmd;
  char param1[80];
  char param2[80];
  int i=0;
  int p1=0;
  int p2=0;  
  
  if(bufPtr==0)
    return;  
  
  cmd = buf[i++];
  
  if(buf[i] == ' ')
  {
    while(buf[++i] != ' ' && buf[i] != 0)
    {
      param1[p1++] = buf[i];
    }
    param1[p1] = 0;
  }
  
  if(buf[i] == ' ')
  {    
    while(buf[++i] != 0)
    {
      param2[p2++] = buf[i];
    }
    param2[p2] = 0;
  }
  
  switch(cmd)
  {
    case 'R':
    {
      printf("\n PC  AC XR YR SP\n");
      printf("%04X %02X %02X %02X %02X\n", cpu_->pc(), cpu_->a(), cpu_->x(), cpu_->y(), cpu_->sp());
      break;
    }
    case 'M':
    {
      int p1 = htoi(param1);
      int p2 = htoi(param2);
           
      if(!p2)
	p2 = p1 + 16;
      
      if(p2>0xFFFF)
      {
	printf("?");
	return;
      }
      
      while(p1 < p2)
      {
	printf("\n%04X: ",p1); 
	int t=p1;
	
	for(int x=0;x<16;x++)
	{
	  printf("%02X", mem_->read_byte(p1++));
	  printf("%c", ' ');
	}
	for(int x=0;x<16;x++)
	{
	  if(mem_->read_byte(t) >31 && mem_->read_byte(t) < 127)
	    printf("%c", mem_->read_byte(t++));
	  else
	  {
	    printf(".");
	    t++;
	  }
	}
      }
  
      break;
    }
    default:
      printf("?");
      break;
  }
}

#endif