#include "./CollectK.h"
#include "../Module/PortingAPI.h"
#include <string.h>

CollectK::CollectK(int width, int height,int cameraId):m_collectKCount(0), m_collectKTotalCount(4), m_logger(NULL),
														m_cameraID(cameraId)
{
	m_frameSize = width * height;
	m_kArray = (unsigned short*)porting_calloc_mem(m_frameSize, sizeof(unsigned short), ITA_WRAPPER_MODULE);
	memset(&m_collectLow, 0, sizeof(ITACollectK));
	memset(&m_collectHigh, 0, sizeof(ITACollectK));
	m_collectLow.baseData = (unsigned short*)porting_calloc_mem(m_frameSize, sizeof(unsigned short), ITA_WRAPPER_MODULE);
	m_collectHigh.baseData = (unsigned short*)porting_calloc_mem(m_frameSize, sizeof(unsigned short), ITA_WRAPPER_MODULE);
}

CollectK::~CollectK()
{
	if (m_kArray) {
		porting_free_mem(m_kArray);
		m_kArray = nullptr;
	}
	if (m_collectLow.baseData) {
		porting_free_mem(m_collectLow.baseData);
		m_collectLow.baseData = nullptr;
	}
	if (m_collectHigh.baseData) {
		porting_free_mem(m_collectHigh.baseData);
		m_collectHigh.baseData = nullptr;
	}
}

void CollectK::setLogger(GuideLog* log)
{
	m_logger = log;
}

bool CollectK::isCollectK()
{
	return (m_collectLow.status == 1 || m_collectLow.status == 2 || m_collectHigh.status == 1 || m_collectHigh.status == 2) ? true : false;
}

ITA_RESULT CollectK::isSaveK(int gear)
{
	return (m_collectLow.status == 3 && m_collectHigh.status == 3 && m_collectHigh.gear == m_collectLow.gear && m_collectLow.gear == gear) ? ITA_OK : ITA_ERROR;
}

unsigned short* CollectK::getLowBase()
{
	return m_collectLow.baseData;
}

unsigned short* CollectK::getHighBase()
{
	return m_collectHigh.baseData;
}

void CollectK::collectKData(short* src)
{
	if (m_collectLow.status == 1)
	{
		m_collectLow.status = 2;
		memcpy(m_kArray, src, m_frameSize * 2);
		m_collectKCount++;
		m_logger->output(LOG_DEBUG, "startCollect m_collectLow.status=%d", m_collectLow.status);
	}
	else if (m_collectLow.status == 2)
	{
		if (m_collectKCount == m_collectKTotalCount)
		{
			m_collectLow.status = 3;
			m_collectKCount = 0;
			memcpy(m_collectLow.baseData, m_kArray, m_frameSize * sizeof(short));
			//通知低温本底采集完成。
			m_collectLow.cb(m_cameraID, m_collectLow.gear, m_collectLow.baseData, m_frameSize * sizeof(short),m_collectLow.para);
			m_logger->output(LOG_DEBUG, "m_collectLow collect finish!");
		}
		else {
			for (int i = 0; i < m_frameSize; i++)
			{
				*(m_kArray + i) = (*(m_kArray + i) + *(src + i)) / 2;
			}
			m_logger->output(LOG_DEBUG, "m_collectLow on doing,current collect %d frame!", m_collectKCount);
			m_collectKCount++;
		}
	}
	if (m_collectHigh.status == 1)
	{
		memcpy(m_kArray, src, m_frameSize * 2);
		m_collectKCount++;
		m_collectHigh.status = 2;
		m_logger->output(LOG_INFO, "startCollect highBase=%d", m_collectHigh.status);
	}
	else if (m_collectHigh.status == 2) {
		if (m_collectKCount == m_collectKTotalCount)
		{
			m_collectHigh.status = 3;
			m_collectKCount = 0;
			memcpy(m_collectHigh.baseData, m_kArray, m_frameSize * sizeof(short));
			//通知高温本底采集完成。
			m_collectHigh.cb(m_cameraID,m_collectHigh.gear, m_collectHigh.baseData, m_frameSize * sizeof(short), m_collectHigh.para);
			m_logger->output(LOG_INFO, "Notify High collect finish!");
		}
		else {
			for (int i = 0; i < m_frameSize; i++)
			{
				*(m_kArray + i) = (*(m_kArray + i) + *(src + i)) / 2;
			}
			m_logger->output(LOG_DEBUG, "m_collectHigh on doing,current collect %d frame!", m_collectKCount);
			m_collectKCount++;
		}
	}
}

void CollectK::initCollectInit()
{
	m_collectLow.status = 0;
	m_collectHigh.status = 0;
}

ITA_RESULT CollectK::registryBaseInfo(int gear, float blackTemp, ITACollectBaseFinish cb, void* userParam)
{
	if (m_collectLow.status == 1 || m_collectLow.status == 2)
	{
		m_logger->output(LOG_ERROR, "collectK highBase ondoing, ret=%d", ITA_INACTIVE_CALL);
		return ITA_INACTIVE_CALL;
	}
	if (m_collectHigh.status == 1 || m_collectHigh.status == 2)
	{
		m_logger->output(LOG_ERROR, "collectK lowBase ondoing, ret=%d", ITA_INACTIVE_CALL);
		return ITA_INACTIVE_CALL;
	}
	if (blackTemp == 20)
	{
		m_collectLow.status = 1;
		m_collectLow.temp = blackTemp;
		m_collectLow.cb = cb;
		m_collectLow.para = userParam;
		m_collectLow.gear = gear;
	}
	else if (blackTemp == 50) {
		m_collectHigh.status = 1;
		m_collectHigh.temp = blackTemp;
		m_collectHigh.cb = cb;
		m_collectHigh.para = userParam;
		m_collectHigh.gear = gear;
	}
	return ITA_OK;
}
