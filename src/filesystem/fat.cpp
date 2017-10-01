
#include <filesystem/fat.h>

using namespace myos;
using namespace myos::filesystem;
using namespace myos::drivers;

Fat32::Fat32(myos::drivers::AdvancedTechnologyAttachment *hd, uint8_t partition)
{
  _hd = hd;
  _partition = partition;
}

Fat32::~Fat32()
{
}

void Fat32::Initialize()
{
  _hd->Read28(0, (uint8_t*)&masterBootRecord, sizeof(MasterBootRecord));
  
  // if this partion is invalid, just exit
  if(masterBootRecord.primaryPartition[_partition].partition_id == 0x00)
    return;
  
  uint32_t partitionOffset;
  partitionOffset = masterBootRecord.primaryPartition[_partition].start_lba;
  _hd->Read28(partitionOffset, (uint8_t*)&biosParameterBlock, sizeof(BiosParameterBlock32));
  
  _fatStart = partitionOffset + biosParameterBlock.reservedSectors;
  _dataStart = _fatStart + biosParameterBlock.sectorsPerFat*biosParameterBlock.fatCopies;
  _rootStart = _dataStart + biosParameterBlock.sectorsPerCluster*(biosParameterBlock.rootCluster-2);
  
  _hd->Read28(_fatStart, _fat, sizeof(_fat));
}

void Fat32::ReadPartitions()
{
  Initialize();
  
  printf("\n\nPartition table\n");
  printf("----------------------------------------------------------\n");
  printf("part # | Bootable | Type |                                \n");
  printf("----------------------------------------------------------\n");
  
  for(int t=0; t<4;t++)
  {
    printf("  %02X       ", t+1);
       
    if(masterBootRecord.primaryPartition[t].bootable == 0x00)
      printf("N");
  
    if(masterBootRecord.primaryPartition[t].bootable == 0x80)
      printf("Y");
    
    printf("         %02X\n", masterBootRecord.primaryPartition[t].partition_id);
  }
  
}

void Fat32::ReadDir()
{
  char volumeLabel[13] = "           \0";
  
  Initialize();

  // set this to filecluster to access a file or subdir
  uint8_t *buffer = ReadNextSectorInChain(biosParameterBlock.rootCluster);
  
  while (_endOfChain == 0)
  {  
    DirectoryEntryFat32 dirent[16];
    //((uint8_t*)&dirent)[0] = *buffer;
    //displayMemory(buffer, 512);
    
    for(int entry=0;entry<16;entry++)
	dirent[entry] = *(DirectoryEntryFat32*)(buffer+(sizeof(DirectoryEntryFat32)*entry));
    
    for(int i=0;i<16;i++)
    {
      if(dirent[i].name[0] == 0x00) continue;//return; 			// end of directory
      if(dirent[i].name[0] == 0xE5) continue;			// deleted file, skip it     
      if(dirent[i].name[0] == 0x05) dirent[i].name[0] = 0xE5;	// japanese char impacted by the delete byte
      
      // bit 3 = volume label
      if((dirent[i].attributes & 0x08) == 0x08)
      {
	for(int j=0;j<8;j++) volumeLabel[j] = dirent[i].name[j];
	for(int j=0;j<3;j++) volumeLabel[j+8] = dirent[i].ext[j];
	printf("\n\nVolume Label: %s\n\n", volumeLabel);
	continue;
      }
      
      printf("\n");
      
      // bit 4 = directories
      if((dirent[i].attributes & 0x10) == 0x10) // directory
	  printf("<");
      
      char fileName[9];
      for(int j=0;j<8;j++) fileName[j] = dirent[i].name[j];fileName[8] = '\0';
      printf(fileName);
      
      // bit 4 = directories
      if((dirent[i].attributes & 0x10) == 0x10) // directory
	  printf(">");
      else
      {
	printf(".");
	char extString[5];
	for(int j=0;j<4;j++) extString[j] = dirent[i].ext[j]; extString[4] = '\0';
	printf(extString);
      }
      
      // Date
      printf("   %d-%d-%d", (dirent[i].wDate>>5) & 0x0f, dirent[i].wDate & 0x1f, 1980+((dirent[i].wDate>>9)& 0x7f));
      
      // Time
      printf(" %d:%d", (dirent[i].wTime>>11 & 0x1f), (dirent[i].wTime>>5) & 0x3f); // hh:mm
      //printf(":%d", (dirent[i].wTime & 0x1f)*2);	//sec
      
      // Get first sector of file
      //uint32_t fileCluster = ((uint32_t)dirent[i].firstClusterHi) << 16 | ((uint32_t)dirent[i].firstClusterLow);
      //printf("     %04X", fileCluster);
      //uint32_t fileSector = dataStart + bpb.sectorsPerCluster * (fileCluster-2);
      //printf("     %d", fileSector);
      
      printf("   ");
      char fsize[10];
      itoa(dirent[i].size, fsize, 10);
      printf("%s",fsize);
    }

    buffer = ReadNextSectorInChain(0);
  }  
}

uint8_t* Fat32::ReadNextSectorInChain(uint32_t startOfChain)
{ 
  static uint8_t buffer[512];
  static uint8_t sectorCtr=0;;
  static uint32_t dataCluster;
  
  if (startOfChain != 0)
  {
    _endOfChain = 0;
    sectorCtr = 0;
    dataCluster = startOfChain;
  }
  //printf("\nFAT start sector = %d", _fatStart);
  //printf("\nData start sector = %d", _dataStart);
  //printf("\nRoot start sector = %d", _rootStart); 
  //printf("\nReadNextSectorInChain: startOfChain: %d, sectorCtr: %d, dataCluster: %d\n",startOfChain,sectorCtr,dataCluster);
  
  uint32_t nextSectorofCluster =  ((dataCluster-2) * biosParameterBlock.sectorsPerCluster)+ _dataStart;
  
  //printf("\n - nextSectorofCluster: %d", nextSectorofCluster);
  
  if (sectorCtr < biosParameterBlock.sectorsPerCluster-1)
  {
    _hd->Read28(nextSectorofCluster+sectorCtr, buffer, sizeof(buffer));
    //displayMemory(buffer, 512);
    
    sectorCtr++;
    return buffer;
  }    
  
  if (sectorCtr == biosParameterBlock.sectorsPerCluster-1)
  {
    // Each FAT32 record is 4 bytes
    dataCluster = (_fat[4 * dataCluster+3] << 24) | (_fat[4 * dataCluster+2] << 16) | 
		  (_fat[4 * dataCluster+1] << 8) | _fat[4 * dataCluster];

    //printf("\n new dataCluster : %d", dataCluster);

    if (dataCluster == ENDOFCLUSTER_FAT32)
    {
      //printf("\nEnd of cluster chain.");
      _endOfChain = 1;
      return 0;
    }
    
    nextSectorofCluster =  ((dataCluster-2) * biosParameterBlock.sectorsPerCluster)+ _dataStart;
    sectorCtr = 0;
    
    _hd->Read28(nextSectorofCluster+sectorCtr, buffer, sizeof(buffer));
    sectorCtr++;
    return buffer;
  }
}

uint32_t Fat32::GetFileSector(char *find)
{
  Initialize();

  // set this to filecluster to access a file or subdir
  uint8_t *buffer = ReadNextSectorInChain(biosParameterBlock.rootCluster);
  
  while (_endOfChain == 0)
  { 
    DirectoryEntryFat32 dirent[16];
   
    for(int entry=0;entry<16;entry++)
	dirent[entry] = *(DirectoryEntryFat32*)(buffer+(sizeof(DirectoryEntryFat32)*entry));
    
    for(int i=0;i<16;i++)
    {
      if(dirent[i].name[0] == 0x00) continue;//return; 		// end of directory
      if(dirent[i].name[0] == 0xE5) continue;			// deleted file, skip it     
      if(dirent[i].name[0] == 0x05) dirent[i].name[0] = 0xE5;	// japanese char impacted by the delete byte
      if((dirent[i].attributes & 0x08) == 0x08) continue;	// volume label
      if((dirent[i].attributes & 0x10) == 0x10) continue;	// directory
      
      char filename[13];
      filename[8] = '.';
      filename[12] = 0;
      
      for(int j=0;j<8;j++) filename[j] = dirent[i].name[j];
      for(int j=0;j<3;j++) filename[j+9] = dirent[i].ext[j]; 
      
      if(!strcmp(filename, find))
      {
	uint32_t retval = ((uint32_t)dirent[i].firstClusterHi) << 16 | ((uint32_t)dirent[i].firstClusterLow);
	return retval;
      }
    }

    buffer = ReadNextSectorInChain(0);
  } 
  
  return 0;
}

void Fat32::ReadFile(char *filename)
{
  Initialize();
  uint32_t startCluster = GetFileSector(filename);
   
  if(startCluster == 0)
    return;

  uint8_t *buffer = ReadNextSectorInChain(startCluster);
  
  while (_endOfChain == 0)
  { 
    displayMemory(buffer, 512);
    buffer = ReadNextSectorInChain(0);
  }  
   
}	
