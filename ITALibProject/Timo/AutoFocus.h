/*************************************************************
Copyright (C), 2021--2022, Wuhan Guide Sensmart Tech Co., Ltd
File name   : AutoFocus.h
Author      : Guide Sensmart RD. wangyan
Version     : 1.0
Date        : 2021/12/9
Description : ITA SDK AutoFocus.
*************************************************************/
#ifndef ITA_AUTOFOCUS_H
#define ITA_AUTOFOCUS_H

#include "../Module/GuideLog.h"
#include "../ITADTD.h"

typedef unsigned char           NI_U8;
typedef unsigned short          NI_U16;
typedef unsigned int            NI_U32;

typedef signed char             NI_S8;
typedef short                   NI_S16;
typedef int                     NI_S32;

typedef unsigned long long      NI_U64;
typedef long long               NI_S64;

#define NAME_FILE_AUTOFOCUS_C "AutoFocus.cpp"
#define NAME_FILE_EEPROM_C "eeprom.c"
//! -----------------------------------------------------------------------------------------------------
//自动调焦函数状态机
#define STATE_AUTOFOCUS_GET_NEAR            255 //调近焦状态
#define STATE_AUTOFOCUS_GET_NEAR_SWITCH     254 //从近焦到远焦切换中间状态
#define STATE_AUTOFOCUS_GET_NEAR_JUDGE      253 //近焦将停判断，如果满足条件会停止或者继续回到调近焦状态
#define STATE_AUTOFOCUS_GET_NEAR_WILL_STOP  252 //近焦将停状态，用于调过峰值回调
#define STATE_AUTOFOCUS_GET_NEAR_EDGE       251 //近焦将停状态，用于调过峰值回调
#define STATE_AUTOFOCUS_GET_FAR             250 //调远焦状态
#define STATE_AUTOFOCUS_GET_FAR_SWITCH      249 //从远焦到近焦切换中间状态
#define STATE_AUTOFOCUS_GET_FAR_JUDGE       248 //远焦将停判断，如果满足条件会停止或者继续回到调远焦状态
#define STATE_AUTOFOCUS_GET_FAR_WILL_STOP   247 //远焦将停状态，用于调过峰值回调
#define STATE_AUTOFOCUS_GET_FAR_EDGE        246 //远焦将停状态，用于调过峰值回调
#define STATE_AUTOFOCUS_GET_STOP            0   //退出状态

//! 自动调焦参数默认值定义------------------------------------------------------------------------------------
//自动调焦参数设置
#define MAX_AUTOFOCUS_TIME_CNT              300                                             //ms
#define MIN_AUTOFOCUS_TIME_CNT              43//41                                              //ms
#define RUN_AUTOFOCUS_TIME_CNT              10//20                                              //ms
#define WAIT_AUTOFOCUS_TIME_CNT             MIN_AUTOFOCUS_TIME_CNT-RUN_AUTOFOCUS_TIME_CNT   //ms
#define DIFF_MIN_AUTOFOCUS_SHARP_VALUE      10000

//自动调焦函数回调设置
#define MAX_AUTOFOCUS_PARA_CAL_CNT          10  //校准模式下最大寻位次数
#define CNT_AUTOFOCUS_GET_NEAR_WILL_STOP    3   //近焦将停步数
#define CNT_AUTOFOCUS_GET_FAR_WILL_STOP     4   //远焦将停步数

#define FREQ_PWM            125000                      //PWM频率定义，125KHz
#define PWM_DURATION_UP     25//1*(1000000000/FREQ_PWM)/4   //占空比定义，25%
#define PWM_DURATION_DOWN   75//3*(1000000000/FREQ_PWM)/4   //占空比定义，75%

//  模组厂烧录数据区域定义
#define IIC_EEPROM_REGADDR_BASE                                 0x0000                                                      // EEPROM地址基地址
#define IIC_EEPROM_MOD_DATALEN_MAX                              1024                                                        // EEPROM数据长度限制
#define IIC_EEPROM_REGADDR_DATA                                 IIC_EEPROM_REGADDR_BASE + 0x0000                            // EEPROM出厂数据正文区域偏移地址
#define IIC_EEPROM_REGADDR_LEN                                  IIC_EEPROM_REGADDR_BASE + IIC_EEPROM_MOD_DATALEN_MAX        // EEPROM出厂数据属性区域偏移地址
// 0000:length[7:0]
// 0001:length[15:8]

//  模组自动调焦参数烧录数据区域定义
#define IIC_EEPROM_AF_DATALEN_MAX                               16                                                          // EEPROM数据长度限制
#define IIC_EEPROM_REGADDR_AFDATA                               IIC_EEPROM_REGADDR_BASE + 2048                              // EEPROM AF数据正文区域偏移地址
#define IIC_EEPROM_REGADDR_AFDATA_LEN                           IIC_EEPROM_REGADDR_BASE + 2048 + IIC_EEPROM_AF_DATALEN_MAX  // EEPROM AF数据属性区域偏移地址
#define IIC_EEPROM_REGADDR_MAX_AUTOFOCUS_TIME_CNT               0                                                           // EEPROM short 最大调节时间偏移地址
#define IIC_EEPROM_REGADDR_MIN_AUTOFOCUS_TIME_CNT               2                                                           // EEPROM char  小步调节时间偏移地址
#define IIC_EEPROM_REGADDR_RUN_AUTOFOCUS_TIME_CNT               3                                                           // EEPROM char  小布调节时间内驱动时间偏移地址
#define IIC_EEPROM_REGADDR_DIFF_MIN_AUTOFOCUS_SHARP_VALUE       4                                                           // EEPROM short 清晰函数最小差值偏移地址
#define IIC_EEPROM_REGADDR_CNT_AUTOFOCUS_GET_NEAR_WILL_STOP     6                                                           // EEPROM char  近焦调节终点回调步数偏移地址
#define IIC_EEPROM_REGADDR_CNT_AUTOFOCUS_GET_FAR_WILL_STOP      7

typedef struct AUTOFOCUSPARA
{
	NI_U16 autofocus_time_cnt;              //调焦等效运行时间
	NI_U16 max_autofocus_time_cnt;         //整个行程最大运行时间
	NI_U8  min_autofocus_time_cnt;         //调焦过程最小运行时间
	NI_U8  run_autofocus_time_cnt;         //调焦过程最小运行时间内实际运行时间
	NI_U8  wait_autofocus_time_cnt;        //调焦过程最小运行时间内实际等待时间
	NI_U16 autofocus_time_cnt_at_af_max;    //调焦中，清晰值最大值出现的运行时间

	NI_U32 autofocus_sharp_value;           //当前帧清晰值
	NI_U32 autofocus_sharp_value_pre0;      //之前一帧清晰值
	NI_U32 autofocus_sharp_value_pre1;      //之前二帧清晰值
	NI_U32 autofocus_sharp_value_max;       //上述3个清晰值的最大值
	NI_U16 diff_min_autofocus_sharp_value; //允许的最小清晰值差值
	NI_U8  cnt_autofocus_get_near_will_stop;//调近焦将停时的步数
	NI_U8  cnt_autofocus_get_far_will_stop;//调远焦将停时的步数

	NI_U8  autofocus_engage;               //调焦运行标记
	NI_U8  autofocus_fail;                 //调焦失败标记
	NI_U8  autofocus_sharp_value_stable;   //清晰值稳定标记
	NI_U8  autofocus_getparastopsucess;    //调焦将停时的步数获取成功标记
}AUTOFOCUSPARA_T;

class AutoFocus
{
public:
	AutoFocus();
	~AutoFocus();
	//TODO 需要增加参数自动获取
	//=================================================================
	//  调用方式： autofocusctl.af_auto(0) 正常调焦,起点为近焦
	//            autofocusctl.af_auto(1) 正常调焦,起点为远焦
	//            autofocusctl.af_auto(2) 校准参数,起点为近焦
	//            autofocusctl.af_auto(3) 校准参数,起点为远焦
	//=================================================================
	ITA_RESULT AFControl(ITA_BC_TYPE type, void *param);
	ITA_RESULT setRegister(ITARegistry *registry, int cameraID);
	void setLogger(GuideLog *logger);
	ITA_RESULT refreshFrame(short* y16Array, int w, int h);
	ITA_RESULT addInterceptor(PostHandle postHandle);
private:
#ifdef MULTI_THREAD_FUNCTION
	static void AFRunningFunc(void* param);
#endif // MULTI_THREAD_FUNCTION
	void AF_GetStepNear(int step_time);                             //近焦函数，用于对准近处部分，镜头往外推
	void AF_GetStepFar(int step_time);                              //远焦函数，用于对准远处部分，镜头往回缩
	void AF_GetNear();
	void AF_GetFar();
	//=================================================================
	//  调用方式： autofocusctl.af_auto(0) 正常调焦,起点为近焦
	//            autofocusctl.af_auto(1) 正常调焦,起点为远焦
	//            autofocusctl.af_auto(2) 校准参数,起点为近焦
	//            autofocusctl.af_auto(3) 校准参数,起点为远焦
	//=================================================================
	void AF_Auto(NI_U8 para_stop_cal_mode);                         //开环自动调焦函数，调用结构体方式为 autofocusctl.af_auto()
	void AF_Auto_Simple(NI_U8 para_stop_cal_mode);                  //开环自动调焦函数，调用结构体方式为 autofocusctl.af_auto() 简易版
	void AF_Init();
	void AF_Para_Init();                                            //自动调焦参数初始化
	NI_S32 AF_GetTotalValue();                                      //图像锐度值计算，本函数为ASIC硬件提供
	void AF_GetParaStop(NI_U8 current_loop, NI_U8 current_status);  //AF停止步数参数获取
	void AF_VcmInit();
	int  AF_GetMax();
	void AF_VCM_Down();
	void AF_VcmSet();
	void AF_VcmSet1();

	void PWM_Stop();            //PWM控制：停止
	void PWM_UpEn();            //PWM控制：调远焦
	void PWM_DownEn();          //PWM控制：调近焦
	void PWM_Drive(int dutyration, int engage);	//PWM控制：调节远焦近焦驱动
	void PWM_Init();            //PWM控制：初始化
	void PWM_TotalFar();
	void PWM_TotalNear();
	void PWM_TotalFar_1();
	void PWM_TotalNear_1();
	NI_U32 eeprom_iic_rd(NI_U16 addr);
	void eeprom_iic_wr(NI_U16 addr, NI_U8 data);
	void ReadData_AFPARA();
	void WriteData_AFPARA();
	int laplacianY16();
private:
	AUTOFOCUSPARA_T autofocuspara;
	GuideLog *m_logger;
	ITARegistry *m_registry;
	int m_cameraID;
#ifdef MULTI_THREAD_FUNCTION
	HANDLE_T hThreadHandle;
	HANDLE_T m_mutex;
	bool isRunning;
	ITA_BC_TYPE actionType;	//初始状态为0
	int actionParam; //0：正常调焦,起点为近焦；1：正常调焦,起点为远焦；2：校准参数,起点为近焦；3：校准参数,起点为远焦。
	short *m_y16Array;
	int m_width, m_height;
	PostHandle postCb;
#endif // MULTI_THREAD_FUNCTION
};

#endif // !ITA_AUTOFOCUS_H

