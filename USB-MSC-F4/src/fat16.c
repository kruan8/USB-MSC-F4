/*
 * fat16.c
 *
 *  Created on: 12. 11. 2019
 *  Author:     Priesol Vladimir
 */

#include "fat16.h"
#include <string.h>
#include <stdio.h>

// http://www.maverick-os.dk/FileSystemFormats/FAT16_FileSystem.html

typedef struct
{
    unsigned char jmp[3];
    char oem[8];
    unsigned short sector_size;
    unsigned char sectors_per_cluster;
    unsigned short reserved_sectors;

    unsigned char number_of_fats;
    unsigned short root_dir_entries;
    unsigned short total_sectors_short; // if zero, later field is used
    unsigned char media_descriptor;
    unsigned short fat_size_sectors;
    unsigned short sectors_per_track;
    unsigned short number_of_heads;
    unsigned long hidden_sectors;

    unsigned long total_sectors_long;

    unsigned char drive_number;
    unsigned char current_head;
    unsigned char boot_signature;
    unsigned long volume_id;
    char volume_label[11];
    char fs_type[8];
    char boot_code[448];
    unsigned short boot_sector_signature;
} __attribute((packed)) Fat16BootSector;

typedef struct
{
    unsigned char filename[8];
    unsigned char ext[3];
    unsigned char attributes;
    // 0x01  Indicates that the file is read only.
    // 0x02  Indicates a hidden file. Such files can be displayed if it is really required.
    // 0x04  Indicates a system file. These are hidden as well.
    // 0x08  Indicates a special entry containing the disk's volume label, instead of describing a file. This kind of entry appears only in the root directory.
    // 0x10  The entry describes a subdirectory.
    // 0x20  This is the archive flag. This can be set and cleared by the programmer or user, but is always set when the file is modified. It is used by backup programs.
    // 0x40  Not used; must be set to 0.
    // 0x80  Not used; must be set to 0.

    unsigned char reserved[8];
    unsigned short ending_cluster;
    unsigned short modify_time;
   /* <------- 0x17 --------> <------- 0x16 -------->
      15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
      h  h  h  h  h  m  m  m  m  m  m  x  x  x  x  x

      hhhhh   indicates the binary number of hours (0-23)
      mmmmmm  indicates the binary number of minutes (0-59)
      xxxxx   indicates the binary number of two-second periods (0-29), representing seconds 0 to 58.
      */

    unsigned short modify_date;
    /* <------- 0x19 --------> <------- 0x18 -------->
       15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
       y  y  y  y  y  y  y  m  m  m  m  d  d  d  d  d
       yyyyyyy   indicates the binary year offset from 1980 (0-119), representing the years 1980 to 2099
       mmmm      indicates the binary month number (1-12)
       ddddd     indicates the binary day number (1-31)
       */


    unsigned short starting_cluster;
    unsigned long file_size;
} __attribute((packed)) Fat16DirEntry;


#define STORAGE_SIZE                  26213376

#define FAT16_SECTOR_SIZE                  512
#define FAT16_FAT_ITEM_SIZE                  2
#define FAT16_SECTORS_PER_CLUSTER            1
#define FAT16_BLOCK_SIZE                  (FAT16_SECTOR_SIZE * FAT16_SECTORS_PER_CLUSTER)
#define FAT16_RESERVED_SECTORS               1
#define FAT16_NUMBER_OF_FATS                 1
#define FAT16_FAT_SIZE_SECTORS             200
#define FAT16_FAT_ITEMS_PER_SECTOR        (FAT16_BLOCK_SIZE / FAT16_FAT_ITEM_SIZE)
#define FAT16_DIR_ENTRIES                  128
#define FAT16_DIR_ENTRY_SIZE                32
#define FAT16_DIR_BLOCKS                  (FAT16_DIR_ENTRIES * FAT16_DIR_ENTRY_SIZE / FAT16_SECTOR_SIZE)
#define FAT16_DIR_ENTRIES_IN_BLOCK        (FAT16_SECTOR_SIZE / FAT16_DIR_ENTRY_SIZE)

#define FAT16_BOOT_BLOK                      0
#define FAT16_FAT_BLOCK                   (FAT16_BOOT_BLOK + FAT16_RESERVED_SECTORS)
#define FAT16_DIR_BLOCK                   (FAT16_FAT_BLOCK + FAT16_NUMBER_OF_FATS * FAT16_FAT_SIZE_SECTORS)
#define FAT16_DATA_BLOCK                  (FAT16_DIR_BLOCK + FAT16_DIR_BLOCKS)


#define FAT16_NAME_SIZE                      8
#define FAT16_EXT_SIZE                       3

const static Fat16BootSector g_BootRecordFAT16struct =
{
   .jmp[0] = 0xEB,
   .jmp[1] = 0x3C,
   .jmp[2] = 0x90,
   .oem = "MSDOS5.0",
   .sector_size = FAT16_SECTOR_SIZE,
   .sectors_per_cluster = FAT16_SECTORS_PER_CLUSTER,
   .reserved_sectors = FAT16_RESERVED_SECTORS,
   .number_of_fats = FAT16_NUMBER_OF_FATS,
   .root_dir_entries = FAT16_DIR_ENTRIES,  // *32 je velikost DIR (8 blocks)
   .total_sectors_short = 65535,
   .media_descriptor = 0xF8,
   .fat_size_sectors = FAT16_FAT_SIZE_SECTORS,
   .sectors_per_track = 63,
   .number_of_heads = 255,
   .hidden_sectors = 129,
   .total_sectors_long = 0,
   .drive_number = 0x80,
   .current_head = 0,
   .boot_signature = 41,
   .volume_id = 0,
   .volume_label = "CAVEATRON  ",
   .fs_type = "FAT16   ",
   .boot_sector_signature = 0xAA55,
};

const static uint8_t g_FATSect0[] =
{
    0xF8, 0xFF, 0xFF, 0xFF
};

static const Fat16DirEntry g_VolumeLabelFile =
{
  .filename = "CAVEATRO",
  .ext = "N  ",
  .attributes = 0x08,
  .modify_time = 0,
  .modify_date = 0,
  .starting_cluster = 0,
  .file_size = 0,
};

//static const Fat16DirEntry g_SubDirFile =
//{
//  .filename = "SUBDIR  ",
//  .ext = "   ",
//  .attributes = 0x10,
//  .modify_time = 0xA35A,
//  .modify_date = 0x4081,
//  .starting_cluster = 15,
//  .file_size = 0,
//};

static Fat16DirEntry g_arrFatFiles[FAT16_DIR_ENTRIES] __attribute__((section(".ccmram")));
static uint16_t      g_nFilesCount;


typedef struct
{
  char strFilename[12];
  uint32_t nDataCount;
} cave_file_t;

const cave_file_t arrFiles[] =
{
    { "CAVE_1", 32 },
    { "CAVE_2", 50 },
    { "CAVE_3", 37 },
    { "CAVE_4", 16 },
    { "CAVE_5", 28 },
};

#define STATION_LEN               4
#define DATA_COUNT               50
#define DATA_ITEM_BUFFER         32     // size has to by power of 2 only
#define ITEMS_PER_SECTOR        (FAT16_BLOCK_SIZE / DATA_ITEM_BUFFER)

typedef struct
{
  uint32_t       nDistance_mm;                 // mm
  uint16_t       nAzimuth;
  int16_t        nIncl;
  char           strFrom[STATION_LEN + 1];
  char           strTo[STATION_LEN + 1];
} cave_data_t;

static cave_data_t  g_arrData[DATA_COUNT];
static char         g_arrItemBuffer[DATA_ITEM_BUFFER + 1];

void FAT16_Init(void)
{
  for (uint16_t i = 0; i < DATA_COUNT; ++i)
  {
    g_arrData[i].nAzimuth = i * 5;
    g_arrData[i].nIncl = i;
    g_arrData[i].nDistance_mm = 1000 + i * 50;
    snprintf(g_arrData[i].strFrom, STATION_LEN, "A%d", i + 1);
    snprintf(g_arrData[i].strTo, STATION_LEN, "A%d", i + 2);
  }

  char*  strHeader = "From;To;Length;Azimuth;Inclination\n";

  // add volume name entry
  memcpy (&g_arrFatFiles[0], &g_VolumeLabelFile, sizeof(Fat16DirEntry));
  g_nFilesCount = 1;

  // add all files from Flash storage
  uint16_t nCluster = 2;
  Fat16DirEntry    dirEntry;
  dirEntry.attributes = 0x21;
  dirEntry.modify_time = 0xA35A;
  dirEntry.modify_date = 0x4E81;
  memcpy(dirEntry.ext, "TXT", 3);

  uint16_t nFilesCount = sizeof(arrFiles) / sizeof(cave_file_t);
  for (uint8_t i = 0; i < nFilesCount; ++i)
  {
    memset(dirEntry.filename, ' ', 8);
    memcpy(dirEntry.filename, arrFiles[i].strFilename, strlen(arrFiles[i].strFilename));
    dirEntry.file_size = arrFiles[i].nDataCount * DATA_ITEM_BUFFER;
    dirEntry.starting_cluster = nCluster;
    nCluster += dirEntry.file_size / FAT16_BLOCK_SIZE;
    dirEntry.ending_cluster = nCluster;

    memcpy(&g_arrFatFiles[g_nFilesCount], &dirEntry, sizeof(Fat16DirEntry));

    nCluster++;
    g_nFilesCount++;
  }
}

void FAT16_CreateBlockFAT(uint8_t *buf, uint16_t nBlockOffset)
{
  uint16_t nSectorIndex = nBlockOffset * FAT16_FAT_ITEMS_PER_SECTOR;
  memset(buf, 0, FAT16_BLOCK_SIZE);

  if (nBlockOffset == 0)
  {
    memcpy(buf, g_FATSect0, sizeof(g_FATSect0));
    nSectorIndex = 2;
  }

  for (uint16_t i = 0; i < g_nFilesCount; ++i)
  {
    if (g_arrFatFiles[i].file_size == 0)
    {
      continue;
    }

    for (uint16_t nFatEntry = g_arrFatFiles[i].starting_cluster; nFatEntry <= g_arrFatFiles[i].ending_cluster; ++nFatEntry)
    {
      if (nFatEntry / FAT16_FAT_ITEMS_PER_SECTOR == nBlockOffset)
      {
        uint16_t* pBuffer = (uint16_t*)buf + nSectorIndex;
        if (nFatEntry == g_arrFatFiles[i].ending_cluster)
        {
          *pBuffer = 0xFFFF;
        }
        else
        {
          *pBuffer = nFatEntry + 1;
        }

        nSectorIndex++;
        if ((nSectorIndex % FAT16_FAT_ITEMS_PER_SECTOR) == 0)
        {
          return;
        }
      }
    }
  }
}

void FAT16_CreateBlockDIR(uint8_t *buf, uint16_t nBlockOffset)
{
  memset(buf, 0, FAT16_BLOCK_SIZE);
  uint16_t  nDirEntryIndex = FAT16_BLOCK_SIZE * nBlockOffset / FAT16_DIR_ENTRY_SIZE;
  for (uint16_t i = 0; g_nFilesCount > nDirEntryIndex && i < FAT16_DIR_ENTRIES_IN_BLOCK; ++nDirEntryIndex, ++i)
  {
    memcpy(buf + i * FAT16_DIR_ENTRY_SIZE, &g_arrFatFiles[nDirEntryIndex], sizeof(Fat16DirEntry));
  }
}

void FAT16_CreateBlockDATA(uint8_t *buf, uint16_t nBlockOffset)
{
  memset(buf, ' ', FAT16_BLOCK_SIZE);

  nBlockOffset += 2;
  for (uint8_t i = 0; i < g_nFilesCount; ++i)
  {
    if (g_arrFatFiles[i].file_size == 0)
    {
      continue;
    }

    if (nBlockOffset >= g_arrFatFiles[i].starting_cluster && nBlockOffset <= g_arrFatFiles[i].ending_cluster)
    {
      uint32_t nDataItem = (nBlockOffset - g_arrFatFiles[i].starting_cluster) * ITEMS_PER_SECTOR;
      for (uint8_t ix = 0; ix < ITEMS_PER_SECTOR; ++ix)
      {
        if ((nDataItem + ix) * DATA_ITEM_BUFFER >= g_arrFatFiles[i].file_size)
        {
          break;
        }

        cave_data_t* pData = &g_arrData[nDataItem + ix];
        snprintf(g_arrItemBuffer, DATA_ITEM_BUFFER + 1, "%6s;%6s;%5lu;%5d;%5d\n", pData->strFrom, pData->strTo, pData->nDistance_mm, pData->nIncl, pData->nAzimuth);
        memcpy(buf + ix * DATA_ITEM_BUFFER, g_arrItemBuffer, strlen(g_arrItemBuffer));
      }

      break;
    }
  }

}

// ------------------------------------------------------------------------------------------
void FAT16_CreateBlock(uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{
  uint32_t nBufPos = 0;
  while (blk_len--)
  {
    if (blk_addr == FAT16_BOOT_BLOK)
    {
      memcpy(buf, &g_BootRecordFAT16struct, sizeof(g_BootRecordFAT16struct));
    }
    else if (blk_addr >= FAT16_FAT_BLOCK && blk_addr < FAT16_DIR_BLOCK)
    {
      FAT16_CreateBlockFAT(buf + nBufPos, blk_addr - FAT16_FAT_BLOCK);
    }
    else if (blk_addr >= FAT16_DIR_BLOCK && blk_addr < FAT16_DATA_BLOCK)
    {
      FAT16_CreateBlockDIR(buf + nBufPos, blk_addr - FAT16_DIR_BLOCK);
    }
    else
    {
      FAT16_CreateBlockDATA(buf + nBufPos, blk_addr - FAT16_DATA_BLOCK);
    }

    nBufPos += FAT16_BLOCK_SIZE;
    blk_addr++;
  }
}

uint32_t FAT16_GetStorageSize(void)
{
  return STORAGE_SIZE;
}
