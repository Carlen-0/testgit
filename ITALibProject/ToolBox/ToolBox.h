/*************************************************************
Copyright (C), 2021--2022, Wuhan Guide Sensmart Tech Co., Ltd
File name   : ToolBox.h
Author      : Guide Sensmart RD. wangyan
Version     : 1.0
Date        : 2023/2/17
Description : ITA Tool Box.
*************************************************************/
#ifndef TOOLBOX_H
#define TOOLBOX_H

#include "../ERROR.h"
#include "../ITADTD.h"
#include "../Module/GuideLog.h"
#include "../Module/Debugger.h"
#include "../ITAToolBox.h"

class ToolBox
{
public:
	ToolBox();

	~ToolBox();

	void setLogger(GuideLog* logger);

	ITA_RESULT AutoResponseRate(short lowY16, short highY16, int currentINT, int *pOutINT, short responseRate, short precision);
	ITA_RESULT AutoGetKMatirx(short *pLowBase, short *pHighBase, int width, int height, unsigned short *pKMatrix);
	ITA_RESULT AutoCorrectBadPoint(unsigned short *pKMatrix, int width, int height, unsigned short thresh, int *pBadPointInfo);
	ITA_RESULT AddBadPoint(unsigned short *pKMatrix, int x, int y, int width, int height);
	ITA_RESULT AddBadLine(unsigned short *pKMatrix, int lineIndex, int width, int height, ITAAddBadLineMode mode);
	ITA_RESULT IfConsistency(float centerPointTemp, float *pOtherPointTemp, int otherPointNum, ITAIfConsistencyType *pResult);
	ITA_RESULT GenerateCurve(double *pTempMatrix, short *pY16, int arrLength, int maxTemp, short *pCurve, int curveLength, int *pValidCurveLength);
	ITA_RESULT AutoGetDrift(ITADriftInfo *pDriftInfo, int driftInfoLength, int *pShutterDrift, int *pLenDrift);
	ITA_RESULT AutoMeasureDistanceCoff(float *pTempMatrix, double *pDistance, int numOfBlackBody, int numOfDistance, 
		int standDistance, int *pDistanceCoff, ITADistanceCoffMode mode);
	ITA_RESULT AutoMeasureAmbientCoff(double *pStandardAmbientTemp, float standardAmbientTemp, 
		double *pCurrentAmbientTemp, float currentAmbientTemp, int tempMatrixSize, int *pK3, int *pB2);
	ITA_RESULT AutoCheckTemperature(float *pStandTemp, float *pMeasureTemp, int arrLength, 
	int *pNonConformTempIndex, int *pNonConformTempNum);

protected:
	//连通域计算
	int FindConnet(unsigned short* sKMatrix, unsigned int* pLabel, int nWidth, int nHeight);
	//8邻域搜索
	void FindNeb8(int nX, int nY, int nLabel, unsigned int* pLabel, unsigned short* sKMatrix, int nWidth, int nHeight);
	bool IfShutter(int nShutterSignal);

	//标记空域坏点接口
	int FindAirSpaceBadPoint(unsigned short* sKMatrix, int nHeight, int nWidth, int nWinSize, unsigned short sThresh);

	//标记时域坏点接口
	int FindTimeSpaceBadPoint(unsigned short* sKMatrix, unsigned short* sX16, int nHeight, int nWidth, int nWinSize, int nFrame, unsigned short sThresh);

	//标记坏列(返回是否合格)
	bool FindBadLine(unsigned short* sKMatrix, int nHeight, int nWidth);

	//标记坏列(将坏列标记在K中)
	int FindBadLineInK(unsigned short* sKMatrix, int nHeight, int nWidth);

	//标记坏斑
	void FindBadArea(unsigned short* sKMatrix, int nHeight, int nWidth, int AAreaWidth, int AAreaHeight, int* nBadAreaNum);

	//曲线异常判断接口
	bool IfLegalCurve(short* pCurve, int nCurveLength);

	void QuickSort(unsigned short* pArr, int nBegin, int nEnd);
	void QuickSort(double* pArr, int nBegin, int nEnd);
	void QuickSort(float* pArr, int nBegin, int nEnd);
	void QuickSort(short* pArr, int nBegin, int nEnd);
	void Polyfit(int nArrLength, double dX[], double dY[], int nPolyN, double dA[]);
	void GaussSolve(int nPolyN, double dA[], double dX[], double dB[]);
	double Polyval(double dX, double dA[3]);
	int DoubleToInt(double dValue);

private:
	GuideLog* m_logger;

	int nResponseRateTime;					//调节响应率的次数
	short sDeltaY16;						//单个INT对应的Y16变化量
	short sLastResponseRate;				//上一次调节的响应率
};
#endif // !TOOLBOX_H
