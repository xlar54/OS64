#include <drivers/ata.h>

using namespace myos;
using namespace myos::drivers;

// Uses PiO (Programmed IO) (slower than DMA)
// Standard IO is 0x01 and 0x06 port and interrupt.  Should get from PCI, but this is mostly standard
// PiO has 28 and 48 bit modes (we use 28 bit)
// Sector is 512 bytes  - 512 x 2^28 = 4GB
// CHS vs LBA address mode.  LBA is newer, doesnt require knowledge of drive geometry

void printf(char* str);
void printfHex(uint8_t);

AdvancedTechnologyAttachment::AdvancedTechnologyAttachment(bool master, uint16_t portBase)
:   dataPort(portBase),				// 16 bit port for data
    errorPort(portBase + 0x1),			// reading error messages
    sectorCountPort(portBase + 0x2),		// tells drive how many sectors to read
    lbaLowPort(portBase + 0x3),			// transmt LBA (logical block address) of sector we want to read (LOW)
    lbaMidPort(portBase + 0x4),			// (MID)
    lbaHiPort(portBase + 0x5),			// (HI)
    devicePort(portBase + 0x6),			// master or slave & part of LBA sector
    commandPort(portBase + 0x7),		// port to send commands
    controlPort(portBase + 0x206)		// status messages
{
    this->master = master;
}

AdvancedTechnologyAttachment::~AdvancedTechnologyAttachment()
{
}
            
void AdvancedTechnologyAttachment::Identify()
{
    devicePort.Write(master ? 0xA0 : 0xB0);	
    controlPort.Write(0);				// clears HOB bit
    
    devicePort.Write(0xA0);				// Reading status from master (not slave)
    uint8_t status = commandPort.Read();
    if(status == 0xFF)					// If 255, no device is on this bus
        return;
    
    
    devicePort.Write(master ? 0xA0 : 0xB0);		// Select which drive to talk to
    sectorCountPort.Write(0);				// How many sectors to read or write
    lbaLowPort.Write(0);				// The sector number
    lbaMidPort.Write(0);				// The sector number
    lbaHiPort.Write(0);					// The sector number
    commandPort.Write(0xEC); 				// Send command (Identify in this case)
    
    
    // Wait until device is ready
    status = commandPort.Read();
    if(status == 0x00)					// If no device, return
        return;
    
    while(((status & 0x80) == 0x80)
       && ((status & 0x01) != 0x01))
        status = commandPort.Read();
        
    if(status & 0x01)
    {
        printf("ERROR");
        return;
    }
    
    // Device is ready.  Perform read/write operation
    for(int i = 0; i < 256; i++)
    {
        uint16_t data = dataPort.Read();
        char *text = "  \0";
        text[0] = (data >> 8) & 0xFF;
        text[1] = data & 0xFF;
        printf(text);
    }
    printf("\n");
}

void AdvancedTechnologyAttachment::Read28(uint32_t sectorNum, int count)
{
    if(sectorNum > 0x0FFFFFFF)
        return;
    
    devicePort.Write( (master ? 0xE0 : 0xF0) | ((sectorNum & 0x0F000000) >> 24) );
    errorPort.Write(0);
    sectorCountPort.Write(1);
    lbaLowPort.Write(  sectorNum & 0x000000FF );
    lbaMidPort.Write( (sectorNum & 0x0000FF00) >> 8);
    lbaHiPort.Write( (sectorNum & 0x00FF0000) >> 16 );
    commandPort.Write(0x20);					// Send write command
    
    // Wait until device is ready
    uint8_t status = commandPort.Read();
    while(((status & 0x80) == 0x80)
       && ((status & 0x01) != 0x01))
        status = commandPort.Read();
        
    if(status & 0x01)
    {
        printf("ERROR");
        return;
    }
    
    
    printf("\n\nReading ATA Drive: ");
    
    for(int i = 0; i < count; i += 2)
    {
        uint16_t wdata = dataPort.Read();
        
        char *text = "  \0";
        text[0] = wdata & 0xFF;
        
        if(i+1 < count)
            text[1] = (wdata >> 8) & 0xFF;
        else
            text[1] = '\0';
        
        printf(text);
    }    
    
    for(int i = count + (count%2); i < 512; i += 2)
        dataPort.Read();
}

void AdvancedTechnologyAttachment::Write28(uint32_t sectorNum, uint8_t* data, uint32_t count)
{
  // You cant write to a sector greater than 28 bits
  if(sectorNum > 0x0FFFFFFF)
      return;
  if(count > 512)
      return;
  
  // Device 0xE0 = master
  // Device 0xF0 = slave
  // 3 ports for data ( 8 bits x 3 = 24), with 4 bits left over

  devicePort.Write( (master ? 0xE0 : 0xF0) | ((sectorNum & 0x0F000000) >> 24) );
  errorPort.Write(0);						// clear previous error messages
  sectorCountPort.Write(1);					// for now, write single sector
  lbaLowPort.Write(  sectorNum & 0x000000FF );			// split sector # into these three ports. Low 8 bits
  lbaMidPort.Write( (sectorNum & 0x0000FF00) >> 8);		// mid 8 bits
  lbaHiPort.Write( (sectorNum & 0x00FF0000) >> 16 );		// high 8 bits
  commandPort.Write(0x30);					// Send write command (0x30)


  printf("\n\nWriting to ATA Drive: ");

  for(int i = 0; i < count; i += 2)
  {
      // Write the data
      uint16_t wdata = data[i];
      if(i+1 < count)
	  wdata |= ((uint16_t)data[i+1]) << 8;
      dataPort.Write(wdata);
      
      // Show the data written to the screen
      char *text = "  \0";
      text[1] = (wdata >> 8) & 0xFF;
      text[0] = wdata & 0xFF;
      printf(text);
  }

  // Device expects to always write a full sector, 
  // otherwise will get an interrupt with an error message
  for(int i = count + (count%2); i < 512; i += 2)
      dataPort.Write(0x0000);

}

void AdvancedTechnologyAttachment::Flush()
{
    devicePort.Write( master ? 0xE0 : 0xF0 );
    commandPort.Write(0xE7);					// flush command

    uint8_t status = commandPort.Read();
    if(status == 0x00)
        return;
    
    // Wait while device is flushing
    while(((status & 0x80) == 0x80)
       && ((status & 0x01) != 0x01))
        status = commandPort.Read();
        
    if(status & 0x01)
    {
        printf("ERROR");
        return;
    }
}
            