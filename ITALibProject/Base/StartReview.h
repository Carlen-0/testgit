#ifndef STARTREVIEW_H
#define STARTREVIEW_H

#include "../ITADTD.h"
#include "../Module/GuideLog.h"

typedef struct ITA_START_REVIEW {
	int temp;
	int collectInterVal;
	int collectCount;
	int collectTotalNum;
	unsigned int lastCollectTime;
	float sum;
	float averTemp;
	float deviation;
	bool isStartReview;
	bool status;
	ITAReviewFinish m_reviewFinish;
}ITAStartReview;

class StartReview {
public:
	StartReview(int camaraId, GuideLog* logger);

	ITA_RESULT init(ITA_RANGE range, ITA_PRODUCT_TYPE productType, int temp, ITAReviewFinish cb, void* userParam);

	void reviewHumanBody(float temp, unsigned int currentTime);

	void reviewIndustry(float temp);

	void deviationHumanBody(float averTemp);

	void deviationIndustry(float averTemp);


	void review(float temp,unsigned int currentTime);

	bool getReviewStatus()
	{
		return m_reviewStatus;
	}

	~StartReview();

private:
	int m_blackTemp;
	int m_collectCount;
	int m_collectTotal;
	ITAReviewFinish m_notify;
	ITA_RANGE m_range;
	int m_collectInterVal;
	bool m_reviewStatus;
	unsigned int lastReviewTime;
	float m_sum;
	float m_average;
	GuideLog* m_logger;
	int m_cameraID;
	ITA_PRODUCT_TYPE m_productType;
	void* param;
};

#endif

