/*************************************************************
Copyright (C), 2022--2023, Wuhan Guide Sensmart Tech Co., Ltd
File name   : ITABB.h
Author      : Guide Sensmart RD. wangyan
Version     : 1.0
Date        : 2022/10/29
Description : ITA SDK interface definition for blackbody temperature measurement products.
*************************************************************/
#ifndef ITA_BB_H
#define ITA_BB_H

#ifdef __cplusplus
#    define ITA_BB_START  extern "C" {
#    define ITA_BB_END    };
#else
#    define ITA_BB_START
#    define ITA_BB_END
#endif

ITA_BB_START

#include "ERROR.h"
#include "ITADTD.h"

//矩形结构体
typedef struct ITA_BB_RECTANGLE_INFO
{
	int nX;												//左上角x
	int nY;												//左上角y
	int nWidth;											//矩形宽
	int nHeight;										//矩形高
}ITABBRectangleInfo;

//测温相关参数结构体
typedef struct ITA_BB_MEASURE_PARAM
{
	int nKf;											//KF,放大100倍，默认100
	int nB0;											//B0,放大10倍，默认0
	int nMaxMeasuLimit;									//最大温度，放大10倍.默认650
	int nMinMeasuLimit;									//最小温度，放大10倍，默认-150
	int nDistance;										//测温距离，放大10倍，默认2
	int nHumidity;										//湿度，放大100倍，默认70
	int nEmissivity;									//发射率，放大100倍，默认100
	int nDistance_a0;									//距离校正系数(定点化1000000000倍，默认0)
	int nDistance_a1;									//距离校正系数(定点化1000000000倍，默认0)
	int nDistance_a2;									//距离校正系数(定点化1000000倍，默认0)
	int nDistance_a3;									//距离校正系数(定点化1000000倍，默认0)
	int nDistance_a4;									//距离校正系数(定点化1000倍，默认0)
	int nDistance_a5;									//距离校正系数(定点化1000倍，默认0)
	float fEnvironmentT;								//环境温度
	float fBlackBodyT;									//黑体温度
	float fManualAdjustT;								//用户手动调节温度
	bool bIfDistanceCorrection;							//距离修正开关
	bool bIfEmissCorrection;							//发射率修正开关
	ITABBRectangleInfo BlackBodyRectArea;
}ITABBMeasureParam;

/**
* @brief:	初始化一个ITA黑体测温模型实例。
* @param:	int w  Y16宽。
* @param:	int h  Y16高。
* @param:	short *pusCurve  曲线数据地址。
* @param:	int curveLength  曲线数据长度。
* @return:	成功，返回实例句柄。失败，返回NULL。
**/
ITA_API HANDLE_T ITA_BBInit(int w, int h, short *pusCurve, int curveLength);

/**
* @brief:	释放一个ITA黑体测温模型实例资源。
* @param:	HANDLE_T instance	ITA黑体测温模型实例。
* @return:	无。
**/
ITA_API void ITA_BBUninit(HANDLE_T instance);

/**
* @brief:	设置测温参数。
* @param:	HANDLE_T instance           ITA黑体测温模型实例。
* @param:	ITABBMeasureParam params    测温参数。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_BBSetParams(HANDLE_T instance, ITABBMeasureParam params);

/**
* @brief:	查询测温参数信息。
* @param:	HANDLE_T instance           ITA黑体测温模型实例。
* @param:	ITABBMeasureParam *params   输出测温参数信息。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_BBGetParams(HANDLE_T instance, ITABBMeasureParam *params);

/**
* @brief:	更新曲线数据。
* @param:	HANDLE_T instance			ITA黑体测温模型实例。
* @param:	short *pusCurve 	        曲线数据地址。
* @param:	int curveLength				曲线数据长度。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_BBSetCurve(HANDLE_T instance, short *pusCurve, int curveLength);

/**
* @brief:	计算点温。
* @param:	HANDLE_T instance           ITA黑体测温模型实例。
* @param:	short y16value				输入Y16。
* @param:	float *temp					输出温度。
* @param:	short *y16Image				输入参数，当前帧Y16矩阵地址。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_BBCalcTemp(HANDLE_T instance, short y16value, short *y16Image, float *temp);

/**
* @brief:	根据温度反算Y16。
* @param:	HANDLE_T instance			ITA黑体测温模型实例。
* @param:	float temp				    输入温度。
* @param:	short *y16value	            输出y16值。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_BBCalcY16(HANDLE_T instance, float temp, short *y16value);

ITA_BB_END

#endif // !ITA_BB_H
