#ifndef FAT_H_INCLUSION_GUARD
#define FAT_H_INCLUSION_GUARD

/**
 * @defgroup Partition Partition handling.
 * @defgroup Dir Directory handling.
 * @defgroup FAT File Allocation Table handling.
 * @defgroup General General.
 */

#if defined(__GNUC__)
#include <stdint.h>
#elif defined(_MSC_VER)
typedef signed char 	int8_t;
typedef unsigned char 	uint8_t;
typedef signed short 	int16_t;
typedef unsigned short 	uint16_t;
typedef signed long int 	int32_t;
typedef unsigned long int 	uint32_t;
#endif

#include "../fat_conf.h"

#if !defined(FAT_ENABLE_FAT32) && !defined(FAT_ENABLE_FAT16)
#error Neither FAT16 nor FAT32 support is enabled!
#endif

#if defined(FAT_ENABLE_FAT16) && defined(FAT_ENABLE_FAT32)
/**
 * @brief This identifier is defined if both FAT16 and FAT32 support is enabled.
 */
#define FAT_ENABLE_BOTH
#endif

#ifdef FAT_ENABLE_FAT32
/** 
 * @brief Identifies a cluster number.
 * @ingroup General
 */
typedef uint32_t TFatClusterNr;
#else
typedef uint16_t TFatClusterNr;
#endif

/**
 * @brief The partition type
 * @see FAT_IsFAT16, FAT_IsFAT32, FAT_GetPartitionType
 * @ingroup Partition
 */
typedef enum {
  FAT_16,  /**< A FAT16 Partition */
  FAT_32   /**< A FAT32 Partition */
} TFatPartitionType;

/**
 * @brief Partition information
 * @see FAT_OpenPartition
 * @ingroup Partition
 */
typedef struct {
  uint8_t*          pBuffer;               /**< A pointer to a buffer large enough for a disk sector. Must be specified by the application. */
  uint32_t          PartitionLBA;          /**< The offset where the partition data begins - in clusters. */
#ifdef FAT_ENABLE_BOTH
  TFatPartitionType Type;                  /**< The partition type (FAT_16 or FAT_32). */ 
#endif
  uint8_t           SectorsPerCluster;     /**< The number of sectors per cluster. */
  uint16_t          ReservedSectors;       /**< The number of reserved sectors. */
  uint32_t          SectorsPerFAT;         /**< The number of sectors per FAT table. */
  uint16_t          RootDirectoryEntries;  /**< The number of root directory entries. Will be zero (0) for a  FAT32 partition. */
} TFatPartition;

/**
 * @brief Identifies a sector location on the disk
 * @ingroup Partition
 */
typedef struct {
  uint32_t          Sector;                /**< The current sector. */
  TFatClusterNr     Cluster;               /**< The current cluster. */
  uint8_t           SectorsLeftInCluster;  /**< The number of sectors left in the cluster, excluding the one in this structure. */
} TFatLocation;

/**
 * @brief Identifies a directory entry location on the disk
 * @see FAT_NUMBER_OF_DIRECTORY_ENTRIES_PER_SECTOR
 * @ingroup Dir
 */
typedef struct {
  TFatLocation Location;                   /**< The sector it is located in. */
  uint8_t      EntryOffset;                /**< The entry offset, ranging from 0 to FAT_NUMBER_OF_DIRECTORY_ENTRIES_PER_SECTOR. */
} TFatDirectoryLocation;

/**
 * @brief Directory entry information.
 * @see FAT_CreateDirEntry, FAT_CreateRootDirEntry, FAT_FindDirEntry, FAT_FindRootDirEntry
 * @see Dir
 */
typedef struct {
  uint8_t Name[11];             /**< The directory entry name. */
  uint8_t Attributes;           /**< Attributes */
  uint8_t ReservedNT;           /**< Reserved. */
  uint8_t CreationTimeTenth;    /**< The creation time, in 10ms units. Valid values are 0-199*/           
  uint16_t CreationTime;        /**< The creation time. */
  uint16_t CreationDate;        /**< The creation date. */
  uint16_t LastAccessDate;      /**< Last access date. */
  uint16_t StartClusterHigh;    /**< The upper 16 bits of the start cluster address. Will be zero for FAT16. */
  uint16_t ModicationTime;      /**< Modification time. */
  uint16_t ModificationDate;    /**< Modification date. */
  uint16_t StartClusterLow;     /**< The lower 16 bits of teh start cluster address. */
  uint32_t FileSize;            /**< The size of the file, in bytes. */
} TFatDirEntry;

/**
 * @brief The number of FAT tables.
 * @ingroup Partition
 */
#define FAT_NUMBER_OF_FATS (2)

/**
 * @brief The number of bytes per sector.
 * @ingroup Partition
 */
#define FAT_BYTES_PER_SECTOR (512)

/**
 * @brief The size of a directory entry.
 * @ingroup Dir
 */
#define FAT_DIRECTORY_ENTRY_SIZE (sizeof(TFatDirEntry))

/**
 * @brief The number of directory entries per sector.
 * @ingroup Dir
 */
#define FAT_NUMBER_OF_DIRECTORY_ENTRIES_PER_SECTOR (FAT_BYTES_PER_SECTOR / FAT_DIRECTORY_ENTRY_SIZE)

/**
 * @brief The number of FAT entries per sector for FAT16.
 * @ingroup FAT
 */
#define FAT_NUMBER_OF_FAT16_ENTRIES_PER_SECTOR (FAT_BYTES_PER_SECTOR / sizeof(uint16_t))

/**
 * @brief The number of FAT entries per sector for FAT32.
 * @ingroup FAT
 */
#define FAT_NUMBER_OF_FAT32_ENTRIES_PER_SECTOR (FAT_BYTES_PER_SECTOR / sizeof(uint32_t))

#if defined(FAT_ENABLE_FAT16) && !defined(FAT_ENABLE_FAT32)
#define FAT_IsFAT16(pPartition) (1)
#define FAT_IsFAT32(pPartition) (0)
#define FAT_Cond(pPartition, Fat16Expr,Fat32Expr) (Fat16Expr)
#elif !defined(FAT_ENABLE_FAT16) && defined(FAT_ENABLE_FAT32)
#define FAT_IsFAT16(pPartition) (0)
#define FAT_IsFAT32(pPartition) (1)
#define FAT_Cond(pPartition, Fat16Expr,Fat32Expr) (Fat32Expr)
#else

/**
 * @brief Indicates if the current partition is a FAT16 partition.
 * @param pPartition The current partition.
 * @return TRUE of the partition is a FAT16 partition. FALSE otherwise.
 * @ingroup Partition
 */
#define FAT_IsFAT16(pPartition) ((pPartition)->Type == FAT_16)

/**
 * @brief Indicates if the current partition is a FAT32 partition.
 * @param pPartition The current partition.
 * @return TRUE of the partition is a FAT32 partition. FALSE otherwise.
 * @ingroup Partition
 */
#define FAT_IsFAT32(pPartition) ((pPartition)->Type != FAT_16)

#define FAT_Cond(pPartition, Fat16Expr,Fat32Expr) (FAT_IsFAT16(pPartition) ? (Fat16Expr) : (Fat32Expr))
#endif

#ifdef FAT_SINGLE_FILE
#define FAT_API static
#else
#define FAT_API
#endif

#ifdef FAT_ENABLE_FAT16
#include "fat16.h"
#endif

#ifdef FAT_ENABLE_FAT32
#include "fat32.h"
#endif

/* These are valid when the MBR is in the buffer */

/**
 * @brief Returns the logical block address of the partition's first sector.
 * @param pMBR A pointer to the contents of the master boot record.
 * @param PartitionNr The partition number, ranging from 0 to 3.
 * @return The logical block address of the first sector in the partition.
 * @ingroup Partition
 */
#define FAT_GetPartitionLBA(pMBR, PartitionNr) *(uint32_t*)(pMBR + 446 + PartitionNr*16 + 8)

/**
 * @brief Returns the partition type.
 * @param pMBR A pointer to the contents of the master boot record.
 * @param PartitionNr The partition number, ranging from 0 to 3.
 * @return The partition type.
 * @ingroup Partition
 */
#define FAT_GetPartitionType(pMBR, PartitionNr) *(pMBR + 446 + PartitionNr*16 + 4)

/**
 * @brief Returns the partition length, in sectors.
 * @param pMBR A pointer to the contents of the master boot record.
 * @param PartitionNr The partition number, ranging from 0 to 3.
 * @return The number of sectors in the partition.
 * @ingroup Partition
 */
#define FAT_GetPartitionLength(pMBR, PartitionNr) *(uint32_t*)(pMBR + 446 + PartitionNr*16 + 12)

/**
 * @brief Indicates if the partition is bootable.
 * @param pMBR A pointer to the contents of the master boot record.
 * @param PartitionNr The partition number, ranging from 0 to 3.
 * @return TRUE if the partition is bootable. FALSE otherwise.
 * @ingroup Partition
 */
#define FAT_IsPartitionBootable(pMBR, PartitionNr) (*(pMBR + 446 + PartitionNr*16 + 0) == 0x80)

/**
 * @brief Indicates if the master boot record is valid.
 * @param pMBR A pointer to the contents of the master boot record.
 * @return TRUE if the master boot record is valid. FALSE otherwise.
 * @ingroup Partition
 */
#define FAT_IsMBRValid(pMBR) (*(uint16_t*)(pMBR + 510) == 0xAA55)

/**
 * @brief Returns the number of sectors per cluster.
 * @param pVolumeID A pointer to the contents of the Volume ID sector.
 * @return The number of sectors per cluster.
 * @ingroup Partition
 */
#define FAT_GetSectorsPerCluster(pVolumeID) *(pVolumeID + 0xd)

/**
 * @brief Returns the number of reserved sectors.
 * @param pVolumeID A pointer to the contents of the Volume ID sector.
 * @return The number of reserved sectors.
 * @ingroup Partition
 */
#define FAT_GetReservedSectors(pVolumeID) *(uint16_t*)(pVolumeID + 0xe)

/**
 * @note The rest of the implementation only supports two (2) tables. According to
 *       the specification, no other value should ever be used.
 * @brief Returns the number of FAT tables.
 * @param pVolumeID A pointer to the contents of the Volume ID sector.
 * @return The number of FAT tables.
 * @ingroup FAT
 */
#define FAT_GetNrOfFATs(pVolumeID) *(pVolumeID + 0x10)

/**
 * @note This value is only used for FAT16. For FAT32, this will always be zero (0).
 * @brief Returns the number of root directory entries.
 * @param pVolumeID A pointer to the contents of the Volume ID sector.
 * @return The number of root directory entries.
 * @ingroup Dir
 */
#define FAT_GetRootDirectoryEntries(pVolumeID) *(uint16_t*)(pVolumeID + 0x11)

/**
 * @brief Returns the number of sectors per FAT table.
 * @param pPartition The current partition
 * @return The number of sectors per FAT table.
 * @ingroup FAT
 */
#define FAT_GetSectorsPerFAT(pPartition) (FAT_Cond(pPartition, (uint32_t)FAT16_GetSectorsPerFAT(pPartition->pBuffer), FAT32_GetSectorsPerFAT(pPartition->pBuffer)))

/**
 * @brief Returns the sector number where the FAT table is located.
 * @param pPartition The current partition.
 * @return The sector number where the FAT table is located
 * @ingroup FAT
 */
#define FAT_GetFATSector(pPartition) (pPartition->PartitionLBA + pPartition->ReservedSectors)

#define ATTR_READ_ONLY 0x01
#define ATTR_HIDDEN    0x02
#define ATTR_SYSTEM    0x04
#define ATTR_VOLUME_ID 0x08
#define ATTR_DIRECTORY 0x10
#define ATTR_ARCHIVE   0x20
#define ATTR_LONG_NAME (ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID)

/**
 * @brief Returns a pointer to the directory entry information
 * @param pPartition   The current partition.
 * @param pDirLocation The directory entry location information.
 * @return A pointer to the directory entry information.
 * @ingroup Dir
 *
 * @see TFatDirEntry
 */
#define FAT_GetDirEntry(pPartition, pDirLocation) ((TFatDirEntry*)((pPartition)->pBuffer + ((pDirLocation)->EntryOffset * FAT_DIRECTORY_ENTRY_SIZE)))

/**
 * @note Long file name entries should be ignored, since they are not supported.
 * @brief Indicates if the directory entry is a long file name entry.
 * @param pDirEntry The directory entry information.
 * @return TRUE if the entry is a long file name entry. FALSE otherwise.
 * @ingroup Dir
 */
#define FAT_IsLongFileName(pDirEntry) ((uint8_t)(pDirEntry->Attributes & 0x0F) == (uint8_t)ATTR_LONG_NAME)

/* To use these, you'll first have to make sure the entry is not a long file name. */
/**
 * @note This function can only be called when it is determined 
 *       that the entry is not a long file name entry.
 * @brief Indicates if the directory entry represents a file.
 * @param pDirEntry The directory entry information.
 * @return TRUE if the entry is a file. FALSE otherwise.
 * @ingroup Dir
 */
#define FAT_IsFile(pDirEntry) (((uint8_t)(pDirEntry->Attributes & (ATTR_VOLUME_ID | ATTR_DIRECTORY))) == 0)

/**
 * @note This function can only be called when it is determined 
 *       that the entry is not a long file name entry.
 * @brief Indicates if the directory entry represents a directory.
 * @param pDirEntry The directory entry information.
 * @return TRUE if the entry is a directroy. FALSE otherwise.
 * @ingroup Dir
 */
#define FAT_IsDirectory(pDirEntry) (uint8_t)(pDirEntry->Attributes & ATTR_DIRECTORY)

/**
 * @note This function can only be called when it is determined 
 *       that the entry is not a long file name entry.
 * @brief Indicates if the directory entry represents a volume identifier.
 * @param pDirEntry The directory entry information.
 * @return TRUE if the entry is a volume identifier. FALSE otherwise.
 * @ingroup Dir
 */
#define FAT_IsVolumeID(pDirEntry) (uint8_t)(pDirEntry->Attributes & ATTR_VOLUME_ID)

/**
 * @note This function can only be called when it is determined 
 *       that the entry is not a long file name entry.
 * @brief Indicates if the directory entry is write protected.
 * @param pDirEntry The directory entry information.
 * @return TRUE if the entry is write protected. FALSE otherwise.
 * @ingroup Dir
 */
#define FAT_IsReadOnly(pDirEntry) (uint8_t)(pDirEntry->Attributes & ATTR_READ_ONLY)


#ifdef FAT_ENABLE_FAT32
/* The FAT32 variant works for FAT16 as well */
/**
 * @note This function can only be called when it is determined 
 *       that the entry is not a long file name entry.
 * @brief Returns the start address, in clusters, of the directory entry.
 * @param pDirEntry The directory entry information.
 * @return The cluster start address of the entry.
 * @ingroup Dir
 */
#define FAT_GetStartCluster(pDirEntry) FAT32_GetStartCluster(pDirEntry)
#else
#define FAT_GetStartCluster(pDirEntry) FAT16_GetStartCluster(pDirEntry)
#endif

/**
 * @note This function can only be called when it is determined 
 *       that the entry is not a long file name entry.
 * @brief Indicates if the directory entry has been deleted.
 * @param pDirEntry The directory entry information.
 * @return TRUE if the entry has been deleted. FALSE otherwise.
 * @ingroup Dir
 */
#define FAT_IsDirEntryDeleted(pDirEntry) (pDirEntry->Name[0] == 0xE5)

/**
 * @note If this function returns TRUE, the entry must not be examined
 *       further, since it is not valid.
 * @brief Indicates if the directory entry is the last entry.
 * @param pPartition    The current partition.
 * @param pDirEntry     The directory entry information.
 * @param pDirLocation  The directory entry location.
 * @return TRUE if the entry is the last entry in the directory.
 * @ingroup Dir
 */
#define FAT_IsLastDirEntry(pPartition, pDirEntry, pDirLocation) ((pDirEntry->Name[0] == 0x00) || !FAT_IsCurrentClusterValid(pPartition, &(pDirLocation)->Location))
#define FAT_IsCurrentClusterValid(pPartition, pLocation) (FAT_Cond(pPartition, FAT16_IsCurrentClusterValid(pPartition, pLocation), FAT32_IsCurrentClusterValid(pPartition, pLocation)))

/**
 * Seeks to the cluster specified. After this call, FAT_ReadSector
 * or FAT_WriteSector may be called with pLocation->Sector as the sector
 * to operate on. 
 *
 * @note Only pLocation will be updated - the sector buffer will not be touched.
 *
 * @brief Returns the first sector of the specified cluster.
 * @param pPartition The current partition.
 * @param pLocation The information about the new location.
 * @param ClusterNr The cluster number to seek to.
 * @return Nothing.
 * @ingroup General
 *
 * @see FAT_ReadSector, FAT_WriteSector
 */
FAT_API void FAT_Seek(const TFatPartition* pPartition, TFatLocation* pLocation, TFatClusterNr ClusterNr);

/**
 * @brief Returns the cluster number that follows the given cluster number in the FAT table.
 * @param pPartition The current partition.
 * @param CurrentCluster The current cluster.
 * @return The cluster following the given cluster.
 * @ingroup FAT
 * 
 * @see FAT_ReadNextCluster
 */
#define FAT_GetNextCluster(pPartition, CurrentCluster) (FAT_Cond(pPartition, FAT16_GetNextCluster(pPartition, CurrentCluster), FAT32_GetNextCluster(pPartition, CurrentCluster)))

/**
 * Opens a FAT partition.
 *
 * If the function indicates a failure, this can be due to an invalid master boot
 * record, an invalid partition type (Only FAT16 and FAT32 is supported, depending 
 * on configuration) or other problems.
 *
 * Only partitions 0-3 are valid.
 *
 * pPartition->pBuffer must be set prior to calling this function and should
 * point to a buffer, large enough for a disk sector (512 bytes). Other 
 * members in this structure will be written by this function and their original
 * values are ignored.
 *
 * This function must be called before using any other function.
 *
 * @brief Opens a FAT partition.
 * @param pPartition The current partition. 
 * @param PartitionNr The partition number to open.
 * @return 1 on success, 0 on failure.
 * @ingroup Partition
 */
FAT_API uint8_t FAT_OpenPartition(TFatPartition* pPartition, uint8_t PartitionNr);

/**
 * @note The next clusters in the cluster chain can be read
 *       using FAT_ReadNextSector
 *
 * @brief Reads the first cluster of a cluster chain.
 * @param pPartition The current partition.
 * @param pLocation The current location.
 * @return Nothing.
 * @ingroup General
 */
#define FAT_ReadFirstSector(pPartition, pLocation) FAT_ReadSector(pPartition, pLocation->Sector)

/**
 * @note After a call to FAT_Seek, the first sector in the cluster
 * must be read by calling FAT_ReadFirstSector.
 * The following sectors can be read using this function.
 *
 * @brief Reads the next sector, following the cluster chain. 
 * @param pPartition The currently active partition.
 * @param pLocation The current location.
 * @return Nothing.
 * @ingroup General
 *
 * @see FAT_Seek, FAT_ReadFirstSector
 */ 
FAT_API void FAT_ReadNextSector(TFatPartition* pPartition, TFatLocation* pLocation);

/**
 * On exit, FAT_IsLastDirectoryEntry should be called to see if
 * an entry is found. To see if the entry is valid, FAT_IsDirEntryDeleted
 * should be called.
 *
 * To iterate to the next directory entry, FAT_GetNextDirectoryEntry should 
 * be used. Note that it's important to preserve pDirLocation 
 * between these calls.
 *
 * To find the first directory of the root directory, use
 * FAT_GetFirstRootDirectory instead.
 *
 * @brief Finds the first directory entry in the directory that starts at 'StartCluster'.
 * @param pPartition   The current partition.
 * @param StartCluster The cluster where the directory starts.
 * @param pDirLocation Contains information about the currently examined directory entry. The initial values are ignored.
 * @return Nothing.
 * @ingroup Dir
 *
 * @see FAT_GetNextDirectoryEntry, FAT_IsLastDirectoryEntry, FAT_IsDirEntryDeleted FAT_GetFirstRootDirEntry
 */
FAT_API void FAT_GetFirstDirectoryEntry(TFatPartition* pPartition, TFatClusterNr StartCluster, TFatDirectoryLocation* pDirLocation);

/**
 * Iterates to the next Directory Entry. FAT_GetFirstDirectoryEntry
 * must have been called prior to calling this function. 
 *
 * To iterate a root directory, use FAT_GetNextRootDirEntry
 * instead. 
 *
 * It is important to preserve the contents of pDirLocation between calls
 * to this function.
 *
 * On exit, FAT_IsLastDirectoryEntry should be called to see if
 * an entry is found. To see if the entry is valid, FAT_IsDirEntryDeleted
 * should be called.
 *
 * @brief Iterates to the next directory entry.
 * @param pPartition The current partition.
 * @param pDirLocation Contains information about the currently examined directory entry. The initial values are ignored.
 * @return Nothing.
 * @ingroup Dir
 *
 * @see FAT_GetFirstDirectoryEntry, FAT_IsLastDirectoryEntry, FAT_IsDirEntryDeleted FAT_GetNextRootDirEntry
 */
FAT_API void FAT_GetNextDirectoryEntry(TFatPartition* pPartition, TFatDirectoryLocation* pDirLocation);

/**
 * Only valid (i.e. not deleted) entries will be searched for and long file names are not supported.
 *
 * pDirLocation will contain the information where the entry is located on success. On failure,
 * its contents will be overwritten. The initial values are ignored.
 *
 * The name specified should be in 8.3 format. Example: "README  TXT". 
 *
 * To find a root directory entry, use FAT_FindRootDirEntry.
 *
 * @brief Finds the directory entry specified.
 * @param pPartition       The current partition.
 * @param DirectoryCluster The first cluster of the directory to be searched.
 * @param pName            The name of the directory entry to match. The name should be in 8.3 format and
 *                         does not have to be null-terminated.
 * @param pDirLocation     Information where the directory entry is located.
 * @return A pointer to the entry information on success. Will be NULL if the entry was not found.
 * @ingroup Dir
 */
FAT_API TFatDirEntry* FAT_FindDirEntry(TFatPartition* pPartition, TFatClusterNr DirectoryCluster, char* pName, TFatDirectoryLocation* pDirLocation);


/**
 * On exit, FAT_IsLastDirectoryEntry should be called to see if
 * an entry is found. To see if the entry is valid, FAT_IsDirEntryDeleted
 * should be called.
 *
 * To iterate to the next directory entry, FAT_GetNextRootDirectoryEntry should 
 * be used. Note that it's important to preserve pDirLocation 
 * between these calls.
 *
 * @brief Finds the first root directory entry
 * @param pPartition   The current partition.
 * @param pDirLocation Contains information about the currently examined directory entry. The initial values are ignored.
 * @return Nothing.
 * @ingroup Dir
 *
 * @see FAT_GetNextRootDirEntry, FAT_IsLastDirectoryEntry, FAT_IsDirEntryDeleted 
 */
#define FAT_GetFirstRootDirEntry(pPartition, pDirLocation) \
  (FAT_Cond(pPartition, \
            FAT16_GetFirstRootDirEntry(pPartition, pDirLocation), \
            FAT32_GetFirstRootDirEntry(pPartition, pDirLocation)))

/**
 * Only valid (i.e. not deleted) entries will be searched for and long file names are not supported.
 *
 * pDirLocation will contain the information where the entry is located on success. On failure,
 * its contents will be overwritten. The initial values are ignored.
 *
 * The name specified should be in 8.3 format. Example: "README  TXT". 
 *
 * @brief Finds the root directory entry specified.
 * @param pPartition       The current partition.
 * @param pName            The name of the directory entry to match. The name should be in 8.3 format and
 *                         does not have to be null-terminated.
 * @param pDirLocation     Information where the directory entry is located.
 * @return A pointer to the entry information on success. Will be NULL if the entry was not found.
 * @ingroup Dir
 */
#define FAT_FindRootDirEntry(pPartition, pName, pDirLocation) \
  (FAT_Cond(pPartition, \
            FAT16_FindRootDirEntry(pPartition, pName, pDirLocation), \
            FAT32_FindRootDirEntry(pPartition, pName, pDirLocation)))

#ifdef FAT_ENABLE_WRITE
/**
 * On success, pDirLocation will contain location information of the new entry. A pointer to the 
 * directory entry structure will also be returned and can be filled out. FAT_WriteSector must
 * be called with pDirLocation->Location.Sector as parameter to finally store the information.
 *
 * On failure, NULL is returned.
 *
 * @brief Create a new Directory Entry to the directory that starts at StartCluster.
 * @param pPartition   The current partition.
 * @param StartCluster The cluster which the directory starts at.
 * @param pDirLocation The location information to the directory entry.
 * @return A pointer to where the directory entry information can be stored, or NULL on failure.
 * @ingroup Dir
 *
 * @see FAT_WriteSector
 */
FAT_API TFatDirEntry* FAT_CreateDirEntry(TFatPartition* pPartition, TFatClusterNr StartCluster, TFatDirectoryLocation* pDirLocation);

/**
 * On success, the FirstCluster specified will link to the newly allocated
 * cluster, continuing the cluster chain. 
 *
 * If FirstCluster is zero (0), the created cluster will not continue 
 * a cluster chain and will be the first cluster in a new cluster chain.
 *
 * On success, pLocation will be updated with the new cluster.
 *
 * If 0 is returned, the disk is full and a new cluster can not be allocated.
 *
 * @brief Creates a new cluster.
 * @param pPartition   The current partition.
 * @param FirstCluster The cluster number that will link to this cluster. 
 *                     May be zero (0) to not link to the new cluster.
 * @param pLocation    Location information about the new cluster on success. 
 * @return 1 on success, 0 on failure.
 * @ingroup FAT
 */
FAT_API uint8_t FAT_CreateCluster(TFatPartition* pPartition, TFatClusterNr FirstCluster, TFatLocation* pLocation);

#define FAT_FindFreeCluster(pPartition) (FAT_Cond(pPartition, FAT16_FindFreeCluster(pPartition), FAT16_FindFreeCluster(pPartition)))

/**
 * Initialises the Directory Entry to all zeros
 * except the filename specified.
 * 
 * @brief Initialises the current Directory Entry
 * @param pPartition The current partition.
 * @param pDirLocation The location information to the directory entry.
 * @param pDirEntryName The name of the directory entry.
 * @return Nothing.
 * @ingroup Dir
 */
void FAT_InitDirEntry(TFatPartition* pPartition, TFatDirectoryLocation* pDirLocation, const char* pDirEntryName);
#endif

/**
 * Since there is no way of recovering from a read error, it is up to the host application
 * to react to disk failures. 
 *
 * @note This function should be implemented by the host application.
 * @brief Reads a sector on the disk.
 * @param pPartition The partition to read. Note that only the pBuffer member should be considered to be valid,
 *                   and it is to this buffer that the sector should be written to.
 * @param SectorNr   The sector number to read.
 * @return Nothing.
 * @ingroup General
 */
FAT_API void FAT_ReadSector(TFatPartition* pPartition, uint32_t SectorNr);

#ifdef FAT_ENABLE_WRITE
/**
 * Since there is no way of recovering from a write error, it is up to the host application
 * to react to disk failures. 
 *
 * @note This function should be implemented by the host application.
 * @brief Reads a sector on the disk.
 * @param pPartition The partition to write. Note that only the pBuffer member should be considered to be valid,
 *                   and it is from this buffer that the sector contents should be read from.
 * @param SectorNr   The sector number to write.
 * @return Nothing.
 * @ingroup General
 */
FAT_API void FAT_WriteSector(TFatPartition* pPartition, uint32_t SectorNr);
#endif

FAT_API uint32_t FAT_GetRootOffset(const TFatPartition* pPartition);


#define FAT_LinkClusters(pPartition, SourceCluster, SecondCluster) (FAT_Cond(pPartition, FAT16_LinkClusters(pPartition, SourceCluster, SecondCluster), FAT16_LinkClusters(pPartition, SourceCluster, SecondCluster)))

#ifdef FAT_DEBUG
/** @brief A debug macro. Whatever is encapsulated with this macro will only be present when debugging. 
 */
#define D_(stmt) stmt
#else
#define D_(stmt) 
#endif

#endif /* FAT_H_INCLUSION_GUARD */

