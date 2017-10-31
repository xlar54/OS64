 
#ifndef __MYOS__DRIVERS__ATA_H
#define __MYOS__DRIVERS__ATA_H

#include <lib/stdint.h>
#include <lib/stdio.h>
#include <hardwarecommunication/interrupts.h>
#include <hardwarecommunication/port.h>

#define IDE_ATA        0x00
#define IDE_ATAPI      0x01

#define _ATA_FIRST		0x1F0	// interrupt 14
#define _ATA_SECOND		0x170	// interrupt 15
#define	_ATA_THIRD		0x1E8
#define _ATA_FOURTH		0x168

#define ATA_CMD_READ_PIO          0x20
#define ATA_CMD_READ_PIO_EXT      0x24
#define ATA_CMD_READ_DMA          0xC8
#define ATA_CMD_READ_DMA_EXT      0x25
#define ATA_CMD_WRITE_PIO         0x30
#define ATA_CMD_WRITE_PIO_EXT     0x34
#define ATA_CMD_WRITE_DMA         0xCA
#define ATA_CMD_WRITE_DMA_EXT     0x35
#define ATA_CMD_CACHE_FLUSH       0xE7
#define ATA_CMD_CACHE_FLUSH_EXT   0xEA
#define ATA_CMD_PACKET            0xA0
#define ATA_CMD_IDENTIFY_PACKET   0xA1
#define ATA_CMD_IDENTIFY          0xEC

#define ATA_SR_BSY     			0x80    // Busy
#define ATA_SR_DRDY    			0x40    // Drive ready
#define ATA_SR_DF      			0x20    // Drive write fault
#define ATA_SR_DSC     			0x10    // Drive seek complete
#define ATA_SR_DRQ     			0x08    // Data request ready
#define ATA_SR_CORR    			0x04    // Corrected data
#define ATA_SR_IDX     			0x02    // Inlex
#define ATA_SR_ERR     			0x01    // Error

#define ATA_IDENT_DEVICETYPE   		0x00
#define ATA_IDENT_CYLINDERS    		0x02
#define ATA_IDENT_HEADS        		0x06
#define ATA_IDENT_SECTORS      		0x0C
#define ATA_IDENT_SERIAL       		0x14
#define ATA_IDENT_MODEL        		0x36
#define ATA_IDENT_CAPABILITIES 		0x62
#define ATA_IDENT_FIELDVALID   		0x6A
#define ATA_IDENT_MAX_LBA      		0x78
#define ATA_IDENT_COMMANDSETS  		0xA4
#define ATA_IDENT_MAX_LBA_EXT  		0xC8


namespace myos
{
    namespace drivers
    {
        
        class AdvancedTechnologyAttachment
        {
        protected:
            bool master;
            hardwarecommunication::Port16Bit dataPort;
            hardwarecommunication::Port8Bit errorPort;
            hardwarecommunication::Port8Bit sectorCountPort;
            hardwarecommunication::Port8Bit lbaLowPort;
            hardwarecommunication::Port8Bit lbaMidPort;
            hardwarecommunication::Port8Bit lbaHiPort;
            hardwarecommunication::Port8Bit devicePort;
            hardwarecommunication::Port8Bit commandPort;
	    
	    hardwarecommunication::Port8Bit sectorCountPort1;
	    hardwarecommunication::Port8Bit lba3;
	    hardwarecommunication::Port8Bit lba4;
	    hardwarecommunication::Port8Bit lba5;
	    
	    hardwarecommunication::Port8Bit regControl;
	    hardwarecommunication::Port8Bit regAltStatus;
	    hardwarecommunication::Port8Bit regDevAddress;
	    
            hardwarecommunication::Port8Bit controlPort;
	    uint8_t lastError;
        public:
            
            AdvancedTechnologyAttachment(bool master, uint16_t portBase);
            ~AdvancedTechnologyAttachment();
            
            void Identify();
            void ReadSector(uint32_t sectorNum, uint8_t* sector, int count = 512);
            int WriteSector(uint32_t sectorNum, uint8_t* data, uint32_t count);           
            
        };
        
    }
}

#endif
