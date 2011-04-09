#include "../include/fat.h"

#ifdef FAT_DEBUG
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#endif

#ifdef FAT_ENABLE_FAT32

FAT_API TFatClusterNr FAT32_GetNextCluster(TFatPartition* pPartition, TFatClusterNr CurrentCluster)
{
  const uint32_t Sector = FAT_GetFATSector(pPartition) + (CurrentCluster / (FAT_BYTES_PER_SECTOR / sizeof(uint32_t)));
  const uint32_t Offset = (CurrentCluster % (FAT_BYTES_PER_SECTOR / sizeof(uint32_t))) * sizeof(uint32_t);
  FAT_ReadSector(pPartition, Sector);

  /* Only the lowest 28 bits of a FAT32 cluster number are valid. */
  return (TFatClusterNr)(*(uint32_t*)(pPartition->pBuffer + Offset)) & 0x0FFFFFFF;
}

FAT_API TFatDirEntry* FAT32_FindRootDirEntry(TFatPartition* pPartition, char* pName, TFatDirectoryLocation* pDirLocation)
{
  /* Read the volume ID */
  FAT_ReadSector(pPartition, pPartition->PartitionLBA); 

  return FAT_FindDirEntry(pPartition, FAT32_GetRootDirectoryCluster(pPartition->pBuffer), pName, pDirLocation);
}


#endif
