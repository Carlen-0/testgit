#ifndef COLLECTK_H
#define COLLECTK_H

#include "../ITADTD.h"
#include "../Module/GuideLog.h"
typedef struct ITA_COLLECT_K
{
	int status;//0:未开始，1：开始采集本底  2：正在采集本底 3：本底采集完成。
	int temp;//黑体温度。
	int gear;//焦温档位。
	unsigned short* baseData;//本底数据。
	ITACollectBaseFinish cb;//通知用户的函数。
	void* para;//用户参数。
}ITACollectK;

class CollectK {
public:
	CollectK(int width,int height, int cameraId);
	~CollectK();

	void setLogger(GuideLog* log);

	//注册采集的信息
	ITA_RESULT registryBaseInfo(int gear, float blackTemp, ITACollectBaseFinish cb, void* userParam);

	bool isCollectK();

	ITA_RESULT isSaveK(int gear);

	void collectKData(short* src);

	unsigned short* getLowBase();
	unsigned short* getHighBase();
	void initCollectInit();

private:
	GuideLog* m_logger;
	int m_frameSize;
	int m_cameraID;
	//制作K数据
	ITACollectK m_collectLow;
	ITACollectK m_collectHigh;
	int m_collectKCount;//采K计数器
	int m_collectKTotalCount;//采K总数
	unsigned short* m_kArray;//采K数组
};

#endif

