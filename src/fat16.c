#include "../include/fat.h"
#include <string.h>

#ifdef FAT_DEBUG
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#endif

#ifdef FAT_ENABLE_FAT16

FAT_API TFatClusterNr FAT16_GetNextCluster(TFatPartition* pPartition, TFatClusterNr CurrentCluster)
{
  const uint32_t Sector = FAT_GetFATSector(pPartition) + ((uint16_t)CurrentCluster / (FAT_BYTES_PER_SECTOR / sizeof(uint16_t)));
  const uint32_t Offset = ((uint16_t)CurrentCluster % (FAT_BYTES_PER_SECTOR / sizeof(uint16_t))) * sizeof(uint16_t);
  FAT_ReadSector(pPartition, Sector);

  return (TFatClusterNr)*(uint16_t*)(pPartition->pBuffer + Offset);
}

FAT_API void FAT16_GetNextRootDirEntry(TFatPartition* pPartition, TFatDirectoryLocation* pDirLocation)
{
  /* Cluster is used as the number of root dir entries left */
  pDirLocation->Location.Cluster--;

  if (pDirLocation->Location.Cluster == 0)
  {
    /* No more directory entries left. */

    /* Setting Cluster to 0xFFFF will result in "no more entries" in FAT_IsLastDirEntry(). */
    pDirLocation->Location.Cluster = 0xFFFF;
    return;
  }
  else if (pDirLocation->EntryOffset == (FAT_NUMBER_OF_DIRECTORY_ENTRIES_PER_SECTOR - 1))
  {
    /* Read the last directory entry for this sector. Must read a new sector */
    pDirLocation->Location.Sector++;
    FAT_ReadSector(pPartition, pDirLocation->Location.Sector);
    pDirLocation->EntryOffset = 0;
  }
  else
  {
    pDirLocation->EntryOffset++;
  }
}

FAT_API void FAT16_GetFirstRootDirEntry(TFatPartition* pPartition, TFatDirectoryLocation* pDirLocation)
{
  pDirLocation->Location.Sector = FAT_GetRootOffset(pPartition);
  /* Reusing Cluster as the number of dir entries left 
   * Maximum: 512 sectors * 16 entries = 8192 entries - uint16_t is enough, and TFatClusterNr may
   *          be a uint16_t.
   */
  pDirLocation->Location.Cluster = (TFatClusterNr)pPartition->RootDirectoryEntries; 
  pDirLocation->EntryOffset = 0;
  FAT_ReadSector(pPartition, pDirLocation->Location.Sector);
}

FAT_API TFatDirEntry* FAT16_FindRootDirEntry(TFatPartition* pPartition, char* pName, TFatDirectoryLocation* pDirLocation)
{
  /* The FAT16 root directory is special. It's a fixed number of sectors
   * located at a specific location. So it's not difficult to search through. 
   */
  FAT16_GetFirstRootDirEntry(pPartition, pDirLocation);

  for (;;)
  {
    TFatDirEntry* pDirEntry = FAT_GetDirEntry(pPartition, pDirLocation);

    if (FAT16_IsLastDirEntry(pPartition, pDirEntry, pDirLocation)) break;

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
        printf("%.11s   <DIR>         %10d\n", pDirEntry->Name, FAT_GetStartCluster(pDirEntry));
      }
#endif
      {
        const void* pCurName = pDirEntry->Name;
        if (memcmp((void*)pName, pCurName, sizeof(pDirEntry->Name)) == 0)
	{
          return pDirEntry;
	}
      }
    }
    FAT16_GetNextRootDirEntry(pPartition, pDirLocation);  
  }
  return NULL;
}

#ifdef FAT_ENABLE_WRITE

/* Returns 1 if an entry can be created. In that case,
 * CurrentSector and DirectoryEntryOffset points to the
 * new entry. If none can be found, 0 is returned.
 * The number of root directory entries is limited on FAT16 
 * (normally for HDDs, there are 512 * (sector size)/(entry_size)
 *  == 8192 entries)
 */
FAT_API TFatDirEntry* FAT16_CreateRootDirEntry(TFatPartition* pPartition, TFatDirectoryLocation* pDirLocation)
{
  FAT16_GetFirstRootDirEntry(pPartition, pDirLocation);

  while (FAT16_IsCurrentClusterValid(pPartition, &pDirLocation->Location))
  {
    TFatDirEntry* pDirEntry = FAT_GetDirEntry(pPartition, pDirLocation);

    if (FAT_IsDirEntryDeleted(pDirEntry) ||  /* Found deleted entry */
        (pDirEntry->Name[0] == 0x00))        /* Found the last entry */
    {
      /* Found an entry that can be used! In case it was a deleted entry,
       * we can just re-use it. If it was the last entry, the remaining entries
       * must be "empty" as well.
       */
      return pDirEntry;
    }
    FAT16_GetNextRootDirEntry(pPartition, pDirLocation);
  }
  /* We went through all entries, and still couldn't find one. */
  return NULL;
}

/* Finds the next free cluster on the disk, which it returns.
 * If zero (0) is returned, no cluster was found.
 */
FAT_API TFatClusterNr FAT16_FindFreeCluster(TFatPartition* pPartition) 
{
  uint16_t LastCluster;
  uint16_t FatSectorOffset;
  uint16_t CurrentCluster = 0; /* TODO: Numbering starts at two? */
  uint32_t FatSector = FAT_GetFATSector(pPartition);

  LastCluster = (uint16_t)(pPartition->SectorsPerFAT / pPartition->SectorsPerCluster);

  /* Search through the FAT table until we find one that is marked as 'free'.
   * 
   * Since there are 256 FAT16 entries, we allow the loop variable to 
   * wrap over.
   */
  do {
    FAT_ReadSector(pPartition, FatSector);
    for (FatSectorOffset = 0; FatSectorOffset < FAT_BYTES_PER_SECTOR; FatSectorOffset += sizeof(uint16_t))
    {
      if ((*(uint16_t*)(pPartition->pBuffer + FatSectorOffset)) == 0x0000) 
      {
	D_(printf("Found free cluster %d\n", CurrentCluster));
        return (TFatClusterNr)CurrentCluster;
      }
      CurrentCluster++;
    }
    FatSector++;
  } while (CurrentCluster < LastCluster); 

  D_(printf("No free cluster found (Disk full?)\n"));

  return 0;
}

/* Links FirstCluster to SecondCluster and terminates the cluster chain after SecondCluster
 *
 * If FirstCluster is 0, it will not link the clusters, but SecondCluster will still be
 * terminated, thus creating the first cluster of a cluster chain.
 */
FAT_API void FAT16_LinkClusters(TFatPartition* pPartition, TFatClusterNr FirstCluster, TFatClusterNr SecondCluster)
{
  uint32_t Sector;
  uint32_t Offset;
  
  D_(printf("Linking Cluster %d -> %d.\n", FirstCluster, SecondCluster));

  /* Link the clusters */
  if (FirstCluster != 0)
  {
    Sector = FAT_GetFATSector(pPartition) + ((uint16_t)FirstCluster / FAT_NUMBER_OF_FAT16_ENTRIES_PER_SECTOR);
    Offset = ((uint16_t)FirstCluster % FAT_NUMBER_OF_FAT16_ENTRIES_PER_SECTOR) * sizeof(uint16_t);
  
    FAT_ReadSector(pPartition, Sector);  

    *(uint16_t*)(pPartition->pBuffer + Offset) = (uint16_t)SecondCluster;

    FAT_WriteSector(pPartition, Sector);
  }

  /* TODO: We could check if the SecondCluster has the same FAT sector 
   * as the first and only read/write once instead of 2*(R+W)
   */

  /* Set SecondCluster to 0xFFFF - which indicates the last cluster. */
  Sector = FAT_GetFATSector(pPartition) + ((uint16_t)SecondCluster / (FAT_BYTES_PER_SECTOR / sizeof(uint16_t)));
  Offset = ((uint16_t)SecondCluster % (FAT_BYTES_PER_SECTOR / sizeof(uint16_t))) * sizeof(uint16_t);
  
  FAT_ReadSector(pPartition, Sector);

  *(uint16_t*)(pPartition->pBuffer + Offset) = 0xFFFF;

  FAT_WriteSector(pPartition, Sector);
  
  D_(printf("Linking done."));
}
#endif
#endif
