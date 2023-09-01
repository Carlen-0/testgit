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
	unsigned short emiss;     //范围1-100，默认98。
	unsigned short relHum;    //范围0-100，默认60。
	unsigned short distance;  //范围5-5000，默认50。
	short reflectedTemper;    //范围-400-5500，默认230。
	short atmosphericTemper;  //范围-400-1000，默认230。
	unsigned short modifyK;   //范围1-200，默认100。
	short modifyB;            //范围-100-100，默认0。
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
* @brief:	初始化一个ITA Coin实例。
* @param:	ITA_COIN_DEVICE_MODEL deviceModel  Coin设备模型。
* @param:	ITA_COIN_LENS_TYPE lensType        Coin设备镜头。
* @return:	成功，返回实例句柄。失败，返回NULL。
**/
ITA_API HANDLE_T ITA_CoinInit(ITA_COIN_DEVICE_MODEL deviceModel, ITA_COIN_LENS_TYPE lensType);

/**
* @brief:	释放一个ITA Coin实例资源。
* @param:	HANDLE_T instance	ITA Coin实例。
* @return:	无。
**/
ITA_API void ITA_CoinUninit(HANDLE_T instance);

/**
* @brief:	测温函数，支持单点测温和矩阵测温。
* @param:	HANDLE_T instance           ITACoin实例。
* @param:	short *pGray                16位的灰度数据缓存起始地址。
* @param:	int grayLen                 灰度数据长度，单位short。长度为1时单点测温。
* @param:	unsigned  char *pParamLine  参数行。
* @param:	int lineLen                 参数行长度，单位byte。
* @param:	ITACoinMTParam mtParam      用户设置的测温参数。
* @param:	float *pTemper              温度矩阵缓存起始地址。温度矩阵长度与grayLen相等。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_CoinTempMatrix(HANDLE_T instance, short *pGray, int grayLen, 
	unsigned  char *pParamLine, int lineLen, ITACoinMTParam mtParam, float *pTemper);

/**
* @brief:	解析并输出参数行信息。
* @param:	HANDLE_T instance           ITACoin实例。
* @param:	unsigned  char *pParamLine  参数行。
* @param:	int lineLen                 参数行长度，单位byte。
* @param:	ITACoinInternalParam* pInfo 输出参数，参数行信息。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_CoinParamInfo(HANDLE_T instance, unsigned  char *pParamLine, int lineLen, ITACoinInternalParam* pInfo);

/**
* @brief:	调试器，设置调试参数。
* @param:	HANDLE_T instance			ITACoin实例。
* @param:	ITA_DEBUG_TYPE type	        参数类型。
* @param:	void *param					参数值，传地址。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_CoinDebugger(HANDLE_T instance, ITA_DEBUG_TYPE type, void *param);

ITA_CUSTOM_END

#endif // !ITA_CUSTOM_H
