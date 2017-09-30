 
#ifndef __MYOS__SYSCALLS_H
#define __MYOS__SYSCALLS_H

#include <lib/stdint.h>
#include <lib/stdio.h>
#include <hardwarecommunication/interrupts.h>
#include <multitasking.h>

namespace myos
{
    
    class SyscallHandler : public hardwarecommunication::InterruptHandler
    {
        
    public:
        SyscallHandler(hardwarecommunication::InterruptManager* interruptManager, uint8_t InterruptNumber);
        ~SyscallHandler();
        
        virtual uint32_t HandleInterrupt(uint32_t esp);

    };
    
    
}


#endif