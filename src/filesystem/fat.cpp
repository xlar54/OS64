
#include <filesystem/fat.h>

using namespace myos;
using namespace myos::filesystem;
using namespace myos::drivers;


void myos::filesystem::ReadBiosParameterBlock(myos::drivers::AdvancedTechnologyAttachment *hd, uint32_t partitionOffset)
{
 
  BiosParameterBlock32 bpb;
  hd->Read28(partitionOffset, (uint8_t*)&bpb, sizeof(BiosParameterBlock32));
 
  printf("\nVolume Label:");
  char foo[12];
  for(int j=0;j<11;j++) foo[j] = bpb.volumeLabel[j]; foo[11] = '\0';
  printf(foo);
  
  printf("\nFilesystem Type:");
  char labelString[9];
  for(int j=0;j<8;j++) labelString[j] = bpb.fatTypeLabel[j]; labelString[9] = '\0';
  printf(labelString);
  printf("\n");


  uint32_t fatStart = partitionOffset + bpb.reservedSectors;
  uint32_t fatSize = bpb.tableSize;

  uint32_t dataStart = fatStart + fatSize*bpb.fatCopies;

  uint32_t rootStart = dataStart + bpb.sectorsPerCluster*(bpb.rootCluster -2);

  DirectoryEntryFat32 dirent[16];
  hd->Read28(rootStart, (uint8_t*)&dirent[0], 16*sizeof(DirectoryEntryFat32));

  for(int i=0;i<16;i++)
  {
    if(dirent[i].name == 0x00)
      break;

    if((dirent[i].attributes & 0x0F) == 0x0F)
	continue;
    
    // bit 3 = volume label
    if((dirent[i].attributes & 0x08) == 0x08) // skip volume label
	continue;
    
    // bit 4 = directories
    if((dirent[i].attributes & 0x10) == 0x10) // directory
	continue;
    
    char fileString[10];
    for(int j=0;j<8;j++) fileString[j] = dirent[i].name[j]; fileString[9] = '\0';
    printf(fileString);
    printf(".");
    
    char extString[5];
    for(int j=0;j<4;j++) extString[j] = dirent[i].ext[j]; extString[4] = '\0';
    printf(extString);
    printf("\n");
    
    // Get first sector of file
    uint32_t fileCluster = ((uint32_t)dirent[i].firstClusterHi) << 16 | ((uint32_t)dirent[i].firstClusterLow);
    uint32_t fileSector = dataStart + bpb.sectorsPerCluster * (fileCluster-2);

    uint32_t buffer[512];

    hd->Read28(fileSector, (uint8_t*)buffer, 512);
    buffer[dirent[i].size] = '\0';
    printf((char *)buffer);
    
  }
		
}

void myos::filesystem::ReadDirectory(myos::drivers::AdvancedTechnologyAttachment *hd, uint8_t partition)
{
  // get the master boot record
  MasterBootRecord mbr;
  hd->Read28(0, (uint8_t*)&mbr, sizeof(MasterBootRecord));
  
  // if this partion is invalid, just exit
  if(mbr.primaryPartition[partition].partition_id == 0x00)
    return;
  
  // Get the bios parameter block
  BiosParameterBlock32 bpb;
  uint32_t partitionOffset;
  partitionOffset = mbr.primaryPartition[partition].start_lba;
  hd->Read28(partitionOffset, (uint8_t*)&bpb, sizeof(BiosParameterBlock32));
  
  uint32_t fatStart = partitionOffset + bpb.reservedSectors;
  uint32_t fatSize = bpb.tableSize;
  uint32_t dataStart = fatStart + fatSize*bpb.fatCopies;
  uint32_t rootStart = dataStart + bpb.sectorsPerCluster*(bpb.rootCluster -2);

  DirectoryEntryFat32 dirent[16];
  hd->Read28(rootStart, (uint8_t*)&dirent[0], 16*sizeof(DirectoryEntryFat32));

  for(int i=0;i<16;i++)
  {
    printf("\n");
    if(dirent[i].name[0] == 0x00)
      break;

    //if((dirent[i].attributes & 0x0F) == 0x0F)
	//continue;
    
    // bit 3 = volume label
    if((dirent[i].attributes & 0x08) == 0x08) // skip volume label
	continue;
    
    // bit 4 = directories
    if((dirent[i].attributes & 0x10) == 0x10) // directory
	printf("<");
    
    char fileString[9];
    for(int j=0;j<8;j++) fileString[j] = dirent[i].name[j]; fileString[8] = '\0';
    printf(fileString);
    
    // bit 4 = directories
    if((dirent[i].attributes & 0x10) == 0x10) // directory
	printf(">");
    else
	printf(".");
    
    char extString[5];
    for(int j=0;j<4;j++) extString[j] = dirent[i].ext[j]; extString[4] = '\0';
    printf(extString);
    
    printf("   ");
    char fsize[10];
    itoa(dirent[i].size, fsize, 10);
    printf(fsize);
    
        
  }
}