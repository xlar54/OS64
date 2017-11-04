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
  printf("P - Partition information\n");
  printf("F - Read File (F <filename>)\n");
  printf("S - Sector Display (S sectorNumInHex #bytesInDec)\n");
  printf("D - Directory (ATA FAT32 Harddisk Master 0)\n");
  printf("E - Erase (delete) file\n");
  printf("N - ReName a file\n");
  printf("L - Load file to RAM (L FILENAME.EXT C000)\n");
  printf("W - Write RAM to file (W FILENAME.EXT C000 C1FF)\n");
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
  char param3[80];
  int i=0;
  int p1=0;
  int p2=0;
  int p3=0;
  
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
    while(buf[++i] != ' ' && buf[i] != 0)
    {
      param2[p2++] = buf[i];
    }
    param2[p2] = 0;
  }
  
  if(buf[i] == ' ')
  {    
    while(buf[++i] != 0)
    {
      param3[p3++] = buf[i];
    }
    param3[p3] = 0;
  }
  
  switch(cmd)
  {
    case 'R':
    {
      printf("\n PC  AC XR YR SP\n");
      printf("%04X %02X %02X %02X %02X\n", cpu_->pc(), cpu_->a(), cpu_->x(), cpu_->y(), cpu_->sp());
      break;
    }
    case 'S':
    {
      int p1 = htoi(param1);
      int p2 = atoi(param2);
      
      if(!p2)
	p2 = 256;
      
      if(p2 > 512)
      {
	printf("?");
	return;
      }
      uint8_t buffer[512];
      fat32_->ReadSector(p1, buffer);
      displayMemory(buffer, p2);
      break;
    }
    case 'P':
    {
      fat32_->ReadPartitions();
      break;
    }
    case 'F':
    {
      uint32_t sz = fat32_->GetFileSize((uint8_t*)param1);
      uint8_t* bytes;
      
      bytes = new uint8_t[sz];
      fat32_->ReadFile((uint8_t*)param1, bytes, sz);
      
      //displayMemory(bytes, sz);
      printf("\n");
      for(int x=0;x<sz;x++)
	printf("%c",bytes[x]);
      delete [] bytes;
      break;
    }
    case 'D':
    {
      int p1 = htoi(param1);
      fat32_->ReadDirectory(p1);
      break;
    }
    case 'L':
    {
	int fstatus = 0;
	fstatus = fat32_->OpenFile(1, (uint8_t*)param1, FILEACCESSMODE_READ);
	if(fstatus == FILE_STATUS_OK)
	{
	  uint16_t m = htoi(param2);
	  
	  if(m == 0)
	    m = 0x800;
	  
	  uint8_t b;
	  int ctr = 0;
	  
	  fstatus = fat32_->ReadNextFileByte(1, &b);
	  
	  while(fstatus != FILE_STATUS_EOF)
	  {
	    mem_->write_byte(m+ctr, b);
	    ctr++;
	    fstatus = fat32_->ReadNextFileByte(1, &b);
	  }
	  fat32_->CloseFile(1);
	  
	  // tell basic where program ends (after 3 zeros)
	  mem_->write_byte(45, (m+ctr) & 0xFF); // poke low byte to 45
	  mem_->write_byte(46, (m+ctr) >> 8); // poke hi byte to 46
	  
	  
	  printf("\n%d bytes written to %04X - %04X", ctr, m, m+ctr); 
	}
	printf("\nstatus=%d", fstatus);
	break;
    }
    case 'E':
    {
      int fstatus = 0;
      fstatus = fat32_->DeleteFile((uint8_t*)param1);
      printf("\nstatus=%d",fstatus);
      break;
    }
    case 'N':
    {
      int fstatus = 0;
      fstatus = fat32_->RenameFile((uint8_t*)param1, (uint8_t*)param2);
      printf("\nstatus=%d",fstatus);
      break;
    }
    case 'J':
    {
      int fstatus = 0;
      fstatus = fat32_->OpenFile(1, (uint8_t*)param1, FILEACCESSMODE_CREATE);
      char msg[50] = "SBCDEFGHIJKLMNOPQRSTUVWXYZ1234567890AAABBBCCCDDDD";
      uint8_t msgCtr = 0;
      if(fstatus == FILE_STATUS_OK)
      {
	while(fstatus == FILE_STATUS_OK && msgCtr < 47)
	{
	  fstatus = fat32_->WriteNextFileByte(1,msg[msgCtr++]);
	}

	fat32_->CloseFile(1);
      }
      printf("\nstatus=%d",fstatus);
      break;
    }
    case 'W':
    {
      int fstatus = 0;
      uint16_t mStart = htoi(param2);
      uint16_t mEnd = htoi(param3);
      
      if(mStart == 0)
      {
	mStart = 0x800;
      }
      
      if(mEnd > 0)
      {
	if(mStart >= mEnd)
	{
	  printf("\n%04X >= %04X", mStart, mEnd);
	  break;
	}
      }
      
      uint8_t eobLo = mem_->read_byte(45);
      uint8_t eobHi = mem_->read_byte(46);
      
      if(mEnd ==0)
	mEnd = (eobHi << 8) + eobLo;
      
      fstatus = fat32_->OpenFile(1, (uint8_t*)param1, FILEACCESSMODE_CREATE);
      
      if(fstatus == FILE_STATUS_OK)
      {
	while(fstatus == FILE_STATUS_OK && mStart <= mEnd)
	{
	  uint8_t b = mem_->read_byte(mStart++);
	  fstatus = fat32_->WriteNextFileByte(1,b);
	}

	fat32_->CloseFile(1);
      }
      printf("\nstatus=%d",fstatus);
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