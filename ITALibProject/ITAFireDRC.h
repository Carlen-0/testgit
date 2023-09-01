/*************************************************************
Copyright (C), 2022--2023, Wuhan Guide Sensmart Tech Co., Ltd
File name   : ITAFireDRC.h
Author      : Guide Sensmart RD. wangyan
Version     : 1.0
Date        : 2022/10/29
Description : ITA SDK interface definition for fire imaging.
*************************************************************/
#ifndef ITA_FIRE_DRC_H
#define ITA_FIRE_DRC_H

#ifdef __cplusplus
#    define ITA_FIRE_DRC_START  extern "C" {
#    define ITA_FIRE_DRC_END    };
#else
#    define ITA_FIRE_DRC_START
#    define ITA_FIRE_DRC_END
#endif

ITA_FIRE_DRC_START

#include "ERROR.h"
#include "ITADTD.h"

typedef enum ITAFirePaletteType
{
	ITA_FIRE_WHITEHOT = 0,	//白热
	ITA_FIRE_BLACKHOT,		//黑热
	ITA_FIRE_IRONRED,		//铁红
	ITA_FIRE_ARCTIC,		//北极
	ITA_FIRE_HOTIRON,		//热铁
	ITA_FIRE_RAINBOW1,		//彩虹1
	ITA_FIRE_FULGURITE,		//熔岩
	ITA_FIRE_TINT1,			//描红1
	ITA_FIRE_TINT3,			//描红3（仅热检测模式使用）
	ITA_FIRE_FIRE			//消防伪彩（默认消防模式使用，不可选）
}ITA_FIRE_PALETTE_TYPE;

typedef struct ITA_Y16_TEMP_MAP{
	int y16minus20;
	int y16_0;				
	int y16_100;
	int y16_125;
	int y16_150;
	int y16_250;
	int y16_300;
	int y16_400;
	int y16_450;
	int y16_500;
	int y16_650;
	int y16_2000;
}ITAY16TempMap;

//消防成像调光参数结构体
typedef struct ITA_FIRE_DRC_PARAM
{
	int mixContrastExp;		//混合调光中线性调光对比度，默认值128，范围0-255。
	int mixBrightExp;		//混合调光中线性调光亮度，默认值255，范围0-512。
	int restrainRangeThre;	//混合调光中线性调光均匀面抑制，默认值128，范围0-256。
	int mixRange;			//混合调光中直方图调光映射范围，默认值255，范围0-512。
	int midValue;			//混合调光中直方图调光映射中值，默认值128，范围0-255。
	int platThresh;			//混合调光中直方图调光平台阈值，默认值200，范围1-1000。
	int upDiscardRatio;		//混合调光中上抛点比例（千分之一），默认值1，范围0-100。
	int downDiscardRatio;	//混合调光中下抛点比例（千分之一），默认值1，范围1-100。
	int enThr;				//细节增强阈值，默认值20，范围0-50。
	int enCoef;				//细节增强系数，默认值32，范围0-128。
	int filterStd;			//细节增强中滤波系数标准差，默认值10，范围1-50。
	ITA_FIRE_PALETTE_TYPE pseudoNo;			//伪彩号，目前支持10种。
	ITA_PIXEL_FORMAT imgType;   //输出图像格式选择
}ITAFireDRCParam;

typedef enum ITA_FIRE_WORKMODE
{
	BASE_FIRE_MODE = 0,
	BLACK_FIRE_EXTINGUISHING_MODE = 1,
	FIRE_DANGER_MODE = 2,
	RESCUE_MODE = 3,
	HEAT_DETECT_MODE = 4,
	STANDART_FIRE_MODE = 5,
}ITAFireWorkMode;

/**
* @brief:	初始化一个ITA消防成像模型实例。
* @param:	int w  Y16宽。
* @param:	int h  Y16高。
* @param:	ITA_PIXEL_FORMAT imgFormat 图像格式,目前仅支持ITA_BGR888图像格式。
* @return:	成功，返回实例句柄。失败，返回NULL。
**/
ITA_API HANDLE_T ITA_FireDRCInit(int w, int h, ITA_PIXEL_FORMAT imgFormat);

/**
* @brief:	释放一个ITA消防成像模型实例资源。
* @param:	HANDLE_T instance	ITA消防成像模型实例。
* @return:	无。
**/
ITA_API void ITA_FireDRCUninit(HANDLE_T instance);

/**
* @brief:	设置消防成像调光参数。
* @param:	HANDLE_T instance           ITA消防成像模型实例。
* @param:	ITABBMeasureParam params    消防成像调光参数。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_FireDRCSetParams(HANDLE_T instance, ITAFireDRCParam params);

/**
* @brief:	查询消防成像调光参数信息。
* @param:	HANDLE_T instance           ITA消防成像模型实例。
* @param:	ITABBMeasureParam *params   输出消防成像调光参数信息。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_FireDRCGetParams(HANDLE_T instance, ITAFireDRCParam *params);

/**
* @brief:	正常调光模式（低测温范围使用）。输入和输出图像数据的内存均由用户申请和释放。
* @param:	HANDLE_T instance			ITA消防成像模型实例。
* @param:	short *srcData 	            输入Y16数据内存首地址。
* @param:	unsigned char* dstData	    输出图像内存首地址，内存大小由ITA_FireDRCInit中的imgFormat和图像宽高决定。
* @param:	int width					输入图像数据的宽度。
* @param:	int height					输入图像数据的高度。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_FireDRCGeneral(HANDLE_T instance, short *srcData, unsigned char* dstData, int width, int height);

/**
* @brief:	消防调光模式（分段调光）。输入和输出图像数据的内存均由用户申请和释放。
* @param:	HANDLE_T instance           ITA消防成像模型实例。
* @param:	short *srcData 	            输入Y16数据内存首地址。
* @param:	unsigned char* dstData	    输出图像内存首地址，内存大小由ITAFireDRCParam中的imgType和图像宽高决定。
* @param:	int width					输入图像数据的宽度。
* @param:	int height					输入图像数据的高度。
* @param:	int upY16					输入高温反查的Y16。
* @param:	int downY16					输入低温反查的Y16。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_FireDRCSection(HANDLE_T instance, short *srcData, unsigned char* dstData, int width, int height, int upY16, int downY16);

/**
* @brief:	计算图像Center温度对应的刻度尺位置
* @param:	HANDLE_T instance           ITA消防成像模型实例。
* @param:	int centerY16				输入中心点Y16。
* @param:	ITAY16TempMap map		输入Y16温度映射表。参数类型为：ITA_Y16_TEMP_MAP。	
* @param:	ITAFireWorkMode workMode	图像显示模式(消防、火灾、热检等)。
* @param:	float* barHeight			输出参数为高度值，表示占总刻度尺的比例。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_CalcROITemperBarHeightSimplify(HANDLE_T instance, int centerY16, ITAY16TempMap map, ITAFireWorkMode workMode, float* barHeight);

/**
* @brief:	zc17a的六种工作模式调光算法函数
* @param:	HANDLE_T instance           ITA消防成像模型实例。
* @param:	short* srcY16				输入原始Y16数据。内存由用户申请。
* @param:	unsigned char* dst			输出带伪彩的y8图像数据。内存由用户申请。
* @param:	int width					Y16图像数据的宽。
* @param:	int height			        Y16图像数据的高。
* @param:	int maxY16Temp				当前工作模式最高温对应的Y16。
* @param:	int threshY16				线性调光和自动调光分界Y16阈值。
* @param:	ITAFireWorkMode work_mode	图像显示模式(消防、火灾、热检等)。
* @param:	ITA_FIRE_PALETTE_TYPE pesudo图像伪彩号。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_DrcFireAdaptive2DifferentMode(HANDLE_T instance, short* srcY16, unsigned char* dst, int width, int height, int maxY16Temp, int threshY16, ITAFireWorkMode work_mode, ITA_FIRE_PALETTE_TYPE pesudo);


ITA_FIRE_DRC_END

#endif // !ITA_FIRE_DRC_H
