/*************************************************************
Copyright (C), 2022--2023, Wuhan Guide Sensmart Tech Co., Ltd
File name   : ITACustom.cpp
Author      : Guide Sensmart RD. wangyan
Version     : 1.0
Date        : 2022/04/29
Description : ITA SDK interface implementation.
*************************************************************/
#include "../ITACustom.h"
#include "ITACoin.h"

ITA_API HANDLE_T ITA_CoinInit(ITA_COIN_DEVICE_MODEL deviceModel, ITA_COIN_LENS_TYPE lensType)
{
#ifdef COIN_EDITION
	ITACoin *p = new ITACoin(deviceModel, lensType);
	return (HANDLE_T)p;
#else
	return 0;
#endif
}

ITA_API void ITA_CoinUninit(HANDLE_T instance)
{
#ifdef COIN_EDITION
	if (!instance)
		return;
	ITACoin *p = (ITACoin *)instance;
	delete p;
#endif
}

ITA_API ITA_RESULT ITA_CoinTempMatrix(HANDLE_T instance, short * pGray, int grayLen, unsigned char * pParamLine, int lineLen, ITACoinMTParam mtParam, float * pTemper)
{
#ifdef COIN_EDITION
	CHECK_NULL_POINTER(instance);
	ITACoin *p = (ITACoin *)instance;
	if (!pGray || !pTemper || !pParamLine)
	{
		return ITA_NULL_PTR_ERR;
	}
	return p->tempMatrix(pGray, grayLen, pParamLine, lineLen, mtParam, pTemper);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_CoinParamInfo(HANDLE_T instance, unsigned  char *pParamLine, int lineLen, ITACoinInternalParam * pInfo)
{
#ifdef COIN_EDITION
	CHECK_NULL_POINTER(instance);
	ITACoin *p = (ITACoin *)instance;
	if (!pInfo)
	{
		return ITA_NULL_PTR_ERR;
	}
	return p->getParamInfo(pParamLine, lineLen, pInfo);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_CoinDebugger(HANDLE_T instance, ITA_DEBUG_TYPE type, void * param)
{
#ifdef COIN_EDITION
	CHECK_NULL_POINTER(instance);
	ITACoin *p = (ITACoin *)instance;
	return p->debugger(type, param);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}
