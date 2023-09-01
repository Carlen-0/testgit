/*************************************************************
Copyright (C), 2021--2022, Wuhan Guide Sensmart Tech Co., Ltd
File name   : ITACoin.h
Author      : Guide Sensmart RD. wangyan
Version     : 1.0
Date        : 2022/04/29
Description : ITA Coin.
*************************************************************/
#ifndef ITA_COIN_H
#define ITA_COIN_H

#include "../ITACustom.h"
#include "../Base/Config.h"
#include "../Module/GuideLog.h"
#include "../Module/Debugger.h"

#ifdef COIN_EDITION

#define COIN_COEF (8192)
#define COIN_CURVE_LENGTH (16384)

class ITACoin
{
public:
	ITACoin(ITA_COIN_DEVICE_MODEL deviceModel, ITA_COIN_LENS_TYPE lensType);
	~ITACoin();

	ITA_RESULT tempMatrix(short *pGray, int grayLen,
		unsigned  char *pParamLine, int lineLen, ITACoinMTParam mtParam, float *pTemper);

	ITA_RESULT debugger(ITA_DEBUG_TYPE type, void *param);

	ITA_RESULT getParamInfo(unsigned  char *pParamLine, int lineLen, ITACoinInternalParam * pInfo);

private:
	ITA_COIN_DEVICE_MODEL m_deviceModel;
	ITA_COIN_LENS_TYPE m_lensType;
	ITACoinInternalParam m_paramInter;
	GuideLog *m_logger;
	bool m_debug;
	Debugger m_debugger;
};
#endif
#endif // !ITA_COIN_H
