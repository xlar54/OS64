
#include <multiboot.h>
#include <lib/stdint.h>
#include <lib/vga.h>
#include <lib/stdio.h>
#include <gdt.h>
#include <memorymanagement.h>
#include <hardwarecommunication/interrupts.h>
#include <syscalls.h>
#include <hardwarecommunication/pci.h>
#include <drivers/driver.h>
#include <drivers/keyscancodes.h>
#include <drivers/keyboard.h>
#include <drivers/mouse.h>
#include <drivers/ata.h>
#include <drivers/serial.h>
#include <drivers/speaker.h>
#include <drivers/rtc.h>
#include <drivers/pit.h>
#include <multitasking.h>
#include <filesystem/fat.h>
#include <c64/c64.h>
#include <c64/monitor.h>

uint32_t current_milli = 0;

multiboot_info_t* mboot_hdr;     
multiboot_info_t *verified_mboot_hdr;
uint32_t mboot_reserved_start;
uint32_t mboot_reserved_end;
uint32_t next_free_frame;

using namespace myos;
using namespace myos::drivers;
using namespace myos::hardwarecommunication;
using namespace myos::filesystem;

// for accessing and sending keyboard or 
// other external data to c64's IO system

C64* c64ptr;	
uint32_t* framebuffer_addr;

class IOKeyboardEventHandler : public KeyboardEventHandler
{
private:
  uint8_t mode;  // 0 = emulation, 1 = terminal
public:
    IOKeyboardEventHandler()
    {
      mode = 0;
    }

    void OnKeyDown(uint8_t c)
    {
      // ESC key will toggle between text mode and emulation
      if(c == 0x01) 
      {
	if (mode == 0) 
	{ 
	  mode = 1; 
	  c64ptr->stop();
	  return;
	}
	else 
	{ 
	  mode = 0; 
	  c64ptr->mon_->Stop();
	  return;
	}
      }
      
      if(c == 0x03) // Reset machine (F10)
      {
	c64ptr->reset = true;
	return;
      }
      
      switch(mode)
      {
	case 0: c64ptr->io_->OnKeyDown(c); break;
	case 1: c64ptr->mon_->OnKeyDown(c); break;
      }
    }
    
    void OnKeyUp(uint8_t c)
    {
      switch(mode)
      {
	case 0: c64ptr->io_->OnKeyUp(c); break;
	case 1: c64ptr->mon_->OnKeyUp(c); break;
      }
    }
};


class IOPITEventHandler : public PITEventHandler
{
private:
public:
  IOPITEventHandler() 
  {
  }
  
  void OnTick() 
  {
    current_milli++;  
    //printf("tick...");
  }
};

// Set up C++ object constructors.  This has to be set up and called manually
typedef void (*constructor)();
extern "C" constructor start_ctors;
extern "C" constructor end_ctors;
extern "C" void callConstructors()
{
    for(constructor* i = &start_ctors; i != &end_ctors; i++)
        (*i)();
}

extern "C" void kernelMain(const void* multiboot_structure, uint32_t /*multiboot_magic*/)
{
    mboot_hdr = (multiboot_info_t *)multiboot_structure;
    
    if ((mboot_hdr->flags & (1<<6)) == 0) {
        // The memory map is not present, we should probably halt the system
        printf("Error: No Multiboot memory map was provided!\n");
        while(1) {};
    }
    
    verified_mboot_hdr = mboot_hdr;
    mboot_reserved_start = (uint32_t)mboot_hdr;
    mboot_reserved_end = (uint32_t)(mboot_hdr + sizeof(multiboot_info_t));
    next_free_frame = 1;
    framebuffer_addr = (uint32_t*)mboot_hdr->framebuffer_addr;
    
    vga_init((uint32_t*)mboot_hdr->framebuffer_addr, (uint32_t)mboot_hdr->framebuffer_width,
	   (uint32_t)mboot_hdr->framebuffer_height, (uint32_t)mboot_hdr->framebuffer_pitch,
	     (uint8_t)mboot_hdr->framebuffer_bpp);
    
    vga_clear();
    
   
    printf("OS/64 Operating System Starting...\n");

    GlobalDescriptorTable gdt;
    
    // Get start of memory that we can safely work from
    uint32_t* memupper = (uint32_t*)(((size_t)multiboot_structure) + 8);
    
    // Define 10MB of heap space for dynamic memory allocation
    size_t heap = 10*1024*1024;
    MemoryManager memoryManager(heap, (*memupper)*1024 - heap - 10*1024);
    
    printf("\n%d Byte Heap @ 0x%08X", heap);
    printf("\n\n");
    
    // How to use memorymanager to allocate dynamic memory from the heap
    //uint8_t *ram1 = (uint8_t *) memoryManager.malloc(65535);
    //uint8_t* ram1 = new uint8_t[65535];
    //printf("\n64KB RAM BANK 1: %06X", ram1);
       
    TaskManager taskManager;
    
    InterruptManager interrupts(0x20, &gdt, &taskManager);
    SyscallHandler syscalls(&interrupts, 0x80);

    DriverManager drvManager;
    
    IOKeyboardEventHandler kbhandler;
    KeyboardDriver keyboard(&interrupts, &kbhandler);
    drvManager.AddDriver(&keyboard);
    
    SerialEventHandler serialhandler;
    SerialDriver serial(&interrupts, &serialhandler);
    drvManager.AddDriver(&serial);
    
    IOPITEventHandler timerhandler;
    PITDriver pit(&interrupts, &timerhandler);
    drvManager.AddDriver(&pit);
    
    PeripheralComponentInterconnectController PCIController;
    PCIController.SelectDrivers(&drvManager, &interrupts);
    drvManager.ActivateAll();
       
    printf("Initializing interrupts..........[OK]\n");
    interrupts.Activate();
       
    printf("\nATA pri master: ");
    AdvancedTechnologyAttachment ata0m(true, _ATA_FIRST);  
    ata0m.Identify();
    
    printf("\nATA pri slave : ");
    AdvancedTechnologyAttachment ata0s(false, _ATA_FIRST);  
    ata0s.Identify();
    
    printf("\nATA sec master: ");
    AdvancedTechnologyAttachment ata1m(true, _ATA_SECOND);  
    ata1m.Identify();
    
    printf("\nATA sec slave : ");
    AdvancedTechnologyAttachment ata1s(false, _ATA_SECOND);  
    ata1s.Identify();
    
    //uint32_t s1 = 0x0FFFFFFE;
    //s1 = 0;
    //ata0m.WriteSector(s1, (uint8_t*)"LBA48-Z", 7);  
    //ata0m.WriteSector(s1+1, (uint8_t*)"LBA48-V", 7);
    //ata0m.WriteSector(s1+2, (uint8_t*)"LBA48-O", 7);
    
    /*
    uint8_t buffer[512];
    ata0m.ReadSector(s1, buffer); displayMemory(buffer, 16);
    ata0m.ReadSector(s1+1, buffer); displayMemory(buffer, 16);
    ata0m.ReadSector(s1+2, buffer); displayMemory(buffer, 16);
    // ata0m.Read28(0, buffer); displayMemory(buffer, 16);
    
    while(1) {};*/
    
    //ata0m.Write28(268435456, (uint8_t*)"LBA28-0", 7);
    //ata0m.Write28(268435457, (uint8_t*)"LBA48-1", 7);
    //ata0m.Write28(268990000, (uint8_t*)"LBA28-2", 7);
    
    //while(1) {};
    
    //displayMemory(sector, 512);
    
    printf("\n\nInitializing filesystem driver...");
    Fat32 fat32(&ata0m,0);
    printf("[OK]");

    //fat32.ReadPartitions();
    //fat32.ReadDir();
    //fat32.WriteDir((uint8_t*)"12345678",(uint8_t*)"EXT", 32);

    //fat32.ReadFile("COMMAND .COM");
    //fat32.ReadFile("TEST0001.TXT");
    
    //while(1) {};
    
    printf("\nObtaining video mode.............[OK]");
    printf(" Screen %d X %d X %d - Pitch %d", (uint32_t)mboot_hdr->framebuffer_width,
	   (uint32_t)mboot_hdr->framebuffer_height, (uint8_t)mboot_hdr->framebuffer_bpp,
	   (uint32_t)mboot_hdr->framebuffer_pitch);

    SpeakerDriver speaker;

    RTCDriver rtc;
    struct datetime* curDateTime;
    rtc.GetRTC(curDateTime);
    
    printf("\n\nGetting date: %d/%d/%d", curDateTime->month, curDateTime->day, curDateTime->year);
    printf("\nGetting time: %d:%d:%d", curDateTime->hour, curDateTime->minute, curDateTime->second);
    
     
    while(true)
    {
      C64 c64;
      c64ptr = &c64;
      c64ptr->sid_->speaker(&speaker);
      c64ptr->io_->init_display((uint32_t*)mboot_hdr->framebuffer_addr, (uint32_t)mboot_hdr->framebuffer_width,
			  (uint32_t)mboot_hdr->framebuffer_height, (uint32_t)mboot_hdr->framebuffer_pitch, 
			  (uint8_t)mboot_hdr->framebuffer_bpp);
      
      c64ptr->io_->fat32(&fat32);
      c64ptr->io_->serial(&serial);
      c64ptr->io_->rtc(&rtc);
      c64ptr->mon_->fat32(&fat32);
      
      c64.start();
      //TODO: Memory leak possible here.  Calling destructor at the moment causes freeze
    }
    
}


