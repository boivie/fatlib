#include "../include/fat.h"

#ifdef FAT_DEBUG
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#endif

#include <string.h>

#ifdef FAT_SINGLE_FILE
#include "fat16.c"
#include "fat32.c"
#endif

/* TODO: What does this actually compute? The start of the partition data block? */
FAT_API uint32_t FAT_GetRootOffset(const TFatPartition* pPartition)
{
  return pPartition->PartitionLBA + pPartition->ReservedSectors + FAT_NUMBER_OF_FATS * pPartition->SectorsPerFAT;
}

FAT_API void FAT_Seek(const TFatPartition* pPartition, TFatLocation* pLocation, TFatClusterNr ClusterNr)
{
  pLocation->Cluster = ClusterNr;
  pLocation->Sector = FAT_GetRootOffset(pPartition) 
    + (ClusterNr - 2) * pPartition->SectorsPerCluster 
    + pPartition->RootDirectoryEntries / (FAT_BYTES_PER_SECTOR / FAT_DIRECTORY_ENTRY_SIZE);
  pLocation->SectorsLeftInCluster = pPartition->SectorsPerCluster - 1;
  return;
}

FAT_API uint8_t FAT_OpenPartition(TFatPartition* pPartition, uint8_t PartitionNr)
{
  /* Read the MBR */
  FAT_ReadSector(pPartition, 0); 

  if (!FAT_IsMBRValid(pPartition->pBuffer)) return 0;

  switch (FAT_GetPartitionType(pPartition->pBuffer, PartitionNr))
  {
#ifdef FAT_ENABLE_FAT32
  case 0x0b:
  case 0x0c:
#ifdef FAT_ENABLE_BOTH
    pPartition->Type = FAT_32;
#endif
    D_(printf("Partition type: FAT32\n"));
    break;
#endif
#ifdef FAT_ENABLE_FAT16
  case 0x04:
  case 0x06:
  case 0x0e:
#ifdef FAT_ENABLE_BOTH
    pPartition->Type = FAT_16;
#endif
    D_(printf("Partition type: FAT16\n"));
    break;
#endif
  default:
    D_(printf("Invalid partition type: %d\n", FAT_GetPartitionType(pPartition->pBuffer, PartitionNr)));
    return 0;
  }

  pPartition->PartitionLBA = FAT_GetPartitionLBA(pPartition->pBuffer, PartitionNr);

  /* Read the Volume ID to the buffer */
  FAT_ReadSector(pPartition, pPartition->PartitionLBA); 

  /* Read other parameters that we need for the other functions to work. */
  pPartition->ReservedSectors      = FAT_GetReservedSectors(pPartition->pBuffer);
  pPartition->SectorsPerCluster    = FAT_GetSectorsPerCluster(pPartition->pBuffer);
  pPartition->SectorsPerFAT        = FAT_GetSectorsPerFAT(pPartition);
  pPartition->RootDirectoryEntries = FAT_GetRootDirectoryEntries(pPartition->pBuffer);

#ifdef FAT_DEBUG
  printf("-----------------------------\n");
  printf("Partition LBA:          %d\n", pPartition->PartitionLBA);
  printf("Reserved sectors:       %d\n", pPartition->ReservedSectors);
  printf("Sectors per cluster:    %d\n", pPartition->SectorsPerCluster);
  printf("Sectors per FAT:        %d\n", pPartition->SectorsPerFAT);
  if (FAT_IsFAT16(pPartition))
    printf("Root directory entries: %d\n", pPartition->RootDirectoryEntries);
  printf("-----------------------------\n");
#endif

  return 1;
}

FAT_API void FAT_ReadNextSector(TFatPartition* pPartition, TFatLocation* pLocation)
{
  /* We need to fetch a new sector. Are there any left in this cluster? */
  if (pLocation->SectorsLeftInCluster == 0)
  { 
    /* There wasn't. So we must fetch the next cluster by following the cluster chain.*/
    TFatClusterNr NextCluster = FAT_GetNextCluster(pPartition, pLocation->Cluster);
#ifdef FAT_ENABLE_BOTH
    if (FAT_IsFAT16(pPartition))
    {
#endif
#ifdef FAT_ENABLE_FAT16
      if (NextCluster == 0xFFFF) return;
#endif
#ifdef FAT_ENABLE_BOTH
    }
    else /* FAT32 */
    {
#endif
#ifdef FAT_ENABLE_FAT32
      if (NextCluster == 0x0FFFFFFF) return;
#endif
#ifdef FAT_ENABLE_BOTH
    }
#endif
    FAT_Seek(pPartition, pLocation, NextCluster);
  }
  else
  {
    /* There are more sectors in this cluster. */
    pLocation->Sector++;
    pLocation->SectorsLeftInCluster--;
  }
  FAT_ReadSector(pPartition, pLocation->Sector);
}

FAT_API void FAT_GetFirstDirectoryEntry(TFatPartition* pPartition, TFatClusterNr StartCluster, TFatDirectoryLocation* pDirLocation)
{
  FAT_Seek(pPartition, &pDirLocation->Location, StartCluster);
  FAT_ReadSector(pPartition, pDirLocation->Location.Sector);
  pDirLocation->EntryOffset = 0;
}

FAT_API void FAT_GetNextDirectoryEntry(TFatPartition* pPartition, TFatDirectoryLocation* pDirLocation)
{ 
  /* At entry, all state variables reflect the previous entry. */
  if (pDirLocation->EntryOffset == (FAT_NUMBER_OF_DIRECTORY_ENTRIES_PER_SECTOR - 1))
  {
    FAT_ReadNextSector(pPartition, &pDirLocation->Location);
    pDirLocation->EntryOffset = 0;
  }
  else
  {
    /* Skip to the next in this sector. */
    pDirLocation->EntryOffset++;
  }
}

FAT_API TFatDirEntry* FAT_FindDirEntry(TFatPartition* pPartition, TFatClusterNr DirectoryCluster, char* pName, TFatDirectoryLocation* pDirLocation)
{
  /* Load the root director sector */
  FAT_GetFirstDirectoryEntry(pPartition, DirectoryCluster, pDirLocation);
  for (;;)
  {
    TFatDirEntry* pDirEntry = FAT_GetDirEntry(pPartition, pDirLocation);

    if (FAT_IsLastDirEntry(pPartition, pDirEntry, pDirLocation)) break;

    if (!FAT_IsDirEntryDeleted(pDirEntry) && 
        !FAT_IsLongFileName(pDirEntry))
    {
#ifdef FAT_DEBUG
      if (FAT_IsVolumeID(pDirEntry))
      {
        printf("Volume: %.11s\n", pDirEntry->Name);
      } 
      else if (FAT_IsFile(pDirEntry))
      {
        printf("%.11s    %10d  %10d\n", pDirEntry->Name, pDirEntry->FileSize, FAT_GetStartCluster(pDirEntry));
      }
      else if (FAT_IsDirectory(pDirEntry))
      {
        printf("%.11s   <DIR>        %10d\n", pDirEntry->Name, FAT_GetStartCluster(pDirEntry));
      }
#endif
      {
        const void* pCurName = pDirEntry->Name;
        if (memcmp((void*)pName, pCurName, 11) == 0)
	{
	  return pDirEntry;
	}
      }
    }
    FAT_GetNextDirectoryEntry(pPartition, pDirLocation);
  }
  return NULL;
}

#ifdef FAT_ENABLE_WRITE

FAT_API uint8_t FAT_CreateCluster(TFatPartition* pPartition, TFatClusterNr FirstCluster, TFatLocation* pLocation)
{
  TFatClusterNr ClusterNr = FAT_FindFreeCluster(pPartition);

  if (ClusterNr == 0)
  {
    return 0;
  }

  FAT_LinkClusters(pPartition, FirstCluster, ClusterNr);
  
  /* Update pLocation */
  FAT_Seek(pPartition, pLocation, ClusterNr);

  return 1;
}

FAT_API TFatDirEntry* FAT_CreateDirEntry(TFatPartition* pPartition, TFatClusterNr StartCluster, TFatDirectoryLocation* pDirLocation)
{
  TFatClusterNr LastCluster = StartCluster;
  
  D_(printf("Creating directory entry, start cluster: %d\n", StartCluster));
  
  FAT_GetFirstDirectoryEntry(pPartition, StartCluster, pDirLocation);

  while (FAT_IsCurrentClusterValid(pPartition, &pDirLocation->Location))
  {
    TFatDirEntry* pDirEntry = FAT_GetDirEntry(pPartition, pDirLocation);

    if (FAT_IsDirEntryDeleted(pDirEntry) ||        /* Found deleted entry */
        (pDirEntry->Name[0] == 0x00)) /* Found the last entry */
    {
      /* Found an entry that can be used! In case it was a deleted entry,
       * we can just re-use it. If it was the last entry, the remaining entries
       * must be "empty" as well.
       */
      D_(printf("Found unused entry at %d::%d\n", pDirLocation->Location.Cluster, pDirLocation->EntryOffset));
      return pDirEntry;
    }
    LastCluster = pDirLocation->Location.Cluster; /* Save it, so we know which one we should link from. */
    FAT_GetNextDirectoryEntry(pPartition, pDirLocation);
  }
  /* We found the last cluster. Bummer. 
   * To extend the directory table, we must find a new
   * cluster, link it together with the last one, and then
   * clear the entire cluster (we're forced to since all entries
   * must be marked as "empty". After that, we're done.
   */
  D_(printf("Didn't find unused in cluster. Have to create new cluster.\n"));

  if (FAT_CreateCluster(pPartition, LastCluster, &pDirLocation->Location))
  {
    /* Clear the entire cluster. */
    uint8_t I;
    uint32_t Sector = pDirLocation->Location.Sector;
    memset((void*)pPartition->pBuffer, 0, FAT_BYTES_PER_SECTOR);
  
    for (I = 0; I < pPartition->SectorsPerCluster; I++)
    {
      FAT_WriteSector(pPartition, Sector);
      Sector++;
    }

    pDirLocation->EntryOffset = 0;
    return (TFatDirEntry*)pPartition->pBuffer; 
  }
  
  /* Could not create a new cluster - The disk is probably full */
  return NULL;
}

void FAT_InitDirEntry(TFatPartition* pPartition, TFatDirectoryLocation* pDirLocation, const char* pDirEntryName)
{
  const TFatDirEntry* pDirEntry = FAT_GetDirEntry(pPartition, pDirLocation);

  memset((void*)pDirEntry, 0, sizeof(*pDirEntry));
  memcpy((void*)pDirEntry->Name, (void*)pDirEntryName, sizeof(pDirEntry->Name));

  FAT_WriteSector(pPartition, pDirLocation->Location.Sector);
  D_(printf("Initialised directory entry with name %s\n", pDirEntryName));
}

#endif

