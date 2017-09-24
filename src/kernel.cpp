
#include <common/types.h>
#include <vga.h>
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

C64* c64ptr;


class IOKeyboardEventHandler : public KeyboardEventHandler
{
private:
  uint8_t shift = 0;
public:
    void OnKeyDown(uint8_t c)
    {
        //char* foo = " ";
        //foo[0] = c;
        //printf(foo);

	// PC keycode to petscii translation.  We are just injecting to the keyboard buffer for now.
	switch(c)
	{
	  case 0x01: { c = 0x03; break; } // RUNSTOP
	  case '1' : { if (shift == 1) c = 0x21; break; } // (
	  case '2' : { if (shift == 1) c = 0x40; break; } // (
	  case '3' : { if (shift == 1) c = 0x23; break; } // (
	  case '4' : { if (shift == 1) c = 0x24; break; } // (
	  case '5' : { if (shift == 1) c = 0x25; break; } // (
	  case '6' : { if (shift == 1) c = 0x20; break; } // (
	  case '7' : { if (shift == 1) c = 0x26; break; } // (
	  case '8' : { if (shift == 1) c = 0x2A; break; } // (
	  case '9' : { if (shift == 1) c = 0x28; break; } // (
	  case '0' : { if (shift == 1) c = 0x29; break; } // )
	  case ',' : { if (shift == 1) c = 0x3C; break; } // )
	  case '.' : { if (shift == 1) c = 0x3E; break; } // )
	  case ';' : { if (shift == 1) c = 0x3A; break; } // )
	  case '/' : { if (shift == 1) c = 0x3F; else c= 0x2F; break; } // )
	  case '\'': { if (shift == 1) c = 0x22; break; } // Double Quote
	  case 0x0E: { c = 0x14; break; } // Backspace
	  case 0x0A: { c = 0x0D; break; } // Return
	  case '=':  { if (shift == 1) c = 0x2B; else c= 0x3D; break; } // )
	  //case 0x3B: { c = 0x85; break; } // F1
	  /*case 0x3C: { c = 0x89; break; } // F2
	  case 0x3D: { c = 0x86; break; } // F3
	  case 0x3E: { c = 0x8A; break; } // F4
	  case 0x3F: { c = 0x87; break; } // F5
	  case 0x40: { c = 0x86; break; } // F6
	  case 0x41: { c = 0x8B; break; } // F7
	  case 0x42: { c = 0x8C; break; } // F8*/

	  case 0x13: { if (shift == 1) c = 0x93; else c= 0x13; break; } // home / clr home
	  
	  case 0x2A: { c = 0x00; shift=1; break; }
	  	  
	  // doesnt work and its making me nuts.
	  // Scroll lock key to switch back to text mode
	  //case 0x46: { setTextModeVGA(0); printf("hello"); return; } 
	  
	}
	if(c != 0x00)
	{
	  c64ptr->mem_->write_byte(631,c);
	  c64ptr->mem_->write_byte(198,1);
	}
    }
    
    void OnKeyUp(uint8_t c)
    {
      if(c == 0xaa)
	shift = 0;
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
    
    printf("Starting Emulation...............[OK]\n");

    C64 c64;
    c64ptr = &c64;
    c64.start();

}


