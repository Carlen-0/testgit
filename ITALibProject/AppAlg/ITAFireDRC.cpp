/*************************************************************
Copyright (C), 2022--2023, Wuhan Guide Sensmart Tech Co., Ltd
File name   : ITAFireDRC.cpp
Author      : Guide Sensmart RD. wangyan
Version     : 1.0
Date        : 2022/10/29
Description : ITA SDK interface implementation for fire imaging.
*************************************************************/
#include "../ITAFireDRC.h"
#include "CIrFireDrc.h"

ITA_API HANDLE_T ITA_FireDRCInit(int w, int h, ITA_PIXEL_FORMAT imgFormat)
{
#ifdef FIRE_DRC_EDITION
	if (w <= 0 || h <= 0)
	{
		return 0;
	}
	CIrFireDrc *p = new CIrFireDrc(w, h, imgFormat);
	return (HANDLE_T)p;
#else
	return 0;
#endif
}

ITA_API void ITA_FireDRCUninit(HANDLE_T instance)
{
#ifdef FIRE_DRC_EDITION
	if (!instance)
		return;
	CIrFireDrc *p = (CIrFireDrc *)instance;
	delete p;
#endif
}

ITA_API ITA_RESULT ITA_FireDRCSetParams(HANDLE_T instance, ITAFireDRCParam params)
{
#ifdef FIRE_DRC_EDITION
	CHECK_NULL_POINTER(instance);
	CIrFireDrc *p = (CIrFireDrc *)instance;
	p->PutDrcPara(params);
	return ITA_OK;
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_FireDRCGetParams(HANDLE_T instance, ITAFireDRCParam *params)
{
#ifdef FIRE_DRC_EDITION
	CHECK_NULL_POINTER(instance);
	CHECK_NULL_POINTER(params);
	CIrFireDrc *p = (CIrFireDrc *)instance;
	p->GetDrcPara(params);
	return ITA_OK;
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_FireDRCGeneral(HANDLE_T instance, short *srcData, unsigned char* dstData, int width, int height)
{
#ifdef FIRE_DRC_EDITION
	CHECK_NULL_POINTER(instance);
	CHECK_NULL_POINTER(srcData);
	CHECK_NULL_POINTER(dstData);
	if (width <= 0 || height <= 0)
		return ITA_ILLEGAL_PAPAM_ERR;
	CIrFireDrc *p = (CIrFireDrc *)instance;
	p->DRCFireLow(srcData, dstData, width, height);
	return ITA_OK;
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_FireDRCSection(HANDLE_T instance, short *srcData, unsigned char* dstData, int width, int height, int upY16, int downY16)
{
#ifdef FIRE_DRC_EDITION
	CHECK_NULL_POINTER(instance);
	CHECK_NULL_POINTER(srcData);
	CHECK_NULL_POINTER(dstData);
	if (width <= 0 || height <= 0)
		return ITA_ILLEGAL_PAPAM_ERR;
	CIrFireDrc *p = (CIrFireDrc *)instance;
	p->DRCFire(srcData, dstData, width, height, upY16, downY16);
	return ITA_OK;
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_CalcROITemperBarHeightSimplify(HANDLE_T instance, int centerY16, ITAY16TempMap map, ITAFireWorkMode workMode, float* barHeight)
{
#ifdef FIRE_DRC_EDITION
	CHECK_NULL_POINTER(instance);
	CHECK_NULL_POINTER(barHeight);
	CIrFireDrc* p = (CIrFireDrc*)instance;
	return p->CalcROITemperBarHeightSimplify(centerY16, map, workMode, barHeight);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_DrcFireAdaptive2DifferentMode(HANDLE_T instance, short* srcY16, unsigned char* dst, int width, int height, int maxY16Temp, int threshY16, ITAFireWorkMode work_mode, ITA_FIRE_PALETTE_TYPE pesudo)
{
#ifdef FIRE_DRC_EDITION
	CHECK_NULL_POINTER(instance);
	CHECK_NULL_POINTER(srcY16);
	CHECK_NULL_POINTER(dst);
	CIrFireDrc* p = (CIrFireDrc*)instance;
	return p->DrcFireAdaptive2DifferentMode(srcY16,dst,width,height, maxY16Temp, threshY16, work_mode, pesudo);
#else

#endif
}