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
	m_fpaTempWhenShutter = 0;	//�ϴο���ʱ����
	m_fpaTempWhenNUC = 0;		//�ϴ�NUCʱ����
	m_nucIntervalBottom = 5;	//��������NUCʱ����������5��
	m_shutterIntervalBottom = 3;//�������ο���ʱ����������3�롣����ʱ����Ƶ������������塣
	if (ITA_256_TIMO == type)
	{
		//����0.2�������Ų���
		m_shutterTempUpper = 20;
		//����3����NUC����
		m_nucTempUpper = 300;
	}
	else if (ITA_120_TIMO == type)
	{
		//����0.25�������Ų���
		m_shutterTempUpper = 25;
		//����1����NUC����
		m_nucTempUpper = 100;
	}
	m_autoShutter = true;
	m_closeShutterPolicy = false;	//Ĭ�ϴ򿪿��ź�NUC���ԣ����û�������Ҫ�����رա�
	m_logger = NULL;
	m_originalTime = 0;	//����ʱ��
	m_bootTimeToNow1 = 0;	//����60������
	m_shutterPeriod1 = 0;	//�Զ���������30��
	m_bootTimeToNow2 = 0;	//����60������120����֮��
	m_shutterPeriod2 = 0;	//�Զ���������60��
	m_bootTimeToNow3 = 0;	//����120����֮��
	m_shutterPeriod3 = 0;	//�Զ���������90��
	m_userPeriod = false;	//�û��Ƿ����ÿ������ڡ�����û������Զ�����ʱ�䣬��ôĬ�ϵ��Զ����Ų��Բ�����Ч��
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
		//����ϵͳʱ�䱻��С��ʱ�������㡣
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
		m_userPeriod = true;	//�û��Ƿ����ÿ������ڡ�����û������Զ�����ʱ�䣬��ôĬ�ϵ��Զ����Ų��Բ�����Ч��
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
		//����ϵͳʱ�䱻��С��ʱ�������㡣
		m_lastShutterTime = 0;
	}
	/*�ֶ�NUC*/
	if (m_nucManual)
	{
		m_lastNUCTime = currentTime;
		m_lastShutterTime = currentTime;
		m_fpaTempWhenNUC = m_realTimeFPATemp;
		m_nucManual = false;
		m_logger->output(LOG_INFO, "manual nuc begin");
		return MONITOR_NUC_NOW;
	}
	/*�ֶ�����*/
	if (m_shutterManual)
	{
		m_lastShutterTime = currentTime;
		m_fpaTempWhenShutter = m_realTimeFPATemp;
		m_shutterManual = false;
		m_logger->output(LOG_INFO, "manual shutter begin");
		return MONITOR_SHUTTER_NOW;
	}
	/*�رղ��Ժ��ټ���Զ����ź�NUC��*/
	if (m_closeShutterPolicy)
		return MONITOR_NONE;

	/*��ʼ����ɺ�Ҫ��һ��NUC��*/
	if (!m_lastNUCTime)
	{
		m_lastNUCTime = currentTime;
		m_lastShutterTime = currentTime;
		m_fpaTempWhenNUC = m_realTimeFPATemp;
		m_logger->output(LOG_INFO, "first nuc begin shutterTempUpper=%d nucTempUpper=%d", m_shutterTempUpper, m_nucTempUpper);
		m_originalTime = currentTime;
		return MONITOR_NUC_NOW;
	}
	/*NUC��������*/
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
					//���¹���ͼ���쳣�������Ѵﲻ�����ȣ���ͼ��Ҫ������
					m_closedLoopStatus = CLOSED_LOOP_ING;
					//m_fpaTempWhenNUC�����£�5����ʱ������´μ�������Res+1�ջ�NUC��
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
					//������ͼ��δ�쳣��������NUC��
					m_logger->output(LOG_INFO, "Monitor: realTimeFPATemp=%d, maxFpaTemp=%d MONITOR_NUC_NOW.", m_realTimeFPATemp, m_maxFpaTemp);
					return MONITOR_NUC_NOW;
				}
				else
				{
					//������NUC�ջ��ѳɹ����¶����ڱ仯��ʹ���ϴγɹ���Res��NUC��
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
				//120ģ�黷���½����������·�Χ��RES�ָ����ݰ��е�ֵ��
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
	/*������������*/
	if (abs(m_realTimeFPATemp - m_fpaTempWhenShutter) >= m_shutterTempUpper && currentTime - m_lastShutterTime >= m_shutterIntervalBottom * 1000)
	{
		m_lastShutterTime = currentTime;
		m_logger->output(LOG_INFO, "temp shutter begin: fpaTemp=%d dif=%d", m_realTimeFPATemp, m_realTimeFPATemp - m_fpaTempWhenShutter);
		m_fpaTempWhenShutter = m_realTimeFPATemp;
		return MONITOR_SHUTTER_NOW;
	}
	/*�Զ�����*/
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
	//120ģ��RESȡֵ��ΧΪ8~11�����ݰ���ͨ����8.
	unsigned char resArray[4] = { 8, 9, 10, 11 };
	int size = 4;
	unsigned char ucRes = 0;
	//�������������3��ֵ�ͷ���0.
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
	/*�����Զ����Ų��Բ������¿�������*/
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
		//�û����ݲ�Ʒ��ҪNUC֮������һ�ο��Ų����������м��ʱ�����ơ�
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
		//�û����ݲ�Ʒ��ҪNUCʧЧ֮���������2��NUC�������м��ʱ�����ơ�
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
		m_bootTimeToNow1 = p1;	//����60������
		m_shutterPeriod1 = p2;	//�Զ���������30��
		m_bootTimeToNow2 = p3;	//����60������120����֮��
		m_shutterPeriod2 = p4;	//�Զ���������60��
		m_bootTimeToNow3 = p5;	//����120����֮��
		m_shutterPeriod3 = p6;	//�Զ���������90��
		m_logger->output(LOG_INFO, "setShutterParam:%d %d %d %d %d %d", p1, p2, p3, p4, p5, p6);
	}
	else
		m_logger->output(LOG_WARN, "setShutterParam:%d %d %d %d %d %d", p1, p2, p3, p4, p5, p6);
}

