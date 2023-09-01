/*************************************************************
Copyright (C), 2021--2022, Wuhan Guide Sensmart Tech Co., Ltd
File name   : Monitor.cpp
Author      : Guide Sensmart RD. wangyan
Version     : 1.0
Date        : 2021/11/9
Description : ITA SDK Monitor.
*************************************************************/
#include "Monitor.h"
#include <string>

Monitor::Monitor(ITA_PRODUCT_TYPE type)
{
	m_type = type;
	m_shutterPeriod = 30;
	m_lastShutterTime = 0;
	m_lastNUCTime = 0;
	m_shutterManual = false;
	m_nucManual = false;
	m_realTimeFPATemp = 0;
	m_fpaTempWhenShutter = 0;	//上次快门时焦温
	m_fpaTempWhenNUC = 0;		//上次NUC时焦温
	m_nucIntervalBottom = 5;	//相邻两次NUC时间间隔不低于5秒
	m_shutterIntervalBottom = 3;//相邻两次快门时间间隔不低于3秒。开机时过于频繁打快门无意义。
	if (ITA_256_TIMO == type)
	{
		//温升0.2度做快门补偿
		m_shutterTempUpper = 20;
		//温升3度做NUC补偿
		m_nucTempUpper = 300;
	}
	else if (ITA_120_TIMO == type)
	{
		//温升0.25度做快门补偿
		m_shutterTempUpper = 25;
		//温升1度做NUC补偿
		m_nucTempUpper = 100;
	}
	m_autoShutter = true;
	m_closeShutterPolicy = false;	//默认打开快门和NUC策略，由用户根据需要主动关闭。
	m_logger = NULL;
	m_originalTime = 0;	//开机时间
	m_bootTimeToNow1 = 0;	//开机60分钟内
	m_shutterPeriod1 = 0;	//自动快门周期30秒
	m_bootTimeToNow2 = 0;	//开机60分钟与120分钟之间
	m_shutterPeriod2 = 0;	//自动快门周期60秒
	m_bootTimeToNow3 = 0;	//开机120分钟之后
	m_shutterPeriod3 = 0;	//自动快门周期90秒
	m_userPeriod = false;	//用户是否设置快门周期。如果用户设置自动快门时间，那么默认的自动快门策略不再有效。
	m_maxFpaTemp = 0;
	m_avgB = 0;
	m_closedLoopStatus = CLOSED_LOOP_NONE;
	m_lastSaveCHTime = 0;
	m_saveStatusPeriod = 10;
}

bool Monitor::isSaveColdHot(unsigned int currentTime)
{
	if (currentTime < m_lastSaveCHTime)
	{
		//表明系统时间被改小，时间线清零。
		m_lastSaveCHTime = 0;
	}
	if (currentTime - m_lastSaveCHTime  >= m_saveStatusPeriod * 1000 * 60) {
		m_lastSaveCHTime = currentTime;
		return true;
	}
	else {
		return false;
	}
}

Monitor::~Monitor()
{
	
}

ITA_RESULT Monitor::setInterval(unsigned int second)
{
	ITA_RESULT ret = ITA_OK;
	if (second >= m_shutterIntervalBottom)
	{
		m_shutterPeriod = second;
		m_userPeriod = true;	//用户是否设置快门周期。如果用户设置自动快门时间，那么默认的自动快门策略不再有效。
	}
	else
		ret = ITA_ARG_OUT_OF_RANGE;
	m_logger->output(LOG_INFO, "setInterval shutterPeriod=%d second=%d", m_shutterPeriod, second);
	return ret;
}

ITA_RESULT Monitor::autoShutter(ITA_SWITCH flag)
{
	if (flag > ITA_ENABLE || flag < ITA_DISABLE)
		return ITA_ARG_OUT_OF_RANGE;
	if (ITA_ENABLE == flag)
		m_autoShutter = true;
	else
		m_autoShutter = false;
	m_logger->output(LOG_INFO, "autoShutter flag=%d", flag);
	return ITA_OK;
}

MONITOR_STATUS Monitor::timeLoopOnce(unsigned int currentTime)
{
	if (currentTime < m_lastShutterTime)
	{
		//表明系统时间被改小，时间线清零。
		m_lastShutterTime = 0;
	}
	/*手动NUC*/
	if (m_nucManual)
	{
		m_lastNUCTime = currentTime;
		m_lastShutterTime = currentTime;
		m_fpaTempWhenNUC = m_realTimeFPATemp;
		m_nucManual = false;
		m_logger->output(LOG_INFO, "manual nuc begin");
		return MONITOR_NUC_NOW;
	}
	/*手动快门*/
	if (m_shutterManual)
	{
		m_lastShutterTime = currentTime;
		m_fpaTempWhenShutter = m_realTimeFPATemp;
		m_shutterManual = false;
		m_logger->output(LOG_INFO, "manual shutter begin");
		return MONITOR_SHUTTER_NOW;
	}
	/*关闭策略后不再检测自动快门和NUC。*/
	if (m_closeShutterPolicy)
		return MONITOR_NONE;

	/*初始化完成后要做一次NUC。*/
	if (!m_lastNUCTime)
	{
		m_lastNUCTime = currentTime;
		m_lastShutterTime = currentTime;
		m_fpaTempWhenNUC = m_realTimeFPATemp;
		m_logger->output(LOG_INFO, "first nuc begin shutterTempUpper=%d nucTempUpper=%d", m_shutterTempUpper, m_nucTempUpper);
		m_originalTime = currentTime;
		return MONITOR_NUC_NOW;
	}
	/*NUC温升策略*/
	if (abs(m_realTimeFPATemp - m_fpaTempWhenNUC) >= m_nucTempUpper && currentTime - m_lastNUCTime >= m_nucIntervalBottom * 1000)
	{
		m_lastNUCTime = currentTime;
		m_lastShutterTime = currentTime;
		m_logger->output(LOG_INFO, "temp nuc begin: fpaTemp=%d dif=%d", m_realTimeFPATemp, m_realTimeFPATemp - m_fpaTempWhenNUC);
		if (ITA_120_TIMO == m_type && m_realTimeFPATemp > m_maxFpaTemp + 300)
		{
			if (m_avgB < AVGB_LOWER_LIMIT)
			{
				if (CLOSED_LOOP_NONE == m_closedLoopStatus || CLOSED_LOOP_FAILED == m_closedLoopStatus)
				{
					//环温过高图像异常，测温已达不到精度，但图像要正常。
					m_closedLoopStatus = CLOSED_LOOP_ING;
					//m_fpaTempWhenNUC不更新，5秒间隔时间过后，下次继续触发Res+1闭环NUC。
					m_logger->output(LOG_INFO, "Monitor: realTimeFPATemp=%d, maxFpaTemp=%d, avgB=%d.", m_realTimeFPATemp, m_maxFpaTemp, m_avgB);
					return MONITOR_NUC_CLOSED_LOOP;
				}
				else {
					return MONITOR_NUC_NOW;
				}
			}
			else
			{
				m_fpaTempWhenNUC = m_realTimeFPATemp;
				if (CLOSED_LOOP_NONE == m_closedLoopStatus)
				{
					//高温下图像还未异常，做正常NUC。
					m_logger->output(LOG_INFO, "Monitor: realTimeFPATemp=%d, maxFpaTemp=%d MONITOR_NUC_NOW.", m_realTimeFPATemp, m_maxFpaTemp);
					return MONITOR_NUC_NOW;
				}
				else
				{
					//高温下NUC闭环已成功，温度仍在变化，使用上次成功的Res做NUC。
					m_logger->output(LOG_INFO, "Monitor: realTimeFPATemp=%d, maxFpaTemp=%d LOOP_REPEAT.", m_realTimeFPATemp, m_maxFpaTemp);
					return MONITOR_NUC_LOOP_REPEAT;
				}
			}
		}
		else
		{
			m_fpaTempWhenNUC = m_realTimeFPATemp;
			if (m_closedLoopStatus > CLOSED_LOOP_NONE)
			{
				//120模组环温下降到正常测温范围，RES恢复数据包中的值。
				m_closedLoopStatus = CLOSED_LOOP_NONE;
				m_logger->output(LOG_INFO, "Monitor: realTimeFPATemp=%d, maxFpaTemp=%d NUC_RECOVERY.", m_realTimeFPATemp, m_maxFpaTemp);
				return MONITOR_NUC_RECOVERY;
			}
			else
			{
				return MONITOR_NUC_NOW;
			}
		}
	}
	/*快门温升策略*/
	if (abs(m_realTimeFPATemp - m_fpaTempWhenShutter) >= m_shutterTempUpper && currentTime - m_lastShutterTime >= m_shutterIntervalBottom * 1000)
	{
		m_lastShutterTime = currentTime;
		m_logger->output(LOG_INFO, "temp shutter begin: fpaTemp=%d dif=%d", m_realTimeFPATemp, m_realTimeFPATemp - m_fpaTempWhenShutter);
		m_fpaTempWhenShutter = m_realTimeFPATemp;
		return MONITOR_SHUTTER_NOW;
	}
	/*自动快门*/
	if (m_autoShutter && currentTime - m_lastShutterTime >= m_shutterPeriod * 1000)
	{
		m_logger->output(LOG_INFO, "auto shutter begin period=%d %d", m_shutterPeriod, currentTime - m_lastShutterTime);
		m_lastShutterTime = currentTime;
		m_fpaTempWhenShutter = m_realTimeFPATemp;
		return MONITOR_SHUTTER_NOW;
	}
	return MONITOR_NONE;
}

ITA_RESULT Monitor::setFPAArray(short * fpaTempArray, int size, unsigned char startRes)
{
	if (!fpaTempArray || size <= 0)
	{
		return ITA_ILLEGAL_PAPAM_ERR;
	}
	m_maxFpaTemp = *(fpaTempArray + (size - 1));
	m_logger->output(LOG_INFO, "Monitor: maxFpaTemp=%d startRes=%d.", m_maxFpaTemp, startRes);
	return ITA_OK;
}

ITA_RESULT Monitor::updateAVGB(short avgB)
{
	m_avgB = avgB;
	if (CLOSED_LOOP_ING == m_closedLoopStatus)
	{
		if (avgB < AVGB_LOWER_LIMIT)
		{
			m_closedLoopStatus = CLOSED_LOOP_FAILED;
			m_logger->output(LOG_INFO, "Monitor: updateAVGB CLOSED_LOOP_FAILED.avgB=%d", avgB);
		}
		else
		{
			m_closedLoopStatus = CLOSED_LOOP_SUCCESS;
			m_logger->output(LOG_INFO, "Monitor: updateAVGB CLOSED_LOOP_SUCCESS.avgB=%d", avgB);
		}
	}
	return ITA_OK;
}

unsigned char Monitor::calcRes(unsigned char currentRes)
{
	//120模组RES取值范围为8~11，数据包中通常是8.
	unsigned char resArray[4] = { 8, 9, 10, 11 };
	int size = 4;
	unsigned char ucRes = 0;
	//如果遍历完其它3个值就返回0.
	for (int i = 0; i < size; i++)
	{
		if (resArray[i] == currentRes)
		{
			if (i + 1 >= size)
			{
				ucRes = resArray[0];
			}
			else
			{
				ucRes = resArray[i + 1];
			}
		}
	}
	m_logger->output(LOG_INFO, "Monitor: calcRes %d", ucRes);
	if (!ucRes)
		ucRes = 9;
	return ucRes;
}

ITA_RESULT Monitor::updatePeriod(unsigned int currentTime)
{
	/*根据自动快门策略参数更新快门周期*/
	if (!m_userPeriod && m_bootTimeToNow1 > 0 && m_originalTime > 0)
	{
		unsigned int dif = (currentTime - m_originalTime) / 1000;
		if (dif >= m_bootTimeToNow2)
		{
			if (m_shutterPeriod != m_shutterPeriod3)
			{
				m_shutterPeriod = m_shutterPeriod3;
				m_logger->output(LOG_INFO, "Update shutter period3:%d %d %d %d", m_shutterPeriod, currentTime, m_originalTime, dif);
			}
			return ITA_OK;
		}
		if (dif >= m_bootTimeToNow1)
		{
			if (m_shutterPeriod != m_shutterPeriod2)
			{
				m_shutterPeriod = m_shutterPeriod2;
				m_logger->output(LOG_INFO, "Update shutter period2:%d %d %d %d", m_shutterPeriod, currentTime, m_originalTime, dif);
			}
			return ITA_OK;
		}
		if (dif > 0)
		{
			if (m_shutterPeriod != m_shutterPeriod1)
			{
				m_shutterPeriod = m_shutterPeriod1;
				m_logger->output(LOG_INFO, "Update shutter period1:%d %d %d %d", m_shutterPeriod, currentTime, m_originalTime, dif);
			}
			return ITA_OK;
		}
	}
	return ITA_OK;
}

int Monitor::updateSensorTemp(int fpaTemp)
{
	m_realTimeFPATemp = fpaTemp;
	return 0;
}

void Monitor::updatePotCoverAlgoPara(MtParams* mtParam, stDGGTPara* potCoverPara)
{
	potCoverPara->fRealShutterT = mtParam->fRealTimeShutterTemp;
	potCoverPara->fLastShutterT = mtParam->fCurrentShutterTemp;
	potCoverPara->fLastLastShutterT = mtParam->fLastShutterTemp;
	potCoverPara->fStartShutterT = mtParam->fOrinalShutterTemp;
	potCoverPara->fDelta = potCoverPara->fRealShutterT - potCoverPara->fStartShutterT;
	m_logger->output(LOG_DEBUG, "guoGai para = %.2f,%.2f,%.2f,%.2f,%.2f", potCoverPara->fRealShutterT, potCoverPara->fLastShutterT,
		potCoverPara->fLastLastShutterT, potCoverPara->fStartShutterT, potCoverPara->fDelta);
}

ITA_RESULT Monitor::shutterManual(int flag)
{
	unsigned int currentTime = porting_get_ms();
	if (1 == flag)
	{
		//用户根据产品需要NUC之后再做一次快门补偿，不能有间隔时间限制。
		m_shutterManual = true;
		return ITA_OK;
	}
	if (currentTime - m_lastShutterTime >= m_shutterIntervalBottom * 1000)
	{
		m_shutterManual = true;
		return ITA_OK;
	}
	else
		return ITA_INACTIVE_CALL;
}

ITA_RESULT Monitor::nucManual(int flag)
{
	unsigned int currentTime = porting_get_ms();
	if (1 == flag)
	{
		//用户根据产品需要NUC失效之后最多再做2次NUC，不能有间隔时间限制。
		m_nucManual = true;
		return ITA_OK;
	}
	if (currentTime - m_lastNUCTime >= m_nucIntervalBottom * 1000)
	{
		m_nucManual = true;
		return ITA_OK;
	}
	else
	{
		m_logger->output(LOG_WARN, "nucManual ignored. The interval is too short.");
		return ITA_INACTIVE_CALL;
	}
}

int Monitor::closeShutterPolicy(bool isClose)
{
	m_closeShutterPolicy = isClose;
	//m_logger->output(LOG_INFO, "closeShutterPolicy %d", m_closeShutterPolicy);
	return 0;
}

void Monitor::setProductType(ITA_PRODUCT_TYPE type)
{
	m_type = type;
}

void Monitor::setLogger(GuideLog * logger)
{
	m_logger = logger;
}

unsigned int Monitor::getLastShutterTime()
{
	return m_lastShutterTime;
}

void Monitor::setShutterTempUpper(float temp)
{
	m_shutterTempUpper = (int)(temp * 100);
	m_logger->output(LOG_INFO, "setShutterTempUpper:%d", m_shutterTempUpper);
}

void Monitor::setNucTempUpper(float temp)
{
	m_nucTempUpper = (int)(temp * 100);
	m_logger->output(LOG_INFO, "setNucTempUpper:%d", m_nucTempUpper);
}

void Monitor::setShutterIntervalBottom(unsigned int interval)
{
	m_shutterIntervalBottom = interval;
	m_logger->output(LOG_INFO, "setShutterIntervalBottom:%d", m_shutterIntervalBottom);
}

void Monitor::setNucIntervalBottom(unsigned int interval)
{
	m_nucIntervalBottom = interval;
	m_logger->output(LOG_INFO, "setNucIntervalBottom:%d", m_nucIntervalBottom);
}

void Monitor::setShutterParam(unsigned int p1, unsigned int p2, unsigned int p3, unsigned int p4, unsigned int p5, unsigned int p6)
{
	if (p1 > 0 && p3 >= p1 && p5 >= p3)
	{
		m_bootTimeToNow1 = p1;	//开机60分钟内
		m_shutterPeriod1 = p2;	//自动快门周期30秒
		m_bootTimeToNow2 = p3;	//开机60分钟与120分钟之间
		m_shutterPeriod2 = p4;	//自动快门周期60秒
		m_bootTimeToNow3 = p5;	//开机120分钟之后
		m_shutterPeriod3 = p6;	//自动快门周期90秒
		m_logger->output(LOG_INFO, "setShutterParam:%d %d %d %d %d %d", p1, p2, p3, p4, p5, p6);
	}
	else
		m_logger->output(LOG_WARN, "setShutterParam:%d %d %d %d %d %d", p1, p2, p3, p4, p5, p6);
}

