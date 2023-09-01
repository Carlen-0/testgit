/*************************************************************
Copyright (C), 2021--2022, Wuhan Guide Sensmart Tech Co., Ltd
File name   : ITADTD.h
Author      : Guide Sensmart RD. wangyan
Version     : 1.0
Date        : 2021/11/9
Description : ITA SDK data structure definition.
*************************************************************/
#ifndef ITA_DTD_H
#define ITA_DTD_H

#include "ERROR.h"

#ifdef WINDOWS_PLATFORM
#    define ITA_API __declspec(dllexport)
#else
#    define ITA_API
#endif // WINDOWS_PLATFORM

#ifdef WINDOWS_PLATFORM
typedef void *				HANDLE_T;	//兼容32bit和64bit操作系统
#else
typedef unsigned long long	HANDLE_T;
#endif // WINDOWS_PLATFORM

typedef enum ITAMode
{
	ITA_X16,
	ITA_MCU_X16,
	ITA_Y16,
	ITA_FPGA_Y16
}ITA_MODE;

typedef enum ITARange
{
	ITA_HUMAN_BODY,     //人体测温范围
	ITA_INDUSTRY_LOW,   //工业测温常温段
	ITA_INDUSTRY_HIGH,  //工业测温高温段
	ITA_CUSTOM_RANGE1,  //TEC测温产品自定义范围1
	ITA_CUSTOM_RANGE2,  //TEC测温产品自定义范围2
	ITA_CUSTOM_RANGE3,   //TEC测温产品自定义范围3
	ITA_CUSTOM_RANGE4   //TEC测温产品自定义范围4
}ITA_RANGE;

typedef enum ITASensorValue
{
	ITA_AD_VALUE,		/* 温传AD值，int型 */
	ITA_TEMP_VALUE		/* 温传温度值，float型 */
} ITA_SENSOR_VALUE;

/*由ITA_SENSOR_VALUE决定是AD值还是温度。*/
typedef enum ITATempType
{
	ITA_SHUTTER_TEMP,	/* 快门温 */
	ITA_FPA_TEMP,		/* 焦温 */
	ITA_LENS_TEMP		/* 镜筒温 */
} ITA_TEMP_TYPE;

typedef enum ITAShutterStatus
{
	ITA_STATUS_OPEN,		//快门弹开状态。
	ITA_STATUS_CLOSE,		//快门闭合状态。
	ITA_STATUS_COMPENSATE	//快门补偿状态。
}ITA_SHUTTER_STATUS;

/*根据用户设置的图像格式、缩放倍数、旋转方式，计算的结果。
* 旋转、翻转对图像和Y16数据都生效，缩放只对图像有效。
*/
typedef struct ITA_IMG_INFO {
	int imgDataLen;			//放大后的图像数据长度，单位Byte。
	int imgW;				//旋转、放大后的分辨率
	int imgH;
	int y16Len;				//数据长度，单位short。
	int w;					//旋转后的分辨率
	int h;
}ITAImgInfo;

/**
* ITA库ISP图像处理函数的输出参数。
*/
typedef struct ITA_ISP_RES {
	unsigned char *imgDst;
	short *y16Data;
	ITAImgInfo info;
}ITAISPResult;

typedef enum ITAMatrixType
{
	ITA_NORMAL_MATRIX,	//温度矩阵
	ITA_FAST_MATRIX		//快速温度矩阵，占用CPU资源相对较少，精度相对ITA_NORMAL_MATRIX降低1%以内，误差在±0.2以内。
}ITA_MATRIX_TYPE;

typedef enum ITASwitch
{
	ITA_DISABLE,		//关闭
	ITA_ENABLE			//打开
}ITA_SWITCH;

typedef enum ITADCType
{
	ITA_DETECTOR_VALID_MODE,       //0：探测器VSYNC和HSYNC高有效；1：探测器VSYNC和HSYNC低有效。默认0。参数类型：整型。
	ITA_DETECTOR_DATA_MODE,        //0：先发16位数据的高8bits；1：先发16位数据的低8bits。默认1。参数类型：整型。
	ITA_DETECTOR_FRAME_FREQUENCY   //设置探测器帧频，范围1-30，单位fps。建议使用以下典型帧频：9、15、20、25和30。默认25。参数类型：整型。
}ITA_DC_TYPE;	//Detector control type

typedef enum ITABCType
{
	ITA_SHUTTER_NOW,			//打快门
	ITA_AUTO_SHUTTER,			//控制自动快门，默认打开。参数类型：ITA_SWITCH。
	ITA_SHUTTER_TIME,			//设置自动快门间隔时间，单位秒。默认30秒。参数类型：整型。如果用户设置自动快门时间，那么默认的自动快门策略不再有效。
	ITA_SINGLE_STEP_TIME,		//设置单步控制快门从闭合到打开整个过程的时间，单位ms。默认约400ms。参数类型：整型。
	ITA_STEP_DELAY_TIME,		//设置单步控制快门时弹开或闭合的延时，单位ms。默认100ms。例如USB连接的产品可以设置400ms。参数类型：整型。
	ITA_B_DELAY,                //设置采集本底延时，单位帧。默认5帧。用户可以根据需要增加延时。参数类型：整型。
	ITA_GET_SHUTTER_STATUS,		//获取快门状态，快门闭合时用户可以定格画面。参数类型：ITA_SHUTTER_STATUS。在X16使用快门策略的情况下用到。
	ITA_FIRST_NUC_FINISH,		//第一次NUC是否完成。参数类型：ITA_SWITCH。ITA_DISABLE，未完成；ITA_ENABLE，已完成。
	ITA_DO_NUC,					//手动NUC
	ITA_AUTO_GEARS,				//自动切焦温档位开关，默认打开。参数类型：ITA_SWITCH。手动切之前先关闭自动切换。
	ITA_CHANGE_GEAR,			//手动切焦温档位。参数类型：整形，>= 0。调用ITA_Version获取档位数量。手动切之前先关闭自动切换。
	ITA_SHUTTER_POLICY,			//控制自动快门和NUC策略的开关。参数类型：ITA_SWITCH。默认打开。通常在X16模式下打开，在Y16模式下关闭（FPGA产品例外）。
	ITA_CLOSE_SHUTTER,			//单步控制快门闭合，无参数。先disable ITA_SHUTTER_POLICY，再单步控制快门。
	ITA_OPEN_SHUTTER,			//单步控制快门弹开，无参数。先disable ITA_SHUTTER_POLICY，再单步控制快门。
	ITA_IMAGE_PROCESS,			//控制图像处理。参数类型：ITA_SWITCH。默认打开。在X16模式下不能关闭，在Y16模式下可以关闭。例如某类产品通过芯片ISP已经生成YUV，使用ITA仅测温。
	ITA_SET_FRAMERATE,			//设置帧率，1-25之间有效。原始帧率25fps。调整帧率可以调节CPU负载。参数类型：整型。
	ITA_GET_FRAMERATE,			//获取实际帧率。参数类型：整型。
	ITA_SET_CONSUME,			//设置执行一次图像处理消耗的平均时间，值可以参考ITA_ISP_CONSUME。参数类型：整型。单位：ms。在使用ITA_SET_FRAMERATE的情况下，该值影响实际帧率。
	ITA_NUC_REPEAT_CORRECTION,  //重复NUC纠错机制。参数类型：ITA_SWITCH。默认关闭。
	ITA_NUC_ANOTHER_SHUTTER,    //重复快门纠错机制。参数类型：ITA_SWITCH。默认关闭。
	ITA_NUC_REPEAT_THRESHOLD,   //判断NUC失效的阈值。参数类型：整形。默认400，适用于120模组。用户根据产品需要调整此值。
	ITA_AUTO_FOCUS,				//自动调焦。参数类型：整形。0：正常调焦,起点为近焦；1：正常调焦,起点为远焦；2：校准参数,起点为近焦；3：校准参数,起点为远焦。
	ITA_AF_NEAR,				//调最近焦
	ITA_AF_FAR,					//调最远焦
	ITA_STEP_NEAR,				//调整一个时间周期步长近焦
	ITA_STEP_FAR,				//调整一个时间周期步长远焦
	ITA_CH_DELAY,				//设置冷热机的启动时间判定
	ITA_SET_CHSTATUS			//设置冷热机的状态：1:冷机 0:热机;参数类型为unsigned char,需在ITA_Register之前调用/*分析软件端使用，设备端上报冷热机的状态给分析软件端*/
}ITA_BC_TYPE;	//Base control type

typedef enum ITAMCType
{
	ITA_SET_DISTANCE,			//有效距离范围由具体产品决定。默认1.5米。参数类型：浮点型。
	ITA_SET_EMISS,				//设置发射率(工业专用)0.01-1.0，默认0.95. 参数类型：浮点型。	
	ITA_SET_TRANSMIT,			//透过率(工业专用，0.01-1, default 1)。参数类型：浮点型。
	ITA_REFLECT_TEMP,			//反射温度(工业专用)。参数类型：浮点型。
	ITA_SET_ENVIRON,			//设置环境温度。在用户不设置的情况下，内部自动计算环温。如果用户设置，则使用设置的值。参数类型：浮点型。
	ITA_GET_ENVIRON,			//获取环温。
	ITA_GET_FPA_TEMP,			//获取焦温。
	ITA_ENVIRON_CORR,			//环温修正开关。默认关闭。参数类型：ITA_SWITCH。
	ITA_LENS_CORR,				//镜筒温漂校正开关。默认打开。参数类型：ITA_SWITCH。
	ITA_DISTANCE_COMPEN,		//距离补偿开关。默认打开。参数类型：ITA_SWITCH。
	ITA_EMISS_CORR,				//发射率校正开关。默认打开。参数类型：ITA_SWITCH。
	ITA_TRANS_CORR,				//透过率校正开关。默认关闭。参数类型：ITA_SWITCH。
	ITA_ATMOSPHERE_CORR,		//大气透过率校正开关。默认关闭。参数类型：ITA_SWITCH。
	ITA_LOW_LENS_CORR_K,		//实时常温档镜筒温漂修正系数。参数类型：浮点型。
	ITA_HIGH_LENS_CORR_K,		//实时高温档镜筒温漂修正系数。参数类型：浮点型。
	ITA_OTHER_LENS_CORR_K,		//实时其他档镜筒温漂修正系数。参数类型：浮点型。
	ITA_SHUTTER_CORR,			//快门温漂校正开关。默认关闭。参数类型：ITA_SWITCH。
	ITA_LOW_SHUTTER_CORR_K,		//常温档快门校正系数。参数类型：浮点型。
	ITA_HIGH_SHUTTER_CORR_K,	//高温档快门校正系数。参数类型：浮点型。
	ITA_OTHER_SHUTTER_CORR_K,	//其他档快门校正系数。参数类型：浮点型。
	ITA_SUB_AVGB,				//测温减去本底均值开关。默认关闭。参数类型：ITA_SWITCH。
	ITA_GET_AVGB,				//获取本底均值。参数类型：short。
	ITA_CENTRAL_TEMPER,  		//获取滤波后的中心温。参数类型：浮点型。
	ITA_SET_HUMI,				//设置湿度(工业专用)0.01-1.0,默认0.6. 参数类型：浮点型。
	ITA_HUMI_CORR,				//湿度校正开关。默认关闭。参数类型：ITA_SWITCH。
	ITA_SET_WINDOWTEMP,			//设置窗口温度(默认23).参数类型：浮点型。
	ITA_SET_ATMOSPHTEMP,		//设置大气温度(默认23).参数类型：浮点型。
	ITA_SET_ATMOSPHTRSMIT		//设置大气透过率(范围0.01-1.0,默认1.0).参数类型：浮点型。
}ITA_MC_TYPE;	//Measure control type

typedef enum ITAPixelFormat
{
	ITA_RGB888,			//默认像素格式
	ITA_BGR888,
	ITA_RGBA8888,
	ITA_RGB565,
	ITA_YUV422_YUYV,
	ITA_YUV422_UYVY,
	ITA_YUV422_Plane,
	ITA_YVU422_Plane,
	ITA_YUV420_YUYV,//NV12
	ITA_YUV420_UYVY,//NV21
	ITA_YUV420_Plane,
	ITA_YVU420_Plane
}ITA_PIXEL_FORMAT;

/**
* 13种伪彩：0白热，1熔岩，2铁红，3热铁，4医疗，5北极，6彩虹1，7彩虹2，8黑热，9人体筛查，10描红，11蓝热，12，绿热。
* 支持增加到26种，用户使用ITA_Version查询伪彩总数。
*/
typedef enum ITAPaletteType
{
	ITA_WHITE_HEAT = 0,		//默认白热
	ITA_LAVA = 1,			//熔岩
	ITA_IRON_RED = 2,		//铁红
	ITA_HOT_IRON = 3,		//热铁
	ITA_MEDICAL_TREAT = 4,	//医疗
	ITA_ARCTIC = 5,			//北极
	ITA_RAINBOW = 6,		//彩虹
	ITA_RAINBOW_EX = 7,		//彩虹2
	ITA_BLACK_HOT = 8,		//黑热
	ITA_BODY_SCREEN = 9,	//人体筛查
	ITA_RED_TINT = 10,		//描红
	ITA_BLUE_HOT = 11,		//蓝热
	ITA_GREEN_HOT = 12,		//绿热
	ITA_MEDICAL_TREAT1 = 13,//医疗1
	ITA_MEDICAL_TREAT2 = 14,//医疗2
	ITA_MEDICAL_TREAT3 = 15,//医疗3
	ITA_RAINBOW3 = 16,		//彩虹3
	ITA_RED_TINT2 = 17,		//描红2
	ITA_RED_TINT3 = 18,		//描红3
	ITA_ICE_FIRE = 19,		//冰火
	ITA_REVERSE_ICEFIRE = 20,//反冰火
	ITA_STEPS = 21,			//分层
	ITA_SEPIA = 22,			//深褐
	ITA_WINTER = 23,		//冬季
	ITA_HIGH_LIGHT = 24,	//高亮
	ITA_AMBER = 25			//琥珀
}ITA_PALETTE_TYPE;

typedef enum ITARotateType
{
	ITA_ROTATE_NONE,	//不旋转
	ITA_ROTATE_90,		//旋转90°
	ITA_ROTATE_180,
	ITA_ROTATE_270
}ITA_ROTATE_TYPE;

typedef enum ITAFlipType
{
	ITA_FLIP_NONE,			//不翻转
	ITA_FLIP_HORIZONTAL,	//水平翻转
	ITA_FLIP_VERTICAL,		//垂直翻转 
	ITA_FLIP_HOR_VER		//水平+垂直翻转
}ITA_FLIP_TYPE;

typedef enum ITAZoomType
{
	ITA_ZOOM_NEAR,
	ITA_ZOOM_BILINEAR,
	ITA_ZOOM_BILINEARFAST
}ITA_ZOOM_TYPE;

typedef enum ITAPotCoverAlgoType
{
	ITA_POTCOVER_FIX,//固定锅盖系数
	ITA_POTCOVER_VAR1,//基于开机快门温计算系数
	ITA_POTCOVER_VAR2//基于上一次快门温计算系数
}ITA_POTCOVER_ALGO_TYPE;

typedef struct ITA_POTCOVER_MODEL_DATA
{
	int width;
	int height;
	short* modelData;
}ITAPotcoverModelData;

typedef enum ITAPotCoverType
{
	ITA_POTCOVER_GET_MODEL,//获取锅盖模板数据。用户可以自己进行保存，方便后面使用。参数类型：ITA_POTCOVER_MODEL_DATA
	ITA_POTCOVER_SET_MODEL,//设置锅盖模板数据。参数类型：ITA_POTCOVER_MODEL_DATA
	ITA_POTCOVER_SET_CALMODEL,//设置计算锅盖模板开关，参数类型：ITA_SWITCH,默认值是ITA_DISABLE。
	ITA_POTCOVER_GET_CALMODEL,//获取计算锅盖模型开关，参数类型：ITA_SWITCH
	ITA_POTCOVER_SET_ALGOTYPE,//设置锅盖算法类型，参数类型：ITA_POTCOVER_ALGO_TYPE，默认值是ITA_POTCOVER_FIX。
	ITA_POTCOVER_GET_ALGOTYPE,//获取锅盖算法类型，参数类型：ITA_POTCOVER_ALGO_TYPE。
	ITA_POTCOVER_SET,//设置锅盖算法开关，参数类型：ITA_SWITCH,默认值是ITA_DISABLE。
	ITA_POTCOVER_GET//获取锅盖算法开关，参数类型：ITA_SWITCH。
}ITA_POTCOVER_OP_TYPE;

typedef enum ITAICType
{
	ITA_SET_FORMAT,			//设置成像像素格式。支持动态设置。参数类型：ITA_PIXEL_FORMAT。
	ITA_SET_PALETTE,		//设置伪彩，通过接口ITA_Version获取伪彩号范围。参数类型：ITA_PALETTE_TYPE。
	ITA_SET_SCALE,			//缩放倍数。支持图像动态实时缩放。参数类型：浮点。
	ITA_SET_CONTRAST,		//设置对比度。对比度0-511。默认255。参数类型：整形。
	ITA_SET_BRIGHTNESS,		//设置亮度。亮度0-255。默认70。参数类型：整形。
	ITA_SET_ROTATE,			//设置旋转方式，RGB和Y16数据均旋转。参数类型：ITA_ROTATE_TYPE。
	ITA_SET_FLIP,			//设置翻转，RGB和Y16数据均翻转。参数类型：ITA_FLIP_TYPE。
	ITA_SET_REVERSE,		//打开或者关闭反向处理。ITA_X16模式下默认打开。参数类型：bool。
	ITA_SET_HSM_WEIGHT,		//设置气体检测帧间差值权重。范围-1000-1000，默认100。参数类型：整形。
	ITA_SET_HSM_STAYTHR,	//设置气体检测静态阈值。范围0-65535，默认30。参数类型：整形。
	ITA_SET_HSM_STAYWEIGHT, //设置气体检测静态帧间差值权重。范围-1000-1000，默认10。参数类型：整形。
	ITA_HSM_INTERVAL,		//设置气体检测缓冲区域间隔。范围0-16383，默认50。参数类型：整形。
	ITA_HSM_MIDFILTER,		//设置气体检测中值滤波开关。范围0，1，默认0。参数类型：unsigned char。
	ITA_HSM_MIDFILTER_WINSIZE,//设置气体检测中值滤波窗口。范围0-20，默认3。参数类型：int。
	ITA_HSM_DELTA_UPTHR,	//设置帧间差值的阈值上限。范围0-16383，默认5000。参数类型：int.
	ITA_HSM_DELTA_DOWNTHR,	//设置帧间差值的阈值下限。范围-16383-0，默认值-5000.参数类型：int.
	ITA_HSM_POINT_THR,		//设置单点噪声阈值。范围0-16383，默认值5.参数类型：int.
	ITA_TIME_FILTER,		//时域滤波开关，可以关闭或者打开。参数类型：bool。
	ITA_SPATIAL_FILTER,		//空域滤波开关，降噪，可以关闭或者打开。参数类型：bool。
	ITA_REMOVE_VERTICAL,	//去竖纹，可以关闭或者打开。参数类型：bool。
	ITA_REMOVE_HORIZONTAL,	//去横纹，可以关闭或者打开。参数类型：bool。
	ITA_HSM,				//气体检测， 可以关闭或者打开。 参数类型：bool。
	ITA_SHARPENING,			//锐化，可以关闭或者打开。参数类型：bool。
	ITA_DETAIL_ENHANCE,		//细节增强，可以关闭或者打开。参数类型：bool。
	ITA_BLOCK_HISTOGRAM,	//分块直方图，可以关闭或者打开。参数类型：bool。该功能废弃。
	ITA_GAMMA_CORRECTION,	//Gamma校正，可以关闭或者打开。参数类型：bool。
	ITA_Y8_ADJUSTBC,		//Y8纠偏，可以关闭或者打开。参数类型：bool。
	ITA_DRT_Y8,				//调光，默认打开。如果关闭，那么不处理Y16转Y8，不会生成imgDst图像数据。参数类型：bool。
	ITA_TFF_STD,            //时域滤波标准差，默认值10。参数类型：整形。
	ITA_RESTRAIN_RANGE,     //调光抑制范围，默认值64。参数类型：整形。
	ITA_DISCARD_UPRATIO,    //调光上抛点比例，默认值1，取值范围0-10。参数类型：整形。
	ITA_DISCARD_DOWNRATIO,    //调光下抛点比例，默认值1。取值范围0-10。参数类型：整形。
	ITA_MIX_THRLOW,			  //混合调光低阈值，默认值100，取值范围0-16383.参数类型：int.
	ITA_MIX_THRHIGH,		  //混合调光高阈值，默认值300，取值范围0-16383.参数类型：int.
	ITA_HEQ_PLAT_THRESH,     //平台直方图调光阈值参数，默认值5。取值范围1-1000。参数类型：整形。
	ITA_HEQ_RANGE_MAX,     //平台直方图调光映射范围参数，默认值230。取值范围0-512。参数类型：整形。
	ITA_HEQ_MIDVALUE,     //平台直方图调光映射中值参数，默认值128。取值范围0-255。参数类型：整形。
	ITA_IIE_ENHANCE_COEF,     //细节增益系数，默认值32。取值范围0-128。参数类型：整形。
	ITA_IIE_GAUSS_STD,     //增强算法参数，默认值5。取值范围1-50。参数类型：整形。
	ITA_GMC_GAMMA,     //Gamma算法参数，默认值1.25。取值范围0-2。参数类型：浮点型。
	ITA_GMC_TYPE,      //Gamma算法参数，默认值0。取值范围0-1。参数类型：整形。
	ITA_SET_ZOOM,	   //设置放大类型，参数类型：ITA_ZOOM_TYPE。
	ITA_SET_LAPLACE_WEIGHT, //锐化参数，设置锐化系数。参数类型：浮点型。取值范围0-10。
	ITA_LINEAR_RESTRAIN_RANGETHR,  //均匀面抑制
	ITA_AUTO_RPBDPS		//自动去坏点，参数类型：unsigned char。取值范围是0，1.目前仅ZM11A在使用。
}ITA_IC_TYPE;	//Image control type

/*Dynamic range transform type 动态范围转换类型*/
typedef enum ITADRTType
{
	ITA_DRT_LINEAR = 0,		//线性调光，默认
	ITA_DRT_PLATHE,			//平台直方图均衡
	ITA_DRT_MIX,			//混合调光
	ITA_DRT_MANUAL			//手动调光
}ITA_DRT_TYPE;

typedef struct ITA_DRT_PARAM
{
	//手动调光参数
	short manltone_maxY16;
	short manltone_minY16;
}ITADRTParam;

typedef enum ITAEqualLineType
{
	ITA_EQUAL_NONE,		//关闭等温线功能
	ITA_EQUAL_HIGH,
	ITA_EQUAL_LOW,
	ITA_EQUAL_HIGHLOW,
	ITA_EQUAL_MIDDLE
}ITA_EQUAL_LINE_TYPE;

typedef struct ITA_EQUAL_LINE_PARAM
{
	short highY16;				//高Y16
	short lowY16;				//低Y16
	int color;					//等温线的一种颜色，rgb888格式。
	int otherColor;				//等温线的另一种颜色，rgb888格式。
}ITAEqualLineParam;

typedef enum ITADebugType
{
	ITA_SET_DEBUGGING,			//设置是否开启调试。参数类型：ITA_SWITCH。ITA_DISABLE，关闭调试。ITA_ENABLE，开启调试。如果未设置日志路径，那么默认保存在程序同级目录下ITA.log。
	ITA_DEBUGGING_PATH,			//设置日志保存的完整路径。注意要在ITA_Init之后调用。必须是已存在的目录，例如/home/app/debug.log。Windows例如"D:/Tools/debug.log"。长度不要超过128。参数类型：字符串，以 \0 结束。
	ITA_DEBUGGING_MODE,			//设置日志输出模式。参数类型：整形。1，日志输出到文件，默认输出方式。2，日志输出到控制台。3，用户自定义日志函数的情况下，同时输出到文件。支持用户自定义输出日志，见ITARegistry。
	ITA_SNAPSHOT,				//拍照保存当前帧图像。如果有设置ITA_DEBUGGING_PATH，那么存储在设置的路径，否则存储在程序目录下。
	ITA_START_RECORD,			//开始录X16/Y16数据。
	ITA_STOP_RECORD,			//停止录X16/Y16数据。
	ITA_MEMORY_USAGE,			//查询动态内存使用量。参数类型：整型。单位：Byte。
	ITA_ISP_CONSUME				//查询执行一次图像处理消耗的时间。参数类型：整型。单位：ms。
}ITA_DEBUG_TYPE;	//Debug type

typedef enum ITACorrectType
{
	ITA_TRY_CORRECT_POINT,	//先尝试标定坏点，实时图像确认标定成功后再添加。参数类型：ITA_POINT。
	ITA_CORRECT_POINT,		//添加坏点，可以多次添加。参数类型：ITA_POINT。
	ITA_SAVE_POINTS 		//保存坏点。待坏点添加完成后，保存指定测温范围下的坏点。参数类型：ITA_RANGE。
}ITA_CORRECT_TYPE;

typedef struct ITA_POINT {
	int x;
	int y;
}ITAPoint;

typedef enum ITACollectType
{
	ITA_COLLECT_DISTANCE,		//距离作为校温参数。
	ITA_COLLECT_ENVIRON 		//环温作为校温参数。
}ITA_COLLECT_TYPE;

typedef struct ITA_COLLECT_PARAM
{
	ITA_COLLECT_TYPE type;	//参数类型
	float param;			//参数值
	float blackTemp;		//黑体温度
}ITACollectParam;

typedef struct ITA_CORRECT_PARAM
{
	int BlackBodyNumber;
	float TBlackBody[10];
	short Y16[10];
	float ErrorNoCorrect[10];
	float ErrorCorrect[10];
}ITACalibrateResult;

//用户自定义伪彩操作类型
typedef enum ITAPaletteOPType
{
	ITA_PALETTE_OP_ADD,			//根据序号添加用户自定义伪彩。参数类型：ITAPaletteInfo。
	ITA_PALETTE_OP_UPDATE,		//根据序号更新用户自定义伪彩。参数类型：ITAPaletteInfo。
	ITA_PALETTE_OP_DELETE 		//根据序号删除用户自定义伪彩。参数类型：ITAPaletteInfo。
}ITA_PALETTE_OP_TYPE;

//用户自定义伪彩信息
typedef struct ITA_PALETTE_INFO
{
	int paletteID;			//序号，用户自定义的伪彩序号应从ITAVersionInfo获取的totalNumberOfPalettes之后开始添加。
	unsigned char *data;	//自定义伪彩数据。每条伪彩数据长度256 * 4B。
	int dataLen;			//伪彩数据长度，单位Byte。
}ITAPaletteInfo;

typedef void(*ITACollectFinish)(int cameraID, void* param);

typedef void(*ITACollectBaseFinish)(int cameraID,int gear,unsigned short* baseData,int dataLen, void *param);

typedef void(*ITAReviewFinish)(int cameraID, bool status, float averTemp, float deviation, void* userParam);

/*校温参数有两组，一组自动校温的参数和一组手动校温的参数。*/
typedef enum ITACalibrateType
{
	ITA_NEAR_KF_AUTO,
	ITA_NEAR_B_AUTO,
	ITA_FAR_KF_AUTO,
	ITA_FAR_B_AUTO,
	ITA_NEAR_KF_MANUAL,
	ITA_NEAR_B_MANUAL,
	ITA_FAR_KF_MANUAL,
	ITA_FAR_B_MANUAL
}ITA_CALIBRATE_TYPE;

/*描述版本号和功能信息。不同的版本包含的功能可能存在差异。*/
typedef struct ITA_VERSION_INFO
{
	char version[64];				//版本号
	int totalNumberOfPalettes;		//伪彩数量
	int totalNumberOfGears;			//焦温档位数量
	char confVersion[64];           //配置文件版本号
	char moduleCode[24];            //模组编号
	float correctionFactors[9];     //内参矩阵参数和校正系数
	int productType;				//产品类型 0:非TEC产品 1：TEC产品
}ITAVersionInfo;

/*定义事件通知机制中的事件类型*/
typedef enum ITAEventType
{
	ITA_NUC_BEGIN,		//开始做NUC
	ITA_NUC_END,		//完成一次NUC
	ITA_SHUTTER_BEGIN,	//开始快门
	ITA_SHUTTER_END,	//完成一次打快门
	ITA_AF_BEGIN,		//开始调焦
	ITA_AF_END			//完成调焦，参数eventParam指示调焦成功或者失败。参数类型：ITA_RESULT。
}ITA_EVENT_TYPE;


typedef enum ITAAFCalibrate
{
	ITA_AF_CALIBRATE_DIFF = 0,		//回程差校准
	ITA_AF_CALIBRATE_LASER			//激光校准
}ITA_AF_CALIBRATE;

typedef enum ITAAFInfo
{
	ITA_AF_BASE_INFO = 0,			//基础参数配置
	ITA_AF_LASER_INFO,				//激光参数配置
	ITA_AF_CONTRAST_INFO			//对比度参数配置
}ITA_AF_INFO;

typedef enum ITAFieldAngle
{
	ITA_ANGLE_56 = 0, //F1.1
	ITA_ANGLE_25 = 1, //F1.0
	ITA_ANGLE_120 = 2, //F1.1
	ITA_ANGLE_50 = 3, //F1.1
	ITA_ANGLE_90 = 4, //F1.0
	ITA_ANGLE_33 = 5, //F1.0
	ITA_ANGLE_25_F11 = 6,//F1.1，变焦
	ITA_ANGLE_45 = 7, //F1.0
	ITA_ANGLE_15 = 8, //F1.2
	ITA_ANGLE_6 = 9, //F1.2
	ITA_CUSTOM_LENS1 = 10,  //产品自定义镜头1，微距，高温镜头等。
	ITA_CUSTOM_LENS2 = 11,  //产品自定义镜头2
	ITA_CUSTOM_LENS3 = 12   //产品自定义镜头3
}ITA_FIELD_ANGLE;

typedef struct ITA_CURVE_PARAM
{
	short temperFreeLens;			//是否免校温镜头。1，是；0，否。
}ITACurveParam;

/*注册机制，在ITA_Init之后调用。ITA使用HAL functions完成初始化。*/
typedef struct ITA_REGISTRY
{
	/**
	* @brief:	读配置文件ITA.conf。可选，在没有配置文件的情况下ITA使用默认参数。
	* @param:	int cameraID			标识设备，支持同时打开多台设备。
	* @param:	unsigned char *buf		缓存首地址，输入参数。
	* @param:	int bufSize				缓存大小，输入参数。
	* @param:	int *length				配置文件长度，输出参数。
	* @param:	void *userParam			用户参数，注册时由用户传入。ITA库调用注册函数时再传给用户。
	* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
	* @note 配置文件是只读的。未经授权的修改无效。
	* @see ITA_Init，camera id由用户输入。
	**/
	ITA_RESULT(*ReadConfig)(int cameraID, unsigned char *buf, int bufSize, int *length, void *userParam);

	/**
	* Register log function. Output logs using registered function.
	* @param:	int cameraID			标识设备，支持同时打开多台设备。
	* @param:	const char *logInfo		一行日志信息。
	* @param:	void *userParam			用户参数，注册时由用户传入。ITA库调用注册函数时再传给用户。
	* @note 当用户注册输出日志函数后，ITA内部不再输出日志。除非用户设置ITA_DEBUGGING_MODE为3。
	*/
	void(*OutputLog)(int cameraID, const char *logInfo, void *userParam);

	/*************************HAL接口定义**************************************************/
	/* Register HAL(Hardware Abstract Layer) functions. Users implement HAL functions 
	 * according to product requirements. Not every interface must be implemented. Please 
	 * refer to the integration documentation for which interfaces need to be implemented.*/
	/**************************************************************************************/
	/**
	* @brief:	控制快门。
	* @param:	int cameraID				标识设备，支持同时打开多台设备。
	* @param:	ITA_SHUTTER_STATUS status	快门状态。
	* @param:	void *userParam				用户参数，注册时由用户传入。ITA库调用注册函数时再传给用户。
	* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
	* @see ITA_Init，camera id由用户输入。
	**/
	ITA_RESULT(*ShutterControl)(int cameraID, ITA_SHUTTER_STATUS status, void *userParam);

	/**
	* @brief:	做NUC。带MCU的产品须实现。
	* @param:	int cameraID				标识设备，支持同时打开多台设备。
	* @param:	void *userParam				用户参数，注册时由用户传入。ITA库调用注册函数时再传给用户。
	* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
	* @see ITA_Init，camera id由用户输入。
	**/
	ITA_RESULT(*DoNUC)(int cameraID, void *userParam);

	/**
	* @brief:	获取温度传感器值。由ITA_SENSOR_VALUE决定输出AD值还是温度值。
	* @param:	int cameraID					标识设备，支持同时打开多台设备。输入参数。
	* @param:	ITA_TEMP_TYPE eType				温传类型。输入参数。
	* @param:	ITA_SENSOR_VALUE *sensorValue	值类型。输出参数，由用户决定。
	* @param:	int *ADValue					AD值，输出参数。当ITA_AD_VALUE==sensorValue时有效。
	* @param:	float *tempValue				温度值，输出参数。当ITA_TEMP_VALUE==sensorValue时有效。
	* @param:	void *userParam					用户参数，注册时由用户传入。ITA库调用注册函数时再传给用户。
	* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
	* @see ITA_Init，camera id由用户输入。
	**/
	ITA_RESULT(*GetSensorTemp)(int cameraID, ITA_TEMP_TYPE eType, ITA_SENSOR_VALUE *sensorValue, int *ADValue, float *tempValue, void *userParam);

	/**
	* @brief:	打开I2C。Timo 256通过I2C配置探测器。
	* @param:	int cameraID				标识设备，支持同时打开多台设备。
	* @param:	unsigned char u32DevAddr
	* @param:	void *userParam				用户参数，注册时由用户传入。ITA库调用注册函数时再传给用户。
	* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
	* @see ITA_Init，camera id由用户输入。
	**/
	ITA_RESULT(*I2COpen)(int cameraID, unsigned char u32DevAddr, void *userParam);

	/**
	* @brief:	关闭I2C。Timo 256通过I2C配置探测器。
	* @param:	int cameraID			标识设备，支持同时打开多台设备。
	* @param:	void *userParam			用户参数，注册时由用户传入。ITA库调用注册函数时再传给用户。
	* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
	* @see ITA_Init，camera id由用户输入。
	**/
	ITA_RESULT(*I2CClose)(int cameraID, void *userParam);

	/**
	* @brief:	I2C读寄存器。Timo 256通过I2C配置探测器。
	* @param:	int cameraID			标识设备，支持同时打开多台设备。
	* @param:	unsigned char u8Addr	寄存器地址。
	* @param:	unsigned char * pu8Val	读出的寄存器值。
	* @param:	void *userParam			用户参数，注册时由用户传入。ITA库调用注册函数时再传给用户。
	* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
	* @see ITA_Init，camera id由用户输入。
	**/
	ITA_RESULT(*I2CRead)(int cameraID, unsigned char u8Addr, unsigned char * pu8Val, void *userParam);

	/**
	* @brief:	I2C写寄存器。Timo 256通过I2C配置探测器。
	* @param:	int cameraID			标识设备，支持同时打开多台设备。
	* @param:	unsigned char u8Addr	寄存器地址。
	* @param:	unsigned char u8Val		写入的寄存器值。
	* @param:	void *userParam			用户参数，注册时由用户传入。ITA库调用注册函数时再传给用户。
	* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
	* @see ITA_Init，camera id由用户输入。
	**/
	ITA_RESULT(*I2CWrite)(int cameraID, unsigned char u8Addr, unsigned char u8Val, void *userParam);

	/**
	* @brief:	SPI传输数据，先写入再读。Timo 120通过SPI配置探测器。
	* @param:	int cameraID			 标识设备，支持同时打开多台设备。
	* @param:	unsigned char *writeBuf  待写入的数据缓存。
	* @param:	unsigned char *readBuf   读数据的缓存。
	* @param:	int bufLen               缓存长度。
	* @param:	void *userParam			 用户参数，注册时由用户传入。ITA库调用注册函数时再传给用户。
	* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
	* @see ITA_Init，camera id由用户输入。
	**/
	ITA_RESULT(*SPITransmit)(int cameraID, unsigned char *writeBuf, unsigned char *readBuf, int bufLen, void *userParam);

	/**
	* @brief:	SPI写数据。Timo 120通过SPI配置探测器。
	* @param:	int cameraID			 标识设备，支持同时打开多台设备。
	* @param:	unsigned char *writeData 待写入的数据。
	* @param:	int writeLen             数据长度。
	* @param:	void *userParam			 用户参数，注册时由用户传入。ITA库调用注册函数时再传给用户。
	* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
	* @see ITA_Init，camera id由用户输入。
	**/
	ITA_RESULT(*SPIWrite)(int cameraID, unsigned char *writeData, int writeLen, void *userParam);

	/**
	* @brief:	模组内置Flash的GPIO口拉低，开始更新寄存器值。使用内置Flash的模组产品需要实现此函数。
	* @param:	int cameraID			 标识设备，支持同时打开多台设备。
	* @param:	void *userParam			 用户参数，注册时由用户传入。ITA库调用注册函数时再传给用户。
	* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
	* @see ITA_Init，camera id由用户输入。
	**/
	ITA_RESULT(*InternalFlashPrepare)(int cameraID, void *userParam);

	/**
	* @brief:	模组内置Flash的GPIO口拉高，结束更新寄存器值。使用内置Flash的模组产品需要实现此函数。
	* @param:	int cameraID			 标识设备，支持同时打开多台设备。
	* @param:	void *userParam			 用户参数，注册时由用户传入。ITA库调用注册函数时再传给用户。
	* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
	* @see ITA_Init，camera id由用户输入。
	**/
	ITA_RESULT(*InternalFlashDone)(int cameraID, void *userParam);

	/**
	* @brief:  读数据包。
	* @param:  int cameraID			标识设备，支持同时打开多台设备。
	* @param:  unsigned char *buf	数据缓存
	* @param:  int offset			相对数据包头的偏移，从偏移处开始读size长度的数据。
	* @param:  int size				读取的数据长度
	* @param:  ITA_RANGE range		测温范围，不同产品支持的温度范围和数量不同，一般不超过3档。
	* @param:	void *userParam		用户参数，注册时由用户传入。ITA库调用注册函数时再传给用户。
	* @return: 返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
	* @see ITA_Init，camera id由用户输入。
	**/
	ITA_RESULT(*ReadPackageData)(int cameraID, unsigned char *buf, int offset, int size, ITA_RANGE range, void *userParam);

	/**
	* @brief:  另一种方式读数据包。用于带有不同镜头热插拔和免校温镜头的模组类测温产品，ReadPackageData和ReadPackageDataEx二者用户只用注册其中一个。如果同时注册那么ReadPackageData优先。
	* @param:  int cameraID			标识设备，支持同时打开多台设备。
	* @param:  unsigned char *buf	数据缓存
	* @param:  int offset			相对数据包头的偏移，从偏移处开始读size长度的数据。
	* @param:  int size				读取的数据长度
	* @param:  ITA_RANGE range		测温范围，不同产品支持的温度范围和数量不同，一般不超过3档。
	* @param:  ITA_FIELD_ANGLE lensType 镜头类型
	* @param:  void *extendParam     扩展参数信息，具体格式见ITACurveParam。
	* @param:  int extendParamSize   扩展参数信息长度。
	* @param:	void *userParam		用户参数，注册时由用户传入。ITA库调用注册函数时再传给用户。
	* @return: 返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
	* @see ITA_Init，camera id由用户输入。
	**/
	ITA_RESULT(*ReadPackageDataEx)(int cameraID, unsigned char *buf, int offset, int size, ITA_RANGE range, 
		ITA_FIELD_ANGLE lensType, void *extendParam, int extendParamSize, void *userParam);

	/**
	* @brief:  写数据包。
	* @param:  int cameraID			标识设备，支持同时打开多台设备。
	* @param:  unsigned char *buf	数据缓存
	* @param:  int offset			相对数据包头的偏移，从偏移处开始读size长度的数据。
	* @param:  int size				写的数据长度
	* @param:  ITA_RANGE range		测温范围，共有3档。
	* @param:	void *userParam		用户参数，注册时由用户传入。ITA库调用注册函数时再传给用户。
	* @return: 返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
	* @see ITA_Init，camera id由用户输入。
	**/
	ITA_RESULT(*WritePackageData)(int cameraID, unsigned char *buf, int offset, int size, ITA_RANGE range, void *userParam);

	/**
	* @brief:  读取校温参数。
	* @param:  int cameraID				标识设备，支持同时打开多台设备。
	* @param:  ITA_RANGE range			测温范围，共有3档。
	* @param:  ITA_CALIBRATE_TYPE type	参数类型。
	* @param:  float *p					参数指针。
	* @param:	void *userParam			用户参数，注册时由用户传入。ITA库调用注册函数时再传给用户。
	* @return: 返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
	* @see ITA_Init，camera id由用户输入。
	**/
	ITA_RESULT(*ReadCalibrateParam)(int cameraID, ITA_RANGE range, ITA_CALIBRATE_TYPE type, float *p, void *userParam);

	/**
	* @brief:  保存校温参数。
	* @param:  int cameraID				标识设备，支持同时打开多台设备。
	* @param:  ITA_RANGE range			测温范围，共有3档。
	* @param:  ITA_CALIBRATE_TYPE type	参数类型。
	* @param:  float value				参数值。
	* @param:	void *userParam			用户参数，注册时由用户传入。ITA库调用注册函数时再传给用户。
	* @return: 返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
	* @see ITA_Init，camera id由用户输入。
	**/
	ITA_RESULT(*WriteCalibrateParam)(int cameraID, ITA_RANGE range, ITA_CALIBRATE_TYPE type, float value, void *userParam);
	
	/**
	* @brief:  读曲线包。用于手持测温和机芯测温产品。
	* @param:  int cameraID			标识设备，支持同时打开多台设备。
	* @param:  unsigned char *buf	数据缓存
	* @param:  int bufSize			数据缓存的长度
	* @param:  int *length			读取的实际长度
	* @param:  ITA_FIELD_ANGLE lensType 镜头类型
	* @param:  ITA_RANGE range		测温范围，不同产品支持的温度范围和数量不同，一般不超过3。建议从ITA_CUSTOM_RANGE1开始。
	* @param:  int gear				焦温档位，手持测温和机芯测温产品通常不超过2档。
	* @param:  void *userParam		用户参数，注册时由用户传入。ITA库调用注册函数时再传给用户。
	* @return: 返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
	* @see ITA_Init，camera id由用户输入。
	**/
	ITA_RESULT(*ReadCurveData)(int cameraID, unsigned char *buf, int bufSize, int *length, ITA_FIELD_ANGLE lensType, ITA_RANGE range, int gear, void *userParam);

	/**
	* @brief:  另一种方式读曲线包。用于带有免校温镜头的手持测温和机芯测温产品。ReadCurveData和ReadCurveDataEx二者用户只用注册其中一个。如果同时注册那么ReadCurveData优先。
	* @param:  int cameraID			标识设备，支持同时打开多台设备。
	* @param:  unsigned char *buf	数据缓存
	* @param:  int bufSize			数据缓存的长度
	* @param:  int *length			读取的实际长度
	* @param:  ITA_FIELD_ANGLE lensType 镜头类型
	* @param:  ITA_RANGE range		测温范围，不同产品支持的温度范围和数量不同，一般不超过3。建议从ITA_CUSTOM_RANGE1开始。
	* @param:  int gear				焦温档位，手持测温和机芯测温产品通常不超过2档。
	* @param:  void *extendParam     扩展参数信息，具体格式见ITACurveParam。
	* @param:  int extendParamSize   扩展参数信息长度。
	* @param:  void *userParam		用户参数，注册时由用户传入。ITA库调用注册函数时再传给用户。
	* @return: 返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
	* @see ITA_Init，camera id由用户输入。
	**/
	ITA_RESULT(*ReadCurveDataEx)(int cameraID, unsigned char *buf, int bufSize, int *length, 
		ITA_FIELD_ANGLE lensType, ITA_RANGE range, int gear, void *extendParam, int extendParamSize, void *userParam);

	/**
	* @brief:  发送PWM脉冲信号。
	* @param:  int cameraID     标识设备，支持同时打开多台设备。
	* @param:  int dutyration	占空比，一个脉冲周期内，高电平的时间与整个周期时间的比例。
	* @param:  int engage		波形使能输出，1，输出，0，不输出。
	* @param:  void *userParam	用户参数，注册时由用户传入。ITA库调用注册函数时再传给用户。
	* @return: 返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
	* @see ITA_Init，camera id由用户输入。
	**/
	ITA_RESULT(*PWMDrive)(int cameraID, int dutyration, int engage, void *userParam);

	/**
	* @brief:  写EEPROM自动调焦参数。
	* @param:  int cameraID			标识设备，支持同时打开多台设备。
	* @param:  unsigned short addr	地址
	* @param:  unsigned char data	数据
	* @param:  void *userParam		用户参数，注册时由用户传入。ITA库调用注册函数时再传给用户。
	* @return: 返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
	* @see ITA_Init，camera id由用户输入。
	**/
	ITA_RESULT(*EEPROMWrite)(int cameraID, unsigned short addr, unsigned char data, void *userParam);

	/**
	* @brief:  读EEPROM自动调焦参数。
	* @param:  int cameraID			标识设备，支持同时打开多台设备。
	* @param:  unsigned short addr	地址
	* @param:  unsigned char *data	数据指针
	* @param:  void *userParam		用户参数，注册时由用户传入。ITA库调用注册函数时再传给用户。
	* @return: 返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
	* @see ITA_Init，camera id由用户输入。
	**/
	ITA_RESULT(*EEPROMRead)(int cameraID, unsigned short addr, unsigned char *data, void *userParam);

	/**
	* @brief:  事件通知机制。当有NUC、快门和AF等事件发生时，通知用户。有些事件带有参数。
	* @param:  int cameraID				标识设备，支持同时打开多台设备。
	* @param:  ITA_EVENT_TYPE eventType	事件类型
	* @param:  void *eventParam			事件参数
	* @param:  void *userParam			用户参数，注册时由用户传入。ITA库调用注册函数时再传给用户。
	* @return: 返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
	* @see ITA_Init，camera id由用户输入。用户实现事件通知函数时须立即返回，否则影响ITA运行效率。
	**/
	ITA_RESULT(*NotifyEvent)(int cameraID, ITA_EVENT_TYPE eventType, void *eventParam, void *userParam);

	/**
	* @brief:  冷热机判断等部分功能需要读写私有数据。如果产品需要此功能，那么请用户实现在存储介质上读写私有数据。
	* @param:  int cameraID				标识设备，支持同时打开多台设备。
	* @param:  unsigned char *readBuf	数据缓存
	* @param:  int readLen  			缓存长度
	* @param:  void *userParam			用户参数，注册时由用户传入。ITA库调用注册函数时再传给用户。
	* @return: 返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
	* @see ITA_Init，camera id由用户输入。
	**/
	ITA_RESULT(*ReadPrivateData)(int cameraID, unsigned char *readBuf, int readLen, void *userParam);

	/**
	* @brief:  冷热机判断等部分功能需要读写私有数据。如果产品需要此功能，那么请用户实现在存储介质上读写私有数据。
	* @param:  int cameraID				标识设备，支持同时打开多台设备。
	* @param:  unsigned char *writeBuf	数据缓存
	* @param:  int writeLen  			缓存长度
	* @param:  void *userParam			用户参数，注册时由用户传入。ITA库调用注册函数时再传给用户。
	* @return: 返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
	* @see ITA_Init，camera id由用户输入。
	**/
	ITA_RESULT(*WritePrivateData)(int cameraID, unsigned char *writeBuf, int writeLen, void *userParam);

	/*用户参数，调用注册函数时再传给用户。*/
	void *userParam;
}ITARegistry;

typedef struct ITA_PARAM_INFO
{
	ITA_RANGE range;
	int fpaGear; //焦温档位
	bool isAutoGear; //自动切档开关
	bool isReverse; //是否反向的开关
	bool isSubAvgB; //减去本底均值的开关
	bool isDebug; //调试开关
	ITA_DRT_TYPE drtType; //调光方式
	ITA_EQUAL_LINE_TYPE equalLineType; //等温线类型
	int frameRate; //用户设置的帧率
	float distance;
	float emiss;
	float transmit;
	float reflectTemp;
	ITA_SWITCH environCorr;
	ITA_SWITCH lensCorr;
	ITA_SWITCH distanceCompen;
	ITA_SWITCH emissCorr;
	ITA_SWITCH transCorr;
	float lowLensCorrK;		//低温档镜头修正系数
	float highLensCorrK;	//高温档镜头修正系数
	float lensCorrK;		//其他档镜头修正系数
	ITA_SWITCH shutterCorr;
	float lowShutterCorrK;	//低温档快门修正系数
	float highShutterCorrK;	//高温档快门修正系数
	float shutterCorrK;		//其他档快门修正系数
	ITA_PIXEL_FORMAT format;
	ITA_PALETTE_TYPE palette;
	float scale;
	int contrast;
	int brightness;
	ITA_ROTATE_TYPE rotate;
	ITA_FLIP_TYPE flip;
	bool isTimeFilter;	//时域滤波开关，可以关闭或者打开。
	bool isSpatialFilter;	//空域滤波开关，降噪，可以关闭或者打开。
	bool isRemoveVertical;//去竖纹，可以关闭或者打开。
	bool isRemoveHorizontal;//去横纹，可以关闭或者打开。
	bool isSharpening;	//锐化，可以关闭或者打开。
	bool isDetailEnhance;	//细节增强，可以关闭或者打开。
	bool isBlockHistogram;//分块直方图，可以关闭或者打开。该功能废弃。
	bool isGammaCorrection;//Gamma校正，可以关闭或者打开。
	bool isY8AdjustBC;	//Y8纠偏，可以关闭或者打开。
	bool isHsm;			//气体检测开关，可以关闭或者打开。
	unsigned char isAutoRpBdPs;	//自动去坏点开关，可以关闭或者打开。
	int tffStd;	//时域滤波标准差
	int restrainRange; //调光抑制范围
	int discardUpratio;  //调光上抛点比例
	int mixThrLow;		//混合调光低阈值
	int mixThrHigh;		//混合调光高阈值
	int discardDownratio;  //调光下抛点比例
	int heqPlatThresh;  //平台直方图调光参数
	int heqRangeMax;  //平台直方图调光参数
	int heqMidvalue;  //平台直方图调光参数
	int iieEnhanceCoef; //细节增益系数
	int iieGaussStd;  //增强算法参数
	float gmcGamma;  //Gamma算法参数
	int gmcType;  //Gamma算法参数
	int hsmWeight;//气体检测算法参数
	int hsmStayThr;//气体检测静态阈值
	int hsmStayWeight; //气体检测静态帧间差值权重
	int hsmInterval;//缓冲区域间隔
	int hsmMidfilterWinSize;//中值滤波窗口
	int hsmDeltaUpthr;//帧间差值的阈值上限
	int hsmDeltaDownthr;//帧间差值的阈值下限
	int hsmPointThr; //单点噪声阈值
	bool isHsmMidfilter;//气体检测中值滤波开关
	float nearKFAuto;
	float nearBAuto;
	float farKFAuto;
	float farBAuto;
	float nearKFManual;
	float nearBManual;
	float farKFManual;
	float farBManual;
	float orinalShutterTemp;//开机快门温度
	float currentShutterTemp;//当前打快门时的快门温度
	float realTimeShutterTemp;//实时快门温度
	float realTimeLensTemp;//实时镜筒温度
	float currentLensTemp;//当前打快门时的镜筒温度
	float realTimeFpaTemp;//实时焦平面温度
	float humidity;		//湿度
	int zoomType;//放大类型,0:Near,1:Bilinear,2:BilinearFast
	unsigned char coldHotStatus;//冷机热机状态：1:冷机 0:热机
	float laplaceWeight;
	int linearRestrainRangethr; //均匀面抑制参数
}ITAParamInfo;

typedef struct ITA_TEC_INFO
{
	int mtType;			//测温范围，使用ITA_SetMeasureRange设置。
	int nGear;			//测温档位, 0:低温档, 1:常温挡。使用ITA_CHANGE_GEAR设置。
	short sY16Offset;					//Y16偏移量(默认0)
	int nKF;							//查曲线时Y16的缩放量(定点化100倍，默认值为100)
	int nB1;							//查曲线时Y16的偏移量(定点化100倍)
	int nDistance_a0;					//距离校正系数(定点化10000000000倍，默认0)
	int nDistance_a1;					//距离校正系数(定点化1000000000倍，默认0)
	int nDistance_a2;					//距离校正系数(定点化10000000倍，默认0)
	int nDistance_a3;					//距离校正系数(定点化100000倍，默认0)
	int nDistance_a4;					//距离校正系数(定点化100000倍，默认0)
	int nDistance_a5;					//距离校正系数(定点化10000倍，默认0)
	int nDistance_a6;					//距离校正系数(定点化1000倍，默认0)
	int nDistance_a7;					//距离校正系数(定点化100倍，默认0)
	int nDistance_a8;					//距离校正系数(定点化100倍，默认0)
	int nK1;							//快门温漂系数(定点化100倍，默认0)
	int nK2;							//镜筒温漂系数(定点化100倍，默认0)
	int nK3;							//环温修正系数(定点化10000倍，默认0)
	int nB2;							//环温修正偏移量(定点化10000倍，默认0)
	int nKFOffset;						//自动校温KF偏移量，置零后恢复出厂校温设置，默认0
	int nB1Offset;						//自动校温B1偏移量，置零后恢复出厂校温设置，默认0
	int fHumidity;						//湿度(定点化100倍，默认60)
	int nAtmosphereTransmittance;		//大气透过率(定点化100倍，范围0~100)
	float fEmiss;						//发射率(0.01-1.0,默认1.0)
	float fDistance;					//测温距离 
	float fReflectT;					//反射温度（低温档默认3，常温档默认23）
	float fAmbient;						//环境温度（取开机快门温）
	float fAtmosphereTemperature;		//大气温度
	float fWindowTransmittance;			//窗口透过率(范围0~1)
	float fWindowTemperature;			//窗口温度（低温档默认3，常温档默认23）
	bool bHumidityCorrection;			//湿度修正开关，默认打开。
	bool bShutterCorrection;			//快门修正开关，默认打开。
	bool bLensCorrection;				//镜筒修正开关，默认打开。
	bool bEmissCorrection;				//发射率修正开关，默认打开。
	bool bDistanceCorrection;			//距离修正开关，默认打开。
	bool bAmbientCorrection;			//环温修正开关，默认打开。
	bool bB1Correction;					//B1修正开关，默认打开。
	bool bAtmosphereCorrection;			//大气透过率修正开关，默认打开。
	bool bWindowTransmittanceCorrection;//窗口透过率开关，默认打开。
	unsigned char burnSwitch;//防灼烧开关
	unsigned char burnFlag;//灼烧标志
	short burnDelayTime;//灼烧持续时间
	int burnThreshold;//灼烧阈值
}ITATECInfo;

typedef struct ITA_MEASURE_INFO
{
	unsigned char          bDistanceCompensate;//距离补偿开关
	unsigned char          bEmissCorrection;//发射率校正开关
	unsigned char          bTransCorrection;//透过率校正开关
	unsigned char          bHumidityCorrection;//湿度校正开关

	float         fNearKf;	//近距离kf系数
	float         fNearB;	//近距离偏置系数
	float         fFarKf;	//远距离kf系数
	float         fFarB;	//远距离偏置系数

	float         fNearKf2;	//近距离kf系数
	float         fNearB2;	//近距离偏置系数
	float         fFarKf2;	//远距离kf系数
	float         fFarB2;	//远距离偏置系数

	float         fHighShutterCorrCoff;		//高温档快门校正系数(工业专用)
	float         fLowShutterCorrCoff;		//常温档快门校正系数(工业专用)
	float         fShutterCorrCoff;		    //其他测温档位快门校正系数(工业专用)

	float         fHighLensCorrK;	//高温档镜筒温漂修正系数
	float         fLowLensCorrK;	//常温档镜筒温漂修正系数
	float         fLensCorrK;	//其他档镜筒温漂修正系数

	float         fDistance;	//目标距离
	float		  fEmiss;		//发射率(工业专用)
	float		  fTransmit;	//透过率(工业专用)
	float         fHumidity;	//湿度(0.01-1.0,默认0.6)
	float		  fReflectTemp;	//反射温度(工业专用)
	float         fCorrectDistance;//校温距离
}ITAMeasureInfo;

/*后处理函数，主要用于Android平台线程中通过postHandle通知应用层释放资源。*/
typedef void(*PostHandle)(int cameraID);

typedef struct ITA_SCENE_PARAM 
{
	void *mtParam;            //测温参数
	int paramLen;             //测温参数数据长度
	unsigned char *curveData; //曲线数据
	int curveLen;             //曲线数据长度，单位Byte。
	void *reservedData;       //保留数据，模组产品使用。
	int reservedLen;          //保留数据长度。如果大于0，那么用户需要保存保留数据。
}ITASceneParam;

typedef enum ITADataType
{
	ITA_DATA_Y8,		  //数据类型为unsigned char型
	ITA_DATA_Y16		  //数据类型为short型
}ITA_DATA_TYPE;

typedef enum ITAY8DataMode
{
	ITA_Y8_NORMAL,		//Y8数据范围：0-255
	ITA_Y8_XINSHENG		//Y8数据来自芯晟的ASIC,Y8数据范围：16-235
}ITA_Y8_DATA_MODE;

typedef struct ITA_FUNCTION_CONFIG {
	char versionNumber[64];
	int productType;						//产品型号
	float shutterTempUpper;					//快门补偿温升条件
	float nucTempUpper;						//NUC补偿温升条件
	int nucIntervalBottom;					//相邻两次NUC最小间隔时间
	int shutterIntervalBottom;				//相邻两次快门最小间隔时间（开机时过于频繁打快门无意义）
	int isSubAvgB;							//Y16减去本底均值的开关
	int shutterCollectInterval;				//上次快门之后间隔多少秒开始采集
	int collectInterval;					//相邻两次采集间隔时间
	int collectNumberOfTimes;				//一次快门之后需要采集的次数
	int bootTimeToNow1;						//开机60分钟内
	int shutterPeriod1;						//自动快门周期30秒
	int bootTimeToNow2;						//开机60分钟与120分钟之间
	int shutterPeriod2;						//自动快门周期60秒
	int bootTimeToNow3;						//开机120分钟之后
	int shutterPeriod3;						//自动快门周期90秒
	int maxFramerate;						//探测器配置的帧率
	int internalFlash;                      //0，模组无内置Flash；1，120 90°模组内置P25Q Flash。
	unsigned char versionNumberS;
	unsigned char productTypeS;							//产品型号
	unsigned char shutterTempUpperS;					//快门补偿温升条件
	unsigned char nucTempUpperS;						//NUC补偿温升条件
	unsigned char nucIntervalBottomS;					//相邻两次NUC最小间隔时间
	unsigned char shutterIntervalBottomS;				//相邻两次快门最小间隔时间（开机时过于频繁打快门无意义）
	unsigned char isSubAvgBS;							//Y16减去本底均值的开关
	unsigned char shutterCollectIntervalS;				//上次快门之后间隔多少秒开始采集
	unsigned char collectIntervalS;						//相邻两次采集间隔时间
	unsigned char collectNumberOfTimesS;				//一次快门之后需要采集的次数
	unsigned char bootTimeToNow1S;						//开机60分钟内
	unsigned char shutterPeriod1S;						//自动快门周期30秒
	unsigned char bootTimeToNow2S;						//开机60分钟与120分钟之间
	unsigned char shutterPeriod2S;						//自动快门周期60秒
	unsigned char bootTimeToNow3S;						//开机120分钟之后
	unsigned char shutterPeriod3S;						//自动快门周期90秒
	unsigned char maxFramerateS;						//探测器配置的帧率
	unsigned char internalFlashS;                       //模组Flash信息
}ITAFunctionConfig;

typedef struct ITA_MT_CONFIG {
	int	lensType;				//ITA_FIELD_ANGLE 视场角类型 0:56°；1:25°；2:120°；3:50°；4:90°；5:33°。
	float mtDistanceRangeN;		//测温距离范围下限
	float mtDistanceRangeF;		//测温距离范围上限
	int	mtType;					//测温模式 0：人体；1：工业低温；2：工业高温。
	float correctDistance;		//自动校温距离
	float distance;				//目标距离
	float lowLensCorrK;			//测温常温档镜筒温漂系数
	float highLensCorrK;		//测温高温档镜筒温漂系数
	float lowShutterCorrCoff;	//测温常温档快门温漂系数
	float highShutterCorrCoff;	//测温高温档快门温漂系数
	int mtDisType;				//测温距离修正类型，0：6参数; 1:9参数
	float coefA1;				//测温距离修正参数
	float coefA2;				//测温距离修正参数
	float coefB1;				//测温距离修正参数
	float coefB2;				//测温距离修正参数
	float coefC1;				//测温距离修正参数
	float coefC2;				//测温距离修正参数
	float coefA3;				//测温距离修正参数
	float coefB3;				//测温距离修正参数
	float coefC3;				//测温距离修正参数
	unsigned char correctDistanceS;		//自动校温距离
	unsigned char distanceS;				//目标距离
	unsigned char lowLensCorrKS;			//测温常温档镜筒温漂系数
	unsigned char highLensCorrKS;		//测温高温档镜筒温漂系数
	unsigned char lowShutterCorrCoffS;	//测温常温档快门温漂系数
	unsigned char highShutterCorrCoffS;	//测温高温档快门温漂系数
	unsigned char mtDisTypeS;			//测温距离修正类型
	unsigned char coefA1S;				//测温距离修正参数
	unsigned char coefA2S;				//测温距离修正参数
	unsigned char coefB1S;				//测温距离修正参数
	unsigned char coefB2S;				//测温距离修正参数
	unsigned char coefC1S;				//测温距离修正参数
	unsigned char coefC2S;				//测温距离修正参数
	unsigned char coefA3S;				//测温距离修正参数
	unsigned char coefB3S;				//测温距离修正参数
	unsigned char coefC3S;				//测温距离修正参数
}ITAMTConfig;
typedef struct ITA_ISP_CONFIG {
	int	lensType;				//视场角类型 0：25°；1:33°；2：50°；3：56°；4：90°；5：120°。
	int	mtType;					//测温模式 0：人体；1：工业低温；2：工业高温。
	int	tffStd;					//时域滤波标准差，控制时域滤波程度，受响应率影响很大，具体产品具体设置。
	int	vStripeWinWidth;		//越小，高频图像含噪越少，去条纹更浅越大，高频图像含噪越多，去条纹更深
	int	vStripeStd;				//灰度标准差越大，去条纹纹作用越明显
	int	vStripeWeightThresh;	//值过大，参与计算像素多，条纹去不掉值过小，参与计算像素少，反条纹加重
	int	vStripeDetailThresh;	//值过小，计算的条纹幅值越小，条纹去不掉值过大，计算的条纹幅值越大，反条纹加重
	int	hStripeWinWidth;		//越小，高频图像含噪越少，去条纹更浅越大，高频图像含噪越多，去条纹更深
	int	hStripeStd;				//灰度标准差越大，去条纹纹作用越明显
	int	hStripeWeightThresh;	//值过大，参与计算像素多，条纹去不掉值过小，参与计算像素少，反条纹加重
	int	hStripeDetailThresh;	//值过小，计算的条纹幅值越小，条纹去不掉值过大，计算的条纹幅值越大，反条纹加重
	int	rnArithType;			//0为高斯空域滤波， 1为高斯灰度滤波
	int	distStd;				//std越大，滤波效果越强，图像越平滑
	int	grayStd;				//std越大，滤波效果越强，图像越平滑
	int	discardUpratio;			//控制上抛点比例
	int	discardDownratio;		//控制下抛点比例
	int	linearBrightness;		//值越大亮度越大
	int	linearContrast;			//值越大对比度越大
	int	linearRestrainRangethre; //直接影响小动态范围的对比度参数(均匀面抑制)
	int	heqPlatThresh;			//影响图像整体对比度
	int	heqRangeMax;			//值越大，映射范围越大，整体对比度越大
	int	heqMidvalue;			//影响图像整体亮度
	int	iieEnhanceCoef;			//细节增强系数，值越大，高频叠加效果越明显，但噪声越显著
	int	iieGrayStd;				//值越大，平滑效果越明显
	int	iieGaussStd;			//值越大，高频分量越多
	int	iieDetailThr;			//细节阈值，值越大增强效果越明显，但噪声越显著
	int	claheCliplimit;			//阈值越大，图像对比度越强
	int	claheHistMax;			//值越大，直方图能映射的最大值越大
	int	claheBlockWidth;		//列方向分块数量
	int	claheBlockHeight;		//行方向分块数量
	float spLaplaceWeight;		//值越大，锐化效果越明显，但会增加噪声。
	int	gmcType;				//0单Gamma校正；1双Gamma校正。
	float gmcGamma;				//Gamma校正值
	int	adjustbcBright;			//决定Y8图像亮度期望，值越大亮度越大
	int	adjustbcContrast;		//决定Y8图像对比度期望，值越大对比度越大
	int	zoomType;				//0为最邻近插值；1为双线性插值。
	int mixThrLow;				//混合调光低阈值
	int mixThrHigh;				//混合调光高阈值
	unsigned char tffStdS;					//时域滤波标准差，控制时域滤波程度，受响应率影响很大，具体产品具体设置。
	unsigned char vStripeWinWidthS;		//越小，高频图像含噪越少，去条纹更浅越大，高频图像含噪越多，去条纹更深
	unsigned char vStripeStdS;				//灰度标准差越大，去条纹纹作用越明显
	unsigned char vStripeWeightThreshS;	//值过大，参与计算像素多，条纹去不掉值过小，参与计算像素少，反条纹加重
	unsigned char vStripeDetailThreshS;	//值过小，计算的条纹幅值越小，条纹去不掉值过大，计算的条纹幅值越大，反条纹加重
	unsigned char hStripeWinWidthS;		//越小，高频图像含噪越少，去条纹更浅越大，高频图像含噪越多，去条纹更深
	unsigned char hStripeStdS;				//灰度标准差越大，去条纹纹作用越明显
	unsigned char hStripeWeightThreshS;	//值过大，参与计算像素多，条纹去不掉值过小，参与计算像素少，反条纹加重
	unsigned char hStripeDetailThreshS;	//值过小，计算的条纹幅值越小，条纹去不掉值过大，计算的条纹幅值越大，反条纹加重
	unsigned char rnArithTypeS;			//0为高斯空域滤波， 1为高斯灰度滤波
	unsigned char distStdS;				//std越大，滤波效果越强，图像越平滑
	unsigned char grayStdS;				//std越大，滤波效果越强，图像越平滑
	unsigned char discardUpratioS;			//控制上抛点比例
	unsigned char discardDownratioS;		//控制下抛点比例
	unsigned char linearBrightnessS;		//值越大亮度越大
	unsigned char linearContrastS;			//值越大对比度越大
	unsigned char linearRestrainRangethreS; //直接影响小动态范围的对比度参数(均匀面抑制)
	unsigned char heqPlatThreshS;			//影响图像整体对比度
	unsigned char heqRangeMaxS;			//值越大，映射范围越大，整体对比度越大
	unsigned char heqMidvalueS;			//影响图像整体亮度
	unsigned char iieEnhanceCoefS;			//细节增强系数，值越大，高频叠加效果越明显，但噪声越显著
	unsigned char iieGrayStdS;				//值越大，平滑效果越明显
	unsigned char iieGaussStdS;			//值越大，高频分量越多
	unsigned char iieDetailThrS;			//细节阈值，值越大增强效果越明显，但噪声越显著
	unsigned char claheCliplimitS;			//阈值越大，图像对比度越强
	unsigned char claheHistMaxS;			//值越大，直方图能映射的最大值越大
	unsigned char claheBlockWidthS;		//列方向分块数量
	unsigned char claheBlockHeightS;		//行方向分块数量
	unsigned char spLaplaceWeightS;		//值越大，锐化效果越明显，但会增加噪声。
	unsigned char gmcTypeS;				//0单Gamma校正；1双Gamma校正。
	unsigned char gmcGammaS;				//Gamma校正值
	unsigned char adjustbcBrightS;			//决定Y8图像亮度期望，值越大亮度越大
	unsigned char adjustbcContrastS;		//决定Y8图像对比度期望，值越大对比度越大
	unsigned char zoomTypeS;				//0为最邻近插值；1为双线性插值。
	unsigned char mixThrLowS;				//混合调光低温阈值
	unsigned char mixThrHighS;				//混合调光高温阈值
}ITAISPConfig;

typedef struct ITA_CONFIG {
	ITAFunctionConfig* functionConfig;
	int mtConfigCount;
	ITAMTConfig* mtConfig;
	int ispConfigCount;
	ITAISPConfig* ispConfig;
}ITAConfig;

typedef enum ITAShapeType
{
    ITA_SHAPE_TYPE_NONE     = 0,  // 非法形状
    ITA_SHAPE_TYPE_POINT    = 1,  // 点 (一个点)
    ITA_SHAPE_TYPE_LINE     = 2,  // 线段 (线段的两端点)
    ITA_SHAPE_TYPE_TRIANGLE = 3,  // 三角形 (每个顶点的坐标,顺时针方向)
    ITA_SHAPE_TYPE_RETANGE  = 4,  // 矩形 (左上和右下顶点坐标)
    ITA_SHAPE_TYPE_PENTAGON = 5,  // 五边形 (每个顶点的坐标,顺时针方向)
    ITA_SHAPE_TYPE_HEXAGON  = 6,  // 六边形 (每个顶点的坐标,顺时针方向)
    ITA_SHAPE_TYPE_POLYGON  = 7,  // 手绘多边形 (每个顶点的坐标,顺时针方向)
    ITA_SHAPE_TYPE_CIRCLE   = 8,  // 圆形 (传入外接矩形的左上和右下顶点坐标)
    ITA_SHAPE_TYPE_ELIPSE   = 9,  // 椭圆 (水平放置的椭圆,传入椭圆外接矩形的左上和右下顶点坐标)
    ITA_SHAPE_TYPE_CONTOUR  = 10  // 手绘多边形 (顶点坐标为轮廓上的点)

} ITA_SHAPE_TYPE;

typedef struct ITA_Shape
{
    ITAPoint* point;            // 一个多边形对象的顶点坐标列表(顶点坐标以顺时针方向排列)
    int            size;        // 一个多边形对象的顶点列表size
    ITA_SHAPE_TYPE type;        // 多边形对象的类型
} ITAShape;

typedef struct ITA_RangeMask
{
	unsigned char* maskArray;	// 一个分析对象的掩码数组
	int size;					// 一个分析对象的掩码数组的长度
}ITARangeMask;


typedef struct ITA_AnalysisTemp {
	float maxVal;
	float minVal;
	float avgVal;
} ITAAnalysisTemp;

typedef struct ITA_TempArray
{
	ITAAnalysisTemp* temp;
	int              size;
} ITATempArray;

typedef struct ITA_AnalysisY16Info {
	short maxY16;
	short minY16;
	short avgY16;
}ITAAnalysisY16Info;

typedef struct ITA_AnalysisY16Array {
	ITAAnalysisY16Info* y16;
	int                size;
} ITAY16InfoArray;

#endif // !ITA_DTD_H