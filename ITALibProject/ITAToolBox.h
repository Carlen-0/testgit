/*************************************************************
Copyright (C), 2022--2023, Wuhan Guide Sensmart Tech Co., Ltd
File name   : ITAToolBox.h
Author      : Guide Sensmart RD. wangyan
Version     : 1.0
Date        : 2023/2/13
Description : ITA ToolBox.
*************************************************************/
#ifndef ITA_TOOLBOX_H
#define ITA_TOOLBOX_H

#ifdef __cplusplus
#    define ITA_TOOLBOX_START  extern "C" {
#    define ITA_TOOLBOX_END    };
#else
#    define ITA_TOOLBOX_START
#    define ITA_TOOLBOX_END
#endif

ITA_TOOLBOX_START

#include "ERROR.h"
#include "ITADTD.h"

/* 面一致性类型 */
typedef enum ITA_IFCONSISTENCY_TYPE
{
	ITA_IFCONSISTENCY_NQ,								//不合格
	ITA_IFCONSISTENCY_QUALIFIED,						//合格
}ITAIfConsistencyType;

/* 距离修正系数计算模式 */
typedef enum ITA_DISTANCECOFF_MODE
{
	ITA_DISTANCECOFF_6GROUP,							//6组距离系数
	ITA_DISTANCECOFF_9GROUP,							//9组距离系数
}ITADistanceCoffMode;

/* 添加坏线模式 */
typedef enum ITA_ADDBADLINE_MODE
{
	ITA_ADDBADLINE_ROW,									//添加坏行
	ITA_ADDBADLINE_COLUMN,								//添加坏列
}ITAAddBadLineMode;

/* 温漂信息结构体 */
typedef struct ITA_DRIFT_INFO
{
	ITA_SHUTTER_STATUS   shutterStatus;				  //快门状态
	short y16;										  //Y16
	float realTimeShutterTemp;						  //实时快门温
	float realTimeLenTemp;							  //实时镜筒温
}ITADriftInfo;

/**
* @brief:	自动调节响应率。
* @param:	HANDLE_T instance									ITA工具箱实例。
* @param:	short lowY16										低温黑体中心点Y16
* @param:	short highY16										高温黑体中心点Y16
* @param:	int currentINT										当前探测器INT值
* @param:	int *pOutINT										输出调整之后的INT值
* @param:	short responseRate									当前档位和测温范围下所要求的响应率值
* @param:	short precision										当前档位和测温范围下所能接受的响应率误差
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_AutoResponseRate(HANDLE_T instance, short lowY16, short highY16, int currentINT, int *pOutINT, short responseRate, short precision);

/**
* @brief:	做K。
* @param:	HANDLE_T instance									ITA工具箱实例。
* @param:	short *pLowBase										低温本底，即对低温黑体时采集的一帧X16数据
* @param:	short *pHighBase									高温本底，即对高温黑体时采集的一帧X16数据
* @param:	int width											图像宽
* @param:	int height											图像高
* @param:	unsigned short *pKMatrix							输出的K矩阵
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_AutoGetKMatirx(HANDLE_T instance, short *pLowBase, short *pHighBase, int width, int height, unsigned short *pKMatrix);

/**
* @brief:	自动标记坏点。
* @param:	HANDLE_T instance									ITA工具箱实例。
* @param:	unsigned short *pKMatrix							输入K矩阵，输出标记后的K矩阵
* @param:	int width											图像宽
* @param:	int height											图像高
* @param:	unsigned short thresh								坏点阈值，一般设为50
* @param:	int *pBadPointInfo									输出坏点、坏列、坏行、坏斑信息，数组长度为51
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_AutoCorrectBadPoint(HANDLE_T instance, unsigned short *pKMatrix, int width, int height, unsigned short thresh, int *pBadPointInfo);

/**
* @brief:	添加坏点。
* @param:	HANDLE_T instance									ITA工具箱实例。
* @param:	unsigned short *pKMatrix							输入K矩阵，输出添加坏点后的K矩阵
* @param:	int x												x坐标
* @param:	int y												y坐标
* @param:	int width											图像宽
* @param:	int height											图像高
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_AddBadPoint(HANDLE_T instance, unsigned short *pKMatrix, int x, int y, int width, int height);

/**
* @brief:	添加坏线。
* @param:	HANDLE_T instance									ITA工具箱实例。
* @param:	unsigned short *pKMatrix							输入K矩阵，输出添加坏线后的K矩阵
* @param:	int lineIndex										指定的行或列
* @param:	int width											图像宽
* @param:	int height											图像高
* @param:	ITAAddBadLineMode mode								添加坏线的模式
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_AddBadLine(HANDLE_T instance, unsigned short *pKMatrix, int lineIndex, int width, int height, ITAAddBadLineMode mode);

/**
* @brief:	判断面一致性。
* @param:	HANDLE_T instance									ITA工具箱实例。
* @param:	float centerPointTemp								面阵中心的实测温度
* @param:	float *pOtherPointTemp								面阵中其他温度点的温度
* @param:	int otherPointNum									其他温度点的个数
* @param:	ITAIfConsistencyType *pResult						输出面一致性是否合格
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_IfConsistency(HANDLE_T instance, float centerPointTemp, float *pOtherPointTemp, int otherPointNum, ITAIfConsistencyType *pResult);

/**
* @brief:	生成测温曲线。
* @param:	HANDLE_T instance									ITA工具箱实例。
* @param:	double *pTempMatrix									黑体中心的温度矩阵
* @param:	short *pY16											温度矩阵对应Y16
* @param:	int arrLength										温度矩阵、Y16的数组长度
* @param:	int maxTemp											最大温度
* @param:	short *pCurve										输出的测温曲线
* @param:	int curveLength										曲线数组长度，一般长度为16384
* @param:	int *pValidCurveLength								输出实际生成的曲线长度
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_GenerateCurve(HANDLE_T instance, double *pTempMatrix, short *pY16, int arrLength, int maxTemp, short *pCurve, int curveLength,
	int *pValidCurveLength);

/**
* @brief:	自动计算温漂修正参数。
* @param:	HANDLE_T instance									ITA工具箱实例。
* @param:	ITADriftInfo *pDriftInfo							录制的温漂信息
* @param:	int driftInfoLength									温漂信息数组长度
* @param:	int *pShutterDrift									输出的快门温漂
* @param:	int *pLenDrift										输出的镜筒温漂
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_AutoGetDrift(HANDLE_T instance, ITADriftInfo *pDriftInfo, int driftInfoLength, int *pShutterDrift, int *pLenDrift);

/**
* @brief:	自动计算距离修正参数。
* @param:	HANDLE_T instance									ITA工具箱实例。
* @param:	float *pTempMatrix									不同距离下的温度矩阵，数组长度=黑体个数x距离个数
* @param:	double *pDistance									距离数组，数组长度为numOfDistance
* @param:	int numOfBlackBody									黑体个数
* @param:	int numOfDistance									距离个数
* @param:	int standDistance									标准距离，取值范围：>0; 一般有1、3、5、7、9米的距离，取对应数值表示以该距离为标准距离
* @param:	int *pDistanceCoff									输出的距离修正系数数组，数组长度为9
* @param:	ITADistanceCoffMode mode							距离修正系数计算模式，ITA_DISTANCECOFF_6GROUP表示输出的距离修正系数有效个数为6，
*																ITA_DISTANCECOFF_9GROUP表示输出的距离修正系数有效个数为9
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_AutoMeasureDistanceCoff(HANDLE_T instance, float *pTempMatrix, double *pDistance, int numOfBlackBody, int numOfDistance, 
	int standDistance, int *pDistanceCoff, ITADistanceCoffMode mode);

/**
* @brief:	自动计算环温修正参数。
* @param:	HANDLE_T instance									ITA工具箱实例。
* @param:	double *pStandardAmbientTemp						标准环温下的温度复核数据
* @param:	float standardAmbientTemp							标准环境温度
* @param:	double *pCurrentAmbientTemp							当前环温下的温度实测数据
* @param:	float currentAmbientTemp							当前环境温度
* @param:	int tempMatrixSize									温度复核数据、温度实测数据的数组长度
* @param:	int *pK3											输出参数，环温修正系数(定点化10000倍，默认0)
* @param:	int *pB2											输出参数，环温修正偏移量(定点化10000倍，默认0)
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_AutoMeasureAmbientCoff(HANDLE_T instance, double *pStandardAmbientTemp, float standardAmbientTemp, 
	double *pCurrentAmbientTemp, float currentAmbientTemp, int tempMatrixSize, int *pK3, int *pB2);

/**
* @brief:	自动复核温度
*			目标温度大于0°时，按照±2,±2%取大值的标准；目标温度小于0°时，按照±3的标准
* @param:	HANDLE_T instance									ITA工具箱实例。
* @param:	float *pStandTemp									目标温度
* @param:	float *pMeasureTemp									实测温度
* @param:	int arrLength										目标温度数组、实测温度数组长度
* @param:	int *pNonConformTempIndex							输出参数，实测温度中不符合复核标准的索引值，长度为arrLength；
* @param:	int *pNonConformTempNum								输出参数，实测温度中不符合复核标准的数量
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_AutoCheckTemperature(HANDLE_T instance, float *pStandTemp, float *pMeasureTemp, int arrLength, 
	int *pNonConformTempIndex, int *pNonConformTempNum);

ITA_TOOLBOX_END

#endif // !ITA_TOOLBOX_H