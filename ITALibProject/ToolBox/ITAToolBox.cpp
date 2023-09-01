/*************************************************************
Copyright (C), 2022--2023, Wuhan Guide Sensmart Tech Co., Ltd
File name   : ITAToolBox.cpp
Author      : Guide Sensmart RD. wangyan
Version     : 1.0
Date        : 2023/2/13
Description : ITA ToolBox.
*************************************************************/
#include "../ITAToolBox.h"
#include "../Base/ITAWrapper.h"

ITA_API ITA_RESULT ITA_AutoResponseRate(HANDLE_T instance, short lowY16, short highY16, int currentINT, int *pOutINT, short responseRate, short precision)
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	ITAWrapper* p = (ITAWrapper*)instance;
	return p->AutoResponseRate(lowY16, highY16, currentINT, pOutINT, responseRate, precision);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_AutoGetKMatirx(HANDLE_T instance, short *pLowBase, short *pHighBase, int width, int height, unsigned short *pKMatrix)
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	ITAWrapper* p = (ITAWrapper*)instance;
	return p->AutoGetKMatirx(pLowBase, pHighBase, width, height, pKMatrix);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_AutoCorrectBadPoint(HANDLE_T instance, unsigned short *pKMatrix, int width, int height, unsigned short thresh, int *pBadPointInfo)
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	ITAWrapper* p = (ITAWrapper*)instance;
	return p->AutoCorrectBadPoint(pKMatrix, width, height, thresh, pBadPointInfo);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_AddBadPoint(HANDLE_T instance, unsigned short *pKMatrix, int x, int y, int width, int height)
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	ITAWrapper* p = (ITAWrapper*)instance;
	return p->AddBadPoint(pKMatrix, x, y, width, height);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_AddBadLine(HANDLE_T instance, unsigned short *pKMatrix, int lineIndex, int width, int height, ITAAddBadLineMode mode)
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	ITAWrapper* p = (ITAWrapper*)instance;
	return p->AddBadLine(pKMatrix, lineIndex, width, height, mode);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_IfConsistency(HANDLE_T instance, float centerPointTemp, float *pOtherPointTemp, int otherPointNum, ITAIfConsistencyType *pResult)
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	ITAWrapper* p = (ITAWrapper*)instance;
	return p->IfConsistency(centerPointTemp, pOtherPointTemp, otherPointNum, pResult);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_GenerateCurve(HANDLE_T instance, double *pTempMatrix, short *pY16, int arrLength, int maxTemp, short *pCurve, int curveLength,
	int *pValidCurveLength)
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	ITAWrapper* p = (ITAWrapper*)instance;
	return p->GenerateCurve(pTempMatrix, pY16, arrLength, maxTemp, pCurve, curveLength, pValidCurveLength);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_AutoGetDrift(HANDLE_T instance, ITADriftInfo *pDriftInfo, int driftInfoLength, int *pShutterDrift, int *pLenDrift)
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	ITAWrapper* p = (ITAWrapper*)instance;
	return p->AutoGetDrift(pDriftInfo, driftInfoLength, pShutterDrift, pLenDrift);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_AutoMeasureDistanceCoff(HANDLE_T instance, float *pTempMatrix, double *pDistance, int numOfBlackBody, int numOfDistance, 
	int standDistance, int *pDistanceCoff, ITADistanceCoffMode mode)
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	ITAWrapper* p = (ITAWrapper*)instance;
	return p->AutoMeasureDistanceCoff(pTempMatrix, pDistance, numOfBlackBody, numOfDistance, standDistance, pDistanceCoff, mode);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_AutoMeasureAmbientCoff(HANDLE_T instance, double *pStandardAmbientTemp, float standardAmbientTemp, 
	double *pCurrentAmbientTemp, float currentAmbientTemp, int tempMatrixSize, int *pK3, int *pB2)
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	ITAWrapper* p = (ITAWrapper*)instance;
	return p->AutoMeasureAmbientCoff(pStandardAmbientTemp, standardAmbientTemp, pCurrentAmbientTemp, currentAmbientTemp, tempMatrixSize, pK3, pB2);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_AutoCheckTemperature(HANDLE_T instance, float *pStandTemp, float *pMeasureTemp, int arrLength, 
	int *pNonConformTempIndex, int *pNonConformTempNum)
{
#ifdef ULTIMATE_EDITION
	CHECK_NULL_POINTER(instance);
	ITAWrapper* p = (ITAWrapper*)instance;
	return p->AutoCheckTemperature(pStandTemp, pMeasureTemp, arrLength, pNonConformTempIndex, pNonConformTempNum);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}