/*************************************************************
Copyright (C), 2021--2022, Wuhan Guide Sensmart Tech Co., Ltd
File name   : Monitor.h
Author      : Guide Sensmart RD. wangyan
Version     : 1.0
Date        : 2021/11/9
Description : ITA SDK Monitor.
*************************************************************/
#ifndef ITA_MONITOR_H
#define ITA_MONITOR_H

#include "../Module/PortingAPI.h"
#include "../Base/Config.h"
#include "../ITA.h"
#include "../Base/Parser.h"
#include "../InfraredImage/ImageProcessor.h"


#define AVGB_LOWER_LIMIT	3000

typedef enum MonitorStatus
{
	MONITOR_NONE,
	MONITOR_SHUTTER_NOW,
	MONITOR_NUC_NOW,
	MONITOR_NUC_CLOSED_LOOP, //120ģ��60�滷��RES+1�ջ�
	MONITOR_NUC_LOOP_REPEAT, //120ģ��60�滷��RES������NUC
	MONITOR_NUC_RECOVERY, //120ģ�黷���½����������·�Χ��RES�ָ����ݰ��е�ֵ��
}MONITOR_STATUS;

//0����ʼ״̬��1������Res�ջ���2���ջ��ɹ���3���ջ�ʧ�ܡ�
typedef enum ClosedLoopStatus
{
	CLOSED_LOOP_NONE,
	CLOSED_LOOP_ING,
	CLOSED_LOOP_SUCCESS,
	CLOSED_LOOP_FAILED, //120ģ��60�滷��RES�ջ�
}CLOSED_LOOP_STATUS;

class Monitor
{
public:
	Monitor(ITA_PRODUCT_TYPE type);
	~Monitor();

	ITA_RESULT setInterval(unsigned int second);

	bool isSaveColdHot(unsigned int second);

	ITA_RESULT autoShutter(ITA_SWITCH flag);

	/*�Զ����ż��*/
	MONITOR_STATUS timeLoopOnce(unsigned int currentTime);

	/*���±���Ϣ��120ģ��RES�ջ��õ���*/
	ITA_RESULT setFPAArray(short* fpaTempArray, int size, unsigned char startRes);

	/*���±��׾�ֵ��120ģ��RES�ջ��õ���*/
	ITA_RESULT updateAVGB(short avgB);

	/*���е�����NUC�ջ������ڣ����ͬʱ���㽹�¸�����߲ɼ�����+3��ͱ��׾�ֵС�����õ�NUC�ջ�ֵ-2000��ִ��RES�ջ���
	������㽹��С�ڵ�����߲ɼ����£���RES����������߰���ֵ��RESȡֵ��ΧΪ8~11�����Բ�ȡ�����ķ������бջ���
	RESԽ��ADֵԽ�󣬽���Խ��ADֵԽ�ͣ����׾�ֵ�����2000���NUC�ջ�ֵ����Ϊ�ж������������������Ϊ�ջ���ɣ�
	С�ڵ�����RES����ֵ��1��������NUC���ȡ���׾�ֵ�������жϡ�*/
	unsigned char calcRes(unsigned char currentRes);

	/*�����Զ����Ų��Բ������¿�������*/
	ITA_RESULT updatePeriod(unsigned int currentTime);

	//���½��£��Ŵ�100����
	int updateSensorTemp(int fpaTemp);

	//���¹��ǲ���
	void updatePotCoverAlgoPara(MtParams* mtParam, stDGGTPara* potCoverPara);

	ITA_RESULT shutterManual(int flag);

	ITA_RESULT nucManual(int flag);

	int closeShutterPolicy(bool isClose);

	/*���ô������ļ��ж�ȡ�Ĳ�Ʒ�ͺ�*/
	void setProductType(ITA_PRODUCT_TYPE type);
	void setLogger(GuideLog *logger);

	unsigned int getLastShutterTime();

	void setShutterTempUpper(float temp);
	void setNucTempUpper(float temp);
	void setShutterIntervalBottom(unsigned int interval);
	void setNucIntervalBottom(unsigned int interval);
	void setShutterParam(unsigned int p1, unsigned int p2, unsigned int p3, unsigned int p4, unsigned int p5, unsigned int p6);
private:
	ITA_PRODUCT_TYPE m_type;
	unsigned int m_shutterPeriod;
	unsigned int m_lastShutterTime;
	unsigned int m_lastNUCTime;
	unsigned int m_lastSaveCHTime;
	bool m_shutterManual;
	bool m_nucManual;
	int m_realTimeFPATemp;
	int m_fpaTempWhenShutter;	//�ϴο���ʱ����
	int m_fpaTempWhenNUC;		//�ϴ�NUCʱ����
	int m_shutterTempUpper;		//��������
	int m_nucTempUpper;
	unsigned int m_shutterIntervalBottom;	//��С���ʱ��
	unsigned int m_nucIntervalBottom;
	bool m_autoShutter;
	bool m_closeShutterPolicy;	//�رղ��Ժ��ټ���Զ����ź�NUC��
	GuideLog *m_logger;
	unsigned int m_originalTime;	//����ʱ��
	unsigned int m_bootTimeToNow1;	//����60������
	unsigned int m_shutterPeriod1;	//�Զ���������30��
	unsigned int m_bootTimeToNow2;	//����60������120����֮��
	unsigned int m_shutterPeriod2;	//�Զ���������60��
	unsigned int m_bootTimeToNow3;	//����120����֮��
	unsigned int m_shutterPeriod3;	//�Զ���������90��
	unsigned int m_saveStatusPeriod;//10���ӱ���һ�����Ȼ�״̬
	bool m_userPeriod;	//�û��Ƿ����ÿ������ڡ�����û������Զ�����ʱ�䣬��ôĬ�ϵ��Զ����Ų��Բ�����Ч��
	/*���е�����NUC�ջ������ڣ����ͬʱ���㽹�¸�����߲ɼ�����+3��ͱ��׾�ֵС�����õ�NUC�ջ�ֵ-2000��ִ��RES�ջ���
	������㽹��С�ڵ�����߲ɼ����£���RES����������߰���ֵ��RESȡֵ��ΧΪ8~11�����Բ�ȡ�����ķ������бջ���
	RESԽ��ADֵԽ�󣬽���Խ��ADֵԽ�ͣ����׾�ֵ�����2000���NUC�ջ�ֵ����Ϊ�ж������������������Ϊ�ջ���ɣ�
	С�ڵ�����RES����ֵ��1��������NUC���ȡ���׾�ֵ�������жϡ�*/
	short m_maxFpaTemp;
	short m_avgB;
	//unsigned char m_startRes;
	CLOSED_LOOP_STATUS m_closedLoopStatus; //0����ʼ״̬��1������Res�ջ���2���ջ��ɹ���3���ջ�ʧ�ܡ�
};

#endif // !ITA_MONITOR_H



