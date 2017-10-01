
#ifndef __MYOS__FILESYSTEM_FAT_H
#define __MYOS__FILESYSTEM_FAT_H



#include <lib/stdint.h>
#include <lib/stdlib.h>
#include <lib/stdio.h>
#include <drivers/ata.h>
using namespace myos::drivers;
// https://www.win.tue.nl/~aeb/linux/fs/fat/fat-1.html

#define ENDOFCLUSTER_FAT32	0x0FFFFFFF
#define BADCLUSTER_FAT32	0x0FFFFFF7

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
	    uint8_t sectorsPerCluster;	// Sectors per Cluster - 1, 2, 4, [8], 16, 32, 64 or 128 sectors
	    uint16_t reservedSectors;	// Number of reserved sectors (1)
					// FAT12 and FAT16 use 1. FAT32 uses 32 (0x20).
	    uint8_t fatCopies;		// Number of FAT copies (2)
	    uint16_t rootDirEntries;	// Number of root directory entries (224) (not used in FAT32)
	    uint16_t totalSectors;	// not used in FAT32
	    uint8_t mediaType;		// Media descriptor type (f0: 1.4 MB floppy, f8: hard disk; 
	    uint16_t fatSectorCount;	// Number of sectors per FAT (9) 0 for FAT32.
	    uint16_t sectorsPerTrack;	// Sectors per Track - 63 (0x3F)
	    uint16_t headCount;		// Number of heads 
	    uint32_t hiddenSectors;	// Number of hidden sectors (0)
	    uint32_t totalSectorCount;	// Total number of sectors in the filesystem
	    uint32_t sectorsPerFat;	// Sectors per FAT
	    uint16_t extFlags;		// Mirror flags
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
	    uint8_t  reserved;		// Reserved - used to be Current Head (used by Windows NT)
	    uint8_t  bootSignature;	// Extended signature (0x29)
					// Indicates that the three following fields are present.
	    uint32_t volumeId;		// Serial number of partition
	    uint8_t volumeLabel[11];	// Volume label
	    uint8_t fatTypeLabel[8];	// Filesystem type ("FAT32   ")
    } __attribute__((packed));
    
    struct DirectoryEntryFat32
    {
	    uint8_t name[8];
	    uint8_t  ext[3];
	    uint8_t attributes;		// Bit 0: read only. Bit 1: hidden.
					// Bit 2: system file. Bit 3: volume label. Bit 4: subdirectory.
					// Bit 5: archive. Bits 6-7: unused.
	    uint8_t reserved;		// Unused in FAT32 but should be left as it was read
	    uint8_t cTimeTenth;		// Unused in FAT32 but should be left as it was read
	    uint16_t cTime;		// Unused in FAT32 but should be left as it was read
	    uint16_t cDate;		// Unused in FAT32 but should be left as it was read
	    uint16_t aTime;		// Unused in FAT32 but should be left as it was read
	    uint16_t firstClusterHi;	// Starting cluster
	    uint16_t wTime;		// Time (5/6/5 bits, for hour/minutes/doubleseconds)
	    uint16_t wDate;		// Date (7/4/5 bits, for year-since-1980/month/day)
	    uint16_t firstClusterLow;	// Starting cluster (0 for an empty file)
	    uint32_t size;		// File size in bytes
    } __attribute__((packed));
	  
    class Fat32 {
    
    private:
	myos::drivers::AdvancedTechnologyAttachment *_hd;
	uint8_t _partition;
	MasterBootRecord masterBootRecord;
	BiosParameterBlock32 biosParameterBlock;
	uint8_t _endOfChain;
	uint32_t _fatStart;
	uint32_t _dataStart;
	uint32_t _rootStart;
	uint8_t _fat[512];
	
	void Initialize();
	uint8_t* ReadNextSectorInChain(uint32_t startOfChain);
	uint32_t GetFileSector(char *filename);
    public:
      Fat32(myos::drivers::AdvancedTechnologyAttachment *hd, uint8_t partition);
      ~Fat32();
      
      void ReadDir();
      void ReadFile(char *filename);      
      void ReadPartitions();

     
      
    };
  }
	
}
#endif