#ifdef _WIN32
#include <windows.h>
#endif

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "../include/fat.h"

#ifdef _WIN32
static HANDLE hFile;
#endif
#ifdef __unix__
static FILE* fp;
#endif

static uint8_t FAT_Buffer[FAT_BYTES_PER_SECTOR];

void FAT_ReadSector(TFatPartition* pPartition, uint32_t SectorNr)
{
#ifdef FAT_DEBUG
  printf(" Reading sector: %d\n", SectorNr);
#endif

#ifdef _WIN32
  {
    DWORD BytesRead;
    assert(SetFilePointer(hFile, SectorNr * FAT_BYTES_PER_SECTOR, NULL, FILE_BEGIN) != INVALID_SET_FILE_POINTER);
    assert(ReadFile(hFile, (LPVOID)pPartition->pBuffer, FAT_BYTES_PER_SECTOR, &BytesRead, NULL) != 0);
    assert(BytesRead == FAT_BYTES_PER_SECTOR);
  }
#endif
#ifdef __unix__
  fseek(fp, SectorNr * FAT_BYTES_PER_SECTOR, SEEK_SET);
  assert(fread((void*)FAT_Buffer, 1, FAT_BYTES_PER_SECTOR, fp) == FAT_BYTES_PER_SECTOR);
#endif
}

void FAT_WriteSector(TFatPartition* pPartition, uint32_t SectorNr)
{
#ifdef FAT_DEBUG
  printf(" Writing sector: %d\n", SectorNr);
#endif

#ifdef _WIN32
  {
    DWORD BytesWritten;
    assert(SetFilePointer(hFile, SectorNr * FAT_BYTES_PER_SECTOR, NULL, FILE_BEGIN) != INVALID_SET_FILE_POINTER);
    assert(WriteFile(hFile, (LPVOID)pPartition->pBuffer, FAT_BYTES_PER_SECTOR, &BytesWritten, NULL) != 0);
    assert(BytesWritten == FAT_BYTES_PER_SECTOR);
  }
#endif
#ifdef __unix__
  fseek(fp, SectorNr * FAT_BYTES_PER_SECTOR, SEEK_SET);
  assert(fwrite((void*)FAT_Buffer, 1, FAT_BYTES_PER_SECTOR, fp) == FAT_BYTES_PER_SECTOR);
#endif
}

int main (int argc, char *argv[])
{
  TFatPartition Partition;

  if (argc != 2)
  {
    printf("Usage: %s <disk_image>\n", argv[0]);
    return EXIT_FAILURE;
  }

#ifdef _WIN32
  hFile = CreateFileA(argv[1],
                     GENERIC_READ,          
                     FILE_SHARE_READ,       
                     NULL,                  
                     OPEN_EXISTING,        
                     FILE_ATTRIBUTE_NORMAL,
                     NULL);                
 
  assert(hFile != INVALID_HANDLE_VALUE);
#elif __unix__
  fp = fopen(argv[1], "r+");
  assert(fp != NULL);
#endif

  Partition.pBuffer = FAT_Buffer;

  if (FAT_OpenPartition(&Partition, 0))
  {
    TFatDirectoryLocation DirLocation;

    FAT_FindRootDirEntry(&Partition, "FIXAT   TT", &DirLocation);

    if (FAT_IsLastDirEntry(&Partition, FAT_GetDirEntry(&Partition, &DirLocation), &DirLocation))
    {
      printf("The file was not found\n");
    }
    else
    {
      printf("Fixat.TXT starts at cluster: %d\n", FAT_GetStartCluster(FAT_GetDirEntry(&Partition, &DirLocation)));
    }

    FAT_FindDirEntry(&Partition, 8, "FIXAT   TT", &DirLocation);

    /*
    if (FAT_CreateDirEntry(4))
    {
      FAT_InitDirEntry("MYFILE  TXT");
      printf("Directory entry created.\n");
      if (FAT_CreateFreeCluster(0))
      {
	printf("1. Found free cluster: %d\n", FAT_CurrentCluster);
	FAT_CreateFreeCluster(FAT_CurrentCluster);
	printf("2. Found free cluster: %d\n", FAT_CurrentCluster);
	FAT_CreateFreeCluster(FAT_CurrentCluster);
	printf("3. Found free cluster: %d\n", FAT_CurrentCluster);
      }
    }
    else*/
    {
      printf("Couldn't create directory entry.\n");
    }
  }
  else
  {
    printf("FATAL: The disk is either corrupt, or has an invalid partition type.\n");
  }
  return 0;
}
