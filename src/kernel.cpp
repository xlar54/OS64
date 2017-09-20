
#include <common/types.h>
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

static uint16_t* VideoMemory = (uint16_t*)0xb8000;

C64* c64ptr;

void printf(char* str)
{
    static uint8_t x=0,y=0;

    for(int i = 0; str[i] != '\0'; ++i)
    {
        switch(str[i])
        {
            case '\n':
                x = 0;
                y++;
                break;
            default:
                VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0xFF00) | str[i];
                x++;
                break;
        }

        if(x >= 80)
        {
            x = 0;
            y++;
        }

        if(y >= 25)
        {
            for(y = 0; y < 25; y++)
                for(x = 0; x < 80; x++)
                    VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0xFF00) | ' ';
            x = 0;
            y = 0;
        }
    }
}

void printfHex(uint8_t key)
{
    char* foo = "00";
    char* hex = "0123456789ABCDEF";
    foo[0] = hex[(key >> 4) & 0xF];
    foo[1] = hex[key & 0xF];
    printf(foo);
}
void printfHex16(uint16_t key)
{
    printfHex((key >> 8) & 0xFF);
    printfHex( key & 0xFF);
}
void printfHex32(uint32_t key)
{
    printfHex((key >> 24) & 0xFF);
    printfHex((key >> 16) & 0xFF);
    printfHex((key >> 8) & 0xFF);
    printfHex( key & 0xFF);
}

class PrintfKeyboardEventHandler : public KeyboardEventHandler
{
public:
    void OnKeyDown(char c)
    {
        char* foo = " ";
        foo[0] = c;
        printf(foo);
	//vga.PutS(foo);
	if(c == 10) c=13;

	c64ptr->mem_->write_byte(631,c);
	c64ptr->mem_->write_byte(198,1);
	

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

uint16_t* ram1;
uint16_t* ram2;

extern "C" void kernelMain(const void* multiboot_structure, uint32_t /*multiboot_magic*/)
{
    printf("* 6502x86 OS\n");

    GlobalDescriptorTable gdt;
    
    uint32_t* memupper = (uint32_t*)(((size_t)multiboot_structure) + 8);
    size_t heap = 10*1024*1024;
    MemoryManager memoryManager(heap, (*memupper)*1024 - heap - 10*1024);
    
    printf("heap: 0x");
    printfHex((heap >> 24) & 0xFF);
    printfHex((heap >> 16) & 0xFF);
    printfHex((heap >> 8 ) & 0xFF);
    printfHex((heap      ) & 0xFF);
    
    uint16_t *ram1 = (uint16_t *) memoryManager.malloc(65535);
    uint16_t *ram2 = (uint16_t *) memoryManager.malloc(65535);
    printf("\n64KB RAM BANK 1: 0x");
    printfHex(((size_t)ram1 >> 24) & 0xFF);
    printfHex(((size_t)ram1 >> 16) & 0xFF);
    printfHex(((size_t)ram1 >> 8 ) & 0xFF);
    printfHex(((size_t)ram1      ) & 0xFF);
    printf("\n\n");
    
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
    
    //VideoGraphicsArray vga;
    //vga.SetMode(320,200,8);
    //vga.foregroundColor = 0xFFFFFF;
    //vga.backgroundColor = 0x0000A8;
    //vga.Clear();
}


