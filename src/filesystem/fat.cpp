
#include <filesystem/fat.h>

using namespace myos;
using namespace myos::filesystem;
using namespace myos::drivers;

Fat32::Fat32(myos::drivers::AdvancedTechnologyAttachment *hd, uint8_t partition)
{
  _hd = hd;
  _partition = partition;
  _hd->ReadSector(0, (uint8_t*)&_mbr, sizeof(MasterBootRecord));
  
    // if this partion is invalid, just exit
  if(_mbr.primaryPartition[_partition].partition_id == 0x00)
    return;
  
  uint32_t partitionOffset = _mbr.primaryPartition[_partition].start_lba;
  _hd->ReadSector(partitionOffset, (uint8_t*)&_bpb, sizeof(BiosParameterBlock32));
  
  _fatStart = partitionOffset + _bpb.reservedSectors;
  _dataStart = _fatStart + _bpb.sectorsPerFat * _bpb.fatCopies;
  _rootStart = _dataStart + _bpb.sectorsPerCluster * (_bpb.rootCluster-2);
  
  _fatBuffer = new uint8_t[_bpb.sectorsPerFat * _bpb.bytesPerSector];  
  _fat = &_fatBuffer[0];
  
  for(int x=0;x<256;x++)
  {
    openFilesList[x].mode = FILEACCESSMODE_CLOSED;
    for(int x1=0;x1<8;x1++) openFilesList[x].filename[x1] = 0; 
    for(int x1=0;x1<3;x1++) openFilesList[x].ext[x1] = 0;
    openFilesList[x].size = 0;
    openFilesList[x].locationPtr = 0;
    openFilesList[x].startingCluster = 0;
    openFilesList[x].buffer = 0;
  }
  
  LoadFAT();
  
}

Fat32::~Fat32()
{
  delete[] _fatBuffer;
}

void Fat32::ReadPartitions()
{ 
  printf("\n\nPartition table\n");
  printf("----------------------------------------------------------\n");
  printf("Part # | Bootable | Type |                                \n");
  printf("----------------------------------------------------------\n");
  
  for(int t=0; t<4;t++)
  {
    printf("  %02X       ", t+1);
       
    if(_mbr.primaryPartition[t].bootable == 0x00)
      printf("N");
  
    if(_mbr.primaryPartition[t].bootable == 0x80)
      printf("Y");
    
    printf("         %02X\n", _mbr.primaryPartition[t].partition_id);
  }
  
    
  printf("\nFAT Start          : %06X", _fatStart);
  printf("\nData Start         : %06X", _dataStart);
  printf("\nRoot cluster       : %06X", _bpb.rootCluster);
  printf("\nRoot directory     : %06X", _rootStart); 
}

void Fat32::LoadFAT()
{
  // Read FAT (should be using placement new here?)
  for(int counter = 0; counter < _bpb.sectorsPerFat; counter++)
  {
    _hd->ReadSector(_fatStart + counter, &_fatBuffer[counter*_bpb.bytesPerSector], _bpb.bytesPerSector);
    
    // Dont bother reading more FAT entries if at end (0)
    if(_fatBuffer[counter*_bpb.bytesPerSector] == 0)
      return;
  }
}

void Fat32::ReadDirectory(uint32_t startCluster)
{
  char volumeLabel[13] = "           \0";
  
  _endOfChain = 0;

  if(startCluster == 0)
    startCluster = _bpb.rootCluster;
  
  // set this to filecluster to access a file or subdir
  uint8_t *buffer = ReadNextSectorInChain(startCluster);
  
  while (_endOfChain == 0)
  {  
    DirectoryEntryFat32 dirent[16];
    ((uint8_t*)&dirent)[0] = *buffer;
    //displayMemory(buffer, 512);
    //while(1) {};
    
    for(int entry=0;entry<16;entry++)
	dirent[entry] = *(DirectoryEntryFat32*)(buffer+(sizeof(DirectoryEntryFat32)*entry));
    
    for(int i=0;i<16;i++)
    {
      if(dirent[i].name[0] == 0x00) return; 			// end of directory
      if(dirent[i].name[0] == 0xE5) continue;			// deleted file, skip it     
      if(dirent[i].name[0] == 0x05) dirent[i].name[0] = 0xE5;	// japanese char impacted by the delete byte
      
      // bit 3 = volume label
      if((dirent[i].attributes & 0x08) == 0x08)
      {
	for(int j=0;j<8;j++) volumeLabel[j] = dirent[i].name[j];
	for(int j=0;j<3;j++) volumeLabel[j+8] = dirent[i].ext[j];
	printf("\n\nVolume Label: %s\n", volumeLabel);
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
      uint32_t fileCluster = ((uint32_t)dirent[i].firstClusterHi) << 16 | ((uint32_t)dirent[i].firstClusterLow);
      printf("     %06X", fileCluster);
      uint32_t fileSector = _dataStart + _bpb.sectorsPerCluster * (fileCluster-2);
      printf("     %06X", fileSector);
      
      printf("   ");
      char fsize[10];
      itoa(dirent[i].size, fsize, 10);
      printf("  %s",fsize);
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
  
  uint32_t nextSectorofCluster =  ((dataCluster-2) * _bpb.sectorsPerCluster)+ _dataStart;
  
  //printf("\n - nextSectorofCluster: %d", nextSectorofCluster);
  
  // read contiguous sectors until we reach the # of sectors per cluster
  if (sectorCtr < _bpb.sectorsPerCluster-1)
  {
    _lastSectorRead = nextSectorofCluster + sectorCtr;
    _hd->ReadSector(_lastSectorRead, buffer, sizeof(buffer));
    //displayMemory(buffer, 512);
    sectorCtr++;
    return buffer;
  }    
  
  // we are at the end of sectors per cluster.  
  // now we need to get the next cluster location from the fat chain
  if (sectorCtr == _bpb.sectorsPerCluster-1)
  {
    // Each FAT32 record is 4 bytes
    dataCluster = (_fat[4 * dataCluster+3] << 24) | (_fat[4 * dataCluster+2] << 16) | 
		  (_fat[4 * dataCluster+1] << 8) | _fat[4 * dataCluster];

    //printf("\n new dataCluster : %d", dataCluster);

    if (dataCluster >= ENDOFCLUSTER_FAT32)
    {
      //printf("\nEnd of cluster chain.");
      _endOfChain = 1;
      return 0;
    }
    
    sectorCtr = 0;
    nextSectorofCluster =  ((dataCluster-2) * _bpb.sectorsPerCluster)+ _dataStart;
    _lastSectorRead = nextSectorofCluster + sectorCtr;
    
    _hd->ReadSector(_lastSectorRead, buffer, sizeof(buffer));
    
    sectorCtr++;
    return buffer;
  }
}

uint32_t Fat32::GetFileCluster(uint8_t* find)
{
  _endOfChain = 0;

  // set this to filecluster to access a file or subdir
  uint8_t *buffer = ReadNextSectorInChain(_bpb.rootCluster);
  
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
      
      if(!strcmp(filename, (char*)find))
      {
	uint32_t retval = ((uint32_t)dirent[i].firstClusterHi) << 16 | ((uint32_t)dirent[i].firstClusterLow);
	return retval;
      }
    }

    buffer = ReadNextSectorInChain(0);
  } 
  
  return 0;
}

uint32_t Fat32::GetFileSize(uint8_t* find)
{
   _endOfChain = 0;

  // set this to filecluster to access a file or subdir
  uint8_t *buffer = ReadNextSectorInChain(_bpb.rootCluster);
  
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
      
      if(!strcmp(filename, (char *)find))
      {
	return dirent[i].size;
      }
    }

    buffer = ReadNextSectorInChain(0);
  } 
  
  return 0;
}

int Fat32::OpenFile(uint8_t filenumber, uint8_t* filename, uint8_t mode)
{
  if(mode == FILEACCESSMODE_READ)
  {
    if(openFilesList[filenumber].mode != FILEACCESSMODE_CLOSED)
      return FILE_STATUS_FILEOPEN; // file already open
    
    uint32_t fileCluster = GetFileCluster(filename);
    
    if(fileCluster == 0)
      return FILE_STATUS_NOTFOUND; // file not found
      
    uint32_t size = GetFileSize(filename);
    uint8_t* buffer = new uint8_t[size];
    
    ReadFile(filename, buffer, size);
    
    openFilesList[filenumber].mode = FILEACCESSMODE_READ;
    
    uint8_t file8[8];
    uint8_t ext[3];
    ParseFilename(filename, file8, ext);   
    for(int x=0;x<8;x++) openFilesList[filenumber].filename[x] = file8[x]; 
    for(int x=0;x<3;x++) openFilesList[filenumber].ext[x] = ext[x];
       
    openFilesList[filenumber].size = size;
    openFilesList[filenumber].locationPtr = 0;
    openFilesList[filenumber].startingCluster = fileCluster;
    openFilesList[filenumber].buffer = buffer;
        
    return FILE_STATUS_OK;	// OK
  }
  
  if(mode == FILEACCESSMODE_CREATE)
  {
    if(openFilesList[filenumber].mode != FILEACCESSMODE_CLOSED)
      return FILE_STATUS_FILEOPEN; // file already open
  
    uint32_t fileCluster = GetFileCluster(filename);
    
    if(fileCluster != 0)
      return FILE_STATUS_FILEEXISTS;
    
    // create a buffer of at least one cluster size
    uint32_t size = _bpb.sectorsPerCluster * 512;
    uint8_t* buffer = new uint8_t[size];

    openFilesList[filenumber].mode = FILEACCESSMODE_WRITE;
    
    uint8_t file8[8], ext[3];
    ParseFilename(filename, file8, ext);   
    for(int x=0;x<8;x++) openFilesList[filenumber].filename[x] = file8[x]; 
    for(int x=0;x<3;x++) openFilesList[filenumber].ext[x] = ext[x];
    
    openFilesList[filenumber].size = 0;
    openFilesList[filenumber].locationPtr = 0;
    openFilesList[filenumber].startingCluster = 0;
    openFilesList[filenumber].buffer = buffer;
    
    CreateDirectoryEntry(filename, ext, 0);
    
    return FILE_STATUS_OK;

  }
}

int Fat32::CloseFile(uint8_t filenumber)
{
  if(openFilesList[filenumber].mode == FILEACCESSMODE_READ)
  {       
    ResetOpenFileListEntry(filenumber);
    return FILE_STATUS_OK;
  }
  
  if(openFilesList[filenumber].mode == FILEACCESSMODE_WRITE)
  {
    FlushWriteBuffer(filenumber);
    
    openFilesList[filenumber].size--;

    UpdateDirectoryEntry(openFilesList[filenumber].filename, openFilesList[filenumber].ext, 
			 openFilesList[filenumber].size, openFilesList[filenumber].startingCluster);
    
    ResetOpenFileListEntry(filenumber);
    return FILE_STATUS_OK;
  }
  
  return FILE_STATUS_FILECLSD;
}

int Fat32::FlushWriteBuffer(uint8_t filenumber)
{
  // Allocate a cluster, write it to disk, then reset the buffer for more data
  
  // Find a free cluster
  uint32_t freeCluster = 0;
  int status = AllocateCluster(&freeCluster);
  
  if(status != FILE_STATUS_OK)
    return status;

  if (openFilesList[filenumber].startingCluster == 0)
    openFilesList[filenumber].startingCluster = freeCluster;

  uint32_t sector = _dataStart + _bpb.sectorsPerCluster * (freeCluster-2);

  _hd->WriteSector(sector, openFilesList[filenumber].buffer, 512);
  
  // reset pointer to start of buffer
  openFilesList[filenumber].locationPtr = 0;

  return FILE_STATUS_OK;
}

int Fat32::ParseFilename(uint8_t* filename, uint8_t* file8, uint8_t* ext)
{
  uint8_t ctr = 0;
  
  if (strlen((char*)filename) > 12)
    return FILE_STATUS_NOTFOUND;
  
  for(int x=0; x<8; x++) file8[x] = ' ';
  for(int x=0; x<3; x++) ext[x] = ' ';
  
  const char *period = strchr((char*)filename, (int)'.');
  
  if (period == NULL)
    return FILE_STATUS_NOTFOUND;
  
  int ploc = period - (char*)filename;

  if (ploc != strlen((char*)filename)-4 || ploc == 0)
    return FILE_STATUS_NOTFOUND;
 
  for(int x=0; x<ploc; x++)
    file8[x] = filename[x];
  
  for(int x=0; x<3; x++)
    ext[x] = filename[ploc+1+x];
  
  //printf("\n%c%c%c%c%c%c%c%c!",file8[0],file8[1],file8[2],file8[3],file8[4],file8[5],file8[6],file8[7]);
  //printf("\n%c%c%c!",ext[0],ext[1],ext[2]);
}

void Fat32::ResetOpenFileListEntry(uint8_t filenumber)
{
    openFilesList[filenumber].mode = FILEACCESSMODE_CLOSED;
    for(int x=0;x<8;x++) openFilesList[filenumber].filename[x] = 0; 
    for(int x=0;x<3;x++) openFilesList[filenumber].ext[x] = 0;
    openFilesList[filenumber].size = 0;
    openFilesList[filenumber].locationPtr = 0;
    openFilesList[filenumber].startingCluster = 0;
    delete [] openFilesList[filenumber].buffer;
}

int Fat32::ReadNextFileByte(uint8_t filenumber, uint8_t* b)
{
  if(openFilesList[filenumber].mode != FILEACCESSMODE_READ)
    return FILE_STATUS_FILECLSD;
  
  if(openFilesList[filenumber].locationPtr == openFilesList[filenumber].size)
    return FILE_STATUS_EOF;
  
  *b = openFilesList[filenumber].buffer[openFilesList[filenumber].locationPtr++];
  
  return FILE_STATUS_OK;	// OK
  
}

void Fat32::ReadFile(uint8_t *filename, uint8_t* data, uint32_t size)
{
  _endOfChain = 0;
  uint32_t startCluster = GetFileCluster(filename);
  
  if(startCluster == 0)
    return;
  
  uint8_t *buffer = ReadNextSectorInChain(startCluster);
  uint32_t byteCtr = 0;
  
  for(int x=0;x<512;x++)
  {
    *data++ = buffer[x];
    byteCtr++;

    if(byteCtr == size)
      return;
  }
  
  while (_endOfChain == 0)
  { 
    //displayMemory(buffer, 512);
    buffer = ReadNextSectorInChain(0);
    
    for(int x=0;x<512;x++)
    {
      *data++ = buffer[x];
      byteCtr++;
      
      if(byteCtr == size)
	return;
    }
  }  
   
}	

void Fat32::ReadSector(uint32_t sector, uint8_t *buffer)
{
  _lastSectorRead = sector;
  _hd->ReadSector(_lastSectorRead, buffer, 512);
}

int Fat32::AllocateCluster(uint32_t* startingCluster)
{
    // Find a free cluster
    // traverse entire FAT cluster, looking for free cluster

    for(int sectorCtr=0; sectorCtr < _bpb.sectorsPerFat; sectorCtr++)
    {
      for(int x=0;x<_bpb.bytesPerSector;x+=4)
      {
	int ptr = (sectorCtr * _bpb.bytesPerSector) + x;
	//uin32_t i32 = _fatBuffer[0] | (_fatBuffer[1] << 8) | (_fatBuffer[2] << 16) | (_fatBuffer[3] << 24);
	if(_fatBuffer[ptr] == 0 && _fatBuffer[ptr+1] == 0 && _fatBuffer[ptr+2] == 0 && _fatBuffer[ptr+3] == 0)
	{
	  // Mark this as the last cluster.  Expansion happens elsewhere
	  _fatBuffer[ptr+0] = 0xFF;  _fatBuffer[ptr+1] = 0xFF;
	  _fatBuffer[ptr+2] = 0xFF;  _fatBuffer[ptr+3] = 0xFF;
	  
	  *startingCluster = ptr/4;
	  _hd->WriteSector(_fatStart + sectorCtr, &_fatBuffer[sectorCtr*_bpb.bytesPerSector], _bpb.bytesPerSector);
	  
	  return FILE_STATUS_OK;
	}
      }

    }
    
    return FILE_STATUS_DISKFULL;
}

int Fat32::WriteNextFileByte(uint8_t filenumber, uint8_t b)
{
  if(openFilesList[filenumber].mode != FILEACCESSMODE_WRITE)
    return FILE_STATUS_FILECLSD;
  
  openFilesList[filenumber].buffer[openFilesList[filenumber].locationPtr] = b;
  openFilesList[filenumber].locationPtr++;
  openFilesList[filenumber].size++;
  
  if(openFilesList[filenumber].locationPtr > _bpb.sectorsPerCluster * 512)
  {
    FlushWriteBuffer(filenumber);
  }
  
  return FILE_STATUS_OK;
}

uint8_t* Fat32::GetCBMDir()
{
  static uint8_t buf[1000];
  
  for(int t=0;t<1000;t++)
    buf[t] = 0;
  
  uint16_t startOfBasic = 0x0801;
  uint16_t nextLinePtr = 30;
  
  buf[0] = (startOfBasic + nextLinePtr) & 0xff;		// next link lo
  buf[1] = ((startOfBasic + nextLinePtr) >> 8) & 0xff;	// next link hi
  
  buf[2] = 0x00;	// line num lo
  buf[3] = 0x00;	// line num hi
  buf[4] = 0x12;	// reverse
  buf[5] = 0x22;	// quote
  buf[6] = 0x20;	// space
  buf[7] = 0x20;	// space
  buf[8] = 0x20;	// space
  buf[9] = 0x20;	// space
  buf[10] = 0x20;	// space
  buf[11] = 0x20;	// space
  buf[12] = 0x20;	// space
  buf[13] = 0x20;	// space
  buf[14] = 0x20;	// space
  buf[15] = 0x20;	// space
  buf[16] = 0x20;	// space
  buf[17] = 0x20;	// space
  buf[18] = 0x20;	// space
  buf[19] = 0x20;	// space
  buf[20] = 0x20;	// space
  buf[21] = 0x20;	// space
  buf[22] = 0x22;	// quote
  buf[23] = 0x20;	// space
  buf[24] = 0x30;	// zero
  buf[25] = 0x30;	// zero
  buf[26] = 0x20;	// space
  buf[27] = 0x32;	// 2
  buf[28] = 0x41;	// A
  buf[29] = 0x00;	// EOL
 
  // set this to filecluster to access a file or subdir
  uint8_t *buffer = ReadNextSectorInChain(_bpb.rootCluster);
  
  while (_endOfChain == 0)
  {  
    DirectoryEntryFat32 dirent[16];
    
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
	char volumeLabel[12] = "           ";
	for(int j=0;j<8;j++) volumeLabel[j] = dirent[i].name[j];
	for(int j=0;j<3;j++) volumeLabel[j+8] = dirent[i].ext[j];
	
	for(int jz=6;jz<17;jz++)
	  buf[jz]= volumeLabel[jz-6];
	
	continue;
      }

      int tempNextLinePtr = nextLinePtr;
      buf[nextLinePtr++] = 0x00; // next link lo
      buf[nextLinePtr++] = 0x00; // next link hi
      
      // file size / line num 
      int size = (dirent[i].size + 254) / 254;
      buf[nextLinePtr++] = size & 0xff; // 0x01; // line num lo
      buf[nextLinePtr++] = (size >> 8) & 0xff; //0x00; // line num hi

      buf[nextLinePtr++] = 0x20; // space
      buf[nextLinePtr++] = 0x20; // space
      buf[nextLinePtr++] = 0x20; // space
      
      if(size > 10) nextLinePtr--;
      if(size > 100) nextLinePtr--;
      
      // file name
      buf[nextLinePtr++] = 0x22; // quote
      
      for(int j=0;j<8;j++)
	buf[nextLinePtr++] = dirent[i].name[j];
      
      buf[nextLinePtr++] = 0x22; // quote

      buf[nextLinePtr++] = 0x20; // space
      buf[nextLinePtr++] = 0x20; // space
      buf[nextLinePtr++] = 0x20; // space
      buf[nextLinePtr++] = 0x20; // space
      buf[nextLinePtr++] = 0x20; // space
      buf[nextLinePtr++] = 0x20; // space
      buf[nextLinePtr++] = 0x20; // space
      buf[nextLinePtr++] = 0x20; // space
      buf[nextLinePtr++] = 0x20; // space
      
      if((dirent[i].attributes & 0x10) == 0x10)
      {
	buf[nextLinePtr++] = 0x44; // D
	buf[nextLinePtr++] = 0x49; // I
	buf[nextLinePtr++] = 0x52; // R
      }
      else
      {
	for(int j=0;j<4;j++)
	  buf[nextLinePtr++] = dirent[i].ext[j];
      }
      
      buf[nextLinePtr++] = 0x00; // EOL
      
      // fix next line link ptr
      buf[tempNextLinePtr] = (startOfBasic + nextLinePtr) & 0xff;		// next link lo
      buf[tempNextLinePtr+1] = ((startOfBasic + nextLinePtr+1) >> 8) & 0xff;	// next link hi
      
    }

    buffer = ReadNextSectorInChain(0);
  } 
  
  int tempNextLinePtr = nextLinePtr;
  
  buf[nextLinePtr++] = 0x00; // next link lo
  buf[nextLinePtr++] = 0x00; // next link hi
  
  buf[nextLinePtr++] = 0x00; // zero
  buf[nextLinePtr++] = 0x00; // zero

  buf[nextLinePtr++] = 0x42; // B
  buf[nextLinePtr++] = 0x4C; // L
  buf[nextLinePtr++] = 0x4F; // O
  buf[nextLinePtr++] = 0x43; // C
  buf[nextLinePtr++] = 0x4B; // K
  buf[nextLinePtr++] = 0x53; // S
  buf[nextLinePtr++] = 0x20; // space
  buf[nextLinePtr++] = 0x46; // F
  buf[nextLinePtr++] = 0x52; // R
  buf[nextLinePtr++] = 0x45; // E
  buf[nextLinePtr++] = 0x45; // E
  buf[nextLinePtr++] = 0x2E; // .
  buf[nextLinePtr++] = 0x00; // EOL
  
  // fix next line link ptr
  buf[tempNextLinePtr] = (startOfBasic + nextLinePtr) & 0xff;		// next link lo
  buf[tempNextLinePtr+1] = ((startOfBasic + nextLinePtr+1) >> 8) & 0xff;	// next link hi
	
	
 return buf;
}

void Fat32::WriteDir(uint8_t* filename, uint8_t* ext, uint32_t size)
{
  _endOfChain = 0;

  // set this to filecluster to access a file or subdir
  uint8_t *buffer = ReadNextSectorInChain(_bpb.rootCluster);

  while (_endOfChain == 0)
  {  
    DirectoryEntryFat32 dirent[16];
    
    for(int entry=0;entry<16;entry++)
	dirent[entry] = *(DirectoryEntryFat32*)(buffer+(sizeof(DirectoryEntryFat32)*entry));
    
    for(int i=0;i<16;i++)
    {
      if(dirent[i].name[0] == 0x00)
      {
	uint8_t *ptr;
	ptr = &buffer[i*sizeof(DirectoryEntryFat32)];
	
	// clear sector
	
	/*for(int z=0;z<sizeof(buffer);z++)
	  buffer[z] = 0;
	
	_hd->Write28(_lastSectorRead, buffer, sizeof(buffer));
	_hd->Flush();
	
	displayMemory(buffer, 256);
	return;*/
	
	// Write filename
	int ctr = 0;
	while(ctr < 8)
	{
	  *ptr = filename[ctr++];
	  ptr++;
	}

	// Write ext
	ctr = 0;
	while(ctr < 3)
	{
	  *ptr = ext[ctr++];
	  ptr++;
	}
	
	*ptr++ = 0x20;  // Write attributes - not sure why, but other OS does this (archive bit)
	*ptr++ = 0x00;	// Write reserved
	*ptr++ = 0x00;	// Write cTimeTenth - unused by FAT32
	*ptr++ = 0x00;	// Write cTime - unused by FAT32
	*ptr++ = 0x00;
	*ptr++ = 0x00;	// Write cDate - unused by FAT32
	*ptr++ = 0x00;
	*ptr++ = 0x00;	// Write aTime - unused by FAT32
	*ptr++ = 0x00;
	
	*ptr++ = 0xF0;	// Write starting cluster hi
	*ptr++ = 0xF0;

	*ptr++ = 0xC7;// Write time
	*ptr++ = 0x83;

	*ptr++ = 0x3D;// Write date
	*ptr++ = 0x4B;
	
	*ptr++ = 0xF1;// Write starting cluster lo
	*ptr++ = 0xF1;
	
	*ptr++ = (size >> (8*0)) & 0xff;	// Write file size
	*ptr++ = (size >> (8*1)) & 0xff;
	*ptr++ = (size >> (8*2)) & 0xff;
	*ptr = (size >> (8*3)) & 0xff;
	
	//displayMemory(buffer, 256);
	
	_hd->WriteSector(_lastSectorRead, buffer, 512);
	return;
      }

    }

    buffer = ReadNextSectorInChain(0);
  }  
}

int Fat32::UpdateDirectoryEntry(uint8_t* filename, uint8_t* ext, uint32_t size, uint32_t startingCluster)
{
  _endOfChain = 0;

  // set this to filecluster to access a file or subdir
  uint8_t *buffer = ReadNextSectorInChain(_bpb.rootCluster);

  while (_endOfChain == 0)
  {  
    DirectoryEntryFat32 dirent[16];
    
    for(int entry=0;entry<16;entry++)
	dirent[entry] = *(DirectoryEntryFat32*)(buffer+(sizeof(DirectoryEntryFat32)*entry));
    
    for(int i=0;i<16;i++)
    {
      if(dirent[i].name[0] == filename[0] 
	&& dirent[i].name[1] == filename[1]
	&& dirent[i].name[2] == filename[2] 
	&& dirent[i].name[3] == filename[3]
	&& dirent[i].name[4] == filename[4]
	&& dirent[i].name[5] == filename[5]
	&& dirent[i].name[6] == filename[6]
	&& dirent[i].name[7] == filename[7]
	&& dirent[i].ext[0] == ext[0]
	&& dirent[i].ext[1] == ext[1]
	&& dirent[i].ext[2] == ext[2]
      )
      {
	uint8_t *ptr;
	ptr = &buffer[i*sizeof(DirectoryEntryFat32)];
		
	ptr[21] = (startingCluster >> 16) & 0xff;	// file cluster hi
	ptr[22] = (startingCluster >> 24) & 0xff;
	ptr[26] = startingCluster & 0xff;		// file cluster lo
	ptr[27] = (startingCluster >> 8) & 0xff;	
	ptr[28] = (size >> (8*0)) & 0xff;	// Write file size
	ptr[29] = (size >> (8*1)) & 0xff;
	ptr[30] = (size >> (8*2)) & 0xff;
	ptr[31] = (size >> (8*3)) & 0xff;

	_hd->WriteSector(_lastSectorRead, buffer, 512);
	
	return FILE_STATUS_OK;
      }

    }

    buffer = ReadNextSectorInChain(0);
  } 
  
  return FILE_STATUS_NOTFOUND;
}

void Fat32::CreateDirectoryEntry(uint8_t* filename, uint8_t* ext, uint32_t size)
{
  _endOfChain = 0;

  // set this to filecluster to access a file or subdir
  uint8_t *buffer = ReadNextSectorInChain(_bpb.rootCluster);

  while (_endOfChain == 0)
  {  
    DirectoryEntryFat32 dirent[16];
    
    for(int entry=0;entry<16;entry++)
	dirent[entry] = *(DirectoryEntryFat32*)(buffer+(sizeof(DirectoryEntryFat32)*entry));
    
    for(int i=0;i<16;i++)
    {
      if(dirent[i].name[0] == 0x00)
      {
	uint8_t *ptr;
	ptr = &buffer[i*sizeof(DirectoryEntryFat32)];
	
	// Write filename
	int ctr = 0;
	while(ctr < 8)
	{
	  *ptr = filename[ctr++];
	  ptr++;
	}

	// Write ext
	ctr = 0;
	while(ctr < 3)
	{
	  *ptr = ext[ctr++];
	  ptr++;
	}
	
	*ptr++ = 0x20;  // Write attributes - not sure why, but other OS does this (archive bit)
	*ptr++ = 0x00;	// Write reserved
	*ptr++ = 0x00;	// Write cTimeTenth - unused by FAT32
	*ptr++ = 0x00;	// Write cTime - unused by FAT32
	*ptr++ = 0x00;
	*ptr++ = 0x00;	// Write cDate - unused by FAT32
	*ptr++ = 0x00;
	*ptr++ = 0x00;	// Write aTime - unused by FAT32
	*ptr++ = 0x00;
	
	//*ptr++ = (startingCluster >> 16) & 0xff;	// file cluster hi
	//*ptr++ = (startingCluster >> 24) & 0xff;
	*ptr++ = 0;
	*ptr++ = 0;
	
	
	*ptr++ = 0xC7;// Write time
	*ptr++ = 0x83;

	*ptr++ = 0x3D;// Write date
	*ptr++ = 0x4B;
	
	//*ptr++ = startingCluster & 0xff;		// file cluster lo
	//*ptr++ = (startingCluster >> 8) & 0xff;	
	*ptr++ = 0;
	*ptr++ = 0;
	
	*ptr++ = (size >> (8*0)) & 0xff;	// Write file size
	*ptr++ = (size >> (8*1)) & 0xff;
	*ptr++ = (size >> (8*2)) & 0xff;
	*ptr = (size >> (8*3)) & 0xff;
	
	//displayMemory(buffer, 256);
	
	_hd->WriteSector(_lastSectorRead, buffer, 512);
	
	 //uint32_t sector = _dataStart + _bpb.sectorsPerCluster * (startingCluster-2);
	 //printf("writing to sector %06X", sector);
	//_hd->WriteSector(sector, (uint8_t*)"TEST", 4);
	return;
      }

    }

    buffer = ReadNextSectorInChain(0);
  }  
}