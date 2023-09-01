/*************************************************************
Copyright (C), 2021--2022, Wuhan Guide Sensmart Tech Co., Ltd
File name   : Config.h
Author      : Guide Sensmart RD. wangyan
Version     : 1.0
Date        : 2021/11/9
Description : Function macro definition.
*************************************************************/
#ifndef ITA_CONFIG_H
#define ITA_CONFIG_H

/*********************功能宏定义如下******************/
#define MULTI_THREAD_FUNCTION			//支持多线程功能，AF功能需要。
//#define PALETTE_TIMO					//版本包含TIMO产品的13条伪彩。
#define PALETTE_ALL					//版本包含所有产品26条伪彩。
//#define EMISS_CURVES_800				//发射率曲线长度16KB short
#define EMISS_CURVES_2100				//发射率曲线长度40KB short
//#define CONTINUE_FRAME_DROP			//采用连续丢帧以满足用户指定的帧率
//#define SOURCE_COPY

/*版本控制宏，每个版本可以单独发布，也支持旗舰版+定制版同时发布在一个版本中。*/
#define ULTIMATE_EDITION                //旗舰版
//#define COIN_EDITION                    //COIN机芯定制版，指定项目才能使用此宏，否则须关闭。
#define BLACKBODY_EDITION				//黑体测温产品，例如IR236X。
#define FIRE_DRC_EDITION				//消防成像调光，例如ZC17A。
#define REFRIGER_EDITION				//制冷探测器测温模型，例如ZC12A。
//#define IMAGE_FUSION_EDITION			//图像融合与标定，例如ZC12A。图像融合与标定依赖OPENCV库，当打开该宏需要引入三方库OPENCV。
//#define CALIBRATE_EDITION				//是否做校温后误差分析。客户二次校温经常不过，需要打开此宏，内部开发需要关闭此宏。
//#define SHIHUI_EDITION				//视辉客户TEC产品在快门闭合状态不允许测温。
//#define MEASURETOOL_EDITION

/*性能加速宏*/
#define COPY_ACCELERATE //拷贝加速


/*********************公共数据类型定义如下******************/
typedef enum ITAProductType
{
	ITA_256_TIMO,
	ITA_120_TIMO,
	ITA_HANDHELD_TEC, //2，手持类产品，B\C\D系列
	ITA_IPT_TEC, //3，IPT类产品，IPT\IPM\PLUG
	ITA_UNDEFINED_PRODUCT
}ITA_PRODUCT_TYPE;

//几种测温算法用到的公共宏
//发射率曲线长度
#define	nEmissCurveLen			40960
#define gdmin(a, b)				((a)<(b)?(a):(b))
#define gdmax(a, b)				((a)>(b)?(a):(b))
#define	MAXSHORT				16384
#define	MINSHORT				-16383

#endif // !ITA_CONFIG_H
