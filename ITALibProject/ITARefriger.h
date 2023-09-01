/*************************************************************
Copyright (C), 2022--2023, Wuhan Guide Sensmart Tech Co., Ltd
File name   : ITARefriger.h
Author      : Guide Sensmart RD. wangyan
Version     : 1.0
Date        : 2022/10/29
Description : Temperature measurement model of refrigeration detector.
*************************************************************/
#ifndef ITA_REFRIGER_H
#define ITA_REFRIGER_H

#ifdef __cplusplus
#    define ITA_REFRIGER_START  extern "C" {
#    define ITA_REFRIGER_END    };
#else
#    define ITA_REFRIGER_START
#    define ITA_REFRIGER_END
#endif

ITA_REFRIGER_START

#include "ERROR.h"
#include "ITADTD.h"

//测温相关参数结构体
typedef struct ITA_REFRIGER_MEASURE_PARAM
{
	short sY16Offset;										//Y16偏移量(默认0)
	int nKF;												//查曲线时Y16的缩放量(定点化100倍,默认值为100)
	int nB1;												//查曲线时Y16的偏移量(定点化100倍，默认0)
	int nDistance_a0;										//距离校正系数(定点化10000000000倍，默认0)
	int nDistance_a1;										//距离校正系数(定点化1000000000倍，默认0)
	int nDistance_a2;										//距离校正系数(定点化10000000倍，默认0)
	int nDistance_a3;										//距离校正系数(定点化100000倍，默认0)
	int nDistance_a4;										//距离校正系数(定点化100000倍，默认0)
	int nDistance_a5;										//距离校正系数(定点化10000倍，默认0)
	int nDistance_a6;										//距离校正系数(定点化1000倍，默认0)
	int nDistance_a7;										//距离校正系数(定点化100倍，默认0)
	int nDistance_a8;										//距离校正系数(定点化100倍，默认0)
	int nK1;												//快门修正系数(定点化100倍，默认0）
	int nK2;												//镜筒温漂系数(定点化100倍，默认0)
	int nK3;												//环温修正系数(定点化10000倍，默认0)
	int nB2;												//环温修正偏移量(定点化10000倍，默认0)
	int nKFOffset;											//自动校温KF偏移量，置零后恢复出厂校温设置，默认0
	int nB1Offset;											//自动校温B1偏移量，置零后恢复出厂校温设置，默认0
	int nGear;												//测温档位, 0:低温档, 1:常温挡
	int nHumidity;											//湿度(定点化100倍，默认60)
	int nAtmosphereTransmittance;							//大气透过率(范围0~100，默认100)
	float nCurrentB;										//当前范围的默认本底温度，默认20.0
	int nBCurveIndex;										//本底反查曲线对应的Y16值
	float fEmiss;											//发射率(0.01-1.0,默认1.0)
	float fDistance;										//测温距离，默认3米
	float fReflectT;										//反射温度（低温档默认3，常温档默认23）
	float fAmbient;											//环境温度（取开机快门温）
	float fAtmosphereTemperature;							//大气温度
	float fWindowTransmittance;								//窗口透过率(范围0~1，默认1)
	float fWindowTemperature;								//窗口温度（低温档默认3，常温档默认23）
	float fRealTimeLensTemp;								//实时镜筒温
	float fCurrentLensTemp;									//最近一次打快门时的镜筒温
	float fOriginLensTemp;									//开机镜筒温
	float fRealShutterTemp;									//实时快门温度
	float fOriShutterTemp;									//开机快门温度
	bool bHumidityCorrection;								//湿度修正开关，默认打开
	bool bLensCorrection;									//镜筒修正开关，默认打开
	bool bShutterCorrection;								//快门修正开关，默认打开
	bool bEmissCorrection;									//发射率修正开关，默认打开
	bool bDistanceCorrection;								//距离修正开关，默认打开
	bool bAmbientCorrection;								//环温修正开关，默认打开
	bool bB1Correction;										//B1修正开关，默认打开
	bool bAtmosphereCorrection;								//大气透过率修正开关，默认打开
	bool bWindowTransmittanceCorrection;					//窗口透过率开关，默认打开
}ITARefrigerMeasureParam;
/**
* @brief:	初始化一个ITA制冷探测器测温模型实例。
* @param:	int w  宽。
* @param:	int h  高。
* @param:	short *pusCurve  曲线数据地址。
* @param:	int curveLength  曲线数据长度。
* @return:	成功，返回实例句柄。失败，返回NULL。
**/
ITA_API HANDLE_T ITA_RefrigerInit(int w, int h, short *pusCurve, int curveLength);

/**
* @brief:	释放一个ITA制冷探测器测温模型实例资源。
* @param:	HANDLE_T instance	ITA黑体测温模型实例。
* @return:	无。
**/
ITA_API void ITA_RefrigerUninit(HANDLE_T instance);

/**
* @brief:	设置测温参数。
* @param:	HANDLE_T instance           ITA制冷探测器测温模型实例。
* @param:	ITARefrigerMeasureParam params    测温参数。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_RefrigerSetParams(HANDLE_T instance, ITARefrigerMeasureParam params);

/**
* @brief:	查询测温参数信息。
* @param:	HANDLE_T instance           ITA制冷探测器测温模型实例。
* @param:	ITARefrigerMeasureParam *params   输出测温参数信息。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_RefrigerGetParams(HANDLE_T instance, ITARefrigerMeasureParam *params);

/**
* @brief:	更新曲线数据。
* @param:	HANDLE_T instance			ITA制冷探测器测温模型实例。
* @param:	short *pusCurve 	        曲线数据地址。
* @param:	int curveLength				曲线数据长度。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_RefrigerSetCurve(HANDLE_T instance, short *pusCurve, int curveLength);

/**
* @brief:	计算点温。
* @param:	HANDLE_T instance           ITA制冷探测器测温模型实例。
* @param:	short y16value				输入Y16。
* @param:	float *temp					输出温度。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_RefrigerCalcTemp(HANDLE_T instance, short y16value, float *temp);

/**
* @brief:	计算温度矩阵。
* @param:	HANDLE_T instance		ITA制冷探测器测温模型实例。
* @param:	short* y16Array			Y16矩阵。
* @param:	y16W,y16H				Y16矩阵分辨率。
* @param:	x,y,w,h					目标矩形区域。注意不要越界，否则会报错。
* @param:	float distance			目标距离，单位：米。当前计算过程有效，完成后将恢复计算前的值。
* @param:	float emiss				发射率。当前计算过程有效，完成后将恢复计算前的值。
* @param:	ITA_MATRIX_TYPE type	矩阵类型
* @param:	float *tempMatrix		输出目标区域的温度矩阵。
* @param:	int matrixSize			矩阵缓存大小，不能小于目标区域w*h。单位float。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_RefrigerCalcTempMatrix(HANDLE_T instance, short* y16Array, int y16W, int y16H,
	int x, int y, int w, int h, float distance, float emiss, ITA_MATRIX_TYPE type, float *tempMatrix, int matrixSize);

/**
* @brief:	根据温度反算Y16。
* @param:	HANDLE_T instance			ITA制冷探测器测温模型实例。
* @param:	float temp				    输入温度。
* @param:	short *y16value	            输出y16值。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_RefrigerCalcY16(HANDLE_T instance, float temp, short *y16value);

/**
* @brief:	根据温度矩阵反查y16矩阵。
* @param:	HANDLE_T instance		ITA制冷探测器测温模型实例。
* @param:	float *tempMatrix		输入温度矩阵。
* @param:	int w					宽
* @param:	int h					高
* @param:	short *y16Matrix		输出Y16矩阵。
* @param:	int matrixSize			矩阵缓存大小
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_RefrigerCalcY16Matrix(HANDLE_T instance, float *tempMatrix, int w, int h, short *y16Matrix, int matrixSize);

/**
* @brief:	判断是否切换测温范围。
* @param:	HANDLE_T instance		ITA制冷探测器测温模型实例。
* @param:	short *pSrc				源Y16图像。内存由用户管理。
* @param:	width, height			源Y16图像分辨率。
* @param:	ITA_RANGE range			当前测温范围。
* @param:	areaTh1, areaTh2		阈值条件。范围请分别参考73/10000和9985/10000。
* @param:	low2high, high2low 		阈值条件。范围请分别参考140和130。
* @param:	int *isChange 		    输出参数，1：切换测温范围，0：保持不变。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_RefrigerIsChangeRange(HANDLE_T instance, short *pSrc, int width, int height, ITA_RANGE range,
	float areaTh1, float areaTh2, int low2high, int high2low, int *isChange);

/**
* @brief:	调试器，设置调试参数。
* @param:	HANDLE_T instance			ITA实例。
* @param:	ITA_DEBUG_TYPE type		    参数类型。
* @param:	void *param					参数值，传地址。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_RefrigerDebugger(HANDLE_T instance, ITA_DEBUG_TYPE type, void* param);

/**
* @brief:	设置K矩阵，固定本底矩阵
* @param:	HANDLE_T instance			ITA实例。
* @param:	short* stationaryB			固定本底矩阵，内存由用户申请。
* @param:	short* kMatrix				K矩阵，内存由用户申请。
* @param:	int length					矩阵长度,单位是short。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_RefrigerSetMatrixData(HANDLE_T instance, short* stationaryB, short* kMatrix, int length);

/*
* @brief:	本底Y16矩阵转换。
* @param:	HANDLE_T instance			ITA实例。
* @param:	short* srcY16			    原始Y16矩阵。
* @param:	short* dstY16				转化后的Y16矩阵。
* @param:	short averageB				本底均值。
* @param:	short* realB				实时本底矩阵。
* @param:	short* stationaryB			固定本底矩阵，内存由用户申请。
* @param:	short* kMatrix				K矩阵，内存由用户申请。
* @param:	int width					图像宽。
* @param:	int height					图像高。
* @param:	bool mode					true：由实时本底转固定本底；false: 否则相反。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
*/
ITA_API ITA_RESULT ITA_RefrigerY16MatrixConvert(HANDLE_T instance, short* srcY16, short* dstY16, short averageB, short* realB, short* stationaryB, short* kMatrix, int width,int height,bool mode);

/*
* @brief:	本底Y16单点转换。
* @param:	HANDLE_T instance			ITA实例。
* @param:	short srcY16			    原始Y16。
* @param:	short* dstY16				转化后的Y16。
* @param:	short stationaryB			固定本底值。
* @param:	short k						K值。
* @param:	short averageB				实时本底均值。
* @param:	short realB					实时本底值。
* @param:	bool mode					true：由实时本底转固定本底；false: 固定本底转实时本底。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
*/
ITA_API ITA_RESULT ITA_RefrigerY16SingleConvert(HANDLE_T instance, short srcY16, short* dstY16, short stationaryB,short averageB, short realB,short k, bool mode);

ITA_REFRIGER_END

#endif // !ITA_REFRIGER_H
