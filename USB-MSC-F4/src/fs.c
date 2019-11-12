/*
 * fs.c
 * File system
 *
 *  Created on: 1. 3. 2019
 *  Author:     Priesol Vladimir
 */

#include "fs.h"
#include "FlashG25.h"
#include "lfs.h"
#include <stdio.h>
#include <string.h>

static int _FS_block_device_read(const struct lfs_config *c, lfs_block_t block,
  lfs_off_t off, void *buffer, lfs_size_t size);
static int _FS_block_device_prog(const struct lfs_config *c, lfs_block_t block,
  lfs_off_t off, const void *buffer, lfs_size_t size);
static int _FS_block_device_erase(const struct lfs_config *c, lfs_block_t block);
static int _FS_block_device_sync(const struct lfs_config *c);
static int _FS_traverse_df_cb(void *p, lfs_block_t block);

// variables used by the filesystem
static lfs_t lfs;
static lfs_file_t file;

// configuration of the filesystem is provided by this struct
const struct lfs_config cfg =
{
    // block device operations
    .read  = _FS_block_device_read,
    .prog  = _FS_block_device_prog,
    .erase = _FS_block_device_erase,
    .sync  = _FS_block_device_sync,

    // block device configuration
    .read_size = 16,
    .prog_size = 16,
    .block_size = 4096,
    .block_count = 128,
//    .lookahead = 128,
};


static const char* g_strSettingsFile = "Settings";

bool FS_Init(void)
{
  // mount the filesystem
  int err = lfs_mount(&lfs, &cfg);

  // reformat if we can't mount the filesystem
  // this should only happen on the first boot
  if (err)
  {
    lfs_format(&lfs, &cfg);
    lfs_mount(&lfs, &cfg);
  }

  return true;
}

int32_t FS_GetNextFilename(const char* strPath, char* strFilename, int32_t nPos)
{
  lfs_dir_t dir;
  struct lfs_info info;

  int result = lfs_dir_open(&lfs, &dir, strPath);

  if (nPos != 0)
  {
    result = lfs_dir_seek(&lfs, &dir, nPos);
  }

  result = 1;
  while (result == 1)
  {
    result = lfs_dir_read(&lfs, &dir, &info);  // OK: res = 1
    if ((strcmp(info.name, ".") == 0 || strcmp(info.name, "..") == 0))
    {
      continue;
    }

    char* pDat = strstr(info.name, ".dat");
    if (pDat != NULL)
    {
      *pDat = 0;
      break;
    }
  }

  nPos = -1;
  if (result == 1)
  {
    strcpy(strFilename, (char*)info.name);
    nPos = lfs_dir_tell(&lfs, &dir);
  }

  lfs_dir_close(&lfs, &dir);
  return nPos;
}

bool FS_DeleteDataFile(char* strSurvey)
{
  char strFilename[50];

  snprintf(strFilename, sizeof(strFilename), "%s.dat", strSurvey);

  int32_t res = lfs_remove(&lfs, strFilename);
  if (res != 0)
  {
    return false;
  }

  return true;
}

bool FS_NewDataFile(const char* strSurvey)
{
  char strFilename[FS_FILENAME_LEN];

  snprintf(strFilename, sizeof(strFilename), "%s.dat", strSurvey);
  int nRes = lfs_file_open(&lfs, &file, strFilename, LFS_O_WRONLY | LFS_O_CREAT | LFS_O_APPEND | LFS_O_EXCL);
  if (nRes == LFS_ERR_OK)
  {
    lfs_file_close(&lfs, &file);
    return true;
  }

  return false;
}

bool FS_SaveData(const char* strSurvey, uint8_t* pBuffer, uint16_t nLen)
{
  // save temporary file with postfix '_'
  char strTmpName[FS_FILENAME_LEN];
  char strFileName[FS_FILENAME_LEN];

  snprintf(strTmpName, FS_FILENAME_LEN, "%s_.dat", strSurvey);
  snprintf(strFileName, FS_FILENAME_LEN, "%s.dat", strSurvey);

  if (lfs_file_open(&lfs, &file, strTmpName, LFS_O_CREAT) != LFS_ERR_OK)
  {
    return false;
  }

  lfs_ssize_t nSize = lfs_file_write(&lfs, &file, pBuffer, nLen);
  if (nSize != nLen)
  {
    return false;
  }

  lfs_file_close(&lfs, &file);

  if (lfs_remove(&lfs, strFileName) != LFS_ERR_OK)
  {
    return false;
  }

  if (lfs_rename(&lfs, strTmpName, strFileName) != LFS_ERR_OK)
  {
    return false;
  }

  return true;
}

int32_t FS_WriteData(const char* strCave, uint8_t* pBuffer, uint16_t nLen)
{
  char strFilename[50];

  snprintf(strFilename, sizeof(strFilename), "%s.dat", strCave);

  lfs_file_open(&lfs, &file, strFilename, LFS_O_WRONLY | LFS_O_APPEND);
  lfs_ssize_t size = lfs_file_write(&lfs, &file, pBuffer, nLen);
  lfs_file_close(&lfs, &file);
  return size;
}

int32_t FS_ReadData(const char* strCave, uint8_t* pBuffer, uint16_t nLen, uint32_t nOffset)
{
  char strFilename[50];

  snprintf(strFilename, sizeof(strFilename), "%s.dat", strCave);

  int32_t nRes;
  nRes = lfs_file_open(&lfs, &file, strFilename, LFS_O_RDONLY);
  if (nRes < 0)
  {
    return nRes;
  }

  lfs_file_seek(&lfs, &file, nOffset, LFS_SEEK_SET);
  lfs_ssize_t size = lfs_file_read(&lfs, &file, pBuffer, nLen);
  lfs_file_close(&lfs, &file);
  return size;
}

int32_t FS_WriteSettings(uint8_t* pBuffer, uint16_t nLen)
{
  lfs_file_open(&lfs, &file, g_strSettingsFile, LFS_O_WRONLY | LFS_O_CREAT);
  lfs_ssize_t size = lfs_file_write(&lfs, &file, pBuffer, nLen);
  lfs_file_close(&lfs, &file);
  return size;
}

int32_t FS_ReadSettings(uint8_t* pBuffer, uint16_t nLen)
{
  lfs_file_open(&lfs, &file, g_strSettingsFile, LFS_O_RDONLY);
  lfs_ssize_t size = lfs_file_read(&lfs, &file, pBuffer, nLen);
  lfs_file_close(&lfs, &file);
  return size;
}

int FS_Format(void)
{
  return lfs_format(&lfs, &cfg);
}

int _FS_traverse_df_cb(void *p, lfs_block_t block)
{
  uint32_t *nb = p;
  *nb += 1;
  return 0;
}

int32_t FS_GetFreeSize(int32_t* pnSize, int32_t* pnFreeSize)
{
  uint32_t _df_nballocatedblock = 0;
  int err = lfs_traverse(&lfs, _FS_traverse_df_cb, &_df_nballocatedblock);
  if (err == LFS_ERR_OK)
  {
    *pnSize = cfg.block_count * cfg.block_size;
    *pnFreeSize = *pnSize - _df_nballocatedblock * cfg.block_size;
  }

  return err;
}

// interface to flash memory ---------------------------

int _FS_block_device_read(const struct lfs_config *c, lfs_block_t block,
  lfs_off_t off, void *buffer, lfs_size_t size)
{
  FlashG25_ReadData((block * c->block_size + off), (uint8_t*)buffer, size);
  return 0;
}

int _FS_block_device_prog(const struct lfs_config *c, lfs_block_t block,
  lfs_off_t off, const void *buffer, lfs_size_t size)
{
  FlashG25_WriteData((block * c->block_size + off), (uint8_t*)buffer, size);

  return 0;
}

int _FS_block_device_erase(const struct lfs_config *c, lfs_block_t block)
{
  FlashG25_SectorErase(block);
  return 0;
}

int _FS_block_device_sync(const struct lfs_config *c)
{
  return 0;
}
