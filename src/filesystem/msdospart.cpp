#include <filesystem/msdospart.h>
#include <filesystem/fat.h>

using namespace myos;
using namespace myos::drivers;
using namespace myos::filesystem;


void MSDOSPartitionTable::ReadPartitions(myos::drivers::AdvancedTechnologyAttachment *hd)
{
  MasterBootRecord mbr;

  hd->Read28(0, (uint8_t*)&mbr, sizeof(MasterBootRecord));
	
  /*printf("MBR: ");
  for(int i=446; i<446+ 4*16;i++)
  {
	  printfHex(((uint8_t*)&mbr)[i]);
	  printf(" ");
  }

  printf("\n");*/
  
  if(mbr.magicnumber != 0xAA55)
  {
    printf("\nInvalid MBR Signature!");
    return;
  }

  for (int i=0;i<4;i++)
  {
    if(mbr.primaryPartition[i].partition_id == 0x00)
	    continue;

    printf("\n\nPartition %X", i);
    //printfHex(i & 0xFF);

    if(mbr.primaryPartition[i].bootable == 0x80)
	    printf(" bootable. Type ");
    else
	    printf(" not bootable. Type ");

    printf("%02X", mbr.primaryPartition[i].partition_id);
    
    //ReadBiosParameterBlock(hd, mbr.primaryPartition[i].start_lba);	
  }
  
  ReadDirectory(hd,0);
  
  
}