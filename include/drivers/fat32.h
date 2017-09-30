#ifndef __DRIVERS__FAT32_H
#define __DRIVERS__FAT32_H

#include <lib/stdint.h>

struct MBR
{
  uint8_t	JMP[3];
  uint8_t	OEM[8];
  uint16_t	NumberOfBytesPerSector;
  uint8_t	NumberOfSectorsPerCluster;
  uint16_t	NumberOfReservedSectors;
  uint8_t	NumberOfFATs;
  uint16_t	NumberOfRootEntries16;
  uint16_t	LowNumbferOfSectors;
  uint8_t	MediaDescriptor;
  uint16_t	NumberOfSectorsPerFAT16;
  uint16_t	NumberOfSectorsPerTrack;
  uint16_t	NumberOfHeads;
  uint32_t	NumberOfHiddenSectors;
  uint32_t	HighNumberOfSectors;
  uint32_t	NumberOfSectorsPerFAT32;
  uint16_t	Flags;
  uint16_t	FATVersionNumber;
  uint32_t	RootDirectoryClusterNumber;
  uint16_t	FSInfoSector;
  uint16_t	BackupSector;
  uint8_t	Reserver[12];
  uint8_t	BiosDrive;
  uint8_t	WindowsNTFlag;
  uint8_t	Signature;
  uint16_t	VolumeSerial;
  uint8_t	VolumeLabel[11];
  uint8_t	SystemID[8];
  uint8_t	CODE[420];
  uint16_t	BPBSignature;
};

#endif