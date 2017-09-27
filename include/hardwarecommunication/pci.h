 
#ifndef __MYOS__HARDWARECOMMUNICATION__PCI_H
#define __MYOS__HARDWARECOMMUNICATION__PCI_H

#include <hardwarecommunication/port.h>
#include <drivers/driver.h>
#include <lib/stdint.h>
#include <hardwarecommunication/interrupts.h>

#include <memorymanagement.h>

namespace myos
{
    namespace hardwarecommunication
    {

        enum BaseAddressRegisterType
        {
            MemoryMapping = 0,
            InputOutput = 1
        };
        
        
        
        class BaseAddressRegister
        {
        public:
            bool prefetchable;
            uint8_t* address;
            uint32_t size;
            BaseAddressRegisterType type;
        };
        
        
        
        class PeripheralComponentInterconnectDeviceDescriptor
        {
        public:
            uint32_t portBase;
            uint32_t interrupt;
            
            uint16_t bus;
            uint16_t device;
            uint16_t function;

            uint16_t vendor_id;
            uint16_t device_id;
            
            uint8_t class_id;
            uint8_t subclass_id;
            uint8_t interface_id;

            uint8_t revision;
            
            PeripheralComponentInterconnectDeviceDescriptor();
            ~PeripheralComponentInterconnectDeviceDescriptor();
            
        };


        class PeripheralComponentInterconnectController
        {
            Port32Bit dataPort;
            Port32Bit commandPort;
            
        public:
            PeripheralComponentInterconnectController();
            ~PeripheralComponentInterconnectController();
            
            uint32_t Read(uint16_t bus, uint16_t device, uint16_t function, uint32_t registeroffset);
            void Write(uint16_t bus, uint16_t device, uint16_t function, uint32_t registeroffset, uint32_t value);
            bool DeviceHasFunctions(uint16_t bus, uint16_t device);
            
            void SelectDrivers(myos::drivers::DriverManager* driverManager, myos::hardwarecommunication::InterruptManager* interrupts);
            myos::drivers::Driver* GetDriver(PeripheralComponentInterconnectDeviceDescriptor dev, myos::hardwarecommunication::InterruptManager* interrupts);
            PeripheralComponentInterconnectDeviceDescriptor GetDeviceDescriptor(uint16_t bus, uint16_t device, uint16_t function);
            BaseAddressRegister GetBaseAddressRegister(uint16_t bus, uint16_t device, uint16_t function, uint16_t bar);
        };

    }
}
    
#endif