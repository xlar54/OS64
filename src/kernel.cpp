
#include <common/types.h>
#include <printf.h>
#include <gdt.h>
#include <memorymanagement.h>
#include <hardwarecommunication/interrupts.h>
#include <syscalls.h>
#include <hardwarecommunication/pci.h>
#include <drivers/driver.h>
#include <drivers/keyboard.h>
#include <drivers/mouse.h>
#include <drivers/vga.h>
#include <drivers/ata.h>
#include <multitasking.h>
#include <c64/c64.h>


using namespace myos;
using namespace myos::drivers;
using namespace myos::hardwarecommunication;

C64* c64ptr;


int leftShift = 0;

class PrintfKeyboardEventHandler : public KeyboardEventHandler
{
private:

public:
    void OnKeyDown(uint8_t c)
    {
        //char* foo = " ";
        //foo[0] = c;
        //printf(foo);
	//vga.PutS(foo);

	// PC keycode to petscii translation.  We are just injecting to the keyboard buffer for now.
	switch(c)
	{
	  case '9' : { if (leftShift == 1) c = 0x28; break; } // (
	  case '0' : { if (leftShift == 1) c = 0x29; break; } // )
	  case '\'': { if (leftShift == 1) c = 0x22; break; } // Double Quote
	  case 0x08: { c = 0x14; break; } // Backspace
	  case 0x0A: { c = 0x0D; break; } // Return
	  
	  case 0x2A: { leftShift = 1; return; }
	}
	
	c64ptr->mem_->write_byte(631,c);
	c64ptr->mem_->write_byte(198,1);
	

    }
    
    void OnKeyUp(uint8_t c)
    {
      if(c == 0xaa)
      {
	leftShift = 0;
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
    
    // How to use memorymanager to allocate dynamic memory from the heap
    //uint16_t *ram1 = (uint16_t *) memoryManager.malloc(65535);
    //printf("\n64KB RAM BANK 1: 0x");
    //printfHex(((size_t)ram1 >> 24) & 0xFF);
    //printfHex(((size_t)ram1 >> 16) & 0xFF);
    //printfHex(((size_t)ram1 >> 8 ) & 0xFF);
    //printfHex(((size_t)ram1      ) & 0xFF);
    //printf("\n\n");
    
    
    TaskManager taskManager;
    
    InterruptManager interrupts(0x20, &gdt, &taskManager);
    SyscallHandler syscalls(&interrupts, 0x80);

    DriverManager drvManager;
    
    PrintfKeyboardEventHandler kbhandler;
    KeyboardDriver keyboard(&interrupts, &kbhandler);
    drvManager.AddDriver(&keyboard);
    
    PeripheralComponentInterconnectController PCIController;
    PCIController.SelectDrivers(&drvManager, &interrupts);
    drvManager.ActivateAll();
    
    printf("Initializing interrupts..........[OK]\n");
    interrupts.Activate();
    
    printf("Initializing video...............[OK]\n");

    C64 c64;
    c64ptr = &c64;
    c64.start();
    
}


