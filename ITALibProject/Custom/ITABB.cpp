/*************************************************************
Copyright (C), 2022--2023, Wuhan Guide Sensmart Tech Co., Ltd
File name   : ITABB.cpp
Author      : Guide Sensmart RD. wangyan
Version     : 1.0
Date        : 2022/10/29
Description : ITA SDK interface implementation.
*************************************************************/
#include "../ITABB.h"
#include "BBMeasureCore.h"

/*模块设计原则：功能独立，不受其它因素影响时，单独作为一个模块。否则需共用一个ITA实例。*/
ITA_API HANDLE_T ITA_BBInit(int w, int h, short *pusCurve, int curveLength)
{
#ifdef BLACKBODY_EDITION
	if (w <= 0 || h <= 0 || !pusCurve || curveLength <= 0)
	{
		return 0;
	}
	BBTempMeasureCore *p = new BBTempMeasureCore(w, h, pusCurve, curveLength);
	return (HANDLE_T)p;
#else
	return 0;
#endif
}

ITA_API void ITA_BBUninit(HANDLE_T instance)
{
#ifdef BLACKBODY_EDITION
	if (!instance)
		return;
	BBTempMeasureCore *p = (BBTempMeasureCore *)instance;
	delete p;
#endif
}

ITA_API ITA_RESULT ITA_BBSetParams(HANDLE_T instance, ITABBMeasureParam params)
{
#ifdef BLACKBODY_EDITION
	CHECK_NULL_POINTER(instance);
	BBTempMeasureCore *p = (BBTempMeasureCore *)instance;
	p->setParams(params);
	return ITA_OK;
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_BBGetParams(HANDLE_T instance, ITABBMeasureParam *params)
{
#ifdef BLACKBODY_EDITION
	CHECK_NULL_POINTER(instance);
	CHECK_NULL_POINTER(params);
	BBTempMeasureCore *p = (BBTempMeasureCore *)instance;
	p->getParams(params);
	return ITA_OK;
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_BBSetCurve(HANDLE_T instance, short *pusCurve, int curveLength)
{
#ifdef BLACKBODY_EDITION
	CHECK_NULL_POINTER(instance);
	CHECK_NULL_POINTER(pusCurve);
	if (curveLength <= 0)
		return ITA_ILLEGAL_PAPAM_ERR;
	BBTempMeasureCore *p = (BBTempMeasureCore *)instance;
	p->setCurve(pusCurve, curveLength);
	return ITA_OK;
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_BBCalcTemp(HANDLE_T instance, short y16value, short *y16Image, float *temp)
{
#ifdef BLACKBODY_EDITION
	CHECK_NULL_POINTER(instance);
	CHECK_NULL_POINTER(temp);
	CHECK_NULL_POINTER(y16Image);
	BBTempMeasureCore *p = (BBTempMeasureCore *)instance;
	float myTemp = 0;
	p->GetTempByY16(myTemp, y16value, y16Image);
	*temp = myTemp;
	return ITA_OK;
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_BBCalcY16(HANDLE_T instance, float temp, short *y16value)
{
#ifdef BLACKBODY_EDITION
	CHECK_NULL_POINTER(instance);
	CHECK_NULL_POINTER(y16value);
	BBTempMeasureCore *p = (BBTempMeasureCore *)instance;
	short myValue = 0;
	p->GetY16ByTemp(myValue, temp);
	*y16value = myValue;
	return ITA_OK;
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}