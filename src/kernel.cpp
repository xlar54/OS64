
#include <lib/stdint.h>
#include <lib/vga.h>
#include <lib/stdio.h>
#include <gdt.h>
#include <memorymanagement.h>
#include <hardwarecommunication/interrupts.h>
#include <syscalls.h>
#include <hardwarecommunication/pci.h>
#include <drivers/driver.h>
#include <drivers/keyboard.h>
#include <drivers/mouse.h>
#include <drivers/ata.h>
#include <multitasking.h>
#include <filesystem/fat.h>
#include <c64/c64.h>


using namespace myos;
using namespace myos::drivers;
using namespace myos::hardwarecommunication;
using namespace myos::filesystem;

// for accessing and sending keyboard or 
// other external data to c64's IO system

C64* c64ptr;	

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
	  vga_set_mode(80,25,16);
	  vga_cursorOn = 1;
	  c64ptr->io_->mon_->Run();
	  return;
	}
	else 
	{ 
	  mode = 0; 
	  vga_set_mode(320,200,8);
	  c64ptr->io_->init_color_palette();
	  return;
	}
      }
      
      switch(mode)
      {
	case 0:
	  c64ptr->io_->OnKeyDown(c);
	  break;
	case 1:
	  c64ptr->io_->mon_->OnKeyDown(c);
	  break;
      }
    }
    
    void OnKeyUp(uint8_t c)
    {
      switch(mode)
      {
	case 0:
	  c64ptr->io_->OnKeyUp(c);
	  break;
	case 1:
	  break;
      }
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
    vga_set_mode(80,25,16);
  
    printf("Emudore 64 Operating System Starting...\n");
 
    GlobalDescriptorTable gdt;
    
    // Get start of memory that we can safely work from
    uint32_t* memupper = (uint32_t*)(((size_t)multiboot_structure) + 8);
    
    // Define 10MB of heap space for dynamic memory allocation
    size_t heap = 10*1024*1024;
    MemoryManager memoryManager(heap, (*memupper)*1024 - heap - 10*1024);
    
    printf("heap: 0x%08X", heap);
    printf("\n\n");
    
    // How to use memorymanager to allocate dynamic memory from the heap
    //uint16_t *ram1 = (uint16_t *) memoryManager.malloc(65535);
    //printf("\n64KB RAM BANK 1: 0x");
    //printfHex(((size_t)ram1 >> 24) & 0xFF);
    //printfHex(((size_t)ram1 >> 16) & 0xFF);
    //printfHex(((size_t)ram1 >> 8 ) & 0xFF);
    //printfHex(((size_t)ram1      ) & 0xFF);
    //
    
    
    TaskManager taskManager;
    
    InterruptManager interrupts(0x20, &gdt, &taskManager);
    SyscallHandler syscalls(&interrupts, 0x80);

    DriverManager drvManager;
    
    IOKeyboardEventHandler kbhandler;
    KeyboardDriver keyboard(&interrupts, &kbhandler);
    drvManager.AddDriver(&keyboard);
    
    PeripheralComponentInterconnectController PCIController;
    PCIController.SelectDrivers(&drvManager, &interrupts);
    drvManager.ActivateAll();
       
    printf("Initializing interrupts..........[OK]\n");
    interrupts.Activate();
       
    printf("\nS-ATA primary master: ");
    AdvancedTechnologyAttachment ata0m(true, _ATA_FIRST);  
    //ata0m.Identify();
    //ata0m.Write28(0, (uint8_t*)"Test", 11);
    //ata0m.Flush();
    
    //displayMemory(sector, 512);
    
    //Fat32 fat32(&ata0m,0);
    //fat32.ReadPartitions();
    //fat32.ReadDir();

    //fat32.ReadFile("COMMAND .COM");
    //fat32.ReadFile("TEST0001.TXT");
    //fat32.ReadPartitions(&ata0m);
    //fat32.ReadDirectory(&ata0m,0);
    
    //while(1) {};
    
    printf("\n\nSetting video mode...............");
    //while(1) {};
    
    //vga_set_mode(640,480,4);
  /*  vga_set_mode(320,240,8);
    vga_gfx_clear(0);
    vga_draw_line(0,0,400,20,15);
    
    while(1) {};
    */
    
    if(vga_set_mode(320,200,8))
    {
      printf("[OK]\n");
      C64 c64;
      c64ptr = &c64;
      c64.start();
    }
    else
    {
      printf("[FAILED]\n");
      while(1) {};
    }
  
    
}


