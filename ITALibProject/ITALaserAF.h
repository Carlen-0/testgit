/*************************************************************
Copyright (C), 2022--2023, Wuhan Guide Sensmart Tech Co., Ltd
File name   : ITALaserAF.h
Author      : Guide Sensmart RD. wangyan
Version     : 1.0
Date        : 2022/10/29
Description : ITALaserAF.
*************************************************************/
#ifndef ITA_LASERAF_H
#define ITA_LASERAF_H

#ifdef __cplusplus
#    define ITA_LASERAF_START  extern "C" {
#    define ITA_LASERAF_END    };
#else
#    define ITA_LASERAF_START
#    define ITA_LASERAF_END
#endif

ITA_LASERAF_START

#include "ERROR.h"
#include "ITADTD.h"

typedef struct ITA_FOCUS_BASE_CFG {
	int s32LimitNear;	//��׼��ͷ������ֵ
	int s32LimitFar;	//��׼��ͷԶ����ֵ
	int s32ReturnDiff;  //�س̲�У׼����ֵ-----------
}ITA_FOCUS_BASE_CFG_S;

typedef struct ITA_FOCUS_LASER_CFG {
	int s32Length;		//�����г�..
	int s32FastSpeed;	//��������ٶ�
	int s32LowSpeed;	//��������ٶ�
	int s32Diff;		//�س̲�
	int s32LaserDiff; //����У׼����ֵ-----------
}ITA_FOCUS_LASER_CFG_S;

typedef struct ITA_FOCUS_CONTRAST_CFG {
	int s32Length;		//�����г�
	int s32FastSpeed;	//��������ٶ�
	int s32LowSpeed;	//��������ٶ�
	int s32LowDiff;		//�����г̻س̲��
	int s32LastDiff;	//���һ���س̲��
	int s32ThresholdRatio;	//�ֵ���ֵ��ֵ1
	int s32ThresholdN1;		//����δ����֡����ֵN1
	int s32ThresholdN2;		//����δ����֡����ֵN1	
}ITA_FOCUS_CONTRAST_CFG_S;

typedef struct ITA_LASERAF_REGISTRY
{
	//��������������������⴫������Ҫ�û�֧�����º�����
	/**
	* @brief:  ���������߰���
	* @param:  int cameraID			��ʶ�豸��֧��ͬʱ�򿪶�̨�豸��
	* @param:  unsigned char *buf	���ݻ���
	* @param:  int bufSize			���ݻ���ĳ���
	* @param:  int *length			��ȡ��ʵ�ʳ���
	* @param:  ITA_FIELD_ANGLE lensType ��ͷ����
	* @param:  void *userParam		�û�������ע��ʱ���û����롣ITA�����ע�ắ��ʱ�ٴ����û���
	* @return: ����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
	* @see ITA_Init��camera id���û����롣
	**/
	ITA_RESULT(*ReadCurveData)(int cameraID, unsigned char *buf, int bufSize, int *length, ITA_FIELD_ANGLE lensType, void *userParam);

	/******************************************************************************
	* ��������:     focus_move_to_hall
	* ����˵��:     �õ�����е�Ŀ�������λ��
	* �������:
		dstHall��		Ŀ�������ֵ
		speed��			����ٶ�
		wait��			�Ƿ�ȴ�
	* �������:
	* �� �� ֵ:
	0         ���ɹ�
	��0       ��ʧ�ܣ���ֵΪ������
	* ע������:
	******************************************************************************/
	ITA_RESULT(*focus_move_to_hall)(int cameraID, int dstHall, int speed, int wait, void *userParam);//�����˶���Ŀ��hallֵ���ٶȿ�����
	ITA_RESULT(*GetClearity)(int cameraID, int *value, void *userParam);
	/*****************************************************************************

	******************************************************************************/
	ITA_RESULT(*GetDistance)(int cameraID, int *value, void *userParam);
	ITA_RESULT(*SetStop)(int cameraID, void *userParam);
	ITA_RESULT(*GetHall)(int cameraID, int *value, void *userParam);

	/*�û�����������ע�ắ��ʱ�ٴ����û���*/
	void *userParam;
}ITALaserAFRegistry;

/**
* @brief:	�û�ע��LaserAF������Ҫ�õ��ĺ�����
* @param:	HANDLE_T instance				ITAʵ����
* @param:	ITALaserAFRegistry *registry	�û�ע�ắ����
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_LaserAFRegister(HANDLE_T instance, ITALaserAFRegistry *registry);

/**
* @brief:	����Խ���
* @param:	HANDLE_T instance				ITAʵ����
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_LaserAF(HANDLE_T instance);

/**
* @brief:	�ԱȶȶԽ���
* @param:	HANDLE_T instance				ITAʵ����
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_ContrastAF(HANDLE_T instance);

/**
* @brief:	У׼����
* @param:	HANDLE_T instance				ITAʵ����
* @param:	ITA_AF_CALIBRATE type			У׼����
* @param:	int *diff						У׼ֵ
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ-3012���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_CalibrateAF(HANDLE_T instance, ITA_AF_CALIBRATE type, int *diff);

/**
* @brief:	AF��������
* @param:	HANDLE_T instance				ITAʵ����
* @param:	ITA_AF_INFO_SET type			��������
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_SetInfoAF(HANDLE_T instance, ITA_AF_INFO type, void *param);

ITA_LASERAF_END

#endif // !ITA_LASERAF_H
