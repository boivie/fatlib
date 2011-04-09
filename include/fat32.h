#ifndef FAT32_H_INCLUSION_GUARD
#define FAT32_H_INCLUSION_GUARD

/**
 * @brief The FAT32 specific implementation of FAT_GetSectorsPerFAT
 * @see FAT_GetSectorsPerFat
 * @ingroup FAT
 */
#define FAT32_GetSectorsPerFAT(pVolumeID) (*(uint32_t*)(pVolumeID + 0x24))

#define FAT32_GetRootDirectoryCluster(pVolumeID) (TFatClusterNr)*(uint32_t*)(pVolumeID + 0x2c)

#define FAT32_GetStartCluster(pDirEntry) (uint32_t)((pDirEntry->StartClusterHigh << 16) + pDirEntry->StartClusterLow)

#define FAT32_IsLastDirEntry(pPartition, pDirEntry, pDirLocation) ((pDirEntry->Name[0] == 0x00) || !FAT32_IsCurrentClusterValid(pPartition, &(pDirLocation)->Location))

#define FAT32_IsCurrentClusterValid(pPartition, pLocation) ((pLocation)->Cluster != 0x0FFFFFFF)

/**
 * @brief The FAT32 specific implementation of FAT_GetNextCluster
 * @see FAT_GetNextCluster
 * @ingroup FAT
 */
FAT_API TFatClusterNr FAT32_GetNextCluster(TFatPartition* pPartition, TFatClusterNr CurrentCluster);

/**
 * @brief The FAT16 specific implementation of FAT_GetFirstRootDirEntry
 * @see FAT_GetFirstRootDirEntry
 * @ingroup Dir
 */
FAT_API void FAT32_GetFirstRootDirEntry(TFatPartition* pPartition, TFatDirectoryLocation* pDirLocation);

/**
 * @brief The FAT32 specific implementation of FAT_FindRootDirEntry
 * @see FAT_FindRootDirEntry
 * @ingroup Dir
 */
FAT_API TFatDirEntry* FAT32_FindRootDirEntry(TFatPartition* pPartition, char* pName, TFatDirectoryLocation* pDirLocation);

#endif
