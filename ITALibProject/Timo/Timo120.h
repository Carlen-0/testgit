/*************************************************************
Copyright (C), 2021--2022, Wuhan Guide Sensmart Tech Co., Ltd
File name   : Timo120.h
Author      : Guide Sensmart RD. wangyan
Version     : 1.0
Date        : 2021/11/15
Description : Configure Timo 120 detector.
*************************************************************/
#ifndef ITA_TIMO120_H
#define ITA_TIMO120_H

#include "DetectorWrapper.h"

typedef struct DATA_DETECTOR {
	/*unsigned char    p10H;
	unsigned char    p11H;
	unsigned char    p12H;
	unsigned char    p13H;
	unsigned char    p14H;
	unsigned char    p15H;
	unsigned char    p16H;
	unsigned char    p17H;
	unsigned char    p18H;
	unsigned char    p19H;
	unsigned char    p1aH;
	unsigned char    p1bH;
	unsigned char    p1cH;
	unsigned char    p20H;
	unsigned char    p21H;
	unsigned char    p22H;
	unsigned char    p23H;
	unsigned char    p29H;
	unsigned char    p2aH;
	unsigned char    F0H;
	unsigned char    F1H;*/
	unsigned char    dps[21];
	unsigned char    reserve[9];
}DetectorParameter;

class Timo120
{
public:
	Timo120(ITARegistry * registry, int cameraID, ITA_RANGE range);
	~Timo120();
	/**
	* @brief:	̽������ʼ����
	* @param:	int clkHz			ʱ�ӣ���λHZ��֧��12M��6M����ʱ�ӡ�
	* @return:	�ɹ�����GUIDEIR_OK��ʧ�ܷ���GUIDEIR_ERR��
	* @note:
	**/
	ITA_RESULT DetectorInit(int clkHz, unsigned char validMode, unsigned char dataMode, unsigned char frameFrequency, DetectorParameter *dp);

	/**
	* @brief:	��NUC��
	* @param:	�ޡ�
	* @return:	�ɹ�����GUIDEIR_OK��ʧ�ܷ���GUIDEIR_ERR��
	* @note:
	**/
	int DoNUC();

	/**
	* @brief:	�л���λ��
	* @param:	ITA_RANGE range		��λ���
	* @return:	�ɹ�����GUIDEIR_OK��ʧ�ܷ���GUIDEIR_ERR��
	* @note:	ͬһ���·�Χ���л���λʱ̽��������Ŀǰһ�������ø��¡��л����·�Χʱ��Ҫ��������̽������
	**/
	int ChangeGears(ITA_RANGE range, unsigned char ucGain, unsigned char ucInt, unsigned char ucRes, bool isNUC);

	void setLogger(GuideLog *logger);
	void setDebug(bool debug);
private:
	ITA_RESULT sensorWriteReg(unsigned char addr, unsigned char data);
	ITA_RESULT sensorReadReg(unsigned char addr, unsigned char *data);
	ITA_RESULT sensorRegInitial(DetectorConf *regData);
	ITA_RESULT startNUC(void);
	ITA_RESULT stopNUC(void);
	ITA_RESULT configGain(unsigned char value);
	ITA_RESULT configInt(unsigned short value);
	ITA_RESULT configRes(unsigned char value);
private:
	SENSORCONFIG_STATUS m_params;
	int m_cameraID;
	ITARegistry *m_registry;
	ITA_RANGE m_range;
	GuideLog *m_logger;
	bool m_debug;
	int m_clkHz;	//120ģ�������ʱ�ӣ�����8MHz��
};

#endif // !ITA_TIMO120_H
