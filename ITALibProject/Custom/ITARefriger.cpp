/*************************************************************
Copyright (C), 2022--2023, Wuhan Guide Sensmart Tech Co., Ltd
File name   : ITARefriger.cpp
Author      : Guide Sensmart RD. wangyan
Version     : 1.0
Date        : 2022/10/29
Description : Temperature measurement model of refrigeration detector.
*************************************************************/
#include "../ITARefriger.h"
#include "RefrigerationMeasureCore.h"

/*模块设计原则：功能独立，不受其它因素影响时，单独作为一个模块。否则需共用一个ITA实例。*/
ITA_API HANDLE_T ITA_RefrigerInit(int w, int h, short *pusCurve, int curveLength)
{
#ifdef REFRIGER_EDITION
	if (w <= 0 || h <= 0 || !pusCurve || curveLength <= 0)
	{
		return 0;
	}
	RefrigerationMeasureCore *p = new RefrigerationMeasureCore(w, h, pusCurve, curveLength);
	return (HANDLE_T)p;
#else
	return 0;
#endif
}

ITA_API void ITA_RefrigerUninit(HANDLE_T instance)
{
#ifdef REFRIGER_EDITION
	if (!instance)
		return;
	RefrigerationMeasureCore *p = (RefrigerationMeasureCore *)instance;
	delete p;
#endif
}

ITA_API ITA_RESULT ITA_RefrigerSetParams(HANDLE_T instance, ITARefrigerMeasureParam params)
{
#ifdef REFRIGER_EDITION
	CHECK_NULL_POINTER(instance);
	RefrigerationMeasureCore *p = (RefrigerationMeasureCore *)instance;
	p->setParams(params);
	return ITA_OK;
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_RefrigerGetParams(HANDLE_T instance, ITARefrigerMeasureParam *params)
{
#ifdef REFRIGER_EDITION
	CHECK_NULL_POINTER(instance);
	CHECK_NULL_POINTER(params);
	RefrigerationMeasureCore *p = (RefrigerationMeasureCore *)instance;
	p->getParams(params);
	return ITA_OK;
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_RefrigerSetCurve(HANDLE_T instance, short *pusCurve, int curveLength)
{
#ifdef REFRIGER_EDITION
	CHECK_NULL_POINTER(instance);
	CHECK_NULL_POINTER(pusCurve);
	if (curveLength <= 0)
		return ITA_ILLEGAL_PAPAM_ERR;
	RefrigerationMeasureCore *p = (RefrigerationMeasureCore *)instance;
	p->setCurve(pusCurve, curveLength);
	return ITA_OK;
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_RefrigerCalcTemp(HANDLE_T instance, short y16value, float *temp)
{
#ifdef REFRIGER_EDITION
	CHECK_NULL_POINTER(instance);
	CHECK_NULL_POINTER(temp);
	RefrigerationMeasureCore *p = (RefrigerationMeasureCore *)instance;
	float myTemp = 0;
	p->GetTempByY16(myTemp, y16value);
	*temp = myTemp;
	return ITA_OK;
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_RefrigerCalcY16(HANDLE_T instance, float temp, short *y16value)
{
#ifdef REFRIGER_EDITION
	CHECK_NULL_POINTER(instance);
	CHECK_NULL_POINTER(y16value);
	RefrigerationMeasureCore *p = (RefrigerationMeasureCore *)instance;
	short myValue = 0;
	p->CalY16ByTemp(myValue, temp);
	*y16value = myValue;
	return ITA_OK;
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_RefrigerCalcTempMatrix(HANDLE_T instance, short* y16Array, int y16W, int y16H,
	int x, int y, int w, int h, float distance, float emiss, ITA_MATRIX_TYPE type, float *tempMatrix, int matrixSize)
{
#ifdef REFRIGER_EDITION
	CHECK_NULL_POINTER(instance);
	CHECK_NULL_POINTER(y16Array);
	CHECK_NULL_POINTER(tempMatrix);
	if (y16W <= 0 || y16H <= 0 || x < 0 || y < 0 || w <= 0 || h <= 0 || distance <= 0 || matrixSize <= 0 || w > y16W || h > y16H)
	{
		return ITA_ILLEGAL_PAPAM_ERR;
	}
	RefrigerationMeasureCore *p = (RefrigerationMeasureCore *)instance;
	if (ITA_FAST_MATRIX == type)
		p->GetTempMatrixFast(y16Array, tempMatrix, y16W, y16H, x, y, w, h, distance, emiss);
	else
		p->GetTempMatrix(y16Array, tempMatrix, y16W, y16H, x, y, w, h, distance, emiss);
	return ITA_OK;
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_RefrigerCalcY16Matrix(HANDLE_T instance, float *tempMatrix, int w, int h, short *y16Matrix, int matrixSize)
{
#ifdef REFRIGER_EDITION
	CHECK_NULL_POINTER(instance);
	CHECK_NULL_POINTER(tempMatrix);
	CHECK_NULL_POINTER(y16Matrix);
	RefrigerationMeasureCore *p = (RefrigerationMeasureCore *)instance;
	if (w <= 0 || h <= 0 || matrixSize <= 0)
	{
		return ITA_ILLEGAL_PAPAM_ERR;
	}
	p->CalY16MatrixByTempMatrix(tempMatrix, y16Matrix, w, h);
	return ITA_OK;
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_RefrigerIsChangeRange(HANDLE_T instance, short *pSrc, int width, int height, ITA_RANGE range,
	float areaTh1, float areaTh2, int low2high, int high2low, int *isChange)
{
#ifdef REFRIGER_EDITION
	CHECK_NULL_POINTER(instance);
	CHECK_NULL_POINTER(pSrc);
	CHECK_NULL_POINTER(isChange);
	if (width <= 0 || height <= 0)
	{
		return ITA_ILLEGAL_PAPAM_ERR;
	}
	RefrigerationMeasureCore *p = (RefrigerationMeasureCore *)instance;
	*isChange = p->autoChangeRange(pSrc, width, height, range, areaTh1, areaTh2, low2high, high2low);
	return ITA_OK;
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_RefrigerDebugger(HANDLE_T instance, ITA_DEBUG_TYPE type, void* param)
{
#ifdef REFRIGER_EDITION
	CHECK_NULL_POINTER(instance);
	CHECK_NULL_POINTER(param);
	RefrigerationMeasureCore* p = (RefrigerationMeasureCore*)instance;
	return p->debugger(type, param);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_RefrigerSetMatrixData(HANDLE_T instance, short* stationaryB, short* kMatrix, int length)
{
#ifdef REFRIGER_EDITION
	CHECK_NULL_POINTER(instance);
	CHECK_NULL_POINTER(stationaryB);
	CHECK_NULL_POINTER(kMatrix);
	RefrigerationMeasureCore* p = (RefrigerationMeasureCore*)instance;
	p->setMatrixData(stationaryB, kMatrix, length);
	return ITA_OK;
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_RefrigerY16MatrixConvert(HANDLE_T instance, short* srcY16, short* dstY16, short averageB, short* realB, short* stationaryB, short* kMatrix, int width, int height, bool mode)
{
#ifdef REFRIGER_EDITION
	CHECK_NULL_POINTER(instance);
	CHECK_NULL_POINTER(srcY16);
	CHECK_NULL_POINTER(dstY16);
	CHECK_NULL_POINTER(realB);
	CHECK_NULL_POINTER(stationaryB);
	CHECK_NULL_POINTER(kMatrix);
	RefrigerationMeasureCore* p = (RefrigerationMeasureCore*)instance;
	return p->Y16MatrixTransUnderDiffB(srcY16, dstY16, averageB, realB, stationaryB, kMatrix,height, width, mode);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_RefrigerY16SingleConvert(HANDLE_T instance, short srcY16, short* dstY16, short stationaryB, short averageB, short realB, short k, bool mode)
{
#ifdef REFRIGER_EDITION
	CHECK_NULL_POINTER(instance);
	CHECK_NULL_POINTER(dstY16);
	RefrigerationMeasureCore* p = (RefrigerationMeasureCore*)instance;
	*dstY16 = p->Y16TransUnderDiffB(srcY16, stationaryB, averageB, realB, k, mode);
	return ITA_OK;
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}