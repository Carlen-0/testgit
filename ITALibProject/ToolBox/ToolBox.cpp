/*************************************************************
Copyright (C), 2021--2022, Wuhan Guide Sensmart Tech Co., Ltd
File name   : ToolBox.cpp
Author      : Guide Sensmart RD. wangyan
Version     : 1.0
Date        : 2023/2/17
Description : ITA Tool Box.
*************************************************************/
#include "ToolBox.h"
#include "../Module/PortingAPI.h"
#include <string.h>
#include <math.h>

#define BAD_CLASS_NUM  (16)					//最多考虑的坏斑种类

ToolBox::ToolBox()
{
	nResponseRateTime = 0;
	sDeltaY16 = 0;
	sLastResponseRate = 0;

	m_logger = NULL;
}

ToolBox::~ToolBox()
{
}

void ToolBox::setLogger(GuideLog* logger)
{
	m_logger = logger;
}

/*
函数功能：自动调节响应率
输入参数为：
short lowY16：低温黑体中心点Y16
short highY16：高温黑体中心点Y16
int currentINT：当前探测器INT值
short responseRate：当前档位和测温范围下所要求的响应率值
short precision：当前档位和测温范围下所能接受的响应率误差
输出参数为：
int *pOutINT：调整之后的INT值
返回值：
ITA_RESULT： 成功，返回ITA_OK；调节响应率未生效，返回ITA_RESPONSE_RATE_INEFFECTIVE
*/
ITA_RESULT ToolBox::AutoResponseRate(short lowY16, short highY16, int currentINT, int *pOutINT, short responseRate, short precision)
{
	if (!pOutINT)
	{
		m_logger->output(LOG_ERROR, "pOutINT is null");
		return ITA_NULL_PTR_ERR;
	}

	//计算当前响应率
	short sCurrentResponseRate = highY16 - lowY16;
	short sINTOffset = 0;
	if (sCurrentResponseRate >= responseRate - precision && sCurrentResponseRate <= responseRate + precision)
	{
		sINTOffset = 0;
	}
	else
	{
		if (nResponseRateTime == 0)
		{

			sINTOffset = 1;
		}
		else
		{
			if (nResponseRateTime == 1)
			{
				sDeltaY16 = sCurrentResponseRate - sLastResponseRate;
				if (sDeltaY16 == 0)
				{
					m_logger->output(LOG_ERROR, "return ITA_RESPONSE_RATE_INEFFECTIVE");
					return ITA_RESPONSE_RATE_INEFFECTIVE;
				}
				sINTOffset = DoubleToInt((responseRate - sCurrentResponseRate) / sDeltaY16);
			}
			else
			{
				sINTOffset = DoubleToInt((responseRate - sCurrentResponseRate) / sDeltaY16);
			}
		}
		sLastResponseRate = sCurrentResponseRate;
	}
	nResponseRateTime++;
	*pOutINT = currentINT + sINTOffset;

	return ITA_OK;
}

/*
函数功能：做K
输入参数为：
short *pLowBase：低温本底，即对低温黑体时采集的一帧X16数据
short *pHighBase：高温本底，即对高温黑体时采集的一帧X16数据
int height, int width：图像宽高
unsigned short *pKMatrix：K矩阵
输出参数为：
unsigned short *pKMatrix：K矩阵
*/
ITA_RESULT ToolBox::AutoGetKMatirx(short *pLowBase, short *pHighBase, int width, int height, unsigned short *pKMatrix)
{
	if (!pLowBase || !pHighBase || !pKMatrix)
	{
		m_logger->output(LOG_ERROR, "pLowBase:%p, pHighBase:%p, pKMatrix:%p", pLowBase, pHighBase, pKMatrix);
		return ITA_NULL_PTR_ERR;
	}

	if (width <= 0 || height <= 0)
	{
		m_logger->output(LOG_ERROR, "width:%d, height:%d", width, height);
		return ITA_ILLEGAL_PAPAM_ERR;
	}

	//K矩阵初始化
	memset(pKMatrix, 0, height * width * sizeof(unsigned short));

	//计算本底均值
	int nLen = height * width;
	unsigned int nSumLowBase = 0;
	unsigned int nSumHighBase = 0;
	for (int i = 0; i < nLen; i++)
	{
		nSumLowBase += pLowBase[i];
		nSumHighBase += pHighBase[i];
	}
	double dLowAverage = nSumLowBase * 1.0 / nLen;
	double dHighAverage = nSumHighBase * 1.0 / nLen;

	//计算中心点的K值
	int nCenterIndex = height / 2 * width + width / 2;
	pKMatrix[nCenterIndex] = static_cast<unsigned short>((dHighAverage - dLowAverage) * 8192 / (pHighBase[nCenterIndex] - pLowBase[nCenterIndex]));
	double dRatio = 8192 * 1.0 / pKMatrix[nCenterIndex];

	//计算K矩阵
	for (int i = 0; i < nLen; i++)
	{
		if (pLowBase[i] == pHighBase[i])
		{
			pKMatrix[i] = 32767;
		}
		else
		{
			pKMatrix[i] = (unsigned short)(((dHighAverage - dLowAverage) * 8192 / (pHighBase[i] - pLowBase[i])) * dRatio + 0.5);
		}
	}
	return ITA_OK;
}

/*
函数功能：自动标记坏点
输入参数为：
unsigned short *pKMatrix：K矩阵
int height, int width：图像宽高
unsigned short thresh：坏点阈值
输出参数为：
unsigned short *pKMatrix：K矩阵
int *pBadPointInfo：坏点、坏列、坏行、坏斑信息保存，大小为51，暂时不用
*/
ITA_RESULT ToolBox::AutoCorrectBadPoint(unsigned short *pKMatrix, int width, int height, unsigned short thresh, int *pBadPointInfo)
{
	if (!pKMatrix || !pBadPointInfo)
	{
		m_logger->output(LOG_ERROR, "pKMatrix:%p, pBadPointInfo:%p", pKMatrix, pBadPointInfo);
		return ITA_NULL_PTR_ERR;
	}

	if (width <= 0 || height <= 0)
	{
		m_logger->output(LOG_ERROR, "width:%d, height:%d", width, height);
		return ITA_ILLEGAL_PAPAM_ERR;
	}

	//空域坏点
	pBadPointInfo[0] = FindAirSpaceBadPoint(pKMatrix, height, width, 5, thresh);
	//坏列、坏行
	pBadPointInfo[1] = FindBadLineInK(pKMatrix, height, width);
	//坏斑
	int* nBadAreaNum = (int*)porting_calloc_mem(BAD_CLASS_NUM * 3, sizeof(int), ITA_OTHER_MODULE);
	FindBadArea(pKMatrix, height, width, (int)(0.5 * width), (int)(0.5 * height), nBadAreaNum);
	memcpy(pBadPointInfo + 3, nBadAreaNum, BAD_CLASS_NUM * 3 * sizeof(int));

	porting_free_mem(nBadAreaNum);

	return ITA_OK;
}

/*
函数功能：手动添加坏点
输入参数为：
unsigned short *pKMatrix：K矩阵
int nX, int nY：坏点坐标
int nHeight, int nWidth：图像宽高
输出参数为：
unsigned short *pKMatrix：K矩阵
*/
ITA_RESULT ToolBox::AddBadPoint(unsigned short *pKMatrix, int x, int y, int width, int height)
{
	if (!pKMatrix)
	{
		m_logger->output(LOG_ERROR, "pKMatrix is null");
		return ITA_NULL_PTR_ERR;
	}

	if (width <= 0 || height <= 0)
	{
		m_logger->output(LOG_ERROR, "width:%d, height:%d", width, height);
		return ITA_ILLEGAL_PAPAM_ERR;
	}

	if (x < 0 || y < 0)
	{
		m_logger->output(LOG_ERROR, "x:%d, y:%d", x, y);
		return ITA_ILLEGAL_PAPAM_ERR;
	}

	if (x > width || y > height)
	{
		m_logger->output(LOG_ERROR, "x:%d, y:%d, width:%d, height:%d", x, y, width, height);
		return ITA_ILLEGAL_PAPAM_ERR;
	}
	else
	{
		pKMatrix[y * width + x] |= 1 << 15;
	}

	return ITA_OK;
}

/*
函数功能：手动添加坏行、坏列
输入参数为：
unsigned short *pKMatrix：K矩阵
int lineIndex：行数或者列数
int height, int width：图像宽高
ITAAddBadLineMode mode：0表示添加坏行模式，1表示添加坏列模式
输出参数为：
unsigned short *pKMatrix：K矩阵
*/
ITA_RESULT ToolBox::AddBadLine(unsigned short *pKMatrix, int lineIndex, int width, int height, ITAAddBadLineMode mode)
{
	if (!pKMatrix)
	{
		m_logger->output(LOG_ERROR, "pKMatrix is null");
		return ITA_NULL_PTR_ERR;
	}

	if (width <= 0 || height <= 0)
	{
		m_logger->output(LOG_ERROR, "width:%d, height:%d", width, height);
		return ITA_ILLEGAL_PAPAM_ERR;
	}

		//添加坏列
	if (mode == ITA_ADDBADLINE_COLUMN)
	{
		if (lineIndex < 0 || lineIndex > width)
		{
			m_logger->output(LOG_ERROR, "lineIndex:%d, width:%d", lineIndex, width);
			return ITA_ILLEGAL_PAPAM_ERR;
		}
		else
		{
			for (int i = 0; i < height; i++)
			{
				pKMatrix[i * width + lineIndex] |= 1 << 15;
			}
		}
	}
	//添加坏行
	else if (mode == ITA_ADDBADLINE_ROW)
	{
		if (lineIndex < 0 || lineIndex > height)
		{
			m_logger->output(LOG_ERROR, "lineIndex:%d, height:%d", lineIndex, height);
			return ITA_ILLEGAL_PAPAM_ERR;
		}
		else
		{
			for (int i = 0; i < width; i++)
			{
				pKMatrix[lineIndex * width + i] |= 1 << 15;
			}
		}
	}

	return ITA_OK;
}

/*
函数功能：判断面一致性
float centerPointTemp：中心的实测温度
float *pOtherPointTemp：面阵中其他温度点的温度
int otherPointNum：其他温度点的个数
输出参数为：
ITAIfConsistencyType *pResult: 输出面一致性是否合格
*/
ITA_RESULT ToolBox::IfConsistency(float centerPointTemp, float *pOtherPointTemp, int otherPointNum, ITAIfConsistencyType *pResult)
{
	if (!pOtherPointTemp || !pResult)
	{
		m_logger->output(LOG_ERROR, "pOtherPointTemp:%p, pResult:%p", pOtherPointTemp, pResult);
		return ITA_NULL_PTR_ERR;
	}

	if (otherPointNum <= 0)
	{
		m_logger->output(LOG_ERROR, "otherPointNum:%d", otherPointNum);
		return ITA_ILLEGAL_PAPAM_ERR;
	}

	float fJudgeDiff = 0.0f;
	if (centerPointTemp <= 100)
	{
		fJudgeDiff = 0.5;
	}
	else
	{
		fJudgeDiff = centerPointTemp * 0.005f;
	}
	for (int i = 0; i < otherPointNum; i++)
	{
		if (fabs(pOtherPointTemp[i] - centerPointTemp) <= fJudgeDiff)
		{
			continue;
		}
		else
		{
			*pResult = ITA_IFCONSISTENCY_NQ;
			return ITA_OK;
		}
	}

	*pResult = ITA_IFCONSISTENCY_QUALIFIED;

	return ITA_OK;
}

/*
函数功能：生成测温曲线
输入参数为：
int arrLength：传入的温度数据、Y16数据的长度
short *pY16：Y16数组
double *pTempMatrix：温度数组
int maxTemp：最大温度
int curveLength：曲线长度
输出参数为：
short *pCurve：测温曲线
int *pValidCurveLength: 输出实际生成的曲线长度
*/
ITA_RESULT ToolBox::GenerateCurve(double *pTempMatrix, short *pY16, int arrLength, int maxTemp, short *pCurve, int curveLength, int *pValidCurveLength)
{
	if (!pY16 || !pTempMatrix || !pCurve || !pValidCurveLength)
	{
		m_logger->output(LOG_ERROR, "pY16:%p, pTempMatrix:%p, pCurve:%p, pValidCurveLength:%p", pY16, pTempMatrix, pCurve, pValidCurveLength);
		return ITA_NULL_PTR_ERR;
	}

	if (arrLength <= 0 || curveLength <= 0)
	{
		m_logger->output(LOG_ERROR, "arrLength:%d, curveLength:%d", arrLength, curveLength);
		return ITA_ILLEGAL_PAPAM_ERR;
	}

	short *sY16 = pY16;
	double *dTemp = pTempMatrix;

	//确定曲线上下限
	int nMinTemp = -40;
	maxTemp = (int)(1.1 * maxTemp);

	//对传入的温度和Y16进行排序
	QuickSort(sY16, 0, arrLength - 1);
	QuickSort(dTemp, 0, arrLength - 1);

	//将最低的Y16调整到2000,保证上下限不偏压
	int nLowOffset = 2000 - sY16[0];
	int nHighOffset = 15000 - sY16[arrLength - 1];
	int nOffset = gdmin(nLowOffset, nHighOffset);

	//整体偏移
	for (int i = 0; i < arrLength; i++)
	{
		sY16[i] += nOffset;
	}
	//三个点一组生成曲线
	//计算分组数
	int group = 0;
	if (arrLength % 2 == 1)
	{
		group = (arrLength - 1) / 2;
	}
	else
	{
		group = arrLength / 2;
	}

	//每三个点计算一组系数
	double* coff = (double*)porting_calloc_mem(group * 3, sizeof(double), ITA_OTHER_MODULE);
	memset(coff, 0, group * 3 * sizeof(double));
	if (arrLength % 2 == 1)
	{
		for (int i = 0; i < group; i++)
		{
			double xtemp[3] = { (double)(sY16[2 * i]), (double)(sY16[2 * i + 1]), (double)(sY16[2 * i + 2]) };
			double ytemp[3] = { dTemp[2 * i], dTemp[2 * i + 1], dTemp[2 * i + 2] };
			double a[3];
			Polyfit(3, xtemp, ytemp, 2, a);
			coff[3 * i] = a[0];
			coff[3 * i + 1] = a[1];
			coff[3 * i + 2] = a[2];
		}
	}
	else
	{
		for (int i = 0; i < group - 1; i++)
		{
			double xtemp[3] = { (double)(sY16[2 * i]), (double)(sY16[2 * i + 1]), (double)(sY16[2 * i + 2]) };
			double ytemp[3] = { dTemp[2 * i], dTemp[2 * i + 1], dTemp[2 * i + 2] };
			double a[3];
			Polyfit(3, xtemp, ytemp, 2, a);
			coff[3 * i] = a[0];
			coff[3 * i + 1] = a[1];
			coff[3 * i + 2] = a[2];
		}
		double a[3];
		double xtemp[3] = { (double)(sY16[2 * group - 3]), (double)(sY16[2 * group - 2]), (double)(sY16[2 * group - 1]) };
		double ytemp[3] = { dTemp[2 * group - 3], dTemp[2 * group - 2], dTemp[2 * group - 1] };
		Polyfit(3, xtemp, ytemp, 2, a);
		coff[3 * group - 3] = a[0];
		coff[3 * group - 2] = a[1];
		coff[3 * group - 1] = a[2];
	}

	//根据系数生成曲线
	for (short j = 0; j < sY16[2]; j++)
	{
		double a[3] = { coff[0],coff[1],coff[2] };
		double val = Polyval(j, a);
		pCurve[j] = (short)(val * 10);
	}
	for (int i = 1; i < group - 1; i++)
	{
		for (double j = sY16[2 * i]; j < sY16[2 * i + 2]; j++)
		{
			double a[3] = { coff[3 * i],coff[3 * i + 1],coff[3 * i + 2] };
			double val = Polyval(j, a);
			pCurve[(short)(j)] = (short)(val * 10);
		}
	}
	for (double j = sY16[arrLength - 3]; j < curveLength; j++)
	{
		double a[3] = { coff[3 * group - 3],coff[3 * group - 2],coff[3 * group - 1] };
		double val = Polyval(j, a);
		pCurve[(short)(j)] = (short)(val * 10);
	}

	if (coff)
	{
		porting_free_mem(coff);
		coff = NULL;
	}

	//两端截断
	for (short i = 0; i < curveLength; i++)
	{
		if (pCurve[i] < (short)(10 * nMinTemp))
		{
			pCurve[i] = (short)(10 * nMinTemp);
		}
		else
		{
			break;
		}
	}

	for (short i = curveLength - 1; i > 0; i--)
	{
		if (pCurve[i] > (short)(10 * maxTemp))
		{
			pCurve[i] = (short)(10 * maxTemp);
		}
		else
		{
			break;
		}
	}

	//曲线校验
	bool sign = IfLegalCurve(pCurve, curveLength);
	if (sign)
	{
		*pValidCurveLength = curveLength;

		return ITA_OK;
	}
	else
	{
		*pValidCurveLength = 0;
		m_logger->output(LOG_ERROR, "Check curve ERROR");
		return ITA_ERROR;
	}
}

/*
函数功能：自动计算温漂修正参数
输入参数为：
ITADriftInfo *pDriftInfo：录制的温漂文件
int driftInfoLength：温漂文件共记录了多少秒数据
输出参数为：
int *pShutterDrift：快门温漂
int *pLenDrift：镜筒温漂
*/
ITA_RESULT ToolBox::AutoGetDrift(ITADriftInfo *pDriftInfo, int driftInfoLength, int *pShutterDrift, int *pLenDrift)
{
	if (!pDriftInfo || !pShutterDrift || !pLenDrift)
	{
		m_logger->output(LOG_ERROR, "pDriftInfo:%p, pShutterDrift:%p, pLenDrift:%p", pDriftInfo, pShutterDrift, pLenDrift);
		return ITA_NULL_PTR_ERR;
	}

	if (driftInfoLength <= 0)
	{
		m_logger->output(LOG_ERROR, "driftInfoLength:%d", driftInfoLength);
		return ITA_ILLEGAL_PAPAM_ERR;
	}

	ITADriftInfo *data = pDriftInfo;

	int cnt = 0;							//定义循环变量
	int nNotShutterFrame = 0;				//非打快门帧数
	int nSearchStep = 500;					//搜索步长
	float fLenSearchAccuracy = 1;			//镜筒温票搜索精度
	float fShutterSearchAccuracy = 0.1f;	//快门温漂搜索精度

	//初始化上一次打快门时的镜筒温和Y16(需要判断第一帧数据是否为快门数据)
	float fLastTimeLenTemp = 0.0;
	short sLastTimeY16 = 0;
	while (cnt < driftInfoLength)
	{
		if (IfShutter(data[cnt].shutterStatus))
		{
			cnt++;
		}
		else
		{
			fLastTimeLenTemp = data[cnt].realTimeLenTemp;
			sLastTimeY16 = data[cnt].y16;
			break;
		}
	}

	bool bShutterEnd = false;
	//计算非打快门的帧数
	float* pTempDetlaLenT = (float*)porting_calloc_mem(driftInfoLength, sizeof(float), ITA_OTHER_MODULE);
	memset(pTempDetlaLenT, 0, driftInfoLength * sizeof(float));
	short* pTempDetlaLenY16 = (short*)porting_calloc_mem(driftInfoLength, sizeof(short), ITA_OTHER_MODULE);
	memset(pTempDetlaLenY16, 0, driftInfoLength * sizeof(short));
	for (cnt; cnt < driftInfoLength; cnt++)
	{
		if (IfShutter(data[cnt].shutterStatus))
		{
			if (!IfShutter(data[cnt + 1].shutterStatus))
			{
				fLastTimeLenTemp = data[cnt + 1].realTimeLenTemp;
				sLastTimeY16 = data[cnt + 1].y16;
			}
		}
		else
		{
			pTempDetlaLenT[nNotShutterFrame] = data[cnt].realTimeLenTemp - fLastTimeLenTemp;
			pTempDetlaLenY16[nNotShutterFrame] = data[cnt].y16 - sLastTimeY16;
			nNotShutterFrame++;
		}
	}

	//计算镜筒温升以及其带来的Y16增加量
	double* pLenTempUp = (double*)porting_calloc_mem(nNotShutterFrame, sizeof(double), ITA_OTHER_MODULE);
	memset(pLenTempUp, 0, nNotShutterFrame * sizeof(double));
	double* pY16Up = (double*)porting_calloc_mem(nNotShutterFrame, sizeof(double), ITA_OTHER_MODULE);
	memset(pY16Up, 0, nNotShutterFrame * sizeof(double));
	for (int i = 0; i < nNotShutterFrame; i++)
	{
		pLenTempUp[i] = pTempDetlaLenT[i];
		pY16Up[i] = pTempDetlaLenY16[i];
	}
	double a[2];
	Polyfit(nNotShutterFrame, pLenTempUp, pY16Up, 1, a);

	//在拟合结果附近小范围搜索一个最合适的镜筒温漂值
	float fMinMeanVar = 1000;
	double dBestLenDrift = 0;
	short* pAfterLenDrift = (short*)porting_calloc_mem(nNotShutterFrame, sizeof(short), ITA_OTHER_MODULE);
	memset(pAfterLenDrift, 0, nNotShutterFrame * sizeof(short));
	int* pNoShuttetIndex = (int*)porting_calloc_mem(nNotShutterFrame, sizeof(int), ITA_OTHER_MODULE);
	memset(pNoShuttetIndex, 0, nNotShutterFrame * sizeof(int));

	short* pAfterLenY16 = (short*)porting_calloc_mem(nNotShutterFrame, sizeof(short), ITA_OTHER_MODULE);
	memset(pAfterLenY16, 0, nNotShutterFrame * sizeof(short));
	for (int i = -nSearchStep; i < nSearchStep + 1; i++)
	{
		//当前镜筒温漂
		double dCurLenDrift = a[1] + fLenSearchAccuracy * i;
		//printf("dCurLenDrift=%f\n", dCurLenDrift);
		//初始化文件指针
		cnt = 0;
		//
		memset(pAfterLenY16, 0, nNotShutterFrame * sizeof(short));
		//short *pAfterLenY16 = new short[nNotShutterFrame];
		//初始化上一次打快门时的镜筒温和Y16(需要判断第一帧数据是否为快门数据)
		while (cnt < driftInfoLength)
		{
			if (IfShutter(data[cnt].shutterStatus))
			{
				cnt++;
			}
			else
			{
				fLastTimeLenTemp = data[cnt].realTimeLenTemp;
				sLastTimeY16 = data[cnt].y16;
				break;
			}
		}
		int nCount = 0;
		pAfterLenY16[nCount] = data[cnt].y16;
		//初始化Y16方差
		double dY16Var = 0.0;
		//计算平方和
		float fY16MeanVar = 0.0;
		for (cnt; cnt < driftInfoLength; cnt++)
		{
			if (IfShutter(data[cnt].shutterStatus))
			{
				if (!IfShutter(data[cnt + 1].shutterStatus))
				{
					fLastTimeLenTemp = data[cnt + 1].realTimeLenTemp;
					sLastTimeY16 = data[cnt + 1].y16;
				}
			}
			else
			{
				pNoShuttetIndex[nCount] = cnt;
				nCount++;
				pAfterLenY16[nCount] = short(data[cnt].y16 - dCurLenDrift * (data[cnt].realTimeLenTemp - fLastTimeLenTemp));
				dY16Var += pow((pAfterLenY16[nCount] - sLastTimeY16), 2);
			}
		}
		fY16MeanVar = (float)(dY16Var / nCount);
		//printf("fY16MeanVar=%f  fMinMeanVar=%f, dY16Var=%f, nCount=%d\n", dBestLenDrift, fMinMeanVar, dY16Var, nCount);
		fY16MeanVar = (float)(pow(fY16MeanVar, 0.5));
		//printf("fY16MeanVar=%f\n", fY16MeanVar);
		if (fY16MeanVar < fMinMeanVar)
		{
			fMinMeanVar = fY16MeanVar;
			dBestLenDrift = dCurLenDrift;
			//printf("fMinMeanVar=%f, dBestLenDrift=%f\n", fMinMeanVar,dBestLenDrift);
			memcpy(pAfterLenDrift, pAfterLenY16, nCount * sizeof(short));
		}
		//printf("\n");
	}

	//计算镜筒温升以及其带来的Y16减小量
	cnt = 0;
	float fStartShutterTemp = 0;
	short sStarShutterY16 = 0;
	while (cnt < driftInfoLength)
	{
		if (IfShutter(data[cnt].shutterStatus))
		{
			cnt++;
		}
		else
		{
			fStartShutterTemp = data[cnt].realTimeShutterTemp;
			sStarShutterY16 = data[cnt].y16;
			break;
		}
	}
	double* pShutterTempUp = (double*)porting_calloc_mem(nNotShutterFrame, sizeof(double), ITA_OTHER_MODULE);
	memset(pShutterTempUp, 0, nNotShutterFrame * sizeof(double));
	double* pShutterY16Up = (double*)porting_calloc_mem(nNotShutterFrame, sizeof(double), ITA_OTHER_MODULE);
	memset(pShutterY16Up, 0, nNotShutterFrame * sizeof(double));
	for (int i = 0; i < nNotShutterFrame; i++)
	{
		pShutterTempUp[i] = data[pNoShuttetIndex[i]].realTimeShutterTemp - fStartShutterTemp;
		pShutterY16Up[i] = pAfterLenDrift[i] - sStarShutterY16;
	}
	//for (int i = 0; i < nNotShutterFrame; i++)
	//{
	//	std::cout << pShutterTempUp[i] << "  " << pShutterY16Up[i] << std::endl;
	//}
	Polyfit(nNotShutterFrame, pShutterTempUp, pShutterY16Up, 1, a);
	float* pAfterShutterDrift = (float*)porting_calloc_mem(nNotShutterFrame, sizeof(float), ITA_OTHER_MODULE);
	memset(pAfterShutterDrift, 0, nNotShutterFrame * sizeof(float));
	//float *pAfterShutterDrift = new float[nNotShutterFrame];
	pAfterShutterDrift[0] = sStarShutterY16;
	float fMinVar = 1000;
	double dBestShutterDrift = 0;
	for (int i = -nSearchStep; i < nSearchStep + 1; i++)
	{
		//当前快门温漂
		double dShutterVar = 0;
		float fShutterMeanVar = 0;
		double dCurShutterDrift = a[1] + fShutterSearchAccuracy * i;
		for (int j = 1; j < nNotShutterFrame; j++)
		{
			pAfterShutterDrift[j] = (float)(pAfterLenDrift[j] - dCurShutterDrift * pShutterTempUp[j]);
			dShutterVar += pow(pAfterShutterDrift[j] - sStarShutterY16, 2);
		}
		fShutterMeanVar = (float)(dShutterVar / nNotShutterFrame);
		if (fShutterMeanVar < fMinVar)
		{
			fMinVar = fShutterMeanVar;
			dBestShutterDrift = dCurShutterDrift;
		}
	}

	*pLenDrift     = DoubleToInt(dBestLenDrift * 100);
	*pShutterDrift = DoubleToInt(dBestShutterDrift * 100);

	//释放内存
	if (pTempDetlaLenT)
	{
		porting_free_mem(pTempDetlaLenT);
		pTempDetlaLenT = NULL;
	}

	if (pTempDetlaLenY16)
	{
		porting_free_mem(pTempDetlaLenY16);
		pTempDetlaLenY16 = NULL;
	}

	if (pLenTempUp)
	{
		porting_free_mem(pLenTempUp);
		pLenTempUp = NULL;
	}

	if (pY16Up)
	{
		porting_free_mem(pY16Up);
	}
	pY16Up = NULL;

	if (pAfterLenDrift)
	{
		porting_free_mem(pAfterLenDrift);
		pAfterLenDrift = NULL;
	}

	if (pNoShuttetIndex)
	{
		porting_free_mem(pNoShuttetIndex);
		pNoShuttetIndex = NULL;
	}

	if (pAfterLenY16)
	{
		porting_free_mem(pAfterLenY16);
		pAfterLenY16 = NULL;
	}

	if (pShutterTempUp)
	{
		porting_free_mem(pShutterTempUp);
		pShutterTempUp = NULL;
	}

	if (pShutterY16Up)
	{
		porting_free_mem(pShutterY16Up);
		pShutterY16Up = NULL;
	}

	if (pAfterShutterDrift)
	{
		porting_free_mem(pAfterShutterDrift);
		pAfterShutterDrift = NULL;
	}

	return ITA_OK;
}

/*
函数功能：自动计算距离修正参数
输入参数为：
float *pTempMatrix：不同距离下的温度矩阵
double *pDistance: 距离数组
int numOfBlackBody：黑体个数
int numOfDistance：距离个数
int standDistance：标准距离
ITADistanceCoffMode mode：ITA_DISTANCECOFF_6GROUP代表6距离参数，输入ITA_DISTANCECOFF_9GROUP代表9距离参数
输出参数为：
int *pDistanceCoff：距离修正系数
*/
ITA_RESULT ToolBox::AutoMeasureDistanceCoff(float *pTempMatrix, double *pDistance, int numOfBlackBody, int numOfDistance, 
		int standDistance, int *pDistanceCoff, ITADistanceCoffMode mode)
{
	if (!pTempMatrix || !pDistance || !pDistanceCoff)
	{
		m_logger->output(LOG_ERROR, "pTempMatrix:%p, pDistance:%p, pDistanceCoff:%p", pTempMatrix, pDistance, pDistanceCoff);
		return ITA_NULL_PTR_ERR;
	}

	if (numOfBlackBody <= 0 || numOfDistance <= 0 || standDistance <= 0)
	{
		m_logger->output(LOG_ERROR, "numOfBlackBody:%d, numOfDistance:%d, standDistance:%d", numOfBlackBody, numOfDistance, standDistance);
		return ITA_ILLEGAL_PAPAM_ERR;
	}

	//转换开式温度
	for (int i = 0; i < numOfBlackBody * numOfDistance; i++)
	{
		pTempMatrix[i] += 273.5;
	}

	//获取标准数据
	double* dStandTemp = (double*)porting_calloc_mem(numOfBlackBody, sizeof(double), ITA_OTHER_MODULE);					//标准温度数据
	for (int j = 0; j < numOfBlackBody; j++)
	{
		dStandTemp[j] = pTempMatrix[(standDistance - 1) * numOfBlackBody + j] * 10;
	}

	//拟合不同距离下的温度信息
	double* dTmpTemp = (double*)porting_calloc_mem(numOfBlackBody, sizeof(double), ITA_OTHER_MODULE);						//当前距离下的温度数据
	double* dTwoCoff = (double*)porting_calloc_mem(numOfDistance, sizeof(double), ITA_OTHER_MODULE);						//二次项系数
	double* dOneCoff = (double*)porting_calloc_mem(numOfDistance, sizeof(double), ITA_OTHER_MODULE);						//一次项系数
	double* dZeroCoff = (double*)porting_calloc_mem(numOfDistance, sizeof(double), ITA_OTHER_MODULE);						//常数项系数
	for (int i = 0; i < numOfDistance; i++)
	{
		for (int j = 0; j < numOfBlackBody; j++)
		{
			dTmpTemp[j] = pTempMatrix[i * numOfBlackBody + j];
		}
		double dTmpTempCoff[3] = { 0, 0, 0 };
		Polyfit(numOfBlackBody, dTmpTemp, dStandTemp, 2, dTmpTempCoff);
		dTwoCoff[i] = dTmpTempCoff[2];
		dOneCoff[i] = dTmpTempCoff[1];
		dZeroCoff[i] = dTmpTempCoff[0];
	}

	//对温度参数进行拟合
	if (mode == ITA_DISTANCECOFF_9GROUP)
	{
		//9距离参数模式
		double dTmp[3];
		Polyfit(numOfDistance, pDistance, dTwoCoff, 2, dTmp);
		pDistanceCoff[0] = DoubleToInt(dTmp[2] * 10000000000);
		pDistanceCoff[1] = DoubleToInt(dTmp[1] * 1000000000);
		pDistanceCoff[2] = DoubleToInt(dTmp[0] * 10000000);
		Polyfit(numOfDistance, pDistance, dOneCoff, 2, dTmp);
		pDistanceCoff[3] = DoubleToInt(dTmp[2] * 100000);
		pDistanceCoff[4] = DoubleToInt(dTmp[1] * 100000);
		pDistanceCoff[5] = DoubleToInt(dTmp[0] * 10000);
		Polyfit(numOfDistance, pDistance, dZeroCoff, 2, dTmp);
		pDistanceCoff[6] = DoubleToInt(dTmp[2] * 1000);
		pDistanceCoff[7] = DoubleToInt(dTmp[1] * 100);
		pDistanceCoff[8] = DoubleToInt(dTmp[0] * 100);
	}
	else if (mode == ITA_DISTANCECOFF_6GROUP)
	{
		//6距离参数模式
		double dTmp[3];
		Polyfit(numOfDistance, pDistance, dTwoCoff, 1, dTmp);
		pDistanceCoff[0] = DoubleToInt(dTmp[1] * 1000000000);
		pDistanceCoff[1] = DoubleToInt(dTmp[0] * 1000000000);
		Polyfit(numOfDistance, pDistance, dOneCoff, 1, dTmp);
		pDistanceCoff[2] = DoubleToInt(dTmp[1] * 1000000);
		pDistanceCoff[3] = DoubleToInt(dTmp[0] * 1000000);
		Polyfit(numOfDistance, pDistance, dZeroCoff, 1, dTmp);
		pDistanceCoff[4] = DoubleToInt(dTmp[1] * 1000);
		pDistanceCoff[5] = DoubleToInt(dTmp[0] * 1000);
		pDistanceCoff[6] = -1;
		pDistanceCoff[7] = -1;
		pDistanceCoff[8] = -1;
	}

	if (dStandTemp)
	{
		porting_free_mem(dStandTemp);
		dStandTemp = NULL;
	}

	if (dTmpTemp)
	{
		porting_free_mem(dTmpTemp);
		dTmpTemp = NULL;
	}

	if (dTwoCoff)
	{
		porting_free_mem(dTwoCoff);
		dTwoCoff = NULL;
	}

	if (dOneCoff)
	{
		porting_free_mem(dOneCoff);
		dOneCoff = NULL;
	}

	if (dZeroCoff)
	{
		porting_free_mem(dZeroCoff);
		dZeroCoff = NULL;
	}

	return ITA_OK;
}

/*
函数功能：自动计算环温修正参数
输入参数为：
int tempMatrixSize：温度矩阵长度
double *pStandardAmbientTemp：标准环温下的温度复核数据
float standardAmbientTemp：标准环境温度
double *pCurrentAmbientTemp：当前环温下的温度实测数据
float currentAmbientTemp：当前环境温度
输出参数为：
int *pK3;
int *pB2
*/
ITA_RESULT ToolBox::AutoMeasureAmbientCoff(double *pStandardAmbientTemp, float standardAmbientTemp, 
		double *pCurrentAmbientTemp, float currentAmbientTemp, int tempMatrixSize, int *pK3, int *pB2)
{
	if (!pStandardAmbientTemp || !pCurrentAmbientTemp || !pK3 || !pB2)
	{
		m_logger->output(LOG_ERROR, "pStandardAmbientTemp:%p, pCurrentAmbientTemp:%p, pK3:%p, pB2:%p", pStandardAmbientTemp, pCurrentAmbientTemp, pK3, pB2);
		return ITA_NULL_PTR_ERR;
	}

	if (tempMatrixSize <= 0)
	{
		m_logger->output(LOG_ERROR, "tempMatrixSize:%d", tempMatrixSize);
		return ITA_ILLEGAL_PAPAM_ERR;
	}
	
	double* dDelta = (double*)porting_calloc_mem(tempMatrixSize, sizeof(double), ITA_OTHER_MODULE);
	memset(dDelta, 0, tempMatrixSize * sizeof(double));
	for (int i = 0; i < tempMatrixSize; i++)
	{
		dDelta[i] = pCurrentAmbientTemp[i] - pStandardAmbientTemp[i];
		dDelta[i] = dDelta[i] / (currentAmbientTemp - standardAmbientTemp);
	}
	double a[2];
	Polyfit(tempMatrixSize, pCurrentAmbientTemp, dDelta, 1, a);
	*pK3 = DoubleToInt(a[1] * 10000);
	*pB2 = DoubleToInt(a[0] * 10000);

	if (dDelta)
	{
		porting_free_mem(dDelta);
		dDelta = NULL;
	}

	return ITA_OK;
}

//目标温度大于0°按照±2,±2%取大值的标准，目标温度小于0°按照±3的标准
ITA_RESULT ToolBox::AutoCheckTemperature(float *pStandTemp, float *pMeasureTemp, int arrLength, 
	int *pNonConformTempIndex, int *pNonConformTempNum)
{
	if (!pStandTemp || !pMeasureTemp || !pNonConformTempIndex || !pNonConformTempNum)
	{
		m_logger->output(LOG_ERROR, "pStandTemp:%p, pMeasureTemp:%p, pNonConformTempIndex:%p, pNonConformTempNum:%p", pStandTemp, pMeasureTemp, pNonConformTempIndex, pNonConformTempNum);
		return ITA_NULL_PTR_ERR;
	}

	if (arrLength <= 0)
	{
		m_logger->output(LOG_ERROR, "arrLength:%d", arrLength);
		return ITA_ILLEGAL_PAPAM_ERR;
	}

	int nNonConformTemp = 0;
	for (int i = 0; i < arrLength; i++)
	{
		if (pStandTemp[i] > 0 && pStandTemp[i] <= 100)
		{
			//±2
			if (fabs(pStandTemp[i] - pMeasureTemp[i]) <= 2)
			{
				continue;
			}
			else
			{
				pNonConformTempIndex[nNonConformTemp] = i;
				nNonConformTemp++;
			}
		}
		else if (pStandTemp[i] > 100)
		{
			//±2%
			float fThreshold = pStandTemp[i] * 0.02f;
			if (fabs(pStandTemp[i] - pMeasureTemp[i]) <= fThreshold)
			{
				continue;
			}
			else
			{
				pNonConformTempIndex[nNonConformTemp] = i;
				nNonConformTemp++;
			}
		}
		else
		{
			//±3
			if (fabs(pStandTemp[i] - pMeasureTemp[i]) <= 3)
			{
				continue;
			}
			else
			{
				pNonConformTempIndex[nNonConformTemp] = i;
				nNonConformTemp++;
			}
		}
	}
	for (int i = nNonConformTemp; i < arrLength; i++)
	{
		pNonConformTempIndex[i] = -1;
	}

	*pNonConformTempNum = nNonConformTemp;

	return ITA_OK;
}

/*
函数功能：标记空域坏点
输入参数为：
unsigned short *sKMatrix：K矩阵
int nHeight, int nWidth：图像宽高
int nWinSize：坏点标记窗口大小
unsigned short sThresh：坏点阈值
返回值：
空域坏点数量
*/
int ToolBox::FindAirSpaceBadPoint(unsigned short *sKMatrix, int nHeight, int nWidth, int nWinSize, unsigned short sThresh)
{
	int nHalfWinSize = (nWinSize - 1) / 2;
	int nNeighboorNum = nWinSize * nWinSize;
	int nBadPointNum = 0;
	unsigned short *us_neighboor_list = (unsigned short*)porting_calloc_mem(nNeighboorNum, sizeof(unsigned short), ITA_OTHER_MODULE);
	memset(us_neighboor_list, 0, nNeighboorNum * sizeof(unsigned short));

	//坏点标记
	for (int i = 0; i < nHeight; i++) {
		for (int j = 0; j < nWidth; j++) {
			{
				//邻域K值采集
				nNeighboorNum = 0;
				for (int m = i - nWinSize; m <= i + nWinSize; m++) {
					for (int n = j - nWinSize; n <= j + nWinSize; n++) {
						if (m >= 0 && m < nHeight && n >= 0 && n < nWidth)
						{
							us_neighboor_list[nNeighboorNum] = sKMatrix[m * nWidth + n];
							nNeighboorNum++;
						}
					}
				}

				//K值排序
				if (nNeighboorNum > 0)
				{
					//快排
					QuickSort(us_neighboor_list, 0, nNeighboorNum);
					////冒泡排序
					//unsigned short us_temp = 0;
					//for (int m = 0; m < nNeighboorNum; m++)
					//{
					//	us_temp = us_neighboor_list[m];
					//	for (int n = m + 1; n < nNeighboorNum; n++)
					//	{
					//		if (us_temp < us_neighboor_list[n])
					//		{
					//			us_neighboor_list[m] = us_neighboor_list[n];
					//			us_neighboor_list[n] = us_temp;
					//			us_temp = us_neighboor_list[m];
					//		}
					//	}
					//}
					if (abs((sKMatrix[i * nWidth + j] - us_neighboor_list[(nNeighboorNum - 1) / 2])) > sThresh)
					{
						sKMatrix[i*nWidth + j] |= 1 << 15;
						nBadPointNum++;
					}
				}
			}
		}
	}

	if (us_neighboor_list)
	{
		porting_free_mem(us_neighboor_list);
		us_neighboor_list = NULL;
	}

	return nBadPointNum;
}


/*
函数功能：标记空域坏点
输入参数为：
unsigned short *sKMatrix：K矩阵
unsigned short *sX16：连续n帧的X16矩阵
int nHeight, int nWidth：图像宽高
int nWinSize：坏点标记窗口大小
int nFrame：传入X16的帧数
unsigned short sThresh：坏点阈值
返回值：
时域坏点数量
*/
int ToolBox::FindTimeSpaceBadPoint(unsigned short *sKMatrix, unsigned short *sX16, int nHeight, int nWidth, int nWinSize, int nFrame, unsigned short sThresh)
{
	int nBadPointNum = 0;
	double* std = (double*)porting_calloc_mem(nWidth*nHeight, sizeof(double), ITA_OTHER_MODULE);
	memset(std, 0, nWidth*nHeight * sizeof(double));
	double sum;
	double sumStd = 0;
	//计算每个像素点的X16时域标准差
	for (int i = 0; i < nHeight; i++) {
		for (int j = 0; j < nWidth; j++) {
			sum = 0;
			for (int k = 0; k < nFrame; k++) {
				sum += sX16[nHeight*nWidth*k + i*nWidth + j];
			}
			sum = sum / nFrame;
			for (int k = 0; k < nFrame; k++) {
				std[i*nWidth + j] += (sX16[nHeight*nWidth*k + i*nWidth + j] - sum)*(sX16[nHeight*nWidth*k + i*nWidth + j] - sum);
			}
			std[i*nWidth + j] = std[i*nWidth + j] / nFrame;
			sumStd += std[i*nWidth + j];
		}
	}
	//计算全图标准差均值
	sumStd = sumStd*1.0 / (nHeight*nWidth);
	sumStd = sumStd*sThresh;

	//时域坏点判断
	for (int i = 0; i < nHeight; i++) {
		for (int j = 0; j < nWidth; j++) {
			if ((std[i*nWidth + j] > sumStd) && ((sKMatrix[i*nWidth + j] & (1 << 15)) == 0)) {
				sKMatrix[i*nWidth + j] |= 1 << 15;
				nBadPointNum++;
			}
		}
	}

	if (std)
	{
		porting_free_mem(std);
		std = NULL;
	}

	return nBadPointNum;
}


/*
函数功能：标记坏列(返回是否合格)
输入参数为：
unsigned short *sKMatrix：K矩阵
int nHeight, int nWidth：图像宽高
返回值：
是否合格
*/
bool ToolBox::FindBadLine(unsigned short *sKMatrix, int nHeight, int nWidth)
{
	int sum;
	//行
	for (int i = 0; i < nHeight; i++) {
		sum = 0;
		for (int j = 0; j < nWidth; j++) {
			sum += (sKMatrix[i*nWidth + j] >> 15);//BP[i*nWidth + j] ;
		}
		if (sum >(nWidth / 2))
			return false;
	}

	//列
	for (int j = 0; j < nWidth; j++) {
		sum = 0;
		for (int i = 0; i < nHeight; i++) {
			sum += (sKMatrix[i*nWidth + j] >> 15); //BP[i*nWidth + j];
		}
		if (sum >(nHeight / 2))
			return false;
	}
	return true;
}

/*
函数功能：标记坏列(将坏列标记在K中)
输入参数为：
unsigned short *sKMatrix：K矩阵
int nHeight, int nWidth：图像宽高
返回值：
unsigned short *sKMatrix：K矩阵
*/

int ToolBox::FindBadLineInK(unsigned short *sKMatrix, int nHeight, int nWidth)
{
	int sum;
	int signal = 0;
	//行
	for (int i = 0; i < nHeight; i++) {
		sum = 0;
		for (int j = 0; j < nWidth; j++) {
			sum += (sKMatrix[i*nWidth + j] >> 15);//BP[i*nWidth + j] ;
		}
		if (sum >(nWidth / 2))
		{
			signal = 1;
			for (int j = 0; j < nWidth; j++)
			{
				sKMatrix[i*nWidth + j] |= 1 << 15;
			}
		}
	}

	//列
	for (int j = 0; j < nWidth; j++) {
		sum = 0;
		for (int i = 0; i < nHeight; i++) {
			sum += (sKMatrix[i*nWidth + j] >> 15); //BP[i*nWidth + j];
		}
		if (sum >(nHeight / 2))
		{
			signal = 1;
			for (int i = 0; i < nHeight; i++)
			{
				sKMatrix[i*nWidth + j] |= 1 << 15;
			}
		}
	}
	return signal;
}

/*
函数功能：标记坏斑
输入参数为：
unsigned short *sKMatrix：K矩阵
int nHeight, int nWidth：图像宽高
返回值：
unsigned short *sKMatrix：K矩阵
*/
void ToolBox::FindBadArea(unsigned short* sKMatrix, int nHeight, int nWidth, int AAreaWidth, int AAreaHeight, int* nBadAreaNum)
{
	unsigned int* pLabel = (unsigned int*)porting_calloc_mem(nWidth * nHeight, sizeof(unsigned int), ITA_OTHER_MODULE);
	memset(pLabel, 0, nWidth * nHeight * sizeof(unsigned int));//连通域标记初始化
	int i;
	int CAnum = FindConnet(sKMatrix, pLabel, nWidth, nHeight);//连通域个数
	int* badS = (int*)porting_calloc_mem(CAnum, sizeof(int), ITA_OTHER_MODULE);//全图坏斑的大小，最多width*height个坏斑
	memset(badS, 0, CAnum * sizeof(int));
	int* badSA = (int*)porting_calloc_mem(CAnum, sizeof(int), ITA_OTHER_MODULE);//A区坏斑的大小，最多width*height个坏斑
	memset(badSA, 0, CAnum * sizeof(int));
	int* badSB = (int*)porting_calloc_mem(CAnum, sizeof(int), ITA_OTHER_MODULE);//B区坏斑的大小，最多width*height个坏斑
	memset(badSB, 0, CAnum * sizeof(int));
	//A B区域16种坏斑个数统计
	memset(nBadAreaNum, 0, BAD_CLASS_NUM * 3 * sizeof(int));
	int sum = 0;
	int xStart = nWidth / 2 - AAreaWidth / 2;
	int yStart = nHeight / 2 - AAreaHeight / 2;
	int xEnd = xStart + AAreaWidth - 1;
	int yEnd = yStart + AAreaHeight - 1;
	for (i = 0; i < nHeight*nWidth; i++) {
		if (pLabel[i] != 0) {
			if (i / nWidth >= yStart&&i / nWidth <= yEnd&&i % nWidth >= xStart&&i % nWidth <= xEnd) {
				badSA[pLabel[i] - 1]++;
			}
			else
				badSB[pLabel[i] - 1]++;
			badS[pLabel[i] - 1]++;
			sum++;
		}
	}
	for (i = 0; i < CAnum; i++) {
		//全图
		if (badS[i] > 0) {
			if (badS[i] >= 16)
				nBadAreaNum[BAD_CLASS_NUM - 1]++;
			else
				nBadAreaNum[badS[i] - 1]++;
		}
		//A区
		if (badSA[i] > 0) {
			if (badSA[i] >= 16)
				nBadAreaNum[2 * BAD_CLASS_NUM - 1]++;
			else
				nBadAreaNum[badSA[i] - 1 + BAD_CLASS_NUM]++;
		}
		//B区
		if (badSB[i] > 0) {
			if (badSB[i] >= 16)
				nBadAreaNum[3 * BAD_CLASS_NUM - 1]++;
			else
				nBadAreaNum[badSB[i] - 1 + 2 * BAD_CLASS_NUM]++;
		}
	}

	if (pLabel)
	{
		porting_free_mem(pLabel);
		pLabel = NULL;
	}

	if (badS)
	{
		porting_free_mem(badS);
		badS = NULL;
	}

	if (badSA)
	{
		porting_free_mem(badSA);
		badSA = NULL;
	}

	if (badSB)
	{
		porting_free_mem(badSB);
		badSB = NULL;
	}
}



int ToolBox::FindConnet(unsigned short* sKMatrix, unsigned int* pLabel, int nWidth, int nHeight)
{
	int label = 0;
	for (int i = 0; i < nHeight; i++) {
		for (int j = 0; j < nWidth; j++) {
			if ((sKMatrix[i * nWidth + j] >> 15) != 0 && pLabel[i * nWidth + j] == 0) {
				label++;
				pLabel[i * nWidth + j] = label;
				FindNeb8(i, j, label, pLabel, sKMatrix, nWidth, nHeight);
			}
		}
	}
	return label;
}


void ToolBox::FindNeb8(int nX, int nY, int nLabel, unsigned int* pLabel, unsigned short* sKMatrix, int nWidth, int nHeight)
{
	int nNeb[] = { -1,0,1,0,-1,-1,1,-1,-1,1,1,1,0,1,0,-1 };
	for (int i = 0; i < 8; i++) {
		int xi = nX + nNeb[2 * i + 0];
		int yi = nY + nNeb[2 * i + 1];
		if (xi >= 0 && xi < nHeight && yi >= 0 && yi < nWidth) {
			if ((sKMatrix[xi * nWidth + yi] >> 15) != 0 && pLabel[xi * nWidth + yi] == 0) {
				pLabel[xi * nWidth + yi] = nLabel;
				FindNeb8(xi, yi, nLabel, pLabel, sKMatrix, nWidth, nHeight);
			}
		}
	}
}

bool ToolBox::IfShutter(int nShutterSignal)
{
	//不是快门，直接取数据
	//if (nShutterSignal == 0)
	if (nShutterSignal == ITA_STATUS_OPEN)
	{
		return false;
	}
	else
	{
		return true;
	}
}

/*
函数功能：判断曲线是否异常
输入参数为：
short *pCurve：测温曲线
int nCurveLength：曲线长度
返回值：
曲线是否异常
*/
bool ToolBox::IfLegalCurve(short *pCurve, int nCurveLength)
{
	//判断是否单调非递减
	bool bIfGood = true;
	for (int i = 0; i < nCurveLength - 1; i++)
	{
		if (pCurve[i] > pCurve[i + 1])
		{
			bIfGood = false;
			break;
		}
	}
	return bIfGood;
}

void ToolBox::QuickSort(unsigned short *pArr, int nBegin, int nEnd)
{
	if (nBegin > nEnd)
	{
		return;
	}
	unsigned short fTmp = pArr[nBegin];
	int i = nBegin;
	int j = nEnd;
	while (i != j)
	{
		while (pArr[j] >= fTmp && j > i)
		{
			j--;
		}
		while (pArr[i] <= fTmp && j > i)
		{
			i++;
		}
		if (j > i)
		{
			unsigned short t = pArr[i];
			pArr[i] = pArr[j];
			pArr[j] = t;
		}
	}
	pArr[nBegin] = pArr[i];
	pArr[i] = fTmp;
	QuickSort(pArr, nBegin, i - 1);
	QuickSort(pArr, i + 1, nEnd);
}

void ToolBox::QuickSort(double *pArr, int nBegin, int nEnd)
{
	if (nBegin > nEnd)
	{
		return;
	}
	double fTmp = pArr[nBegin];
	int i = nBegin;
	int j = nEnd;
	while (i != j)
	{
		while (pArr[j] >= fTmp && j > i)
		{
			j--;
		}
		while (pArr[i] <= fTmp && j > i)
		{
			i++;
		}
		if (j > i)
		{
			double t = pArr[i];
			pArr[i] = pArr[j];
			pArr[j] = t;
		}
	}
	pArr[nBegin] = pArr[i];
	pArr[i] = fTmp;
	QuickSort(pArr, nBegin, i - 1);
	QuickSort(pArr, i + 1, nEnd);
}

void ToolBox::Polyfit(int nArrLength, double dX[], double dY[], int nPolyN, double dA[])
{
	//porting_calloc_mem(nNotShutterFrame, sizeof(float), ITA_OTHER_MODULE)
	int n = nArrLength;
	int i, j;
	double *pTempX, *pTempY, *pSumXX, *pSumXY, *pAta;
	pTempX = (double*)porting_calloc_mem(n, sizeof(double), ITA_OTHER_MODULE);
	pSumXX = (double*)porting_calloc_mem(nPolyN * 2 + 1, sizeof(double), ITA_OTHER_MODULE);
	pTempY = (double*)porting_calloc_mem(n, sizeof(double), ITA_OTHER_MODULE);
	pSumXY = (double*)porting_calloc_mem(nPolyN + 1, sizeof(double), ITA_OTHER_MODULE);
	pAta = (double*)porting_calloc_mem((nPolyN + 1) * (nPolyN + 1), sizeof(double), ITA_OTHER_MODULE);
	//赋予初值
	for (i = 0; i<n; i++)
	{
		pTempX[i] = 1;
		pTempY[i] = dY[i];
	}
	//计算n，Σxi，Σxi^2......Σxi^n
	for (i = 0; i<2 * nPolyN + 1; i++)
		for (pSumXX[i] = 0, j = 0; j<n; j++)
		{
			pSumXX[i] += pTempX[j];
			pTempX[j] *= dX[j];
		}
	//计算Σyi，Σyi*xi，Σyi*xi^2......Σyi*xi^n
	for (i = 0; i<nPolyN + 1; i++)
		for (pSumXY[i] = 0, j = 0; j<n; j++)
		{
			pSumXY[i] += pTempY[j];
			pTempY[j] *= dX[j];
		}
	for (i = 0; i<nPolyN + 1; i++)
		for (j = 0; j<nPolyN + 1; j++)
			pAta[i*(nPolyN + 1) + j] = pSumXX[i + j];
	GaussSolve(nPolyN + 1, pAta, dA, pSumXY);

	if (pTempX)
	{
		porting_free_mem(pTempX);
		pTempX = NULL;
	}

	if (pSumXX)
	{
		porting_free_mem(pSumXX);
		pSumXX = NULL;
	}

	if (pTempY)
	{
		porting_free_mem(pTempY);
		pTempY = NULL;
	}

	if (pSumXY)
	{
		porting_free_mem(pSumXY);
		pSumXY = NULL;
	}

	if (pAta)
	{
		porting_free_mem(pAta);
		pAta = NULL;
	}
}

void ToolBox::QuickSort(short *pArr, int nBegin, int nEnd)
{
	if (nBegin > nEnd)
	{
		return;
	}
	short sTmp = pArr[nBegin];
	int i = nBegin;
	int j = nEnd;
	while (i != j)
	{
		while (pArr[j] >= sTmp && j > i)
		{
			j--;
		}
		while (pArr[i] <= sTmp && j > i)
		{
			i++;
		}
		if (j > i)
		{
			short t = pArr[i];
			pArr[i] = pArr[j];
			pArr[j] = t;
		}
	}
	pArr[nBegin] = pArr[i];
	pArr[i] = sTmp;
	QuickSort(pArr, nBegin, i - 1);
	QuickSort(pArr, i + 1, nEnd);
}

void ToolBox::QuickSort(float *pArr, int nBegin, int nEnd)
{
	if (nBegin > nEnd)
	{
		return;
	}
	float fTmp = pArr[nBegin];
	int i = nBegin;
	int j = nEnd;
	while (i != j)
	{
		while (pArr[j] >= fTmp && j > i)
		{
			j--;
		}
		while (pArr[i] <= fTmp && j > i)
		{
			i++;
		}
		if (j > i)
		{
			float t = pArr[i];
			pArr[i] = pArr[j];
			pArr[j] = t;
		}
	}
	pArr[nBegin] = pArr[i];
	pArr[i] = fTmp;
	QuickSort(pArr, nBegin, i - 1);
	QuickSort(pArr, i + 1, nEnd);
}

void ToolBox::GaussSolve(int nPolyN, double dA[], double dX[], double dB[])
{
	int i, j, k, r;
	double dMax;
	for (k = 0; k<nPolyN - 1; k++)
	{
		dMax = fabs(dA[k*nPolyN + k]); /*find maxmum*/
		r = k;
		for (i = k + 1; i<nPolyN - 1; i++)
			if (dMax<fabs(dA[i*nPolyN + i]))
			{
				dMax = fabs(dA[i*nPolyN + i]);
				r = i;
			}
		if (r != k)
			for (i = 0; i<nPolyN; i++) /*change array:dA[k]&dA[r] */
			{
				dMax = dA[k*nPolyN + i];
				dA[k*nPolyN + i] = dA[r*nPolyN + i];
				dA[r*nPolyN + i] = dMax;
			}
		dMax = dB[k]; /*change array:dB[k]&dB[r] */
		dB[k] = dB[r];
		dB[r] = dMax;
		for (i = k + 1; i<nPolyN; i++)
		{
			for (j = k + 1; j<nPolyN; j++)
				dA[i*nPolyN + j] -= dA[i*nPolyN + k] * dA[k*nPolyN + j] / dA[k*nPolyN + k];
			dB[i] -= dA[i*nPolyN + k] * dB[k] / dA[k*nPolyN + k];
		}
	}

	for (i = nPolyN - 1; i >= 0; dX[i] /= dA[i*nPolyN + i], i--)
		for (j = i + 1, dX[i] = dB[i]; j<nPolyN; j++)
			dX[i] -= dA[i*nPolyN + j] * dX[j];
}

double ToolBox::Polyval(double dX, double dA[3])
{
	double dVal = dA[2] * dX * dX + dA[1] * dX + dA[0];
	return dVal;
}

int ToolBox::DoubleToInt(double dValue)
{
	if (dValue >= 0)
	{
		return (int)(dValue + 0.5);
	}
	else
	{
		return (int)(dValue - 0.5);
	}
}