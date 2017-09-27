
#include <common/types.h>
#include <lib/vga.h>
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
#include <c64/c64.h>


using namespace myos;
using namespace myos::drivers;
using namespace myos::hardwarecommunication;

// for accessing and sending keyboard or 
// other external data to c64's IO system

C64* c64ptr;	

class IOKeyboardEventHandler : public KeyboardEventHandler
{
private:
  uint8_t mode = 0;  // 0 = emulation, 1 = terminal
public:
    void OnKeyDown(uint8_t c)
    {
      // ESC key will toggle between text mode and emulation
      if(c == 0x01) 
      {
	if (mode == 0) 
	{ 
	  mode = 1; 
	  c64ptr->io_->mon_->Run();
	  return;
	}
	else 
	{ 
	  mode = 0; 
	  write_regs(g_320x200x256);
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
    setTextModeVGA(0);
    printf("Emudore 64 Operating System Starting...\n");

    GlobalDescriptorTable gdt;
    
    // Get start of memory that we can safely work from
    uint32_t* memupper = (uint32_t*)(((size_t)multiboot_structure) + 8);
    
    // Define 10MB of heap space for dynamic memory allocation
    size_t heap = 10*1024*1024;
    MemoryManager memoryManager(heap, (*memupper)*1024 - heap - 10*1024);
    
    printf("heap: 0x");
    printfHex((heap >> 24) & 0xFF);
    printfHex((heap >> 16) & 0xFF);
    printfHex((heap >> 8 ) & 0xFF);
    printfHex((heap      ) & 0xFF);
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
    
    // 0x1F0 = primary		interrupt 14
    // 0x170 = secondary	interrupt 15
    // 0x1E8 = third
    // 0x168 = fourth
    
    printf("\nS-ATA primary master: ");
    AdvancedTechnologyAttachment ata0m(true, 0x1F0);  
    //ata0m.Identify();
  
    //ata0m.Write28(0, (uint8_t*)"Test", 11);
    //ata0m.Flush();
    //ata0m.Read28(0);
    
    printf("Starting Emulation...............[OK]\n");
   
    //while(1) {};
    
    C64 c64;
    c64ptr = &c64;
    c64.start();

}


