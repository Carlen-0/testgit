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
	int s32LimitNear;	//标准镜头近焦限值
	int s32LimitFar;	//标准镜头远焦限值
	int s32ReturnDiff;  //回程差校准补偿值-----------
}ITA_FOCUS_BASE_CFG_S;

typedef struct ITA_FOCUS_LASER_CFG {
	int s32Length;		//慢速行程..
	int s32FastSpeed;	//电机快速速度
	int s32LowSpeed;	//电机慢速速度
	int s32Diff;		//回程差
	int s32LaserDiff; //激光校准补偿值-----------
}ITA_FOCUS_LASER_CFG_S;

typedef struct ITA_FOCUS_CONTRAST_CFG {
	int s32Length;		//慢速行程
	int s32FastSpeed;	//电机快速速度
	int s32LowSpeed;	//电机慢速速度
	int s32LowDiff;		//慢速行程回程差补偿
	int s32LastDiff;	//最后一步回程差补偿
	int s32ThresholdRatio;	//粗调比值阈值1
	int s32ThresholdN1;		//连续未更新帧数阈值N1
	int s32ThresholdN2;		//连续未更新帧数阈值N1	
}ITA_FOCUS_CONTRAST_CFG_S;

typedef struct ITA_LASERAF_REGISTRY
{
	//电机、霍尔传感器、激光传感器需要用户支持以下函数：
	/**
	* @brief:  读距离曲线包。
	* @param:  int cameraID			标识设备，支持同时打开多台设备。
	* @param:  unsigned char *buf	数据缓存
	* @param:  int bufSize			数据缓存的长度
	* @param:  int *length			读取的实际长度
	* @param:  ITA_FIELD_ANGLE lensType 镜头类型
	* @param:  void *userParam		用户参数，注册时由用户传入。ITA库调用注册函数时再传给用户。
	* @return: 返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
	* @see ITA_Init，camera id由用户输入。
	**/
	ITA_RESULT(*ReadCurveData)(int cameraID, unsigned char *buf, int bufSize, int *length, ITA_FIELD_ANGLE lensType, void *userParam);

	/******************************************************************************
	* 函数名称:     focus_move_to_hall
	* 功能说明:     让电机运行到目标霍尔的位置
	* 输入参数:
		dstHall：		目标霍尔的值
		speed：			电机速度
		wait：			是否等待
	* 输出参数:
	* 返 回 值:
	0         ：成功
	非0       ：失败，其值为错误码
	* 注意事项:
	******************************************************************************/
	ITA_RESULT(*focus_move_to_hall)(int cameraID, int dstHall, int speed, int wait, void *userParam);//快速运动至目标hall值，速度可设置
	ITA_RESULT(*GetClearity)(int cameraID, int *value, void *userParam);
	/*****************************************************************************

	******************************************************************************/
	ITA_RESULT(*GetDistance)(int cameraID, int *value, void *userParam);
	ITA_RESULT(*SetStop)(int cameraID, void *userParam);
	ITA_RESULT(*GetHall)(int cameraID, int *value, void *userParam);

	/*用户参数，调用注册函数时再传给用户。*/
	void *userParam;
}ITALaserAFRegistry;

/**
* @brief:	用户注册LaserAF功能需要用到的函数。
* @param:	HANDLE_T instance				ITA实例。
* @param:	ITALaserAFRegistry *registry	用户注册函数。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_LaserAFRegister(HANDLE_T instance, ITALaserAFRegistry *registry);

/**
* @brief:	激光对焦。
* @param:	HANDLE_T instance				ITA实例。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_LaserAF(HANDLE_T instance);

/**
* @brief:	对比度对焦。
* @param:	HANDLE_T instance				ITA实例。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_ContrastAF(HANDLE_T instance);

/**
* @brief:	校准流程
* @param:	HANDLE_T instance				ITA实例。
* @param:	ITA_AF_CALIBRATE type			校准类型
* @param:	int *diff						校准值
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值-3012，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_CalibrateAF(HANDLE_T instance, ITA_AF_CALIBRATE type, int *diff);

/**
* @brief:	AF参数设置
* @param:	HANDLE_T instance				ITA实例。
* @param:	ITA_AF_INFO_SET type			参数类型
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_SetInfoAF(HANDLE_T instance, ITA_AF_INFO type, void *param);

ITA_LASERAF_END

#endif // !ITA_LASERAF_H
