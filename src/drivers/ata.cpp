#include <drivers/ata.h>

using namespace myos;
using namespace myos::drivers;

// Uses PiO (Programmed IO) (slower than DMA)
// Standard IO is 0x01 and 0x06 port and interrupt.  Should get from PCI, but this is mostly standard
// PiO has 28 and 48 bit modes (this driver supports both)
// Sector is 512 bytes  - 512 x 2^28 = 4GB
// CHS vs LBA address mode.  LBA is newer, doesnt require knowledge of drive geometry


AdvancedTechnologyAttachment::AdvancedTechnologyAttachment(bool master, uint16_t portBase)
:   dataPort(portBase),				// 16 bit port for data
    errorPort(portBase + 0x1),			// reading error messages
    sectorCountPort(portBase + 0x2),		// tells drive how many sectors to read
    lbaLowPort(portBase + 0x3),			// transmt LBA (logical block address) of sector we want to read (LOW)
    lbaMidPort(portBase + 0x4),			// (MID)
    lbaHiPort(portBase + 0x5),			// (HI)
    devicePort(portBase + 0x6),			// master or slave & part of LBA sector
    commandPort(portBase + 0x7),		// port to send commands
    sectorCountPort1(portBase + 0x8),		
    lba3(portBase + 0x9),
    lba4(portBase + 0xA),
    lba5(portBase + 0xB),
    regControl(portBase + 0xB),
    regAltStatus(portBase + 0xC),
    regDevAddress(portBase + 0xD),
    controlPort(portBase + 0x206),		// status messages
    lastError(0)
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
    if(status == 0xFF || status == 0x00)		// If 255, no device is on this bus
    {
      printf("no device\n");
      return;
    }
    
    devicePort.Write(master ? 0xA0 : 0xB0);		// Select which drive to talk to
    sectorCountPort.Write(0);				// How many sectors to read or write
    lbaLowPort.Write(0);				// The sector number
    lbaMidPort.Write(0);				// The sector number
    lbaHiPort.Write(0);					// The sector number
    commandPort.Write(ATA_CMD_IDENTIFY); 		// Send command (Identify in this case)
   
    // Wait until device is ready
    status = commandPort.Read();
    if(status == 0x00)					// If no device, return
    {
      printf("No device");
      return;
    }

    //while(((status & ATA_SR_BSY) == ATA_SR_BSY) && ((status & ATA_SR_ERR) != ATA_SR_ERR))
    //    status = commandPort.Read();
    uint8_t err = 0;
    
    while(1) 
    {
      status = commandPort.Read();
      if ((status & ATA_SR_ERR)) {err = 1; break;} // If Err, Device is not ATA.
      if (!(status & ATA_SR_BSY) && (status & ATA_SR_DRQ)) break; // Everything is right.
    }
    
    // Get device type
    uint8_t type = IDE_ATA;
    
    if (err != 0) {
      unsigned char cl = lbaMidPort.Read(); //ide_read(i, ATA_REG_LBA1);
      unsigned char ch = lbaHiPort.Read(); //ide_read(i, ATA_REG_LBA2);

      if (cl == 0x14 && ch ==0xEB)
	  type = IDE_ATAPI;
      else if (cl == 0x69 && ch == 0x96)
	  type = IDE_ATAPI;
    }

    // Read the command respose
    status = commandPort.Read();
    
    uint8_t buffer[512];
    for(int x=0;x<512;x++)
      buffer[x] = 0;
    
    int ctr=0;
    for(int i=0; i < 256; i++)
    {
	uint16_t wdata = dataPort.Read();
	uint16_t swap = (wdata << 8) | ((wdata >> 8) & 0x00ff); // swap endianess
	buffer[ctr++] = (swap >> 8) & 0xFF;
	buffer[ctr++] = swap & 0xFF;
    }
    
    // Get serial #
    uint8_t serial[21];
    for(int x=0;x<20;x+=2)
    {
      serial[x] = buffer[ATA_IDENT_SERIAL+x+1];
      serial[x+1] = buffer[ATA_IDENT_SERIAL+x];
    }
    serial[20] = 0;

    // Get model #
    uint8_t model[41];
    for(int x=0;x<40;x+=2)
    {
      model[x] = buffer[ATA_IDENT_MODEL+x+1];
      model[x+1] = buffer[ATA_IDENT_MODEL+x];
    }
    model[40] = 0;
    
    // Get size
    uint32_t commandSets  = *((uint32_t *)(buffer + ATA_IDENT_COMMANDSETS));
    uint32_t size = 0;
        
    if (commandSets & (1 << 26))
      size   = *((uint32_t *)(buffer + ATA_IDENT_MAX_LBA_EXT));  //48-Bit Addressing
    else
      size   = *((uint32_t *)(buffer + ATA_IDENT_MAX_LBA));	// CHS or 28-Bit Addressing
    
    if (type == IDE_ATA) printf("ATA");
    if (type == IDE_ATAPI) printf("ATAPI");
      
    printf("\n       Serial : %s ", &serial);
    printf("\n        Model : %s", &model);
    printf("\n    Size (MB) : %d - ", size*512/1024/1024);
    
    if (commandSets & (1 << 26)) 
      printf("LBA48");
    else 
      printf("LBA28");
    
    //displayMemory(buffer, 256);
}

void AdvancedTechnologyAttachment::ReadSector(uint32_t sectorNum, uint8_t* sector, int count)
{
  uint8_t lbamode, head;
  
  if(sectorNum > 0x0FFFFFFF)
  {
    // LBA48
    lbamode = 2;
    head = 0;
  }
  else
  {
    // LBA28
    lbamode = 1;
    head = ((sectorNum & 0x0F000000) >> 24);
  }
    
  devicePort.Write((master ? 0xE0 : 0xF0) | head);
  errorPort.Write(0);
  
  if(lbamode == 2)
  {
    //printf("\nLBA48");
    // LBA48 mode  
    sectorCountPort.Write(0);  // numsectors >> 8
    lbaLowPort.Write((sectorNum & 0xFF000000) >> 24); 
    lbaMidPort.Write(0);
    lbaHiPort.Write(0);
  }
    
  sectorCountPort.Write(1);					// read one sector
  lbaLowPort.Write(sectorNum & 0x000000FF);
  lbaMidPort.Write((sectorNum & 0x0000FF00) >> 8);
  lbaHiPort.Write((sectorNum & 0x00FF0000) >> 16);

  if(lbamode ==2)
    commandPort.Write(ATA_CMD_READ_PIO_EXT);			// Send read command
  else
    commandPort.Write(ATA_CMD_READ_PIO);			// Send read command
     
  // Wait until device is ready
  uint8_t status = commandPort.Read();
  while(((status & ATA_SR_BSY) == ATA_SR_BSY) && ((status & ATA_SR_ERR) != ATA_SR_ERR))
      status = commandPort.Read();
      
  if(status & ATA_SR_ERR)
  {
      printf("ERROR");
      return;
  }

  int ctr=0;
  for(int i=0; i < 256; i++)
  {
      uint16_t wdata = dataPort.Read();
      uint16_t swap = (wdata << 8) | ((wdata >> 8) & 0x00ff); // swap endianess
      sector[ctr++] = (swap >> 8) & 0xFF;
      sector[ctr++] = swap & 0xFF;
  }
  
  lastError = 0;
  return;

}

int AdvancedTechnologyAttachment::WriteSector(uint32_t sectorNum, uint8_t* data, uint32_t count)
{ 
  
  uint8_t lbamode, head;
  
  if(sectorNum > 0x0FFFFFFF)
  {
    // LBA48
    lbamode = 2;
    head = 0;
  }
  else
  {
    // LBA28
    lbamode = 1;
    head = ((sectorNum & 0x0F000000) >> 24);
  }
  
  if(count > 512)
    return 1;
  
  devicePort.Write((master ? 0xE0 : 0xF0) | head);
  errorPort.Write(0);
  
  if(lbamode == 2)
  {
    //printf("LBA48\n");
    // LBA48 mode  
    sectorCountPort.Write(0); // numsectors >> 8
    lbaLowPort.Write((sectorNum & 0xFF000000) >> 24); 
    lbaMidPort.Write(0);
    lbaHiPort.Write(0);
  }

  sectorCountPort.Write(1);					// for now, write single sector
  lbaLowPort.Write(  sectorNum & 0x000000FF );			// split sector # into these three ports. Low 8 bits
  lbaMidPort.Write( (sectorNum & 0x0000FF00) >> 8);		// mid 8 bits
  lbaHiPort.Write( (sectorNum & 0x00FF0000) >> 16 );		// high 8 bits
  
  if(lbamode ==2)
    commandPort.Write(ATA_CMD_WRITE_PIO_EXT);			// Send write command
  else
    commandPort.Write(ATA_CMD_WRITE_PIO);			// Send write command

  // Wait until device is ready
  uint8_t status = commandPort.Read();
  while(((status & ATA_SR_BSY) == ATA_SR_BSY) && ((status & ATA_SR_ERR) != ATA_SR_ERR))
    status = commandPort.Read();
      
  if(status & ATA_SR_ERR)
    return 2;
   
  // Write the bytes
  for(int i = 0; i < count; i += 2)
  {
    // Write the data
    uint16_t wdata = data[i];
    if(i+1 < count)
	wdata |= ((uint16_t)data[i+1]) << 8;
    dataPort.Write(wdata);
  }

  // Device expects to always write a full sector, 
  // otherwise will get an interrupt with an error message
  for(int i = count + (count%2); i < 512; i += 2)
  {
    //printf("\nnot full sector!");
    dataPort.Write(0x0000);
  }
  
  status = commandPort.Read();
  if(status == 0x00)
    return 3;
  
  // Wait while device is flushing
  while(((status & ATA_SR_BSY) == ATA_SR_BSY) && ((status & ATA_SR_ERR) != ATA_SR_ERR))
    status = commandPort.Read();
  
  if(lbamode == 2)
    commandPort.Write(ATA_CMD_CACHE_FLUSH_EXT);			// flush command
  else
    commandPort.Write(ATA_CMD_CACHE_FLUSH);
    
  status = commandPort.Read();
  if(status == 0x00)
    return 4;
  
  // Wait while device is flushing
  while(((status & ATA_SR_BSY) == ATA_SR_BSY) && ((status & ATA_SR_ERR) != ATA_SR_ERR))
      status = commandPort.Read();
      
  if(status & ATA_SR_ERR)
    return 5;
  
}
        