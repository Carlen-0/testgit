#ifndef COLLECTK_H
#define COLLECTK_H

#include "../ITADTD.h"
#include "../Module/GuideLog.h"
typedef struct ITA_COLLECT_K
{
	int status;//0:δ��ʼ��1����ʼ�ɼ�����  2�����ڲɼ����� 3�����ײɼ���ɡ�
	int temp;//�����¶ȡ�
	int gear;//���µ�λ��
	unsigned short* baseData;//�������ݡ�
	ITACollectBaseFinish cb;//֪ͨ�û��ĺ�����
	void* para;//�û�������
}ITACollectK;

class CollectK {
public:
	CollectK(int width,int height, int cameraId);
	~CollectK();

	void setLogger(GuideLog* log);

	//ע��ɼ�����Ϣ
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
	//����K����
	ITACollectK m_collectLow;
	ITACollectK m_collectHigh;
	int m_collectKCount;//��K������
	int m_collectKTotalCount;//��K����
	unsigned short* m_kArray;//��K����
};

#endif

