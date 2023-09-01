/*************************************************************
Copyright (C), 2022--2023, Wuhan Guide Sensmart Tech Co., Ltd
File name   : ITALaserAF.cpp
Author      : Guide Sensmart RD. wangyan
Version     : 1.0
Date        : 2022/10/29
Description : ITALaserAF interface implementation.
*************************************************************/
#include "../ITALaserAF.h"
#include "../Base/ITAWrapper.h"

/*模块设计原则：功能独立，不受其它因素影响时，单独作为一个模块。否则需共用一个ITA实例。*/
ITA_API ITA_RESULT ITA_LaserAFRegister(HANDLE_T instance, ITALaserAFRegistry *registry)
{	
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	CHECK_NULL_POINTER(registry);
	if (!registry->GetClearity || !registry->GetDistance || !registry->GetHall || !registry->focus_move_to_hall || !registry->ReadCurveData || !registry->SetStop)
	{
		return ITA_NULL_PTR_ERR;
	}
	ITAWrapper *p = (ITAWrapper *)instance;
	return p->laserAFRegister(registry);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_LaserAF(HANDLE_T instance)
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	ITAWrapper *p = (ITAWrapper *)instance;
	return p->laserAF();
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_ContrastAF(HANDLE_T instance)
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	ITAWrapper *p = (ITAWrapper *)instance;
	return p->contrastAF();
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_CalibrateAF(HANDLE_T instance, ITA_AF_CALIBRATE type, int *diff)//--------------
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	ITAWrapper *p = (ITAWrapper *)instance;
	return p->calibrateAF(type, diff);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_SetInfoAF(HANDLE_T instance, ITA_AF_INFO type, void *param)//--------------
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	ITAWrapper *p = (ITAWrapper *)instance;
	return p->setInfoAF(type, param);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}


