/*************************************************************
Copyright (C), 2022--2023, Wuhan Guide Sensmart Tech Co., Ltd
File name   : Flash120.h
Author      : Guide Sensmart RD. wangyan
Version     : 1.0
Date        : 2022/4/21
Description : ITA SDK Flash120.
*************************************************************/
#ifndef ITA_FLASH120_H
#define ITA_FLASH120_H

#include "../ITADTD.h"
#include "../Module/GuideLog.h"

#define P25Qx_OK            ((unsigned char)0x00)
#define P25Qx_ERROR         ((unsigned char)0x01)
#define P25Qx_BUSY          ((unsigned char)0x02)
#define P25Qx_TIMEOUT		((unsigned char)0x03)

#define P25Q128FV_BULK_ERASE_MAX_TIME         250000
#define P25Q128FV_SECTOR_ERASE_MAX_TIME       3000
#define P25Q128FV_SUBSECTOR_ERASE_MAX_TIME    800
#define P25Qx_TIMEOUT_VALUE 1000

/**
* @brief  P25Q128FV Commands
*/
/* Reset Operations */
#define RESET_ENABLE_CMD                     0x66//
#define RESET_MEMORY_CMD                     0x99//

#define ENTER_QPI_MODE_CMD                   0x38
#define EXIT_QPI_MODE_CMD                    0xFF

/* Identification Operations */
#define READ_ID_CMD                          0x90//
#define DUAL_READ_ID_CMD                     0x92//
#define QUAD_READ_ID_CMD                     0x94//
#define READ_JEDEC_ID_CMD                    0x9F//

#define READ_UID_CMD                          0x4B//

/* Read Operations */
#define READ_CMD                             0x03//read
#define FAST_READ_CMD                        0x0B//
#define DUAL_OUT_FAST_READ_CMD               0x3B//
#define DUAL_INOUT_FAST_READ_CMD             0xBB//
#define QUAD_OUT_FAST_READ_CMD               0x6B//
#define QUAD_INOUT_FAST_READ_CMD             0xEB//

/* Write Operations */
#define WRITE_ENABLE_CMD                     0x06//WEL
#define WRITE_DISABLE_CMD                    0x04//

/* Register Operations */
#define READ_STATUS_REG1_CMD                  0x05//read out status register
#define READ_STATUS_REG2_CMD                  0x35//Read out status register-1
#define READ_STATUS_REG3_CMD                  0x15

#define WRITE_STATUS_REG1_CMD                 0x01//
#define WRITE_STATUS_REG2_CMD                 0x31
#define WRITE_STATUS_REG3_CMD                 0x11

/* Program Operations */
#define PAGE_PROG_CMD                        0x02//write
#define QUAD_INPUT_PAGE_PROG_CMD             0x32//

/* Erase Operations */
#define PAGE_ERASE_CMD                      0x81//
#define SECTOR_ERASE_CMD                     0x20//
#define CHIP_ERASE_CMD                       0xC7//

#define PROG_ERASE_RESUME_CMD                0x7A//
#define PROG_ERASE_SUSPEND_CMD               0x75//

/* Flag Status Register */
#define P25Q128FV_FSR_BUSY                    ((unsigned char)0x01)    /*!< busy */
#define P25Q128FV_FSR_WREN                    ((unsigned char)0x02)    /*!< write enable */
#define P25Q128FV_FSR_QE                      ((unsigned char)0x02)    /*!< quad enable */

#define SECTOR_SIZE 0x1000    //0x1000 = 4096
#define PAGE_SIZE (256)

class Flash120
{
public:
	Flash120(ITARegistry * registry, int cameraID);
	~Flash120();
	/*120 90°模组现阶段使用的是P25Q20U版本，容量大小256Kbyte。后面会用到P25Q40U版本，容量大小512Kbyte。*/
	ITA_RESULT P25QInit();
	ITA_RESULT P25QExit();
	ITA_RESULT P25QReadID(unsigned short *pID);
	ITA_RESULT P25QReadInfo(unsigned char *pBuf, int size);
	ITA_RESULT P25QReadCurve(int sectorID, unsigned char *pBuf, int size);
	ITA_RESULT P25QWriteCurve(int sectorID, unsigned char *pBuf, int size);
	/*ITA_RESULT P25QReadLowCurve(int sectorID, unsigned char *pBuf, int size);
	ITA_RESULT P25QWriteLowCurve(int sectorID, unsigned char *pBuf, int size);
	ITA_RESULT P25QReadHighCurve(int sectorID, unsigned char *pBuf, int size);
	ITA_RESULT P25QWriteHighCurve(int sectorID, unsigned char *pBuf, int size);*/
	void setLogger(GuideLog *logger);
	void setDebug(int flag);
private:
	ITA_RESULT P25QCSEnable();
	ITA_RESULT P25QCSDisable();
	ITA_RESULT P25QReadSector(unsigned int sectorNum, unsigned char *pBuf, int size);
	ITA_RESULT P25QProgramSector(unsigned int sectorNum, unsigned char *pBuf, int size);
	ITA_RESULT P25QEraseSector(unsigned int sectorNum);
	ITA_RESULT P25QReadSR(unsigned short *SR);
	ITA_RESULT P25QWriteEnable();
	int P25QGetStatus();
private:
	unsigned char Flash_buff[4 + PAGE_SIZE];
	GuideLog *m_logger;
	ITARegistry *m_registry;
	int m_cameraID;
	bool m_debug;
};

#endif // !ITA_FLASH120_H
