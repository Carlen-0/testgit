#include "StartReview.h"
#include <math.h>

StartReview::StartReview(int camaraId,GuideLog* logger)
{
	m_cameraID = camaraId;
	m_logger = logger;
}

ITA_RESULT StartReview::init(ITA_RANGE range, ITA_PRODUCT_TYPE productType, int temp, ITAReviewFinish cb, void* userParam)
{
	m_reviewStatus = true;
	m_blackTemp = temp;
	m_notify = cb;
	m_collectCount = 0;
	m_range = range;
	m_sum = 0.0;
	m_average = 0.0;
	param = userParam;
	m_productType = productType;
	if (m_range == ITA_HUMAN_BODY)
	{
		m_collectInterVal = 1;
		m_collectTotal = 30;
	}
	else {
		m_collectTotal = 50;
	}
	return ITA_OK;
}

void StartReview::deviationHumanBody(float averTemp)
{
	float deviation = fabs(averTemp - m_blackTemp);
	bool deviationStatus;
	if (deviation < 0.5) {
		deviationStatus = true;
	}
	else {
		deviationStatus = false;
	}
	//复核结束通知用户
	m_logger->output(LOG_INFO, "status:%d,temp:%d,averTemp:%.2f,deviation:%.2f", deviationStatus, m_blackTemp, averTemp, deviation);
	m_notify(m_cameraID, deviationStatus, averTemp, deviation, param);
}

void StartReview::deviationIndustry(float averTemp)
{
	float deviation = fabs(averTemp - m_blackTemp);
	bool deviationStatus;
	if (m_blackTemp <= 0) {
		if (m_productType == ITA_120_TIMO) {
			if (deviation <= 5.0) {
				deviationStatus = true;
			}
			else {
				deviationStatus = false;
			}
		}
		else {
			if (deviation <= 4.0) {
				deviationStatus = true;
			}
			else {
				deviationStatus = false;
			}
		}
	}
	else if (m_blackTemp > 0 && m_blackTemp <= 100) {
		if (deviation < 2.0) {
			deviationStatus = true;
		}
		else {
			deviationStatus = false;
		}
	}
	else {
		if ((deviation / m_blackTemp) < 0.02) {
			deviationStatus = true;
		}
		else {
			deviationStatus = false;
		}
	}
	//复核结束通知用户
	m_logger->output(LOG_INFO, "status:%d,temp:%d,averTemp:%.2f,deviation:%.2f,collectCount=%d", deviationStatus, m_blackTemp, averTemp, deviation, m_collectCount);
	m_notify(m_cameraID, deviationStatus, averTemp, deviation, param);
}

void StartReview::reviewHumanBody(float temp, unsigned int currentTime)
{
	if (currentTime - lastReviewTime >= m_collectInterVal * 1000)
	{
		m_logger->output(LOG_INFO, "lastCollectTime=%u,currentTime=%d,collectCount=%d\n", lastReviewTime, currentTime, m_collectCount);
		lastReviewTime = currentTime;
		m_sum += temp;
		m_collectCount++;
	}
	if (m_collectCount >= m_collectTotal)
	{
		m_reviewStatus = false;
		m_collectCount = 0;
		m_average = m_sum / m_collectTotal;
		m_logger->output(LOG_INFO, "m_sum=%.2f,m_average=%.2f", m_sum, m_average);
		deviationHumanBody(m_average);
	}
}

void StartReview::reviewIndustry(float temp)
{
	if (m_collectCount >= m_collectTotal)
	{
		m_reviewStatus = false;
		m_collectCount = 0;
		m_average = m_sum / m_collectTotal;
		m_logger->output(LOG_INFO, "m_sum=%.2f,m_average=%.2f", m_sum, m_average);
		deviationIndustry(temp);
	}
	else {
		m_sum += temp;
		m_logger->output(LOG_INFO, "temp=%.2f,collectCount=%d", temp, m_collectCount);
		m_collectCount++;
	}
}

void StartReview::review(float temp, unsigned int currentTime)
{
	switch (m_range){
	case ITA_HUMAN_BODY:
	{
		reviewHumanBody(temp,currentTime);
		break;
	}
	case ITA_INDUSTRY_LOW:
	case ITA_INDUSTRY_HIGH:
	{
		reviewIndustry(temp);
		break;
	}
	default:
		break;
	}
}

StartReview::~StartReview()
{

}