
#ifndef __MYOS__FILESYSTEM_FAT_H
#define __MYOS__FILESYSTEM_FAT_H



#include <lib/stdint.h>
#include <lib/stdlib.h>
#include <lib/stdio.h>
#include <lib/vector.h>
#include <drivers/ata.h>
using namespace myos::drivers;
// https://www.win.tue.nl/~aeb/linux/fs/fat/fat-1.html

#define FREECLUSTER_FAT32	0x00000000
#define ENDOFCLUSTER_FAT32	0x0FFFFFF8
#define BADCLUSTER_FAT32	0x0FFFFFF7


#define FILEACCESSMODE_CLOSED	0x00
#define FILEACCESSMODE_READ	0x01
#define FILEACCESSMODE_WRITE	0x02
#define FILEACCESSMODE_CREATE	0x03

#define FILE_STATUS_OK		0x00
#define FILE_STATUS_FILEOPEN	0x01
#define FILE_STATUS_NOTFOUND	0x02
#define FILE_STATUS_FILECLSD	0x03
#define FILE_STATUS_EOF		0x04
#define FILE_STATUS_NODEVICE	0x05
#define FILE_STATUS_FILEEXISTS	0x06
#define FILE_STATUS_DISKFULL	0x07

#define MAX_CBM_FILES_OPEN	0x0A

namespace myos
{
  namespace filesystem
  { 
    struct PartitionTableEntry
    {
      uint8_t bootable;
      uint8_t start_head;
      uint8_t start_sector : 6;
      uint16_t start_cylinder: 10;
      
      uint8_t partition_id;
      
      uint8_t end_head;
      uint8_t end_sector : 6;
      uint16_t end_cylinder: 10;
      
      uint32_t start_lba;
      uint32_t length;
    } __attribute__((packed));
   
    struct MasterBootRecord
    {
      uint8_t bootloader[440];
      uint32_t signature;
      uint16_t unused;
      
      PartitionTableEntry primaryPartition[4];
      
      uint16_t magicnumber;
    } __attribute__((packed));
 
   
    struct BiosParameterBlock32
    {
      uint8_t jump[3];
      uint8_t softName[8];	// OEM name/version
      uint16_t bytesPerSector;	// Number of bytes per sector (512)
				  // Must be one of [512], 1024, 2048, 4096 - normally 512
      uint8_t sectorsPerCluster;// Sectors per Cluster - 1, 2, 4, [8], 16, 32, 64 or 128 sectors
      uint16_t reservedSectors;	// Number of reserved sectors (1)
				  // FAT12 and FAT16 use 1. FAT32 uses 32 (0x20).
      uint8_t fatCopies;	// Number of FAT copies (2)
      uint16_t rootDirEntries;	// Number of root directory entries (224) (not used in FAT32)
      uint16_t totalSectors;	// not used in FAT32
      uint8_t mediaType;	// Media descriptor type (f0: 1.4 MB floppy, f8: hard disk; 
      uint16_t fatSectorCount;	// Number of sectors per FAT (9) 0 for FAT32.
      uint16_t sectorsPerTrack;	// Sectors per Track - 63 (0x3F)
      uint16_t headCount;	// Number of heads 
      uint32_t hiddenSectors;	// Number of hidden sectors (0)
      uint32_t totalSectorCount;// Total number of sectors in the filesystem
      uint32_t sectorsPerFat;	// Sectors per FAT
      uint16_t extFlags;	// Mirror flags
				  // Bits 0-3: number of active FAT (if bit 7 is 1)
				  // Bits 4-6: reserved
				  // Bit 7: one: single active FAT; zero: all FATs are updated at runtime
				  // Bits 8-15: reserved
      uint16_t fatVersion;	// Filesystem version
      uint32_t rootCluster;	// First cluster of root directory (usually 2)
      uint16_t fatInfo;		// Filesystem information sector number in FAT32 reserved area (usually 1)
      uint16_t backupSector;	// Backup boot sector location or 0 or 0xffff if none (usually 6)
      uint8_t  reserved0[12];	// Reserved
      uint8_t  driveNumber;	// Logical Drive Number (for use with INT 13, e.g. 0 or 0x80)
      uint8_t  reserved;	// Reserved - used to be Current Head (used by Windows NT)
      uint8_t  bootSignature;	// Extended signature (0x29)
				// Indicates that the three following fields are present.
      uint32_t volumeId;	  // Serial number of partition
      uint8_t volumeLabel[11];	// Volume label
      uint8_t fatTypeLabel[8];	// Filesystem type ("FAT32   ")
    } __attribute__((packed));
    
    struct DirectoryEntryFat32
    {
      uint8_t name[8];
      uint8_t  ext[3];
      uint8_t attributes;	// Bit 0: read only. Bit 1: hidden.
				// Bit 2: system file. Bit 3: volume label. Bit 4: subdirectory.
				// Bit 5: archive. Bits 6-7: unused.
      uint8_t reserved;		// Unused in FAT32 but should be left as it was read
      uint8_t cTimeTenth;	// Unused in FAT32 but should be left as it was read
      uint16_t cTime;		// Unused in FAT32 but should be left as it was read
      uint16_t cDate;		// Unused in FAT32 but should be left as it was read
      uint16_t aTime;		// Unused in FAT32 but should be left as it was read
      uint16_t firstClusterHi;	// Starting cluster
      uint16_t wTime;		// Time (5/6/5 bits, for hour/minutes/doubleseconds)
      uint16_t wDate;		// Date (7/4/5 bits, for year-since-1980/month/day)
      uint16_t firstClusterLow;	// Starting cluster (0 for an empty file)
      uint32_t size;		// File size in bytes
    } __attribute__((packed));
    
    struct FileStatus 
    {
      uint8_t mode;
      uint8_t filename[8];
      uint8_t ext[3];
      uint32_t size;
      uint32_t locationPtr;
      uint8_t* buffer;
      //Vector<uint8_t> fileBuffer;
      uint32_t startingCluster;
    };

    class Fat32 {
    
    private:
	myos::drivers::AdvancedTechnologyAttachment *_hd;
	uint8_t _partition;
	MasterBootRecord _mbr;
	BiosParameterBlock32 _bpb;
	uint8_t _endOfChain;
	uint32_t _fatStart;
	uint32_t _dataStart;
	uint32_t _rootStart;
	uint8_t* _fat;
	uint32_t _lastSectorRead;
	uint8_t *_fatBuffer;
	
	void LoadFAT();
	uint8_t* ReadNextSectorInChain(uint32_t startOfChain);
	struct FileStatus openFilesList[MAX_CBM_FILES_OPEN];

	
    public:
      Fat32(myos::drivers::AdvancedTechnologyAttachment *hd, uint8_t partition);
      ~Fat32();
      
      void ReadDirectory(uint32_t startCluster);
      void ReadFile(uint8_t *filename, uint8_t* data, uint32_t size);     
      void ReadPartitions();
      void ReadSector(uint32_t sector, uint8_t *buffer);
      uint32_t GetFileSize(uint8_t* filename);
      uint32_t GetFileCluster(uint8_t* filename);
      
      int OpenFile(uint8_t filenumber, uint8_t* filename, uint8_t mode);
      int CloseFile(uint8_t filenumber);
      int ReadNextFileByte(uint8_t filenumber, uint8_t* b);
      
      int ParseFilename(uint8_t* filename, uint8_t* filename8, uint8_t* ext);
      int AllocateCluster(uint32_t* startingCluster);
      int WriteNextFileByte(uint8_t filenumber, uint8_t b);
      int FlushWriteBuffer(uint8_t filenumber);
      void ResetOpenFileListEntry(uint8_t filenumber);
      
      void CreateDirectoryEntry(uint8_t* filename, uint8_t* ext, uint32_t size);
      int UpdateDirectoryEntry(uint8_t* filename, uint8_t* ext, uint32_t size, uint32_t startingCluster);
      int DeleteFile(uint8_t* filename);
      int RenameFile(uint8_t* currentFilename, uint8_t* newFilename);
      
      void WriteDir(uint8_t* filename, uint8_t* ext, uint32_t size);
      
      uint8_t* GetCBMDir();
     
      
    };
  }
	
}
#endif