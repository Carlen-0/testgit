#include "Flash120.h"
#include <string.h>
#include "../Module/PortingAPI.h"

#define SPI_SKIP_CMD //我们用的公板SPI驱动，READ_CMD这条命令transfer读取的数据开头包含CMD 4B。但是客户海信读取的未包含CMD，海信应该改过驱动。

Flash120::Flash120(ITARegistry * registry, int cameraID)
{
	m_registry = registry;
	m_cameraID = cameraID;
	m_logger = NULL;
	memset(Flash_buff, 0, 4 + PAGE_SIZE);
	m_debug = false;
}

Flash120::~Flash120()
{
}
/*Initializes the P25Q128FV interface.*/
ITA_RESULT Flash120::P25QInit()
{
	/* Reset P25Qxxx */
	ITA_RESULT ret = ITA_OK;
	unsigned char cmd;
	if (!m_registry || !m_registry->SPIWrite || !m_registry->SPITransmit)
	{
		ret = ITA_HAL_UNREGISTER;
		m_logger->output(LOG_ERROR, "P25QInit failed.%d", ret);
		return ret;
	}
	cmd = RESET_ENABLE_CMD;//66
	ret = P25QCSEnable();
	if (ITA_OK != ret)
	{
		m_logger->output(LOG_ERROR, "P25QInit P25QCSEnable failed.%d", ret);
		return ret;
	}
	ret = m_registry->SPIWrite(m_cameraID, &cmd, 1, m_registry->userParam);
	if (ITA_OK != ret)
	{
		m_logger->output(LOG_ERROR, "P25QInit SPIWrite failed.%d", ret);
		return ret;
	}
	//HAL_SPI_Transmit(&hspi4, &cmd, 1, P25Qx_TIMEOUT_VALUE);
	ret = P25QCSDisable();
	if (ITA_OK != ret)
	{
		m_logger->output(LOG_ERROR, "P25QInit P25QCSDisable failed.%d", ret);
		return ret;
	}
	cmd = RESET_MEMORY_CMD;//99
	P25QCSEnable();
	ret = m_registry->SPIWrite(m_cameraID, &cmd, 1, m_registry->userParam);
	//HAL_SPI_Transmit(&hspi4, &cmd, 1, P25Qx_TIMEOUT_VALUE);
	P25QCSDisable();

	unsigned char cmdArray[] = { READ_STATUS_REG1_CMD, 0 };
	unsigned char rx[] = { 0, 0 };
	unsigned char status;

	P25QCSEnable();
	ret = m_registry->SPITransmit(m_cameraID, cmdArray, rx, 2, m_registry->userParam);
	//HAL_SPI_TransmitReceive(&hspi4, cmdArray, rx, 2, P25Qx_TIMEOUT_VALUE);
	ret = P25QCSDisable();
	status = rx[1];

	/* Check the value of the register */
	if ((status & P25Q128FV_FSR_BUSY) != 0)
	{
		ret = ITA_ERROR;
		m_logger->output(LOG_ERROR, "P25QInit P25Q128FV_FSR_BUSY.%d", ret);
		//return P25Qx_BUSY;
	}
	/*else
	{
		return P25Qx_OK;
	}*/
	return ret;
}

ITA_RESULT Flash120::P25QExit()
{
	return ITA_OK;
}

ITA_RESULT Flash120::P25QReadID(unsigned short * pID)
{
	unsigned char cmd[8] = { 0 };
	unsigned char rx_ID[8] = { 0 };
	ITA_RESULT ret = ITA_OK;
	cmd[0] = READ_ID_CMD;
	P25QCSEnable();
	ret = m_registry->SPITransmit(m_cameraID, cmd, rx_ID, 8, m_registry->userParam);
	//HAL_SPI_TransmitReceive(&hspi4, &cmd, &rx_ID[0], 8, P25Qx_TIMEOUT_VALUE);
	P25QCSDisable();

	*pID = rx_ID[4] | (rx_ID[5] << 8);
	return ret;
}

ITA_RESULT Flash120::P25QReadInfo(unsigned char * pBuf, int size)
{
	return P25QReadSector(0, pBuf, size);
}

ITA_RESULT Flash120::P25QReadCurve(int sectorID, unsigned char * pBuf, int size)
{
	return P25QReadSector(sectorID, pBuf, size);
}

ITA_RESULT Flash120::P25QWriteCurve(int sectorID, unsigned char * pBuf, int size)
{
	return P25QProgramSector(sectorID, pBuf, size);
}

//ITA_RESULT Flash120::P25QReadLowCurve(int sectorID, unsigned char * pBuf, int size)
//{
//	return P25QReadSector(1, pBuf, size);
//}
//
//ITA_RESULT Flash120::P25QWriteLowCurve(int sectorID, unsigned char * pBuf, int size)
//{
//	return P25QProgramSector(1, pBuf, size);
//}
//
//ITA_RESULT Flash120::P25QReadHighCurve(int sectorID, unsigned char * pBuf, int size)
//{
//	return P25QReadSector(64, pBuf, size);
//}
//
//ITA_RESULT Flash120::P25QWriteHighCurve(int sectorID, unsigned char * pBuf, int size)
//{
//	return P25QProgramSector(64, pBuf, size);
//}

void Flash120::setLogger(GuideLog * logger)
{
	m_logger = logger;
}

void Flash120::setDebug(int flag)
{
	m_logger->output(LOG_INFO, "Flash120 debug %d.", flag);
	if (flag)
		m_debug = true;
	else
		m_debug = false;
}

ITA_RESULT Flash120::P25QCSEnable()
{
	if (!m_registry->InternalFlashPrepare)
		return ITA_HAL_UNREGISTER;
	return m_registry->InternalFlashPrepare(m_cameraID, m_registry->userParam);
	//HAL_DEV_GPIO_Write(0, 0, 4, 0);
}

ITA_RESULT Flash120::P25QCSDisable()
{
	if (!m_registry->InternalFlashDone)
		return ITA_HAL_UNREGISTER;
	return m_registry->InternalFlashDone(m_cameraID, m_registry->userParam);
	//HAL_DEV_GPIO_Write(0, 0, 4, 1);
}

ITA_RESULT Flash120::P25QReadSector(unsigned int sectorNum, unsigned char * pBuf, int size)
{
	unsigned int ReadAddr = sectorNum * SECTOR_SIZE;
	unsigned int cnt = size / PAGE_SIZE;
	unsigned int left = size % PAGE_SIZE;

	while (cnt > 0)
	{
		/* Configure the command */
		Flash_buff[0] = READ_CMD;//0x03
		Flash_buff[1] = (unsigned char)(ReadAddr >> 16);
		Flash_buff[2] = (unsigned char)(ReadAddr >> 8);
		Flash_buff[3] = (unsigned char)(ReadAddr);

		P25QCSEnable();
		if(m_debug)
			m_logger->output(LOG_INFO, "P25QReadSector1 %d %d %#x %#x %#x %#x %#x %#x", sectorNum, size, Flash_buff[0], Flash_buff[1], Flash_buff[2], Flash_buff[3], Flash_buff[4], Flash_buff[5]);
		if(ITA_OK != m_registry->SPITransmit(m_cameraID, Flash_buff, Flash_buff, (PAGE_SIZE + 4), m_registry->userParam))
		//if (HAL_SPI_TransmitReceive(&hspi4, Flash_buff, Flash_buff, (PAGE_SIZE + 4), P25Qx_TIMEOUT_VALUE) != HAL_SUCCESS)
		{
			P25QCSDisable();
			m_logger->output(LOG_ERROR, "P25QReadSector SPITransmit error.");
			return ITA_SPI_ERROR;
		}

		P25QCSDisable();
		if (m_debug)
			m_logger->output(LOG_INFO, "P25QReadSector2 %#x %#x %#x %#x %#x %#x %#x %#x %#x %#x %#x %#x %#x %#x %#x %#x %#x %#x %#x %#x %#x %#x %#x %#x", 
				Flash_buff[0], Flash_buff[1], Flash_buff[2], Flash_buff[3], Flash_buff[4], Flash_buff[5],
				Flash_buff[6], Flash_buff[7], Flash_buff[8], Flash_buff[9], Flash_buff[10], Flash_buff[11],
				Flash_buff[12], Flash_buff[13], Flash_buff[14], Flash_buff[15], Flash_buff[16], Flash_buff[17],
				Flash_buff[18], Flash_buff[19], Flash_buff[20], Flash_buff[21], Flash_buff[22], Flash_buff[23]);
#ifdef SPI_SKIP_CMD
		memcpy(pBuf, &Flash_buff[4], PAGE_SIZE);
#else
		memcpy(pBuf, Flash_buff, PAGE_SIZE);
#endif // SPI_SKIP_CMD

		ReadAddr += PAGE_SIZE;
		pBuf += PAGE_SIZE;
		cnt--;
	}

	if (left != 0)
	{
		/* Configure the command */
		Flash_buff[0] = READ_CMD;
		Flash_buff[1] = (unsigned char)(ReadAddr >> 16);
		Flash_buff[2] = (unsigned char)(ReadAddr >> 8);
		Flash_buff[3] = (unsigned char)(ReadAddr);

		P25QCSEnable();
		if (m_debug)
			m_logger->output(LOG_INFO, "P25QReadSector3 %#x %#x %#x %#x %#x %#x", Flash_buff[0], Flash_buff[1], Flash_buff[2], Flash_buff[3], Flash_buff[4], Flash_buff[5]);
		if(ITA_OK != m_registry->SPITransmit(m_cameraID, Flash_buff, Flash_buff, (left + 4), m_registry->userParam))
		//if (HAL_SPI_TransmitReceive(&hspi4, Flash_buff, Flash_buff, (left + 4), P25Qx_TIMEOUT_VALUE) != HAL_SUCCESS)
		{
			P25QCSDisable();
			m_logger->output(LOG_ERROR, "P25QReadSector SPITransmit2 error.");
			return ITA_SPI_ERROR;
		}

		P25QCSDisable();
		if (m_debug)
			m_logger->output(LOG_INFO, "P25QReadSector4 %#x %#x %#x %#x %#x %#x %#x %#x %#x %#x %#x %#x %#x %#x %#x %#x %#x %#x %#x %#x %#x %#x %#x %#x",
				Flash_buff[0], Flash_buff[1], Flash_buff[2], Flash_buff[3], Flash_buff[4], Flash_buff[5],
				Flash_buff[6], Flash_buff[7], Flash_buff[8], Flash_buff[9], Flash_buff[10], Flash_buff[11],
				Flash_buff[12], Flash_buff[13], Flash_buff[14], Flash_buff[15], Flash_buff[16], Flash_buff[17],
				Flash_buff[18], Flash_buff[19], Flash_buff[20], Flash_buff[21], Flash_buff[22], Flash_buff[23]);
#ifdef SPI_SKIP_CMD
		memcpy(pBuf, &Flash_buff[4], left);
#else
		memcpy(pBuf, Flash_buff, left);
#endif // SPI_SKIP_CMD		
	}

	return ITA_OK;
}

ITA_RESULT Flash120::P25QProgramSector(unsigned int sectorNum, unsigned char * pBuf, int size)
{
	unsigned int WriteAddr = sectorNum * SECTOR_SIZE;
	long long tickstart = porting_get_time_ms();
	unsigned short StatusReg;
	ITA_RESULT ret;
	 ret = P25QEraseSector(sectorNum);
	 if (ITA_OK != ret)
		 return ret;
	porting_thread_sleep(2);
	ret = P25QReadSR(&StatusReg);
	if (ITA_OK != ret)
		return ret;
	unsigned int cnt = size / PAGE_SIZE;
	unsigned int left = size % PAGE_SIZE;
	m_logger->output(LOG_INFO, "P25QProgramSector %d count=%d left=%d.", sectorNum, cnt, left);
	/* Perform the write page by page */
	while (cnt > 0)
	{
		/* Configure the command */
		Flash_buff[0] = PAGE_PROG_CMD;//0x02
		Flash_buff[1] = (unsigned char)(WriteAddr >> 16);
		Flash_buff[2] = (unsigned char)(WriteAddr >> 8);
		Flash_buff[3] = (unsigned char)(WriteAddr);
		memcpy(&Flash_buff[4], pBuf, PAGE_SIZE);

		/* Enable write operations */
		P25QWriteEnable();
		porting_thread_sleep(1);
		P25QCSEnable();
		/* Transmission of the data */
		if(ITA_OK != m_registry->SPIWrite(m_cameraID, Flash_buff, (PAGE_SIZE + 4), m_registry->userParam))
		//if (HAL_SPI_Transmit(&hspi4, Flash_buff, (PAGE_SIZE + 4), P25Qx_TIMEOUT_VALUE) != HAL_SUCCESS)
		{
			P25QCSDisable();
			m_logger->output(LOG_ERROR, "P25QProgramSector SPIWrite error.");
			return ITA_SPI_ERROR;
		}
		P25QCSDisable();
		porting_thread_sleep(2);

		/* Wait the end of Flash writing */
		while (P25QGetStatus() == P25Qx_BUSY)
		{
			/* Check for the Timeout */
			if ((porting_get_time_ms() - tickstart) > P25Qx_TIMEOUT_VALUE)
			{
				m_logger->output(LOG_ERROR, "P25QProgramSector timeout error.");
				return ITA_WAIT_TIMEOUT;
			}
		}
		m_logger->output(LOG_INFO, "P25QProgramSector write count=%d.", cnt);
		/* Update the address and size variables for next page programming */
		WriteAddr += PAGE_SIZE;
		pBuf += PAGE_SIZE;
		cnt--;
	}

	if (left != 0)
	{
		/* Configure the command */
		Flash_buff[0] = PAGE_PROG_CMD;
		Flash_buff[1] = (unsigned char)(WriteAddr >> 16);
		Flash_buff[2] = (unsigned char)(WriteAddr >> 8);
		Flash_buff[3] = (unsigned char)(WriteAddr);
		memcpy(&Flash_buff[4], pBuf, left);

		/* Enable write operations */
		P25QWriteEnable();

		porting_thread_sleep(1);
		P25QCSEnable();
		/* Transmission of the data */
		if (ITA_OK != m_registry->SPIWrite(m_cameraID, Flash_buff, (left + 4), m_registry->userParam))
		//if (HAL_SPI_Transmit(&hspi4, Flash_buff, (left + 4), P25Qx_TIMEOUT_VALUE) != HAL_SUCCESS)
		{
			P25QCSDisable();
			m_logger->output(LOG_ERROR, "P25QProgramSector SPIWrite2 error.");
			return ITA_SPI_ERROR;
		}

		P25QCSDisable();
		porting_thread_sleep(2);

		/* Wait the end of Flash writing */
		while (P25QGetStatus() == P25Qx_BUSY)
		{
			/* Check for the Timeout */
			if ((porting_get_time_ms() - tickstart) > P25Qx_TIMEOUT_VALUE)
			{
				m_logger->output(LOG_ERROR, "P25QProgramSector timeout2 error.");
				return ITA_WAIT_TIMEOUT;
			}
		}
		m_logger->output(LOG_INFO, "P25QProgramSector write left=%d.", left);
	}
	return ITA_OK;
}

ITA_RESULT Flash120::P25QEraseSector(unsigned int sectorNum)
{
	unsigned int Address = sectorNum * SECTOR_SIZE;
	unsigned char cmd[4];
	long long tickstart = porting_get_time_ms();
	ITA_RESULT ret;

	cmd[0] = SECTOR_ERASE_CMD;//0x20
	cmd[1] = (unsigned char)(Address >> 16);
	cmd[2] = (unsigned char)(Address >> 8);
	cmd[3] = (unsigned char)(Address);

	/* Enable write operations */
	ret = P25QWriteEnable();
	if (ITA_OK != ret)
	{
		m_logger->output(LOG_ERROR, "P25QEraseSector failed.%d", ret);
		return ret;
	}
	porting_thread_sleep(5);

	P25QCSEnable();
	ret = m_registry->SPIWrite(m_cameraID, cmd, 4, m_registry->userParam);
	if (ITA_OK != ret)
	{
		m_logger->output(LOG_ERROR, "P25QEraseSector SPIWrite failed.%d", ret);
		return ret;
	}
	P25QCSDisable();

	porting_thread_sleep(2);

	/* Wait the end of Flash writing */
	while (P25QGetStatus() == P25Qx_BUSY)
	{
		/* Check for the Timeout */
		if ((porting_get_time_ms() - tickstart) > P25Q128FV_SECTOR_ERASE_MAX_TIME)
		{
			//            P25Qx_CS_Disable();
			//return P25Qx_TIMEOUT;
			m_logger->output(LOG_ERROR, "P25QEraseSector timeout error.");
			return ITA_ERROR;
		}
	}
	m_logger->output(LOG_INFO, "P25QEraseSector %d ret=%d", sectorNum, ret);
	return ret;
}

ITA_RESULT Flash120::P25QReadSR(unsigned short * SR)
{
	unsigned char cmd1 = READ_STATUS_REG1_CMD;//05
	unsigned char cmd2 = READ_STATUS_REG2_CMD;//35
	unsigned char rx1;
	unsigned char rx2;
	ITA_RESULT ret;
	ret = P25QCSEnable();
	if (ITA_OK != ret)
	{
		m_logger->output(LOG_ERROR, "P25QReadSR failed.%d", ret);
		return ret;
	}
	ret = m_registry->SPITransmit(m_cameraID, &cmd1, &rx1, 1, m_registry->userParam);
	if (ITA_OK != ret)
	{
		m_logger->output(LOG_ERROR, "P25QReadSR SPITransmit failed.%d", ret);
		return ret;
	}
	//HAL_SPI_TransmitReceive(&hspi4, &cmd1, &rx1, 1, P25Qx_TIMEOUT_VALUE);
	P25QCSDisable();

	porting_thread_sleep(8);

	P25QCSEnable();
	ret = m_registry->SPITransmit(m_cameraID, &cmd2, &rx2, 1, m_registry->userParam);
	if (ITA_OK != ret)
	{
		m_logger->output(LOG_ERROR, "P25QReadSR SPITransmit failed.%d", ret);
		return ret;
	}
	//HAL_SPI_TransmitReceive(&hspi4, &cmd2, &rx2, 1, P25Qx_TIMEOUT_VALUE);
	P25QCSDisable();

	*SR = rx1 | (rx2 << 8);
	m_logger->output(LOG_INFO, "P25QReadSR %d ret=%d", *SR, ret);
	return ret;
}

ITA_RESULT Flash120::P25QWriteEnable()
{
	long long tickstart = porting_get_time_ms();
	unsigned char cmd = WRITE_ENABLE_CMD;
	ITA_RESULT ret;
	ret = P25QCSEnable();
	if (ITA_OK != ret)
	{
		m_logger->output(LOG_ERROR, "P25QWriteEnable failed.%d", ret);
		return ret;
	}
	ret = m_registry->SPIWrite(m_cameraID, &cmd, 1, m_registry->userParam);
	if (ITA_OK != ret)
	{
		m_logger->output(LOG_ERROR, "P25QWriteEnable SPIWrite failed.%d", ret);
		return ret;
	}
	//HAL_SPI_Transmit(&hspi4, &cmd, 1, P25Qx_TIMEOUT_VALUE);
	P25QCSDisable();

	/* Wait the end of Flash writing */

	while (P25QGetStatus() == P25Qx_BUSY)
	{
		/* Check for the Timeout */
		if ((porting_get_time_ms() - tickstart) > P25Qx_TIMEOUT_VALUE)
		{
			//return P25Qx_TIMEOUT;
			m_logger->output(LOG_ERROR, "P25QWriteEnable timeout error.");
			return ITA_ERROR;
		}
	}

	return ret;
}
/**
* @brief  Reads current status of the P25Q128FV.
* @retval P25Q128FV memory status
*/
int Flash120::P25QGetStatus()
{
	unsigned char cmd[] = { READ_STATUS_REG1_CMD, 0 };
	unsigned char rx[] = { 0, 0 };
	unsigned char status;
	ITA_RESULT ret;
	ret = P25QCSEnable();
	if (ITA_OK != ret)
	{
		m_logger->output(LOG_ERROR, "P25QGetStatus failed.%d", ret);
		return ret;
	}
	ret = m_registry->SPITransmit(m_cameraID, cmd, rx, 2, m_registry->userParam);
	if (ITA_OK != ret)
	{
		m_logger->output(LOG_ERROR, "P25QGetStatus SPITransmit failed.%d", ret);
		return ret;
	}
	//HAL_SPI_TransmitReceive(&hspi4, cmd, rx, 2, P25Qx_TIMEOUT_VALUE);
	P25QCSDisable();
	status = rx[1];

	/* Check the value of the register */
	if ((status & P25Q128FV_FSR_BUSY) != 0)
	{
		return P25Qx_BUSY;
	}
	else
	{
		return P25Qx_OK;
	}
}
