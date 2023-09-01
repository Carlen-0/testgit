/*************************************************************
Copyright (C), 2021--2022, Wuhan Guide Sensmart Tech Co., Ltd
File name   : ITA.cpp
Author      : Guide Sensmart RD. wangyan
Version     : 1.0
Date        : 2021/11/9
Description : ITA SDK interface implementation.
*************************************************************/

#include "../ITA.h"
#include <string>
#include "ITAWrapper.h"

ITA_API HANDLE_T ITA_Init(ITA_MODE mode, ITA_RANGE range, int width, int height, int cameraID)
{
#ifdef ULTIMATE_EDITION
	if (width <= 0 || height <= 0)
	{
		return 0;
	}
	ITAWrapper *p = new ITAWrapper(mode, range, width, height, cameraID);
	return (HANDLE_T)p;
#else
	return 0;
#endif
}

ITA_API void ITA_Uninit(HANDLE_T instance)
{
#ifdef ULTIMATE_EDITION
	if (!instance)
		return;
	ITAWrapper *p = (ITAWrapper *)instance;
	delete p;
#endif
}

ITA_API ITA_RESULT ITA_Register(HANDLE_T instance, ITARegistry * registry)
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	ITAWrapper *p = (ITAWrapper *)instance;
	return p->setRegister(registry);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_SetMeasureRange(HANDLE_T instance, ITA_RANGE range)
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	ITAWrapper *p = (ITAWrapper *)instance;
	return p->setMeasureRange(range);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_DetectorControl(HANDLE_T instance, ITA_DC_TYPE type, void * param)
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	ITAWrapper *p = (ITAWrapper *)instance;
	return p->detectorControl(type, param);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_ConfigureDetector(HANDLE_T instance, int clock)
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	ITAWrapper *p = (ITAWrapper *)instance;
	return p->configureDetector(clock);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_Prepare(HANDLE_T instance, ITAImgInfo * pInfo)
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	ITAWrapper *p = (ITAWrapper *)instance;
	return p->prepare(pInfo);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_ISP(HANDLE_T instance, unsigned char * srcData, int srcSize, ITAISPResult * pResult)
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	ITAWrapper *p = (ITAWrapper *)instance;
	return p->doISP(srcData, srcSize, pResult, NULL, 0);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_MeasureTempPoint(HANDLE_T instance, short y16, float * surfaceTemp)
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	ITAWrapper *p = (ITAWrapper *)instance;
	return p->measureTempPoint(y16, surfaceTemp);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_MeasureTempMatrix(HANDLE_T instance, short * y16Array, int y16W, int y16H, 
	int x, int y, int w, int h, float distance, ITA_MATRIX_TYPE type, float *tempMatrix, int matrixSize)
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	if (w <= 0 || h <= 0 || w*h > matrixSize)
	{
		return ITA_ILLEGAL_PAPAM_ERR;
	}
	ITAWrapper *p = (ITAWrapper *)instance;
	return p->measureTempMatrix(y16Array, y16W, y16H, x, y, w, h, distance, type, tempMatrix);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_BaseControl(HANDLE_T instance, ITA_BC_TYPE type, void * param)
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	ITAWrapper *p = (ITAWrapper *)instance;
	return p->baseControl(type, param);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_MeasureControl(HANDLE_T instance, ITA_MC_TYPE type, void * param)
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	ITAWrapper *p = (ITAWrapper *)instance;
	return p->measureControl(type, param);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_ImageControl(HANDLE_T instance, ITA_IC_TYPE type, void * param)
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	ITAWrapper *p = (ITAWrapper *)instance;
	return p->imageControl(type, param);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_GetParamInfo(HANDLE_T instance, ITAParamInfo *info)
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	ITAWrapper *p = (ITAWrapper *)instance;
	return p->getParamInfo(info);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_Debugger(HANDLE_T instance, ITA_DEBUG_TYPE type, void * param)
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	ITAWrapper *p = (ITAWrapper *)instance;
	return p->debugger(type, param);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_CustomPalette(HANDLE_T instance, ITA_PALETTE_OP_TYPE type, void* param)
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	ITAWrapper* pITA = (ITAWrapper*)instance;
	return pITA->customPalette(type, param);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_GetColorImage(HANDLE_T instance, unsigned char *pY8, int w, int h, unsigned char * pPalette, int paletteLen, unsigned char * pImage, int imgLen)
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	ITAWrapper *p = (ITAWrapper *)instance;
	return p->getColorImage(pY8, w, h, pPalette, paletteLen, pImage, imgLen);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_CalcY16ByTemp(HANDLE_T instance, float surfaceTemp, short * y16)
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	ITAWrapper *p = (ITAWrapper *)instance;
	return p->calcY16ByTemp(surfaceTemp, y16);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_CalcY16MatrixByTemp(HANDLE_T instance, float *tempMatrix, int w, int h, short *y16Matrix, int matrixSize)
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	ITAWrapper *p = (ITAWrapper *)instance;
	return p->calcY16MatrixByTemp(tempMatrix, w, h, y16Matrix, matrixSize);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_Version(HANDLE_T instance, ITAVersionInfo * pVersion)
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	ITAWrapper *p = (ITAWrapper *)instance;
	return p->getVersion(pVersion);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_UpdateParamLine(HANDLE_T instance, unsigned char * paramLine, int len)
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	ITAWrapper *p = (ITAWrapper *)instance;
	if (!paramLine)
		return ITA_NULL_PTR_ERR;
	if (len < 240)
		return ITA_ILLEGAL_PAPAM_ERR;
	return p->updateParamLine(paramLine, len);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_MeasureBody(HANDLE_T instance, float surfaceTemp, float envirTemp, float * bodyTemp)
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	ITAWrapper *p = (ITAWrapper *)instance;
	return p->measureBody(surfaceTemp, envirTemp, bodyTemp);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_DimmingType(HANDLE_T instance, ITA_DRT_TYPE type, ITADRTParam *param)
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	ITAWrapper *p = (ITAWrapper *)instance;
	return p->setDimmingType(type, param);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_EqualLine(HANDLE_T instance, ITA_EQUAL_LINE_TYPE type, ITAEqualLineParam param)
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	ITAWrapper *p = (ITAWrapper *)instance;
	return p->setEqualLine(type, param);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_CorrectBadPoints(HANDLE_T instance, ITA_CORRECT_TYPE type, void * param)
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	ITAWrapper *p = (ITAWrapper *)instance;
	return p->correctBadPoints(type, param);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_StartCollect(HANDLE_T instance, ITACollectParam collectParam, ITACollectFinish cb, void * param)
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	ITAWrapper *p = (ITAWrapper *)instance;
	return p->startCollect(collectParam, cb, param);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_StartCalibrate(HANDLE_T instance, float * kf, float * b, ITACalibrateResult *result)
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	ITAWrapper *p = (ITAWrapper *)instance;
	return p->startCalibrate(kf, b, result);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}


/**
* @brief:  开始复核温度，完成后通过回调函数通知。本接口使用非阻塞方式，立即返回。
* @param:  HANDLE_T instance		ITA实例。
* @param:  float distance			距离
* @param:  float temper				黑体温度
* @param:  ITAReviewFinish cb
* @return: 返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
**/

ITA_API ITA_RESULT ITA_StartReview(HANDLE_T instance, float temper, ITAReviewFinish cb, void* userParam)
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	CHECK_NULL_POINTER(cb);
	CHECK_NULL_POINTER(userParam);
	ITAWrapper* pITA = (ITAWrapper*)instance;
	return pITA->startReview(temper, cb, userParam);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_CalibrateByUser(HANDLE_T instance, ITA_RANGE range, ITA_CALIBRATE_TYPE type, float p)
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	ITAWrapper *pITA = (ITAWrapper *)instance;
	return pITA->calibrateByUser(range, type, p);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_ISP_Y8(HANDLE_T instance, unsigned char * srcData, int srcSize, ITAISPResult * pResult, unsigned char * y8Data, int y8Size)
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	ITAWrapper *p = (ITAWrapper *)instance;
	if (!y8Data)
		return ITA_NULL_PTR_ERR;
	if (y8Size <= 0)
		return ITA_ILLEGAL_PAPAM_ERR;
	return p->doISP(srcData, srcSize, pResult, y8Data, y8Size);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_AddInterceptor(HANDLE_T instance, PostHandle postHandle)
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	ITAWrapper *pITA = (ITAWrapper *)instance;
	return pITA->addInterceptor(postHandle);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_SetTECInfo(HANDLE_T instance, ITATECInfo info)
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	ITAWrapper *pITA = (ITAWrapper *)instance;
	if (info.mtType < 0 || info.nGear < 0
		|| info.nAtmosphereTransmittance < 0 || info.nAtmosphereTransmittance > 100
		|| info.fEmiss < 0.01 || info.fEmiss > 1.0
		|| info.fWindowTransmittance < 0 || info.fWindowTransmittance > 1.0
		|| info.fDistance <=0 || info.fDistance > 10000)
	{
		return ITA_ARG_OUT_OF_RANGE;
	}
	return pITA->setTECInfo(info);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_GetTECInfo(HANDLE_T instance, ITATECInfo * pInfo)
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	ITAWrapper *pITA = (ITAWrapper *)instance;
	return pITA->getTECInfo(pInfo);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_SetMeasureInfo(HANDLE_T instance, ITAMeasureInfo info)
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	ITAWrapper* pITA = (ITAWrapper*)instance;
	return pITA->setMeasureInfo(info);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_GetMeasureInfo(HANDLE_T instance, ITAMeasureInfo* info)
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	CHECK_NULL_POINTER(info);
	ITAWrapper* pITA = (ITAWrapper*)instance;
	return pITA->getMeasureInfo(info);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_GetScene(HANDLE_T instance, ITASceneParam *param)
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	ITAWrapper *pITA = (ITAWrapper *)instance;
	return pITA->getScene(param);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_SetScene(HANDLE_T instance, ITASceneParam param)
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	ITAWrapper *pITA = (ITAWrapper *)instance;
	CHECK_NULL_POINTER(param.curveData);
	CHECK_NULL_POINTER(param.mtParam);
	return pITA->setScene(param);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_ZoomY16(HANDLE_T instance, short * srcY16, int srcW, int srcH, short * dstY16, float times)
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	ITAWrapper *pITA = (ITAWrapper *)instance;
	int dstW = int(srcW*times);
	int dstH = int(srcH*times);
	return pITA->zoomY16(srcY16, srcW, srcH, dstY16, dstW, dstH, times);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_IsChangeRange(HANDLE_T instance, short * pSrc, int width, int height, ITA_RANGE range, float areaTh1, float areaTh2, int low2high, int high2low, int * isChange)
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	CHECK_NULL_POINTER(isChange);
	ITAWrapper *pITA = (ITAWrapper *)instance;
	if (!pSrc || !isChange)
		return ITA_NULL_PTR_ERR;
	if (width <= 0 || height <= 0 || range < ITA_HUMAN_BODY || range > ITA_CUSTOM_RANGE3)
	{
		return ITA_ARG_OUT_OF_RANGE;
	}
	return pITA->isChangeRange(pSrc, width, height, range, areaTh1, areaTh2, low2high, high2low, isChange);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_IsMultiChangeRange(HANDLE_T instance, short* pSrc, int width, int height, ITA_RANGE range,
	float areaTh1, float areaTh2, int low2mid, int mid2low, int mid2high, int high2mid, int* isChange)
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	CHECK_NULL_POINTER(isChange);
	ITAWrapper* pITA = (ITAWrapper*)instance;
	if (!pSrc || !isChange)
		return ITA_NULL_PTR_ERR;
	if (width <= 0 || height <= 0 || range < ITA_HUMAN_BODY || range > ITA_CUSTOM_RANGE3)
	{
		return ITA_ARG_OUT_OF_RANGE;
	}
	return pITA->isMultiChangeRange(pSrc, width, height, range, areaTh1, areaTh2, low2mid, mid2low, mid2high, high2mid, isChange);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_SrBicubic(HANDLE_T instance, void* dst, void* src, int w, int h, int zoom, ITA_DATA_TYPE dataType)
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	ITAWrapper* pITA = (ITAWrapper*)instance;
	int wZoom = w*zoom;
	int hZoom = h*zoom;
	return pITA->processBiCubic(dst, src, w, h, wZoom, hZoom, dataType);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_ConvertY8(HANDLE_T instance, const unsigned char* src, int srcW, int srcH, unsigned char* dst, int dstW, int dstH, int dateLen, ITA_Y8_DATA_MODE datamode)
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	CHECK_NULL_POINTER(src);
	CHECK_NULL_POINTER(dst);
	ITAWrapper* pITA = (ITAWrapper*)instance;
	return pITA->convertY8(src, srcW, srcH, dst,dstW,dstH,dateLen, datamode);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_CollectK(HANDLE_T instance, int gear, float blackTemp, ITACollectBaseFinish cb, void* userParam)
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	CHECK_NULL_POINTER(cb);
	ITAWrapper* pITA = (ITAWrapper*)instance;
	return pITA->collectK(gear, blackTemp, cb, userParam);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_SaveK(HANDLE_T instance, int gear, ITACollectBaseFinish cb, void* param)
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	CHECK_NULL_POINTER(cb);
	CHECK_NULL_POINTER(param);
	ITAWrapper* pITA = (ITAWrapper*)instance;
	return pITA->saveK(gear, cb,param);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_PotCoverRestrain(HANDLE_T instance, ITA_POTCOVER_OP_TYPE type, void* para)
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	CHECK_NULL_POINTER(para);
	ITAWrapper* pITA = (ITAWrapper*)instance;
	return pITA->potCoverRestrain(type,para);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_AutoCorrectTemp(HANDLE_T instance, float *pBlackBodyTemp, short *pY16, int arrLength)
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	ITAWrapper* p = (ITAWrapper*)instance;
	return p->AutoCorrectTemp(pBlackBodyTemp, pY16, arrLength);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}
