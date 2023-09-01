/*************************************************************
Copyright (C), 2021--2022, Wuhan Guide Sensmart Tech Co., Ltd
File name   : ITA.h
Author      : Guide Sensmart RD. wangyan
Version     : 1.0
Date        : 2021/11/9
Description : ITA SDK interface definition.
*************************************************************/
#ifndef ITA_LIB_H
#define ITA_LIB_H

#ifdef __cplusplus
#    define ITA_LIB_START  extern "C" {
#    define ITA_LIB_END    };
#else
#    define ITA_LIB_START
#    define ITA_LIB_END
#endif

ITA_LIB_START

#include "ITADTD.h"

/**
* @brief:	初始化一个伊塔（ITA）库实例。ITA（image and temperature measurement algorithm）。
* @param:	ITA_MODE mode	模式，X16（探测器输出的原始数据）或Y16（前端图像算法处理过的数据）。
* @param:	ITA_RANGE range	范围，分人体测温、工业测温常温段和工业测温高温段。初始化时由用户指定测温范围，也可以实时切换测温范围。
* @param:	int width		分辨率 宽
* @param:	int height		分辨率 高
* @param:	int cameraID	camera id由用户输入，用来区分设备。支持同时打开多台设备。
* @return:	成功，返回实例句柄。失败，返回NULL。
* @see ITA_SetMeasureRange，实时切换测温范围。ITA_ShutterControl，cameraID用来标识设备，传递给HAL接口。
**/
ITA_API HANDLE_T ITA_Init(ITA_MODE mode, ITA_RANGE range, int width, int height, int cameraID);

/**
* @brief:	释放一个伊塔（ITA）库实例资源。
* @param:	HANDLE_T instance	ITA实例。
* @return:	无。
**/
ITA_API void ITA_Uninit(HANDLE_T instance);

/**
* @brief:	注册机制。在ITA_Init之后调用。用于满足不同产品的定制需求，增强可扩展性。
* @param:	HANDLE_T instance		ITA实例。
* @param:	ITARegistry *registry	用户注册函数。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_Register(HANDLE_T instance, ITARegistry *registry);

/**
* @brief:  切换测温范围。
* @param:  HANDLE_T instance	ITA实例。
* @param:  ITA_RANGE range		范围，分人体测温、工业测温常温段和工业测温高温段。
* @return: 返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_SetMeasureRange(HANDLE_T instance, ITA_RANGE range);

/**
* @brief:	在初始化配置探测器之前，用户根据需求设置探测器参数。不支持动态设置，务必在ITA_ConfigureDetector之前调用。
* @param:	HANDLE_T instance			ITA实例。
* @param:	ITA_DC_TYPE type			参数类型。
* @param:	void *param					参数值，传地址。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_DetectorControl(HANDLE_T instance, ITA_DC_TYPE type, void *param);

/**
* @brief:	设置探测器时钟参数，配置探测器。在ITA_Register之后调用。
* @param:	HANDLE_T instance		ITA实例。
* @param:	int clock				探测器时钟参数，单位Hz。例如12000000Hz。256模组支持12M和6M两种时钟。120模组时钟通常是5M，不超过8M。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_ConfigureDetector(HANDLE_T instance, int clock);

/**
* @brief:	获取图像信息。
* @param:	HANDLE_T instance		ITA实例。
* @param:	ITAImgInfo *pInfo		图像信息，输出参数。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
* @note:	
**/
ITA_API ITA_RESULT ITA_Prepare(HANDLE_T instance, ITAImgInfo *pInfo);

/**
* @brief:	图像信号处理。输入X或Y，输出图像和能量数据。当接口返回成功时，此帧图像可以显示。
* @param:	HANDLE_T instance		ITA实例。
* @param:	unsigned char *srcData	输入的源数据，X16或者Y16。内存由用户申请和释放。
* @param:	int srcSize				源数据长度，单位Byte。
* @param:	ITAISPResult *pResult	输出参数。imgDst和y16Data使用SDK申请的内存，不需要用户申请。用户不要修改pResult的值。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
            失败可能是以下几种情况：ITA_SHUTTER_CLOSING、ITA_FIRST_NUC_NOT_FINISH、ITA_SKIP_FRAME等。
* @note:	输入参数srcData的内存由用户申请和释放。输出参数pResult的内存由ITA库管理，用户不要修改pResult的值。
**/
ITA_API ITA_RESULT ITA_ISP(HANDLE_T instance, unsigned char *srcData, int srcSize, ITAISPResult *pResult);

/**
* @brief:	根据Y16值计算温度。
* @param:	HANDLE_T instance		ITA实例。
* @param:	short y16				Y16值。
* @param:	float *surfaceTemp		输出表面温度。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_MeasureTempPoint(HANDLE_T instance, short y16, float *surfaceTemp);

/**
* @brief:	计算温度矩阵。
* @param:	HANDLE_T instance		ITA实例。
* @param:	short* y16Array			Y16矩阵。如果有旋转和翻转，那么这是旋转和翻转后的Y16数据。
* @param:	y16W,y16H				Y16矩阵分辨率。如果有旋转和翻转，那么这是旋转和翻转后的分辨率。
* @param:	x,y,w,h					目标矩形区域。如果设置旋转，那么注意不要越界，否则会报错。
* @param:	float distance			目标距离，单位：米。当前计算过程有效，完成后将恢复计算前的值。
* @param:	ITA_MATRIX_TYPE type	矩阵类型
* @param:	float *tempMatrix		输出目标区域的温度矩阵。
* @param:	int matrixSize			矩阵缓存大小，不能小于目标区域w*h。单位float。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_MeasureTempMatrix(HANDLE_T instance, short* y16Array, int y16W, int y16H,
				int x, int y, int w, int h, float distance, ITA_MATRIX_TYPE type, float *tempMatrix, int matrixSize);

/**
* @brief:	基本控制。
* @param:	HANDLE_T instance			ITA实例。
* @param:	ITA_BC_TYPE type			参数类型。
* @param:	void *param					参数值，传地址。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_BaseControl(HANDLE_T instance, ITA_BC_TYPE type, void *param);

/**
* @brief:	设置测温参数。
* @param:	HANDLE_T instance			ITA实例。
* @param:	ITA_MC_TYPE type			参数类型。
* @param:	void *param					参数值，传地址。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_MeasureControl(HANDLE_T instance, ITA_MC_TYPE type, void *param);

/**
* @brief:	设置成像参数。
* @param:	HANDLE_T instance			ITA实例。
* @param:	ITA_IC_TYPE type		    参数类型。
* @param:	void *param					参数值，传地址。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_ImageControl(HANDLE_T instance, ITA_IC_TYPE type, void *param);

/**
* @brief:	获取成像和测温参数信息。
* @param:	HANDLE_T instance		ITA实例。
* @param:	ITAParamInfo *info		输出参数信息。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_GetParamInfo(HANDLE_T instance, ITAParamInfo *info);

/**
* @brief:	调试器，设置调试参数。
* @param:	HANDLE_T instance			ITA实例。
* @param:	ITA_DEBUG_TYPE type		    参数类型。
* @param:	void *param					参数值，传地址。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_Debugger(HANDLE_T instance, ITA_DEBUG_TYPE type, void *param);

/**
* @brief:	用户自定义伪彩数据
* @param:	HANDLE_T instance			ITA实例。
* @param:	ITA_PALETTE_OP_TYPE type	用户自定义伪彩操作类型。
* @param:	void *param					参数值，传地址。类型参考ITA_PALETTE_OP_TYPE注释。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_CustomPalette(HANDLE_T instance, ITA_PALETTE_OP_TYPE type, void *param);

/**
* @brief:	根据输入的Y8和伪彩生成红外图像，输出的图像格式为RGB888。
* @param:	HANDLE_T instance				ITA实例。
* @param:	unsigned char *pY8				输入的y8
* @param:	int w							y8宽
* @param:	int h							y8高
* @param:	unsigned char *pPalette			伪彩数据，RGBA格式。每条伪彩数据长度256*4B。
* @param:	int paletteLen					伪彩长度
* @param:   unsigned char *pImage			输出的图像数据
* @param:   int imgLen						图像缓存大小
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_GetColorImage(HANDLE_T instance, unsigned char *pY8, int w, int h, unsigned char *pPalette, int paletteLen, unsigned char *pImage, int imgLen);

/**
* @brief:	根据温度反查y16。
* @param:	HANDLE_T instance		ITA实例。
* @param:	float surfaceTemp		输入温度。
* @param:	short *y16				输出Y16值。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_CalcY16ByTemp(HANDLE_T instance, float surfaceTemp, short *y16);

/**
* @brief:	根据温度矩阵反查y16矩阵。
* @param:	HANDLE_T instance		ITA实例。
* @param:	float *tempMatrix		输入温度矩阵。
* @param:	int w					宽
* @param:	int h					高
* @param:	short *y16Matrix		输出Y16矩阵。
* @param:	int matrixSize			矩阵缓存大小
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_CalcY16MatrixByTemp(HANDLE_T instance, float *tempMatrix, int w, int h, short *y16Matrix, int matrixSize);

/**
* @brief:  查询版本号和版本功能信息。不同的版本包含的功能可能存在差异。
* @param:  HANDLE_T instance		ITA实例。
* @param:  ITA_VERSION_INFO *pVersion
* @return: 返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_Version(HANDLE_T instance, ITAVersionInfo *pVersion);

/**
* @brief: 用户更新参数行数据，ITA解析快门状态值、温传值和档位等信息。在ITA_ISP之前调用。
* @param: HANDLE_T instance					ITA实例。
* @param: unsigned char *paramLine			参数行数据。
* @param: int len							数据长度。
* @return: 返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
* @note:   Y16模式下，自动快门、NUC策略和自动切档一般在下位机上已实现并通过参数行传递（FPGA除外）。
* 用户应该调用ITA_UpdateParamLine传递参数行给ITA。此种情况下一般用户可以通过控制协议完成手动快门和NUC，不必调用ITA接口。
**/
ITA_API ITA_RESULT ITA_UpdateParamLine(HANDLE_T instance, unsigned char *paramLine, int len);

/**
* @brief:	用于人体测温，根据体表温度计算人体温度。
* @param:	HANDLE_T instance		ITA实例。
* @param:	float surfaceTemp		输入表面温度。
* @param:	float envirTemp			环境温度。
* @param:	float *bodyTemp			输出人体温度。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
* @note 通过ITA_GET_ENVIRON获取内部计算的环温。注意受整机散热方面的影响，内部计算的环温与实际值有偏差，
* 一般需要减去偏移量得到实际环温。具体的偏差值由用户测试多台整机后得到。
**/
ITA_API ITA_RESULT ITA_MeasureBody(HANDLE_T instance, float surfaceTemp, float envirTemp, float *bodyTemp);

/**
* @brief:	设置调光方式。
* @param:	HANDLE_T instance			ITA实例。
* @param:	ITA_DRT_TYPE type			调光方式。
* @param:	ITADRTParam *param		    手动调光参数。其它调光方式传NULL。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_DimmingType(HANDLE_T instance, ITA_DRT_TYPE type, ITADRTParam *param);

/**
* @brief:  等温线。
* @param:  HANDLE_T instance			ITA实例。
* @param:  ITA_EQUAL_LINE_TYPE type     等温线类型
* @param:  ITAEqualLineParam param		等温线参数				
* @return: 返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_EqualLine(HANDLE_T instance, ITA_EQUAL_LINE_TYPE type, ITAEqualLineParam param);

/*以下是校坏点接口*/

/**
* @brief: 校坏点。
* @param: HANDLE_T instance			ITA实例。
* @param: ITA_CORRECT_TYPE type		参数类型。
* @param: void *param				参数值，传地址。
* @return: 返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_CorrectBadPoints(HANDLE_T instance, ITA_CORRECT_TYPE type, void *param);

/*以下是自动校温接口*/
/**
* @brief:  开始采集数据，过程约1分钟左右，完成后通过回调函数通知。本接口使用非阻塞方式，立即返回。
* @param:  HANDLE_T instance            ITA实例。
* @param:  ITACollectParam collectParam 采集参数
* @param:  ITACollectFinish cb          采集完成后通知
* @param:  void *userParam              用户参数，在ITACollectFinish时传给用户。
* @return: 返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_StartCollect(HANDLE_T instance, ITACollectParam collectParam, ITACollectFinish cb, void *userParam);

/**
* @brief: 开始自动校温(非TEC)。注意校准的是当前测温范围的参数。
* @param: HANDLE_T instance					ITA实例。
* @param: float *kf							输出校温参数。
* @param: float *b							输出校温参数。
* @param: ITACalibrateResult *result		输出校温结果。
* @return: 返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_StartCalibrate(HANDLE_T instance, float *kf, float *b, ITACalibrateResult *result);


/**
* @brief:  手动校温。
* @param:  HANDLE_T instance			ITA实例。
* @param:  ITA_RANGE range              测温范围。
* @param:  ITA_CALIBRATE_TYPE type		校温参数类型。
* @param:  float p						参数值
* @return: 返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_CalibrateByUser(HANDLE_T instance, ITA_RANGE range, ITA_CALIBRATE_TYPE type, float p);

/**
* @brief:  模组开始采集K，完成后通过回调函数通知。本接口使用非阻塞方式，立即返回。
* @param:  HANDLE_T instance		ITA实例。
* @param:  int gear     			焦温档位，从0开始。焦温档位数量通过ITAVersionInfo查询。
* @param:  float blackTemp			黑体温度
* @param:  ITACollectBaseFinish cb      采集完成后通知
* @param:  void *userParam          用户参数，在ITACollectFinish时传给用户。
* @return: 返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_CollectK(HANDLE_T instance, int gear, float blackTemp, ITACollectBaseFinish cb, void *userParam);

/**
* @brief:  模组保存K，ITA_CollectK采集完成后再保存。
* @param:  HANDLE_T instance		ITA实例。
* @param:  int gear     			焦温档位，从0开始。焦温档位数量通过ITAVersionInfo查询。
* @param:  ITACollectBaseFinish cb	采集完成后通知
* @param:  void* param				用户参数，在ITACollectFinish时传给用户。
* @return: 返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_SaveK(HANDLE_T instance, int gear, ITACollectBaseFinish cb,void* param);

/**
* @brief:	图像信号处理。输入X或Y，输出图像、能量和y8数据。当接口返回成功时，此帧图像可以显示。
* @param:	HANDLE_T instance		ITA实例。
* @param:	unsigned char *srcData	输入的源数据，X16或者Y16。内存由用户申请和释放。
* @param:	int srcSize				源数据长度，单位Byte。
* @param:	ITAISPResult *pResult	输出参数。imgDst和y16Data使用SDK申请的内存，不需要用户申请。用户不要修改pResult的值。
* @param:	unsigned char *y8Data	y8数据缓存地址。内存由用户申请和释放。
* @param:	int y8Size				y8数据缓存大小，单位Byte。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
失败可能是以下几种情况：ITA_SHUTTER_CLOSING、ITA_FIRST_NUC_NOT_FINISH、ITA_SKIP_FRAME等。
* @note:	输入参数srcData的内存由用户申请和释放。y8Data的内存也由用户管理。输出参数pResult的内存由ITA库管理，用户不要修改pResult的值。
**/
ITA_API ITA_RESULT ITA_ISP_Y8(HANDLE_T instance, unsigned char *srcData, int srcSize, ITAISPResult *pResult, unsigned char *y8Data, int y8Size);

/**
* @brief:	增加后拦截器，主要用于Android平台线程中通过postHandle通知应用层释放资源。
* @param:	HANDLE_T instance	 	实例句柄。
* @param:	PostHandle postHandle	后处理函数
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
* @note:
**/
ITA_API ITA_RESULT ITA_AddInterceptor(HANDLE_T instance, PostHandle postHandle);

/**
* @brief:	设置手持和机芯产品测温参数信息。
* @param:	HANDLE_T instance		ITA实例。
* @param:	ITATECInfo info		    输入参数信息。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_SetTECInfo(HANDLE_T instance, ITATECInfo info);

/**
* @brief:	获取手持和机芯产品测温参数信息。
* @param:	HANDLE_T instance		ITA实例。
* @param:	ITATECInfo *pInfo		输出参数信息。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_GetTECInfo(HANDLE_T instance, ITATECInfo *pInfo);

/**
* @brief:	设置非TEC产品测温参数信息。
* @param:	HANDLE_T instance		ITA实例。
* @param:	ITAMeasureInfo info		输入参数信息。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_SetMeasureInfo(HANDLE_T instance, ITAMeasureInfo info);

/**
* @brief:	获取非TEC产品测温参数信息。
* @param:	HANDLE_T instance		ITA实例。
* @param:	ITAMeasureInfo *Info	输出参数信息。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_GetMeasureInfo(HANDLE_T instance, ITAMeasureInfo* info);

/**
* @brief:	用于离线测温（红外图片分析）功能。用户获取测温参数和曲线数据后，保存数据。
* @param:	HANDLE_T instance		ITA实例
* @param:	ITASceneParam *param	输出测温参数和曲线数据
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_GetScene(HANDLE_T instance, ITASceneParam *param);

/**
* @brief:	用于离线测温（红外图片分析）功能。用户创建一个新ITA实例，设置测温参数和曲线数据后，开始测温。
* @param:	HANDLE_T instance		ITA实例
* @param:	ITASceneParam param		输入测温参数和曲线数据
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_SetScene(HANDLE_T instance, ITASceneParam param);

/**
* @brief:	Y16放大功能。计算超分辨率温度矩阵时使用。最大支持放大20倍。
* @param:	HANDLE_T instance		ITA实例。
* @param:	short* srcY16			源Y16矩阵。内存由用户管理。
* @param:	srcW, srcH				源Y16矩阵分辨率。
* @param:	short* dstY16			目标Y16矩阵。内存由用户管理。
* @param:	float times 			放大倍数。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_ZoomY16(HANDLE_T instance, short* srcY16, int srcW, int srcH,
				short* dstY16, float times);

/**
* @brief:	判断是否切换测温范围。建议用户至少间隔1s调用监控一次。
* @param:	HANDLE_T instance		ITA实例。
* @param:	short *pSrc				源Y16图像。内存由用户管理。
* @param:	width, height			源Y16图像分辨率。
* @param:	ITA_RANGE range			当前测温范围。
* @param:	areaTh1, areaTh2		阈值条件。范围请分别参考73/10000和9985/10000。
* @param:	low2high, high2low 		阈值条件。范围请分别参考140和130。
* @param:	int *isChange 		    输出参数，1：切换测温范围，0：保持不变。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_IsChangeRange(HANDLE_T instance, short *pSrc, int width, int height, ITA_RANGE range,
	            float areaTh1, float areaTh2, int low2high, int high2low, int *isChange);

/**
* @brief:	判断多挡位是否切换测温范围。建议用户至少间隔1s调用监控一次。
* @param:	HANDLE_T instance		ITA实例。
* @param:	short *pSrc				源Y16图像。内存由用户管理。
* @param:	width, height			源Y16图像分辨率。
* @param:	ITA_RANGE range			当前测温范围。
* @param:	areaTh1, areaTh2		阈值条件。范围请分别参考73/10000和9985/10000。
* @param:	low2mid					阈值条件。低温档切高温档温度阈值140。
* @param:	mid2low 				阈值条件。高温档切低温档温度阈值130。
* @param:	mid2high 				阈值条件。低温档切高温档温度阈值600。
* @param:	high2mid 				阈值条件。高温档切低温档温度阈值580。
* @param:	areaTh1, areaTh2		阈值条件。范围分别参考0.0020，0.9995。
* @param:	int *isChange 		    输出参数，-1表示向下切档，0表示不切档，1表示向上切档。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_IsMultiChangeRange(HANDLE_T instance, short* pSrc, int width, int height, ITA_RANGE range,
	float areaTh1, float areaTh2, int low2mid, int mid2low, int mid2high, int high2mid, int* isChange);

/**
* @brief:	超分放大,输入Y8/Y16，	实现图像数据的超分放大功能。最大支持放大8倍。
* @param:	HANDLE_T instance	 	实例句柄。
* @param:	void* dst				输出的目的数据。数据类型为short或者unsigned char。内存由用户申请和释放。
* @param:	void* src				输入的源数据Y8/Y16。数据类型为short或者unsigned char。内存由用户申请和释放。
* @param:	int w					输入的源图像数据的宽度。
* @param:	int h					输入的源图像数据的高度。
* @param:	int zoom				超分放大倍数。
* @param:	ITA_DATA_TYPE dataType	指定处理的数据类型。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
* @note:
**/
ITA_API ITA_RESULT ITA_SrBicubic(HANDLE_T instance, void* dst, void* src, int w, int h, int zoom, ITA_DATA_TYPE dataType);

/**
* @brief:	根据输入的Y8和伪彩号生成指定格式的红外图像，旋转角度,图像格式,放大倍数,放大类型,和伪彩号由ITA_ImageControl接口指定
* @param:	HANDLE_T instance	 			实例句柄。
* @param:	unsigned char* src				输入的Y8图像数据。内存由用户申请和释放。
* @param:	int srcW						输入的Y8图像数据的宽。
* @param:	int srcH						输入的Y8图像数据的高。
* @param:	unsigned char* dst				输出的带伪彩图像数据,内存由用户申请和释放。
* @param:	int dstW					    输出的Y8图像数据的宽度。
* @param:	int dstH					    输出的Y8图像数据的高度。
* @param:	int dataLen					    输出的Y8图像数据的数据长度。
* @param:	ITA_Y8_DATA_MODE datamode		输入Y8数据来源，ITA_Y8_NORMAL y8取值范围0-255，ITA_Y8_XINSHENG:16-235。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
* @note:
**/
ITA_API ITA_RESULT ITA_ConvertY8(HANDLE_T instance, const unsigned char* src, int srcW, int srcH, unsigned char* dst, int dstW, int dstH,int dateLen, ITA_Y8_DATA_MODE datamode = ITA_Y8_NORMAL);

/**
* @brief:	去锅盖操作。
* @param:	HANDLE_T instance			ITA实例。
* @param:	ITA_GUOGAI_OP_TYPE type		用户去锅盖的操作类型。
* @param:	void *param					参数值，传地址。类型参考ITA_POTCOVER_OP_TYPE的注释。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_PotCoverRestrain(HANDLE_T instance, ITA_POTCOVER_OP_TYPE type, void* para);

/**
* @brief:  开始复核温度，完成后通过回调函数通知。本接口使用非阻塞方式，立即返回。
* @param:  HANDLE_T instance		ITA实例。
* @param:  float temper				黑体温度
* @param:  ITAReviewFinish cb		用户注册的通知函数
* @param:  void* userParam			用户参数
* @return: 返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_StartReview(HANDLE_T instance, float temper, ITAReviewFinish cb, void* userParam);

/**
* @brief:	自动校温,自动计算出新的KF、B1参数
* @param:	HANDLE_T instance									ITA实例。
* @param:	float *pBlackBodyTemp								黑体中心的温度
* @param:	short *pY16											黑体对应的Y16
* @param:	int arrLength										黑体温度数组、Y16数组长度
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/
ITA_API ITA_RESULT ITA_AutoCorrectTemp(HANDLE_T instance, float *pBlackBodyTemp, short *pY16, int arrLength);

/**
* @brief:    增加一个分析对象。
* @param:    HANDLE_T instance           ITA实例。
* @param:    ITAShape* shape             一个数组，包含一个到多个多边形对象。
* @param:    int size                    数组的长度。
* @return:   返回值类型：ITA_RESULT。成功，返回ITA_OK。失败，其他
**/
ITA_API ITA_RESULT ITA_AnalysisAdd(HANDLE_T instance, ITAShape* shape, int size);

/**
* @brief:    获取分析对象区域内的掩码。
* @param:    HANDLE_T instance           ITA实例。
* @param:    ITARangeMask* rangeMask	 返回值：分析对象区域内掩码。区域内值为255,区域外值为0.内存由用户申请。
* @param:    int size					 分析对象区域数组长度。
* @return:   返回值类型：ITA_RESULT。成功，返回ITA_OK。失败，其他
**/
ITA_API ITA_RESULT ITA_AnalysisGetRangeMask(HANDLE_T instance, ITARangeMask* rangeMask,int size);

/**
* @brief:    清空分析对象。
* @param:    OA_HANDLE_T instance            ITA实例。
* @return:   返回值类型：ITA_RESULT。成功，返回ITA_OK。失败，其他
**/
ITA_API ITA_RESULT ITA_AnalysisClear(HANDLE_T instance);

/**
* @brief:    对分析对象内部区域的温度做分类处理,获取分析对象的温度信息(最大值,最小值,平均值)。
* @param:    HANDLE_T instance                 ITA实例。
* @param:    short* y16Data                    Y16数据
* @param:	 int width  int height			   Y16图像分辨率。
* @param:	 ITAAnalysisTemp* analysisTemp	   输出参数:分析对象的温度数组，内存由用户申请和释放
* @param:	 int size						   分析对象的温度数组(analysisTemp)的大小(size个数必须与添加的分析对象个数相同)
* @return:   返回值类型：ITA_RESULT。成功，返回ITA_OK。失败，其他
**/
ITA_API ITA_RESULT ITA_AnalysisTemp(HANDLE_T instance, short* y16Data, int width, int height, ITAAnalysisTemp* analysisTemp, int size);

/**
* @brief:    对分析对象内部区域的Y16做分类处理,获取分析对象的Y16信息(最大值,最小值,平均值)。
* @param:    HANDLE_T instance							ITA实例。
* @param:    short* y16Data								Y16数据。
* @param:	 int width  int height						Y16图像分辨率。
* @param:    ITAAnalysisY16Info* analysisY16Info        输出参数:处理后的分析对象Y16数组,包含每个分析对象内最大Y16,最小Y16以及平均Y16,内存有用户申请释放
* @param:	 int size									分析对象的Y16信息数组(analysisY16Info)的大小(size个数必须与添加的分析对象个数相同)
* @return:   返回值类型：ITA_RESULT。成功，返回ITA_OK。失败，其他
**/
ITA_API ITA_RESULT ITA_AnalysisY16Info(HANDLE_T instance, short* y16Data, int width, int height, ITAAnalysisY16Info* analysisY16Info, int size);

/**
* @brief:    判断一个点是否在一个分析对象内部。
* @param:    ITAShape* shape             一个分析对象(内部包含一个多边形的多个顶点)
* @param:    ITAPoint point              一个点坐标。
* @param:    int *flag              	 0：表示点不在分析对象内。  1：表示点在分析对象内。
* @return:   返回值类型：ITA_RESULT。成功，返回ITA_OK。失败，其他
**/
ITA_API ITA_RESULT ITA_PointInPolygon(ITAShape* shape, ITAPoint point, int* flag);

ITA_LIB_END

#endif // !ITA_LIB_H

