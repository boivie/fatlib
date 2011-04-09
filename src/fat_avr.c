#include "fat.c"
#include <stdio.h>

uint8_t FAT_Buffer[FAT_BYTES_PER_SECTOR];

void FAT_ReadSector(TFatPartition* pPartition, uint32_t SectorNr)
{
  uint8_t* const pBuffer = pPartition->pBuffer;
  volatile uint8_t foo;
  uint16_t bar;
  for (bar = 0; bar < FAT_BYTES_PER_SECTOR; bar++)
    *(pBuffer + bar) = foo;
  return;
}

void FAT_WriteSector(TFatPartition* pPartition, uint32_t SectorNr)
{
}

#define MIN(a,b) ((a) > (b) ? (b) : (a))

int main(void) 
{
  TFatPartition Partition;
  TFatDirectoryLocation DirLocation;

  Partition.pBuffer = FAT_Buffer;
  
  if (FAT_OpenPartition(&Partition, 0))
  {
    TFatDirEntry* pDirEntry = FAT_FindRootDirEntry(&Partition, "FIXAT   TXT", &DirLocation);

    if (pDirEntry == NULL)
    {
      return 1; 
    } else
    {
      TFatLocation Location;
      uint8_t SectorsLeft = 4;

      FAT_Seek(&Partition, &Location, FAT_GetStartCluster(pDirEntry));

      while (--SectorsLeft != 0)
      {
	FAT_ReadNextSector(&Partition, &Location);
      }
    }
    //    FAT16_CreateRootDirEntry();
  }
  return 0;
}
