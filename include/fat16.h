#ifndef FAT16_H_INCLUSION_GUARD
#define FAT16_H_INCLUSION_GUARD

#include "../fat_conf.h"

/**
 * @brief The FAT16 specific implementation of FAT_GetSectorsPerFAT
 * @see FAT_GetSectorsPerFat
 * @ingroup FAT
 */
#define FAT16_GetSectorsPerFAT(pVolumeID) (*(uint16_t*)(pVolumeID + 0x16))

#define FAT16_GetRootDirectorySector(pVolumeID) (FAT_PartitionLBA + FAT_GetReservedSectors(pVolumeID) + FAT_NUMBER_OF_FATS * FAT_GetSectorsPerFAT(pVolumeID))

#define FAT16_GetStartCluster(pDirEntry) (uint16_t)(pDirEntry->StartClusterLow)
#define FAT16_IsLastDirEntry(pPartition, pDirEntry, pDirLocation) ((pDirEntry->Name[0] == 0x00) || !FAT16_IsCurrentClusterValid(pPartition, &(pDirLocation)->Location))
#define FAT16_IsCurrentClusterValid(pPartition, pLocation) ((pLocation)->Cluster != 0xFFFF)

/**
 * @brief The FAT16 specific implementation of FAT_GetNextCluster
 * @see FAT_GetNextCluster
 * @ingroup FAT
 */
FAT_API TFatClusterNr FAT16_GetNextCluster(TFatPartition* pPartition, TFatClusterNr CurrentCluster);

/**
 * @brief The FAT16 specific implementation of FAT_GetFirstRootDirEntry
 * @see FAT_GetFirstRootDirEntry
 * @ingroup Dir
 */
FAT_API void FAT16_GetFirstRootDirEntry(TFatPartition* pPartition, TFatDirectoryLocation* pDirLocation);

/**
 * @brief The FAT16 specific implementation of FAT_GetNextRootDirEntry
 * @see FAT_GetNextRootDirEntry
 * @ingroup Dir
 */
FAT_API void FAT16_GetNextRootDirEntry(TFatPartition* pPartition, TFatDirectoryLocation* pDirLocation);

/**
 * @brief The FAT16 specific implementation of FAT_FindRootDirEntry
 * @see FAT_FindRootDirEntry
 * @ingroup Dir
 */
FAT_API TFatDirEntry* FAT16_FindRootDirEntry(TFatPartition* pPartition, char* pName, TFatDirectoryLocation* pDirLocation);

#ifdef FAT_ENABLE_WRITE
FAT_API TFatClusterNr FAT16_FindFreeCluster(TFatPartition* pPartition);

FAT_API void FAT16_LinkClusters(TFatPartition* pPartition, TFatClusterNr FirstCluster, TFatClusterNr SecondCluster);
#endif

#endif
