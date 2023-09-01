/*************************************************************
Copyright (C), 2022--2023, Wuhan Guide Sensmart Tech Co., Ltd
File name   : ITACustom.h
Author      : Guide Sensmart RD. wangyan
Version     : 1.0
Date        : 2022/04/29
Description : ITA SDK interface definition.
*************************************************************/
#ifndef ITA_CUSTOM_H
#define ITA_CUSTOM_H

#ifdef __cplusplus
#    define ITA_CUSTOM_START  extern "C" {
#    define ITA_CUSTOM_END    };
#else
#    define ITA_CUSTOM_START
#    define ITA_CUSTOM_END
#endif

ITA_CUSTOM_START

#include "ITADTD.h"

typedef enum ITACoinDeviceModel
{
	ITA_COIN_612,
	ITA_COIN_417
}ITA_COIN_DEVICE_MODEL;

typedef enum ITACoinLensType
{
	ITA_COIN_LENS_8,
	ITA_COIN_LENS_9,
	ITA_COIN_LENS_13,
	ITA_COIN_LENS_19
}ITA_COIN_LENS_TYPE;

typedef struct ITA_COIN_MT_PARAM
{
	unsigned short emiss;     //��Χ1-100��Ĭ��98��
	unsigned short relHum;    //��Χ0-100��Ĭ��60��
	unsigned short distance;  //��Χ5-5000��Ĭ��50��
	short reflectedTemper;    //��Χ-400-5500��Ĭ��230��
	short atmosphericTemper;  //��Χ-400-1000��Ĭ��230��
	unsigned short modifyK;   //��Χ1-200��Ĭ��100��
	short modifyB;            //��Χ-100-100��Ĭ��0��
}ITACoinMTParam;

typedef struct ITA_COIN_INTERNAL_PARAM
{
	unsigned short relHum;
	unsigned short distance;
	unsigned short emiss;
	short reflectedTemper;
	short KF;
	short K1;
	short K2;
	short K3;
	short K4;
	short B1;
	int A0;
	int A1;
	short KJ;
	short KB;
	short gear;
	short sdeltaY16;
	short centerY16;
	short maxY16;
	int A2;
	short shutter_realtimeTemp;
	short lens_lastCalibrateTemp;
	short shutter_lastCalibrateTemp;
	short shutter_startTemp;
	short shutter_flag;
	short y16_average;
	short K5;
	short K6;
	int K7;
	short shutter_coldStartTemp;
	short lens_tempRaiseValue;
	short showMode;
	short lens_startTemp;
	short shutter_flashColdStartTemp;
	short K8;
	short deltaY16z;
	short shutterDrift_coldStartTemp;
	short empty1;
	short empty2;
	short hot_xPosition;
	short hot_yPosition;
	short hot_temp;
	short cold_xPosition;
	short cold_yPosition;
	short cold_temp;
	short cursor_xPosition;
	short cursor_yPosition;
	short cursor_temp;
	short region_avgTemp;
	short u16modify_k;
	short s16modify_b;
	short kbr_x100;
}ITACoinInternalParam;

/**
* @brief:	��ʼ��һ��ITA Coinʵ����
* @param:	ITA_COIN_DEVICE_MODEL deviceModel  Coin�豸ģ�͡�
* @param:	ITA_COIN_LENS_TYPE lensType        Coin�豸��ͷ��
* @return:	�ɹ�������ʵ�������ʧ�ܣ�����NULL��
**/
ITA_API HANDLE_T ITA_CoinInit(ITA_COIN_DEVICE_MODEL deviceModel, ITA_COIN_LENS_TYPE lensType);

/**
* @brief:	�ͷ�һ��ITA Coinʵ����Դ��
* @param:	HANDLE_T instance	ITA Coinʵ����
* @return:	�ޡ�
**/
ITA_API void ITA_CoinUninit(HANDLE_T instance);

/**
* @brief:	���º�����֧�ֵ�����º;�����¡�
* @param:	HANDLE_T instance           ITACoinʵ����
* @param:	short *pGray                16λ�ĻҶ����ݻ�����ʼ��ַ��
* @param:	int grayLen                 �Ҷ����ݳ��ȣ���λshort������Ϊ1ʱ������¡�
* @param:	unsigned  char *pParamLine  �����С�
* @param:	int lineLen                 �����г��ȣ���λbyte��
* @param:	ITACoinMTParam mtParam      �û����õĲ��²�����
* @param:	float *pTemper              �¶Ⱦ��󻺴���ʼ��ַ���¶Ⱦ��󳤶���grayLen��ȡ�
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_CoinTempMatrix(HANDLE_T instance, short *pGray, int grayLen, 
	unsigned  char *pParamLine, int lineLen, ITACoinMTParam mtParam, float *pTemper);

/**
* @brief:	�����������������Ϣ��
* @param:	HANDLE_T instance           ITACoinʵ����
* @param:	unsigned  char *pParamLine  �����С�
* @param:	int lineLen                 �����г��ȣ���λbyte��
* @param:	ITACoinInternalParam* pInfo �����������������Ϣ��
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_CoinParamInfo(HANDLE_T instance, unsigned  char *pParamLine, int lineLen, ITACoinInternalParam* pInfo);

/**
* @brief:	�����������õ��Բ�����
* @param:	HANDLE_T instance			ITACoinʵ����
* @param:	ITA_DEBUG_TYPE type	        �������͡�
* @param:	void *param					����ֵ������ַ��
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_CoinDebugger(HANDLE_T instance, ITA_DEBUG_TYPE type, void *param);

ITA_CUSTOM_END

#endif // !ITA_CUSTOM_H
