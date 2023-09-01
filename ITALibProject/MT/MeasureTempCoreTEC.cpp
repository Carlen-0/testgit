#include "MeasureTempCoreTEC.h"
#include <float.h>
#include <algorithm>

/*函数功能：构造函数
输入参数：
int w:							图像宽
int h:							图像高
int curveLength:				曲线总长度
short *pusCurve:				曲线数据
IMAGE_MEASURE_STRUCT *params:	测温结构体
short *y16Image:				图片原始Y16数据
返回参数：
无
*/
MeasureTempCoreTEC::MeasureTempCoreTEC(int w, int h, int curveLength, short *pusCurve,IMAGE_MEASURE_STRUCT *params)
{
	////获取SDK版本
	//GetSDKVerision();

	//获取图像宽高
	/*ImageWidth = w;
	ImageHidth = h;*/

	//设置曲线长度
	curveDataLength = curveLength;

	//曲线赋值
	CurveData = pusCurve;

	//测温结构体赋值
	TMparams = params;
	m_fTempMatrix = NULL;
	m_tempLen = 0;
	m_logger = NULL;
	m_sortY16 = NULL;
	m_sortLen = 0;
	////计算快门温漂、镜筒温漂
	//ShutterTempDriftCorr();
	//LensTempDriftCorr();

	////初始化反射温度对应的Y16
	//TMparams->sY16Reflect = GetY16FromT(TMparams->fReflectT * 10);

	////初始化窗口温度对应的Y16
	//TMparams->sY16Windows = GetY16FromT(TMparams->fWindowTemperature * 10);
}


/*函数功能：析构函数
输入参数：
无
返回参数：
无
*/
MeasureTempCoreTEC::~MeasureTempCoreTEC() 
{
	m_logger->output(LOG_INFO, "MeasureTempCoreTEC1 %#x", m_fTempMatrix);
	if (m_fTempMatrix)
	{
		porting_free_mem(m_fTempMatrix);
		m_fTempMatrix = NULL;
		m_tempLen = 0;
	}
	m_logger->output(LOG_INFO, "MeasureTempCoreTEC2 %#x", m_sortY16);
	if (m_sortY16)
	{
		porting_free_mem(m_sortY16);
		m_sortY16 = NULL;
		m_sortLen = 0;
	}
	m_logger->output(LOG_INFO, "MeasureTempCoreTEC3");
	/*if (CurveData)
	{
		free(CurveData);
	}
	CurveData = NULL;*/
}

//// 获取测温SDK版本
//char* TempMeasureCore::GetSDKVerision()
//{
//	printf("MtVersion: %s\n", VERSION);
//	return VERSION;
//}

//// 刷新Y16、参数行、更新测温参数
//void TempMeasureCore::RefreshMeasureParam(IMAGE_MEASURE_STRUCT &Params, short *SrcY16)
//{
//	memcpy(&TMparams, &Params, sizeof(IMAGE_MEASURE_STRUCT));
//
//	//刷新Y16
//	memcpy(pSrcY16, SrcY16, ImageWidth * ImageHidth * sizeof(short));
//
//	//刷新快门温漂、镜筒温漂
//	ShutterTempDriftCorr();
//	LensTempDriftCorr();
//
//	//反查快门温对应的Y16
//	int shutterTemp = int(TMparams->fRealTimeShutterTemp * 10);
//	int TempIndex = 0;
//	for (int i = 0; i < curveDataLength; i++)
//	{
//		if (CurveData[i] > shutterTemp)
//		{
//			TempIndex = i;
//			break;
//		}
//	}
//	for (int i = curveDataLength - 1; i > 0; i--)
//	{
//		if (CurveData[i] < shutterTemp)
//		{
//			TMparams->nShutterCurveIndex = i;
//			break;
//		}
//	}
//	TMparams->nShutterCurveIndex = int((TMparams->nShutterCurveIndex + TempIndex) / 2);
//
//	//反射温度发生了变化，才再次调用
//	if (TMparams->bIsReflectTChange)
//	{
//		TMparams->sY16Reflect = GetY16FromT(TMparams->fReflectT * 10);
//	}
//
//	//窗口温度发生了变化，才再次调用
//	if (TMparams->bIsWindowsTChange)
//	{
//		TMparams->sY16Windows = GetY16FromT(TMparams->fWindowTemperature * 10);
//	}
//
//	//判断距离参数类型
//	if (TMparams->nDistance_a6 == -1 && TMparams->nDistance_a7 == -1 && TMparams->nDistance_a8 == -1)
//	{
//		TMparams->disType = 0;
//	}
//	else
//	{
//		TMparams->disType = 1;
//	}
//}


//刷新曲线，切换档位或测温范围时调用
//void TempMeasureCore::RefreshCurve(short *curve)
//{
//	memcpy(CurveData, curve, curveDataLength * sizeof(short));
//}


void MeasureTempCoreTEC::reload(int curveLength, short * pusCurve, IMAGE_MEASURE_STRUCT * params)
{
	//设置曲线长度
	curveDataLength = curveLength;
	//曲线赋值
	CurveData = pusCurve;
	//测温结构体赋值
	TMparams = params;
}

/*函数功能：测温接口，将Y16数据转化为温度
输入参数：
short y16value：	原始Y16
返回参数：
float &Temp：		Y16对应的温度
*/
void MeasureTempCoreTEC::GetTempByY16(float &Temp, short y16value)
{

	//反查快门温对应的Y16
	int shutterTemp = int(TMparams->fRealTimeShutterTemp * 10);
	int TempIndex = 0;
	for (int i = 0; i < curveDataLength; i++)
	{
		if (CurveData[i] > shutterTemp)
		{
			TempIndex = i;
			break;
		}
	}
	for (int i = curveDataLength - 1; i > 0; i--)
	{
		if (CurveData[i] < shutterTemp)
		{
			nShutterCurveIndex = i;
			break;
		}
	}
	nShutterCurveIndex = int((nShutterCurveIndex + TempIndex) / 2);

	////反射温度发生了变化，才再次调用
	//if (TMparams->bIsReflectTChange)
	//{
	//	TMparams->sY16Reflect = GetY16FromT(TMparams->fReflectT * 10);
	//}

	////窗口温度发生了变化，才再次调用
	//if (TMparams->bIsWindowsTChange)
	//{
	//	TMparams->sY16Windows = GetY16FromT(TMparams->fWindowTemperature * 10);
	//}

	//Y16修正
	y16value += TMparams->sY16Offset;

	//湿度修正
	if (TMparams->bHumidityCorrection)
	{
		y16value = HumidityCorrection(y16value, TMparams->fHumidity);
	}

	//float fTemp;
	//温漂修正
	Temp = StandardY16ToTempWithY16Correction(y16value);

	//B1修正
	if (TMparams->bB1Correction)
	{
		Temp = Temp + ((TMparams->nB1 + TMparams->nB1Offset) / 100.0f);
	}

	//距离修正
	if (TMparams->bDistanceCorrection)
	{
		if (TMparams->nDistance_a6 == -1 && TMparams->nDistance_a7 == -1 && TMparams->nDistance_a8 == -1)
		{
			Temp = DistanceCorrectionWithSixPara(Temp);
		}
		else
		{
			Temp = DistanceCorrectionWithNinePara(Temp);
		}
	}

	//发射率修正
	if (TMparams->bEmissCorrection)
	{
		Temp = EmissionCorrectionNew(Temp);
	}


	if (TMparams->mtType)
	{
		//环温修正
		if (TMparams->bAmbientCorrection)
		{
			Temp = EnvironmentCorrection(Temp);
		}

		//大气透过率修正
		if (TMparams->bAtmosphereCorrection)
		{
			Temp = AtmosphereCorrection(Temp);
		}

		//窗口透过率修正
		if (TMparams->bWindowTransmittanceCorrection)
		{
			Temp = WindowTransmittanceCorrection(Temp);
		}
	}
}

//// 坐标点测温
//void TempMeasureCore::GetTempByCoord(float &fTemp, unsigned short x, unsigned short y)
//{
//	unsigned short y16value = 0;
//
//	if (x > 0 && x < ImageWidth - 1 && y > 0 && y < ImageHidth - 1)
//	{
//		y16value = pSrcY16[(y - 1) * ImageWidth + x - 1] + pSrcY16[(y - 1) * ImageWidth + x] + pSrcY16[(y - 1) * ImageWidth + x + 1] +
//			pSrcY16[y * ImageWidth + x - 1] + pSrcY16[y * ImageWidth + x] + pSrcY16[y * ImageWidth + x + 1] +
//			pSrcY16[(y + 1) * ImageWidth + x - 1] + pSrcY16[(y + 1) * ImageWidth + x] + pSrcY16[(y + 1) * ImageWidth + x + 1];
//	}
//	else
//	{
//		y16value = pSrcY16[y * ImageWidth + x];
//	}
//
//	y16value = y16value / 9.0;
//
//	GetTempByY16(fTemp, y16value);
//}


/*函数功能：温度反算Y16，将温度数据转化为y16
输入参数：
float &Temp：		原始温度
返回参数：
short y16value：	温度对应的Y16
*/
void MeasureTempCoreTEC::CalY16ByTemp(short &y16, float fTemp)
{
	if (TMparams->bLogPrint)
	{
		m_logger->output(LOG_DEBUG, "CalY16ByTemp:%d %.2f %d %d", y16, fTemp, TMparams->mtType, TMparams->nGear);
	}
	if (TMparams->mtType)
	{
		//窗口透过率修正反算
		if (TMparams->bWindowTransmittanceCorrection)
		{
			fTemp = WindowTransmittanceReverseCorrection(fTemp);
		}
		//大气透过率修正反算
		if (TMparams->bAtmosphereCorrection)
		{
			fTemp = AtmosphereReverseCorrection(fTemp);
		}
		//环温修正反算
		if (TMparams->bAmbientCorrection)
		{
			fTemp = EnvironmentReverseCorrection(fTemp);
		}
	}

	//发射率修正反算
	if (TMparams->bEmissCorrection)
	{
		fTemp = EmissionReverseCorrectionNew(fTemp);
	}
	//距离修正反算
	if (TMparams->bDistanceCorrection)
	{
		if (TMparams->nDistance_a6 == -1 && TMparams->nDistance_a7 == -1 && TMparams->nDistance_a8 == -1)
		{
			fTemp = DistanceReverseCorrectionWithSixPara(fTemp);
		}
		else
		{
			fTemp = DistanceReverseCorrectionWithNinePara(fTemp);
		}
	}
	//B1修正反算
	if (TMparams->bB1Correction)
	{
		fTemp = fTemp - ((TMparams->nB1 + TMparams->nB1Offset) / 100.0f);
	}
	//反查快门温对应的Y16
	int shutterTemp = int(TMparams->fRealTimeShutterTemp * 10);
	int TempIndex = 0;
	for (int i = 0; i < curveDataLength; i++)
	{
		if (CurveData[i] > shutterTemp)
		{
			TempIndex = i;
			break;
		}
	}
	for (int i = curveDataLength - 1; i > 0; i--)
	{
		if (CurveData[i] < shutterTemp)
		{
			nShutterCurveIndex = i;
			break;
		}
	}
	nShutterCurveIndex = int((nShutterCurveIndex + TempIndex) / 2);
	//温漂修正反算
	short y16_min = StandardTempToY16(fTemp, false);
	short y16_max = StandardTempToY16(fTemp, true);
	y16 = short((y16_min + y16_max) / 2);


	//湿度修正反算
	if (TMparams->bHumidityCorrection)
	{
		y16 = HumidityReverseCorrection(y16, TMparams->fHumidity);
	}
	y16 -= TMparams->sY16Offset;
	if (TMparams->bLogPrint)
	{
		m_logger->output(LOG_DEBUG, "CalY16ByTemp:%d %.2f", y16, fTemp);
	}
}

int MeasureTempCoreTEC::CalY16MatrixByTempMatrix(float * pTempMatrix, short * pDst, int nImageHeight, int nImageWidth)
{
	memset(pDst, 0, nImageHeight * nImageWidth * sizeof(short));
	//计算最大最小温度
	float fMin = 2500.f;
	float fMax = -40.f;
	for (int i = 0; i < nImageHeight; i++)
	{
		for (int j = 0; j < nImageWidth; j++)
		{
			float tmp = pTempMatrix[i * nImageWidth + j];
			//超出量程
			if (tmp > 2500.f || tmp <-40.f)
			{
				return ITA_MT_OUT_OF_RANGE;
			}
			if (tmp >= fMax)
			{
				fMax = tmp;
			}
			if (tmp <= fMin)
			{
				fMin = tmp;
			}
		}
	}

	//反算Y16表
	int nNum = (int)((fMax - fMin) / 0.1f);
	short sValue[25400] = { 0 };
	for (int i = 0; i <= nNum; i++)
	{
		float fCurrTemp = fMin + 0.1f * i;
		short sCurrY16;
		CalY16ByTemp(sCurrY16, fCurrTemp);
		sValue[i] = sCurrY16;
	}

	//计算原图Y16
	for (int i = 0; i < nImageHeight; i++)
	{
		for (int j = 0; j < nImageWidth; j++)
		{
			int index = i * nImageWidth + j;
			pDst[index] = sValue[int((pTempMatrix[index] - fMin) * 10)];
		}
	}
	return 0;
}

/*函数功能：Y16修正，包括快门温漂修正、镜头温漂修正、KF调整
输入参数：
short y16value：		原始Y16
返回参数：
float temp：			Y16对应的温度
*/
float MeasureTempCoreTEC::StandardY16ToTempWithY16Correction(short y16value)
{

	//刷新快门温漂、镜筒温漂
	ShutterTempDriftCorr();
	LensTempDriftCorr();

	if (TMparams->bShutterCorrection)
	{
		y16value = y16value - (short)fCorrection1;
	}

	if (TMparams->bLensCorrection)
	{
		y16value = y16value - (short)fCorrection2;
	}

	int index = (int)(y16value * ((TMparams->nKF + TMparams->nKFOffset) / 100.f)) + nShutterCurveIndex;
	if (index < 0) 
	{
		index = 0;
	}
	else if (index >= curveDataLength) 
	{
		index = curveDataLength - 1;
	}

	int temp = CurveData[index];

	return temp / 10.0f;
}


/*函数功能：湿度修正
输入参数：
float fObjY16：			目标Y16
float fHumidity：		湿度值
返回参数
short Y16：				修正后Y16
*/
short MeasureTempCoreTEC::HumidityCorrection(short fObjY16, int fHumidity)
{
	short Y16 = 0;
	Y16 = short((1.0f - (fHumidity - 60) / 5000.0f) * fObjY16);
	return Y16;
}


/*函数功能：快门温漂修正，自动计算快门温漂
输入参数：
无
返回参数
无
*/
void MeasureTempCoreTEC::ShutterTempDriftCorr()
{
	fCorrection1 = TMparams->nK1 / 100.f * (TMparams->fRealTimeShutterTemp - TMparams->fOrinalShutterTemp);
}

/*函数功能：镜筒温漂修正，自动计算镜筒温漂
输入参数：
无
返回参数
无
*/
void MeasureTempCoreTEC::LensTempDriftCorr()
{
	fCorrection2 = TMparams->nK2 / 100.f * (TMparams->fRealTimeLensTemp - TMparams->fCurrentLensTemp);
}

/*函数功能：发射率修正
输入参数：
float iTemperature：		发射率修正前温度
返回参数
float Temp：				发射率修正后温度
*/
float MeasureTempCoreTEC::EmissionCorrectionNew(float iTemperature)
{
	short sY16Reflect = GetY16FromT(int(TMparams->fReflectT * 10));
	short nTNoEmiss = short(iTemperature * 10);
	int nEmiss = int(TMparams->fEmiss * 100);//
	short TEmiss = EmissCor(nTNoEmiss, sY16Reflect, nEmiss);
	//short DeTEmiss = DeEmissCor(nTNoEmiss, sY16Reflect, nEmiss);
	return TEmiss / 10.0f;
}

short MeasureTempCoreTEC::EmissCor(short nTNoEmiss, short sY16Reflect, int nEmiss)
{
	//小于0异常，直接返回最高温
	if (nEmiss <= 0)
		return nEmissCurve[nEmissCurveLen - 1];
	//发射率大于0.98，不做发射率校正
	if (nEmiss > 98)
		return nTNoEmiss;

	int nY16NoEmiss;
	int nTEmiss, nTEmissFloor, nTEmissCeil;
	int nEmissIndexFloor, nEmissIndexCeil;
	//float fEmissIndex, fY16NoEmiss;
	float fEmissIndex, fY16NoEmiss;
	/*short Y16NoEmiss = GetY16FromT(TNoEmiss, mBoschParam, NewCurveBuffer);*/

	//得到未做发射率校正的温度对应的辐射量
	if (nTNoEmiss >= nEmissCurve[nEmissCurveLen - 1])
	{
		nY16NoEmiss = nEmissCurveLen - 1;
	}
	else if (nTNoEmiss <= nEmissCurve[0])
	{
		nY16NoEmiss = 0;
	}
	else
	{
		int nMidTemp;
		int nLeftValue = 0, nRightValue = nEmissCurveLen - 1, nMidValue;
		while (nLeftValue <= nRightValue)
		{
			nMidValue = (nLeftValue + nRightValue) >> 1;
			nMidTemp = nEmissCurve[nMidValue];
			if (nMidTemp < nTNoEmiss)
				nLeftValue = nMidValue + 1;
			else if (nMidTemp > nTNoEmiss)
				nRightValue = nMidValue - 1;
			else
			{
				break;
			}
		}
		nY16NoEmiss = nMidValue;
	}

	//提高精度，将整型nY16NoEmiss转成浮点型fY16NoEmiss //插值减小温度波动的幅值
	if ((nY16NoEmiss > 0) && (nY16NoEmiss < (nEmissCurveLen - 1)))
	{
		int nDeltaT = nTNoEmiss - nEmissCurve[nY16NoEmiss];
		if (nDeltaT > 0)
			fY16NoEmiss = nY16NoEmiss + ((nTNoEmiss - nEmissCurve[nY16NoEmiss]) * 1.0f / (nEmissCurve[nY16NoEmiss + 1] - nEmissCurve[nY16NoEmiss]));
		else if (nDeltaT < 0)
			fY16NoEmiss = nY16NoEmiss - ((nEmissCurve[nY16NoEmiss] - nTNoEmiss) * 1.0f / (nEmissCurve[nY16NoEmiss] - nEmissCurve[nY16NoEmiss - 1]));
		else
			fY16NoEmiss = float(nY16NoEmiss);
	}
	else
		fY16NoEmiss = float(nY16NoEmiss);

	//发射率修正
	fEmissIndex = (fY16NoEmiss * 100.0f - (100.0f - nEmiss) * sY16Reflect) / nEmiss;
	nEmissIndexFloor = int(fEmissIndex);
	if (nEmissIndexFloor < 0)
	{
		nEmissIndexFloor = 0;
	}
	else if (nEmissIndexFloor >= nEmissCurveLen)
	{
		nEmissIndexFloor = nEmissCurveLen - 1;
	}
	nTEmissFloor = nEmissCurve[nEmissIndexFloor];

	nEmissIndexCeil = nEmissIndexFloor + 1;
	if (nEmissIndexCeil < 0)
	{
		nEmissIndexCeil = 0;
	}
	else if (nEmissIndexCeil >= nEmissCurveLen)
	{
		nEmissIndexCeil = nEmissCurveLen - 1;
	}
	nTEmissCeil = nEmissCurve[nEmissIndexCeil];

	//插值减小温度波动的幅值
	nTEmiss = int(nTEmissFloor + (nTEmissCeil - nTEmissFloor) * (fEmissIndex - nEmissIndexFloor));

	return (short)nTEmiss;
}


/*函数功能：反查发射率曲线
输入参数：
int ReflectT：			反射温度
返回参数
short nReflectY16：		反射温度对应的Y16
*/
short MeasureTempCoreTEC::GetY16FromT(int ReflectT)
{
	int nReflectY16;
	//得到未做发射率校正的温度对应的Y16
	if (ReflectT >= nEmissCurve[nEmissCurveLen - 1])
	{
		nReflectY16 = nEmissCurveLen - 1;
		return (short)nReflectY16;
	}
	else if (ReflectT <= nEmissCurve[0])
	{
		nReflectY16 = 0;
		return (short)nReflectY16;
	}
	else
	{
		int nMidTemp;
		int nLeftValue = 0, nRightValue = nEmissCurveLen - 1, nMidValue;
		while (nLeftValue <= nRightValue)
		{
			nMidValue = (nLeftValue + nRightValue) >> 1;
			nMidTemp = nEmissCurve[nMidValue];
			if (nMidTemp < ReflectT)
				nLeftValue = nMidValue + 1;
			else if (nMidTemp > ReflectT)
				nRightValue = nMidValue - 1;
			else
			{
				break;
			}
		}
		nReflectY16 = nMidValue;
	}
	return (short)nReflectY16;
}


/*函数功能：距离修正（6参数）
输入参数：
float iTemperature：		距离修正前温度
返回参数
float Temp：				距离修正后温度
*/
float MeasureTempCoreTEC::DistanceCorrectionWithSixPara(float iTemperature)
{
	double a0 = TMparams->nDistance_a0 / 1000000000.0;
	double a1 = TMparams->nDistance_a1 / 1000000000.0;
	double b0 = TMparams->nDistance_a2 / 1000000.0;
	double b1 = TMparams->nDistance_a3 / 1000000.0;
	double c0 = TMparams->nDistance_a4 / 1000.0;
	double c1 = TMparams->nDistance_a5 / 1000.0;


	double a = a0 * TMparams->fDistance + a1;
	double b = b0 * TMparams->fDistance + b1;
	double c = c0 * TMparams->fDistance + c1;

	if (a == 0 && b == 0) {
		return iTemperature;
	}
	else {
		float x = iTemperature + 273.5f;
		float y = float(a * x * x + b * x + c);

		return y / 10.0f - 273.5f;
	}
}

/*函数功能：距离修正（9参数）
输入参数：
float iTemperature：		距离修正前温度
返回参数
float Temp：				距离修正后温度
*/
float MeasureTempCoreTEC::DistanceCorrectionWithNinePara(float iTemperature)
{
	double a0 = TMparams->nDistance_a0 / 10000000000.0;
	double a1 = TMparams->nDistance_a1 / 1000000000.0;
	double a2 = TMparams->nDistance_a2 / 10000000.0;
	double b0 = TMparams->nDistance_a3 / 100000.0;
	double b1 = TMparams->nDistance_a4 / 100000.0;
	double b2 = TMparams->nDistance_a5 / 10000.0;
	double c0 = TMparams->nDistance_a6 / 1000.0;
	double c1 = TMparams->nDistance_a7 / 100.0;
	double c2 = TMparams->nDistance_a8 / 100.0;

	double a = a0 * TMparams->fDistance * TMparams->fDistance + a1 * TMparams->fDistance + a2;
	double b = b0 * TMparams->fDistance * TMparams->fDistance + b1 * TMparams->fDistance + b2;
	double c = c0 * TMparams->fDistance * TMparams->fDistance + c1 * TMparams->fDistance + c2;

	if (a == 0 && b == 0) {
		return iTemperature;
	}
	else {
		float x = iTemperature + 273.5f;
		float y = float(a * x * x + b * x + c);

		return y / 10.0f - 273.5f;
	}
}

/*函数功能：环温修正
输入参数：
float iTemperature：		环温修正前温度
返回参数
float Temp：				环温修正后温度
*/
float MeasureTempCoreTEC::EnvironmentCorrection(float temp)
{
	float correction = TMparams->nK3 / 10000.f * temp + TMparams->nB2 / 10000.f;
	float delta = 0.0f;
	if (TMparams->nGear == 0) {
		delta = (TMparams->fAmbient - 3) * correction;
	}
	else if (TMparams->nGear == 1) {
		delta = (TMparams->fAmbient - 23) * correction;
	}
	if (TMparams->bLogPrint)
	{
		m_logger->output(LOG_DEBUG, "EC %f %f %d %d %d %f %f %f", 
			temp, temp - delta, TMparams->nGear, TMparams->nK3, TMparams->nB2, correction, TMparams->fAmbient, delta);
	}
	return temp - delta;
}

////亿嘉和环温修正
//float TempMeasureCore::YJHEnvironmentCorrection(float temp)
//{
//	float delta = 0;
//	if (TMparams->Ambient < 23)
//	{
//		float correction = TMparams->K3 / 10000.f * temp + TMparams->B2 / 10000.f;
//		delta = (TMparams->Ambient - 23) * correction;
//	}
//	else if (TMparams->Ambient > 23)
//	{
//		float correction = TMparams->K4 / 10000.f * temp + TMparams->B3 / 10000.f;
//		delta = (TMparams->Ambient - 23) * correction;
//	}
//	else
//	{
//		delta = 0;
//	}
//	return temp - delta;
//}
//
////亿嘉和环温修正反算
//float TempMeasureCore::YJHEnvironmentReverseCorrection(float temp)
//{
//	float tempz = 0.f;
//	if (TMparams->Ambient < 23)
//	{
//		tempz = (temp + (TMparams->B2 / 10000.f)*(TMparams->Ambient - 23)) / (1 - (TMparams->Ambient - 23)*(TMparams->K3 / 10000.f));
//		return tempz;
//	}
//	else if (TMparams->Ambient > 23)
//	{
//		tempz = (temp + (TMparams->B3 / 10000.f)*(TMparams->Ambient - 23)) / (1 - (TMparams->Ambient - 23)*(TMparams->K4 / 10000.f));
//		return tempz;
//	}
//	else
//	{
//		return temp;
//	}
//}


/*函数功能：发射率修正反算
输入参数：
float iTemperature：		发射率修正后温度
返回参数
float Temp：				发射率修正前温度
*/
float MeasureTempCoreTEC::EmissionReverseCorrectionNew(float iTemperature)
{
	short sY16Reflect = GetY16FromT(int(TMparams->fReflectT * 10));
	short nTNoEmiss = short(iTemperature * 10);
	int nEmiss = int(TMparams->fEmiss * 100);//
	short DeTEmiss = DeEmissCor(nTNoEmiss, sY16Reflect, nEmiss);
	return DeTEmiss / 10.0f;
}

short MeasureTempCoreTEC::DeEmissCor(short nTEmiss, short sY16Reflect, int nEmiss)
{
	int nY16Emiss;
	int nTNoEmiss;
	if (nTEmiss >= nEmissCurve[nEmissCurveLen - 1])
	{
		nY16Emiss = nEmissCurveLen - 1;
	}
	else if (nTEmiss <= nEmissCurve[0])
	{
		nY16Emiss = 0;
	}
	else
	{
		int nMidTemp;
		int nLeftValue = 0, nRightValue = nEmissCurveLen - 1, nMidValue;
		while (nLeftValue <= nRightValue)
		{
			nMidValue = (nLeftValue + nRightValue) >> 1;
			nMidTemp = nEmissCurve[nMidValue];
			if (nMidTemp < nTEmiss)
				nLeftValue = nMidValue + 1;
			else if (nMidTemp > nTEmiss)
				nRightValue = nMidValue - 1;
			else
			{
				break;
			}
		}
		nY16Emiss = nMidValue;
	}

	/******************20210810 改发射率反算浮点取整问题********************************/
	float fNoEmissIndex, fY16Emiss;
	int nNoEmissIndexFloor, nNoEmissIndexCeil;
	int nTNoEmissFloor, nTNoEmissCeil;
	if ((nY16Emiss > 0) && (nY16Emiss < (nEmissCurveLen - 1)))
	{
		int nDeltaT = nTEmiss - nEmissCurve[nY16Emiss];
		if (nDeltaT > 0)
			fY16Emiss = nY16Emiss + ((nTEmiss - nEmissCurve[nY16Emiss]) * 1.0f / (nEmissCurve[nY16Emiss + 1] - nEmissCurve[nY16Emiss]));
		else if (nDeltaT < 0)
			fY16Emiss = nY16Emiss - ((nEmissCurve[nY16Emiss] - nTEmiss) * 1.0f / (nEmissCurve[nY16Emiss] - nEmissCurve[nY16Emiss - 1]));
		else
			fY16Emiss = float(nY16Emiss);
	}
	else
		fY16Emiss = float(nY16Emiss);

	//发射率修正
	fNoEmissIndex = (fY16Emiss * nEmiss + (100 - nEmiss) * sY16Reflect) / 100;
	//fEmissIndex = (fY16Emiss * 1000.0 - (1000.0 - nEmiss * 10) * nY16Reflect) / (nEmiss * 10);
	nNoEmissIndexFloor = int(fNoEmissIndex);
	if (nNoEmissIndexFloor < 0)
	{
		nNoEmissIndexFloor = 0;
	}
	else if (nNoEmissIndexFloor >= nEmissCurveLen)
	{
		nNoEmissIndexFloor = nEmissCurveLen - 1;
	}
	nTNoEmissFloor = nEmissCurve[nNoEmissIndexFloor];

	nNoEmissIndexCeil = nNoEmissIndexFloor + 1;
	if (nNoEmissIndexCeil < 0)
	{
		nNoEmissIndexCeil = 0;
	}
	else if (nNoEmissIndexCeil >= nEmissCurveLen)
	{
		nNoEmissIndexCeil = nEmissCurveLen - 1;
	}
	nTNoEmissCeil = nEmissCurve[nNoEmissIndexCeil];

	//插值减小温度波动的幅值
	nTNoEmiss = int(nTNoEmissFloor + (nTNoEmissCeil - nTNoEmissFloor) * (fNoEmissIndex - nNoEmissIndexFloor));

	return (short)nTNoEmiss;
	/******************20210810 改发射率反算浮点取整问题********************************/
}

//距离修正反算--九参数
float MeasureTempCoreTEC::DistanceReverseCorrectionWithNinePara(float iTemperature)
{
	if (TMparams->fDistance > 0)
	{
		double a0 = TMparams->nDistance_a0 / 10000000000.0;
		double a1 = TMparams->nDistance_a1 / 1000000000.0;
		double a2 = TMparams->nDistance_a2 / 10000000.0;
		double b0 = TMparams->nDistance_a3 / 100000.0;
		double b1 = TMparams->nDistance_a4 / 100000.0;
		double b2 = TMparams->nDistance_a5 / 10000.0;
		double c0 = TMparams->nDistance_a6 / 1000.0;
		double c1 = TMparams->nDistance_a7 / 100.0;
		double c2 = TMparams->nDistance_a8 / 100.0;

		double a = a0 * TMparams->fDistance * TMparams->fDistance + a1 * TMparams->fDistance + a2;
		double b = b0 * TMparams->fDistance * TMparams->fDistance + b1 * TMparams->fDistance + b2;
		double c = c0 * TMparams->fDistance * TMparams->fDistance + c1 * TMparams->fDistance + c2;

		float after = (iTemperature + 273.5f) * 10;

		float y = 0;
		if (a == 0)
		{
			if (b == 0)
			{
				return iTemperature;
			}
			else
			{
				return float((after - c) / b - 273.5f);
			}
		}
		else
		{
			//解二元一次方程
			y = float((pow(b * b + 4 * a * after - 4 * a * c, (float)0.5) - b) / a / 2);

			return y - 273.5f;
		}
	}
	else
	{
		return iTemperature;
	}
}


//距离修正反算--六参数
float MeasureTempCoreTEC::DistanceReverseCorrectionWithSixPara(float iTemperature)
{
	if (TMparams->fDistance > 0)
	{
		double a0 = TMparams->nDistance_a0 / 1000000000.0;
		double a1 = TMparams->nDistance_a1 / 1000000000.0;
		double b0 = TMparams->nDistance_a2 / 1000000.0;
		double b1 = TMparams->nDistance_a3 / 1000000.0;
		double c0 = TMparams->nDistance_a4 / 1000.0;
		double c1 = TMparams->nDistance_a5 / 1000.0;

		double a = a0 * TMparams->fDistance + a1;
		double b = b0 * TMparams->fDistance + b1;
		double c = c0 * TMparams->fDistance + c1;

		float after = (iTemperature + 273.5f) * 10;

		float y = 0;
		if (a == 0)
		{
			if (b == 0)
			{
				return iTemperature;
			}
			else
			{
				return float((after - c) / b - 273.5f);
			}
		}
		else
		{
			//解二元一次方程
			y = float((pow(b * b + 4 * a * after - 4 * a * c, (float)0.5) - b) / a / 2);

			return y - 273.5f;
		}
	}
	else
	{
		return iTemperature;
	}
}

//湿度修正反算
short MeasureTempCoreTEC::HumidityReverseCorrection(short y16, int fHumidity)
{
	short y = 0;
	y = short(y16 * (5000.0f / (5060 - fHumidity)));
	return y;
}

//环温修正反算
float  MeasureTempCoreTEC::EnvironmentReverseCorrection(float temp)
{
	float tempz = 0.f;
	if (TMparams->nGear == 0) {
		tempz = (temp + (TMparams->nB2 / 10000.f)*(TMparams->fAmbient - 3)) / (1 - (TMparams->fAmbient - 3)*(TMparams->nK3 / 10000.f));
	}
	else if (TMparams->nGear == 1) {
		tempz = (temp + (TMparams->nB2 / 10000.f)* (TMparams->fAmbient - 23)) / (1 - (TMparams->fAmbient - 23)*(TMparams->nK3 / 10000.f));
	}
	return tempz;
}


//温漂修正反算
short MeasureTempCoreTEC::StandardTempToY16(float iTemperature, bool fromSmallToBig)
{
	//刷新快门温漂、镜筒温漂
	ShutterTempDriftCorr();
	LensTempDriftCorr();
	
	//反查曲线
	int temp = (int)(iTemperature * 10);

	int index = 0;
	if (fromSmallToBig)
	{
		index = curveDataLength - 1;
		for (int i = 0; i < curveDataLength; i++)
		{
			if (CurveData[i] >= temp)
			{
				index = i;
				break;
			}
		}
	}
	else
	{
		for (int i = curveDataLength - 1; i >= 0; i--)
		{
			if (CurveData[i] <= temp)
			{
				index = i;
				break;
			}
		}
	}

	//KF修正反算
	int y16value = (int)((index - nShutterCurveIndex) / ((TMparams->nKF + TMparams->nKFOffset) / 100.0));

	//温漂修正反算
	if (TMparams->bShutterCorrection) {
		y16value = y16value + (short)fCorrection1;
	}
	if (TMparams->bLensCorrection) {
		y16value = y16value + (short)fCorrection2;
	}

	if (y16value > (short)MAXSHORT)
	{
		y16value = (short)MAXSHORT;
	}
	else if (y16value < (short)MINSHORT)
	{
		y16value = (short)MINSHORT;
	}

	return (short)y16value;
}

//大气透过率修正
float MeasureTempCoreTEC::AtmosphereCorrection(float iTemperature)
{
	//float coEfficient_relhum = 1.0f - (TMparams->nAtmosphereTransmittance - 60) / 5000.0f;
	//iTemperature *= coEfficient_relhum;
	//return iTemperature;
	short sY16Reflect = GetY16FromT(int(TMparams->fAtmosphereTemperature * 10));
	short nTNoAtmosphere = short(iTemperature * 10);
	int nTransmittance = TMparams->nAtmosphereTransmittance;//
	short TEmiss = EmissCor(nTNoAtmosphere, sY16Reflect, nTransmittance);
	//short DeTEmiss = DeEmissCor(nTNoEmiss, sY16Reflect, nEmiss);
	return TEmiss / 10.0f;
}

//大气透过率反算
float MeasureTempCoreTEC::AtmosphereReverseCorrection(float iTemperature)
{
	//float reverse_coff_relhum = 1.0f / (1.0f - ((TMparams->nAtmosphereTransmittance - 60) / 5000.0f));
	//iTemperature *= reverse_coff_relhum;
	//return iTemperature;
	short sY16Reflect = GetY16FromT(int(TMparams->fAtmosphereTemperature * 10));
	short nTNoAtmosphere = short(iTemperature * 10);
	int nTransmittance = TMparams->nAtmosphereTransmittance;//
	short DeTEmiss = DeEmissCor(nTNoAtmosphere, sY16Reflect, nTransmittance);
	return DeTEmiss / 10.0f;
}


//窗口透过率修正
float MeasureTempCoreTEC::WindowTransmittanceCorrection(float iTemperature)
{
	short sY16Windows = GetY16FromT(int(TMparams->fWindowTemperature * 10));
	short nTNoEmiss = short(iTemperature * 10);
	int nWindowTransmittance = int(TMparams->fWindowTransmittance * 100);//
	short TEmiss = EmissCor(nTNoEmiss, sY16Windows, nWindowTransmittance);
	return TEmiss / 10.0f;
}

//窗口透过率修正反算
float MeasureTempCoreTEC::WindowTransmittanceReverseCorrection(float iTemperature)
{
	short sY16Windows = GetY16FromT(int(TMparams->fWindowTemperature * 10));
	short nTNoEmiss = short(iTemperature * 10);
	int nWindowTransmittance = int(TMparams->fWindowTransmittance * 100);//
	short DeTEmiss = DeEmissCor(nTNoEmiss, sY16Windows, nWindowTransmittance);
	return DeTEmiss / 10.0f;
}



//二次校温
void MeasureTempCoreTEC::SecondCorrectByOnePoint(float bb_temperature, short bb_y16)
{
	bb_y16 += TMparams->sY16Offset;

	float AftreB1Temp = 0.f;
	float BeforeB1Temp = 0.f;

	if (TMparams->mtType)
	{
		//窗口透过率修正反算
		if (TMparams->bWindowTransmittanceCorrection)
		{
			AftreB1Temp = WindowTransmittanceReverseCorrection(bb_temperature);
		}

		//大气透过率修正反算
		if (TMparams->bAtmosphereCorrection)
		{
			AftreB1Temp = AtmosphereReverseCorrection(AftreB1Temp);
		}

		//环温修正反算
		if (TMparams->bAmbientCorrection)
		{
			AftreB1Temp = EnvironmentReverseCorrection(AftreB1Temp);
		}
	}
	
	//发射率修正反算
	if (TMparams->bEmissCorrection)
	{
		AftreB1Temp = EmissionReverseCorrectionNew(AftreB1Temp);
	}

	//距离修正反算
	if (TMparams->bDistanceCorrection)
	{
		if (TMparams->nDistance_a6 == -1 && TMparams->nDistance_a7 == -1 && TMparams->nDistance_a8 == -1)
		{
			AftreB1Temp = DistanceReverseCorrectionWithSixPara(AftreB1Temp);
		}
		else
		{
			AftreB1Temp = DistanceReverseCorrectionWithNinePara(AftreB1Temp);
		}
	}

	BeforeB1Temp = StandardY16ToTempWithY16CorrectionInSecondCorrect(bb_y16, TMparams->nKF, 0);
	TMparams->nB1Offset = int((AftreB1Temp - BeforeB1Temp) * 100) - TMparams->nB1;
}

float MeasureTempCoreTEC::StandardY16ToTempWithY16CorrectionInSecondCorrect(short y16value,int KF, int nOffset)
{
	if (TMparams->bShutterCorrection)
	{
		y16value = y16value - (short)fCorrection1;
	}

	if (TMparams->bLensCorrection)
	{
		y16value = y16value - (short)fCorrection2;
	}

	int index = (int)(y16value * ((KF + nOffset) / 100.f)) + nShutterCurveIndex;
	if (index < 0)
	{
		index = 0;
	}
	else if (index >= curveDataLength)
	{
		index = curveDataLength - 1;
	}

	int temp = CurveData[index];

	return temp / 10.0f;
}

//二次校温
void MeasureTempCoreTEC::SecondCorrectByTwoPoint(float bb_temperature_1, float bb_temperature_2, short bb_y16_1, short bb_y16_2, int* coff)
{
	bb_y16_1 += TMparams->sY16Offset;
	bb_y16_2 += TMparams->sY16Offset;

	float AftreB1Temp1 = 0.f;
	float AftreB1Temp2 = 0.f;



	if (TMparams->mtType)
	{
		//窗口透过率修正反算
		if (TMparams->bWindowTransmittanceCorrection)
		{
			AftreB1Temp1 = WindowTransmittanceReverseCorrection(bb_temperature_1);
			AftreB1Temp2 = WindowTransmittanceReverseCorrection(bb_temperature_2);
		}

		//大气透过率修正反算
		if (TMparams->bAtmosphereCorrection)
		{
			AftreB1Temp1 = AtmosphereReverseCorrection(AftreB1Temp1);
			AftreB1Temp2 = AtmosphereReverseCorrection(AftreB1Temp2);
		}

		//环温修正反算
		if (TMparams->bAmbientCorrection)
		{
			AftreB1Temp1 = EnvironmentReverseCorrection(AftreB1Temp1);
			AftreB1Temp2 = EnvironmentReverseCorrection(AftreB1Temp2);
		}
	}
	
	//发射率修正反算
	if (TMparams->bEmissCorrection)
	{
		AftreB1Temp1 = EmissionReverseCorrectionNew(AftreB1Temp1);
		AftreB1Temp2 = EmissionReverseCorrectionNew(AftreB1Temp2);
	}

	//距离修正反算
	if (TMparams->bDistanceCorrection)
	{
		if (TMparams->nDistance_a6 == -1 && TMparams->nDistance_a7 == -1 && TMparams->nDistance_a8 == -1)
		{
			AftreB1Temp1 = DistanceReverseCorrectionWithSixPara(AftreB1Temp1);
			AftreB1Temp2 = DistanceReverseCorrectionWithSixPara(AftreB1Temp2);
		}
		else
		{
			AftreB1Temp1 = DistanceReverseCorrectionWithNinePara(AftreB1Temp1);
			AftreB1Temp2 = DistanceReverseCorrectionWithNinePara(AftreB1Temp2);
		}
	}

	//由于查表的存在，无法线性拟合KF,B1,因此采取小范围循环遍历方式
	float DiffMax = FLT_MAX;
	int BestKFOffset = 0;
	int BestB1Offset = 0;
	//KFOffset -5~5
	for (int i = - 10; i < 11; i++)
	{
		for (int j = -2000; j < 2000; j += 10)
		{
			float Diff1 = abs(StandardY16ToTempWithY16CorrectionInSecondCorrect(bb_y16_1, TMparams->nKF, i) + (TMparams->nB1 + j) / 100.f - AftreB1Temp1);
			float Diff2 = abs(StandardY16ToTempWithY16CorrectionInSecondCorrect(bb_y16_2, TMparams->nKF, i) + (TMparams->nB1 + j) / 100.f - AftreB1Temp2);
			if (Diff1 + Diff2 < DiffMax)
			{
				BestKFOffset = i;
				BestB1Offset = j;
				DiffMax = Diff1 + Diff2;
			}
		}
	}
	coff[0] = BestKFOffset;
	coff[1] = BestB1Offset;
}

//计算温度矩阵接口(全图无损计算)
int MeasureTempCoreTEC::GetTempMatrix(short *pSrc, float *pTempMatrix, int nImageWidth, int nImageHeight)
{
	short sMin = 32767;
	short sMax = -32767;
	int nLen = nImageWidth * nImageHeight;
	memset(pTempMatrix, 0, nLen * sizeof(float));

	// 计算全图最高AD值与最低AD值
	for (int i = 0; i < nLen; i++)
	{
		sMin = gdmin(pSrc[i], sMin);
		sMax = gdmax(pSrc[i], sMax);
	}

	int tLen = sMax - sMin + 1;
	if (tLen > m_tempLen)
	{
		if (m_fTempMatrix)
		{
			porting_free_mem(m_fTempMatrix);
			m_fTempMatrix = NULL;
			m_tempLen = 0;
		}
	}
	if (!m_fTempMatrix)
	{
		m_fTempMatrix = (float*)porting_calloc_mem(tLen, sizeof(float), ITA_MT_MODULE);
		m_tempLen = tLen;
	}
	memset(m_fTempMatrix, 0, m_tempLen * sizeof(float));

	for (int i = sMin; i <= sMax; i++)
	{
		GetTempByY16(m_fTempMatrix[i - sMin], (short)i);
	}

	for (int i = 0; i < nLen; i++)
	{
		pTempMatrix[i] = m_fTempMatrix[pSrc[i] - sMin];
	}
	return 0;
}


//计算温度矩阵接口(ROI区域无损计算)
int MeasureTempCoreTEC::GetTempMatrix(short *pSrc, float *pTempMatrix, int nImageWidth, int nImageHeight, int x, int y, int ROIWidth, int ROIHeight, float ROIDistance, float ROIEmiss)
{
	//记录原有的距离和发射率
	float TempDistance = TMparams->fDistance;
	float TempEmis = TMparams->fEmiss;

	//对区域中的距离、发射率进行更新
	TMparams->fDistance = ROIDistance;
	TMparams->fEmiss = ROIEmiss;
	
	short sMin = 32767;
	short sMax = -32767;
	memset(pTempMatrix, 0, ROIWidth * ROIHeight * sizeof(float));

	// 计算ROI区域最高AD值与最低AD值
	for (int i = y; i < y + ROIHeight; i++)
	{
		for (int j = x; j < x + ROIWidth; j++)
		{
			sMin = gdmin(pSrc[i * nImageWidth + j], sMin);
			sMax = gdmax(pSrc[i * nImageWidth + j], sMax);
		}
	}

	int tLen = sMax - sMin + 1;
	if (tLen > m_tempLen)
	{
		if (m_fTempMatrix)
		{
			porting_free_mem(m_fTempMatrix);
			m_fTempMatrix = NULL;
			m_tempLen = 0;
		}
	}
	if (!m_fTempMatrix)
	{
		m_fTempMatrix = (float*)porting_calloc_mem(tLen, sizeof(float), ITA_MT_MODULE);
		m_tempLen = tLen;
	}
	memset(m_fTempMatrix, 0, m_tempLen * sizeof(float));
	//Y16值异常的情况下温度矩阵函数无限循环无法跳出。short类型循环变量溢出导致。
	for (int i = sMin; i <= sMax; i++)
	{
		GetTempByY16(m_fTempMatrix[i - sMin], (short)i);
	}

	for (int i = y; i < y + ROIHeight; i++)
	{
		for (int j = x; j < x + ROIWidth; j++)
		{
			pTempMatrix[(i - y) * ROIWidth + j - x] = m_fTempMatrix[pSrc[i * nImageWidth + j] - sMin];
		}
	}

	//距离、发射率还原
	TMparams->fDistance = TempDistance;
	TMparams->fEmiss = TempEmis;
	return 0;
}



//计算温度矩阵接口(全图快速计算)
int MeasureTempCoreTEC::GetTempMatrixFast(short *pSrc, float *pTempMatrix, int nImageWidth, int nImageHeight)
{
	short sMin = 32767;
	short sMax = -32767;
	int nLen = nImageWidth * nImageHeight;
	memset(pTempMatrix, 0, nLen * sizeof(float));

	//	for (int i = 0; i < nLen; i++)
	//	{
	//		pSrc[i] -= sAvgB;
	//	}

	// 计算全图最高AD值与最低AD值
	for (int i = 0; i < nLen; i++)
	{
		sMin = gdmin(pSrc[i], sMin);
		sMax = gdmax(pSrc[i], sMax);
	}

	// 分为 NSEPERATENUM 个小段，每一段调用一次测温接口
	short sValue[NSEPERATENUM] = { 0 };
	float fTemp[NSEPERATENUM] = { 0 };

	for (int i = 0; i < NSEPERATENUM; i++)
	{
		sValue[i] = short(sMin + i * (sMax - sMin) *1.0f / (NSEPERATENUM - 1));

		GetTempByY16(fTemp[i], sValue[i]);
	}

	// 预先计算比例表
	float fK[NSEPERATENUM] = { 0 };
	for (int n = 0; n < NSEPERATENUM - 1; n++)
	{
		if (sValue[n + 1] - sValue[n] > 0)
		{
			fK[n] = (fTemp[n + 1] - fTemp[n]) *1.0f / (sValue[n + 1] - sValue[n]);
		}
		else
		{
			fK[n] = 0;
		}
	}

	// 全图测温
	for (int i = 0; i < nLen; i++)
	{
		if (pSrc[i] >= sValue[NSEPERATENUM - 1])
		{
			pTempMatrix[i] = fTemp[NSEPERATENUM - 1];
		}
		else if (pSrc[i] < sValue[0])
		{
			pTempMatrix[i] = fTemp[0];
		}
		else
		{
			for (int n = 0; n < NSEPERATENUM - 1; n++)
			{
				if (pSrc[i] >= sValue[n] && pSrc[i] < sValue[n + 1])
				{
					pTempMatrix[i] = fTemp[n] + 1.0f * (pSrc[i] - sValue[n]) * fK[n];
					break;
				}
			}
		}
	}
	return 0;
}


//计算温度矩阵接口(ROI区域快速计算)
int MeasureTempCoreTEC::GetTempMatrixFast(short *pSrc, float *pTempMatrix, int nImageWidth, int nImageHeight, int x, int y, int ROIWidth, int ROIHeight, float ROIDistance, float ROIEmiss)
{
	//记录原有的距离和发射率
	float TempDistance = TMparams->fDistance;
	float TempEmis = TMparams->fEmiss;

	//对区域中的距离、发射率进行更新
	TMparams->fDistance = ROIDistance;
	TMparams->fEmiss = ROIEmiss;

	short sMin = 32767;
	short sMax = -32767;
	int nLen = ROIWidth * ROIHeight;
	memset(pTempMatrix, 0, nLen * sizeof(float));

	// 计算ROI区域最高AD值与最低AD值
	for (int i = y; i < y + ROIHeight; i++)
	{
		for (int j = x; j < x + ROIWidth; j++)
		{
			sMin = gdmin(pSrc[i * nImageWidth + j], sMin);
			sMax = gdmax(pSrc[i * nImageWidth + j], sMax);
		}
	}

	// 分为 NSEPERATENUM 个小段，每一段调用一次测温接口
	short sValue[NSEPERATENUM] = { 0 };
	float fTemp[NSEPERATENUM] = { 0 };

	for (int i = 0; i < NSEPERATENUM; i++)
	{
		sValue[i] = short(sMin + i * (sMax - sMin) *1.0f / (NSEPERATENUM - 1));

		GetTempByY16(fTemp[i], sValue[i]);
	}

	// 预先计算比例表
	float fK[NSEPERATENUM] = { 0 };
	for (int n = 0; n < NSEPERATENUM - 1; n++)
	{
		if (sValue[n + 1] - sValue[n] > 0)
		{
			fK[n] = (fTemp[n + 1] - fTemp[n]) *1.0f / (sValue[n + 1] - sValue[n]);
		}
		else
		{
			fK[n] = 0;
		}
	}

	// 区域测温
	for (int i = y; i < y + ROIHeight; i++)
	{
		for (int j = x; j < x + ROIWidth; j++)
		{
			int ImageIndex = i * nImageWidth + j;
			int ROIIndex = (i - y) * ROIWidth + j - x;
			if (pSrc[ImageIndex] >= sValue[NSEPERATENUM - 1])
			{
				pTempMatrix[ROIIndex] = fTemp[NSEPERATENUM - 1];
			}
			else if (pSrc[ImageIndex] < sValue[0])
			{
				pTempMatrix[ROIIndex] = fTemp[0];
			}
			else
			{
				for (int n = 0; n < NSEPERATENUM - 1; n++)
				{
					if (pSrc[ImageIndex] >= sValue[n] && pSrc[ImageIndex] < sValue[n + 1])
					{
						pTempMatrix[ROIIndex] = fTemp[n] + 1.0f * (pSrc[ImageIndex] - sValue[n]) * fK[n];
						break;
					}
				}
			}
		}
	}

	//距离、发射率还原
	TMparams->fDistance = TempDistance;
	TMparams->fEmiss = TempEmis;

	return 0;
}


/*函数功能：体表体内温度映射
输入参数：
float fSurfaceTemp，目标体表温度
float fEnvirTemp，目标所处的环温
返回参数：目标体内温度
*/
short MeasureTempCoreTEC::SurfaceTempToBody(float &fBodyTemp, float fSurfaceTemp, float fEnvirTemp)
{
	int nSensitivity = 2;
	double gamma_table[5] = { 1.5, 1.2, 1, 0.8, 0.6 };

	const double p00 = 52.33;
	const double p10 = 1.603;
	const double p01 = -0.4126;
	const double p20 = 0.00294;
	const double p11 = -0.08027;
	const double p02 = -0.0422;
	const double p30 = -5.163e-05;
	const double p21 = 7.151e-05;
	const double p12 = 0.0007524;
	const double p03 = 0.001295;

	const double fMapTempMin = 35.8;//映射最低温 
	const double fMapTempMax = 42.0;//映射最高温 
	fSurfaceTemp = float(fmin(40, fmax(fSurfaceTemp, 28)));//体表温度限值 

	double x = fEnvirTemp;
	double z[121] = { 0 };
	int n = 0;
	double min_z = 1000;
	double max_z = -1000;
	int min_index = 0;
	int max_index = 0;
	for (float y = 28.0; y <= 40.0f; y = y + 0.1f)
	{
		z[n] = p00 + p10*x + p01*y + p20*x*x + p11*x*y + p02*y*y + p30*x*x*x + p21*x*x * y + p12*x*y*y + p03*y*y*y;

		max_z = fmax(max_z, z[n]);
		min_z = fmin(min_z, z[n]);
		n++;
	}
	for (int i = 0; i < 121; i++)
	{
		if (z[i] == min_z)
		{
			min_index = i;
			break;
		}
	}
	double fSurfaceTempMin = min_index * 0.1 + 28;

	for (int i = 120; i >= 0; i--)
	{
		if (z[i] == max_z)
		{
			max_index = i;
			break;
		}
	}
	double fSurfaceTempMax = max_index * 0.1 + 28;

	x = fEnvirTemp;
	double y = 0.0;
	if (fSurfaceTemp < fSurfaceTempMin)
	{
		y = fSurfaceTempMin;
	}
	else if (fSurfaceTemp > fSurfaceTempMax)
	{
		y = fSurfaceTempMax;
	}
	else
	{
		y = fSurfaceTemp;
	}
	fBodyTemp = float(p00 + p10*x + p01*y + p20*x*x + p11*x*y + p02*y*y + p30*x*x*x + p21*x*x * y + p12*x*y*y + p03*y*y*y);

	fBodyTemp = float(fmin(fMapTempMax, fmax(fMapTempMin, fBodyTemp)));
	float gamma = float(gamma_table[nSensitivity - 1]);
	float input = float((fBodyTemp - fMapTempMin) / (fMapTempMax - fMapTempMin));
	float output = float(fMapTempMin + pow(input, gamma) * (fMapTempMax - fMapTempMin));
	fBodyTemp = float(fmin(fMapTempMax, fmax(fMapTempMin, output)));
	return 0;
}

void MeasureTempCoreTEC::setLogger(GuideLog * logger)
{
	m_logger = logger;
}

//int low2high = 140;//低温档切高温档温度阈值
//int high2low = 130;//高温档切低温档温度阈值
//float areaTh1 = 0.0020;//低温档切高温档面积占比阈值 >140 个数> areaTh1*width*height
//float areaTh2 = 0.9995;//高温档切低温档面积占比阈值 >130 个数< (1-areaTh2)*width*height
int MeasureTempCoreTEC::autoChangeRange(short *pSrc, int width, int height, int range,
	float areaTh1, float areaTh2, int low2high, int high2low)
{
	int num = 0;
	int ifShiting = 0;

	if (m_sortLen < width * height)
	{
		if (m_sortY16)
		{
			porting_free_mem(m_sortY16);
			m_sortY16 = NULL;
			m_sortLen = 0;
		}
	}
	if (!m_sortY16)
	{
		m_sortY16 = (short *)porting_calloc_mem(width * height, sizeof(short), ITA_MT_MODULE);
		m_sortLen = width * height;
	}
	memcpy(m_sortY16, pSrc, width * height * sizeof(short));
	std::sort(m_sortY16, m_sortY16 + width * height);

	int areaNum = range == 0 ? round(areaTh1 * width * height) : round((1 - areaTh2) * width * height);
	int tempTh = range == 0 ? low2high : high2low;
	for (int i = width * height - 1; i >= 0; i--)
	{
		if (num >= areaNum)
		{
			ifShiting = range == 0 ? 1 : 0;
			break;
		}
		else
		{
			float temp = StandardY16ToTempWithY16Correction(m_sortY16[i]);
			if (temp <= tempTh)
			{
				ifShiting = range == 0 ? 0 : 1;
				break;
			}
			else
				num++;
		}
	}
	return ifShiting;
}

int MeasureTempCoreTEC::autoMultiChangeRange(short* pSrc, int width, int height, int range,
	float areaTh1, float areaTh2, int low2mid, int mid2low, int mid2high, int high2mid)
{
	int ifShiting = 0;                 //切档标志位
	int NumPasslow2mid = 0;           //温度超过140的数量
	int NumPassmid2low = 0;           //温度低过130的数量
	int NumPassmid2high = 0;          //温度超过600的数量
	int NumPasshigh2mid = 0;          //温度低于580的数量

	if (m_sortLen < width * height)
	{
		if (m_sortY16)
		{
			porting_free_mem(m_sortY16);
			m_sortY16 = NULL;
			m_sortLen = 0;
		}
	}
	if (!m_sortY16)
	{
		m_sortY16 = (short*)porting_calloc_mem(width * height, sizeof(short), ITA_MT_MODULE);
		m_sortLen = width * height;
	}
	memcpy(m_sortY16, pSrc, width * height * sizeof(short));
	std::sort(m_sortY16, m_sortY16 + width * height);

	int areaNumDownToUp = round(areaTh1 * width * height);                     //向上切档的数量阈值  
	int areaNumUpToDown = round((1 - areaTh2) * width * height);              //不切档的数量阈值 

	m_logger->output(LOG_DEBUG, "AutoChange Y16 \t%d\t%d", m_sortY16[0], m_sortY16[width * height - 1]);

	float temp;
	for (int i = width * height - 1; i >= 0; i--)
	{

		GetTempByY16(temp, m_sortY16[i]);

		if (range == 1)
		{
			if (temp < low2mid)
			{
				m_logger->output(LOG_DEBUG, " AutoChange:low remain");
				ifShiting = 0;
				break;
			}
			else
			{
				NumPasslow2mid++;
				if (NumPasslow2mid >= areaNumDownToUp)
				{
					m_logger->output(LOG_DEBUG, " AutoChange:low to mid");
					ifShiting = 1;
					break;
				}
			}
		}
		else if (range == 2)
		{
			if (temp < mid2low)
			{
				m_logger->output(LOG_DEBUG, " AutoChange:mid to low");
				ifShiting = -1;
				break;
			}
			else if ((temp < mid2high) && (temp >= mid2low))
			{
				NumPassmid2low++;
				if (NumPassmid2low >= areaNumUpToDown)
				{
					m_logger->output(LOG_DEBUG, " AutoChange:mid remain");
					ifShiting = 0;
					break;
				}
			}
			else
			{
				NumPassmid2high++;
				if (NumPassmid2high >= areaNumDownToUp)
				{
					m_logger->output(LOG_DEBUG, " AutoChange:mid to high");
					ifShiting = 1;
					break;
				}
			}
		}
		else
		{
			if (temp < high2mid)
			{
				m_logger->output(LOG_DEBUG, " AutoChange:high to mid");
				ifShiting = -1;
				break;
			}
			else
			{
				NumPasshigh2mid++;
				if (NumPasshigh2mid >= areaNumDownToUp)
				{
					m_logger->output(LOG_DEBUG, " AutoChange:high remain");
					ifShiting = 0;
					break;
				}
			}
		}
	}
	return ifShiting;
}

//自动校温接口
void MeasureTempCoreTEC::AutoCorrectTemp(float* fBlackBodyTemps, short* sY16s, int nLength)
{
	//对传进来的黑体温度以及对应的y16值进行排序
	Quick_Sort(fBlackBodyTemps, 0, nLength - 1);
	Quick_Sort(sY16s, 0, nLength - 1);

	short* pTempY16s = (short*)malloc(nLength * sizeof(short));
	memset(pTempY16s, 0, nLength * sizeof(short));
	float* pTmpBlackBodyTemp = (float*)malloc(nLength * sizeof(float));
	memset(pTmpBlackBodyTemp, 0, nLength * sizeof(float));

	for (int i = 0; i < nLength; i++)
	{
		pTempY16s[i] = sY16s[i] + TMparams->sY16Offset;
		pTmpBlackBodyTemp[i] = fBlackBodyTemps[i];
		if (TMparams->mtType)
		{
			//窗口透过率修正反算
			if (TMparams->bWindowTransmittanceCorrection)
			{
				pTmpBlackBodyTemp[i] = WindowTransmittanceReverseCorrection(pTmpBlackBodyTemp[i]);
			}

			//大气透过率修正反算
			if (TMparams->bAtmosphereCorrection)
			{
				pTmpBlackBodyTemp[i] = AtmosphereReverseCorrection(pTmpBlackBodyTemp[i]);
			}

			//环温修正反算
			if (TMparams->bAmbientCorrection)
			{
				pTmpBlackBodyTemp[i] = EnvironmentReverseCorrection(pTmpBlackBodyTemp[i]);
			}
		}

		//发射率修正反算
		if (TMparams->bEmissCorrection)
		{
			pTmpBlackBodyTemp[i] = EmissionReverseCorrectionNew(pTmpBlackBodyTemp[i]);
		}

		//距离修正反算
		if (TMparams->bDistanceCorrection)
		{
			if (TMparams->nDistance_a6 == -1 && TMparams->nDistance_a7 == -1 && TMparams->nDistance_a8 == -1)
			{
				pTmpBlackBodyTemp[i] = DistanceReverseCorrectionWithSixPara(pTmpBlackBodyTemp[i]);
			}
			else
			{
				pTmpBlackBodyTemp[i] = DistanceReverseCorrectionWithNinePara(pTmpBlackBodyTemp[i]);
			}
		}
	}

	//由于查表的存在，无法线性拟合KF,B1,因此采取小范围循环遍历方式
	float fDiffMax = FLT_MAX;
	int nBestKF = 0;
	int nBestB1 = 0;
	//KF 在原始值的0.8~1.2倍范围内调整
	for (int i = int(TMparams->nKF * 0.8); i < int(TMparams->nKF * 1.2); i++)
	{
		for (int j = TMparams->nB1 - 2000; j < TMparams->nB1 + 2000; j += 10)
		{
			float fDiff = 0.0f;
			for (int m = 0; m < nLength; m++)
			{
				fDiff += (abs(StandardY16ToTempWithY16CorrectionInSecondCorrect(pTempY16s[m], i, TMparams->nKFOffset) + (j + TMparams->nB1Offset) / 100.f - pTmpBlackBodyTemp[m]) / pTmpBlackBodyTemp[m]);
			}

			if (fDiff < fDiffMax)
			{
				nBestKF = i;
				nBestB1 = j;
				fDiffMax = fDiff;
			}
		}
	}
	TMparams->nKF = nBestKF;
	TMparams->nB1 = nBestB1;

	if (pTempY16s)
	{
		free(pTempY16s);
	}
	pTempY16s = NULL;

	if (pTmpBlackBodyTemp)
	{
		free(pTmpBlackBodyTemp);
	}
	pTmpBlackBodyTemp = NULL;
}

void Quick_Sort(float *arr, int begin, int end)
{
	if (begin > end)
	{
		return;
	}
	float tmp = arr[begin];
	int i = begin;
	int j = end;
	while (i != j)
	{
		while (arr[j] >= tmp && j > i)
		{
			j--;
		}
		while (arr[i] <= tmp && j > i)
		{
			i++;
		}
		if (j > i)
		{
			float t = arr[i];
			arr[i] = arr[j];
			arr[j] = t;
		}
	}
	arr[begin] = arr[i];
	arr[i] = tmp;
	Quick_Sort(arr, begin, i - 1);
	Quick_Sort(arr, i + 1, end);
}

void Quick_Sort(short *arr, int begin, int end)
{
	if (begin > end)
	{
		return;
	}
	short tmp = arr[begin];
	int i = begin;
	int j = end;
	while (i != j)
	{
		while (arr[j] >= tmp && j > i)
		{
			j--;
		}
		while (arr[i] <= tmp && j > i)
		{
			i++;
		}
		if (j > i)
		{
			short t = arr[i];
			arr[i] = arr[j];
			arr[j] = t;
		}
	}
	arr[begin] = arr[i];
	arr[i] = tmp;
	Quick_Sort(arr, begin, i - 1);
	Quick_Sort(arr, i + 1, end);
}