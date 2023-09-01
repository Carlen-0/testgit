#include "MeasureTempCore.h"
#include "../Module/GuideLog.h"
#include "EmissCurves.h"
#include <algorithm>


/*函数功能：湿度修正
输入参数：
float fObjY16，目标Y16
float fHumidity，湿度值
返回参数：目标Y16
by gzy
by huangqiang (使用内联函数)
*/
inline static short  HumidityCorr(short fObjY16, float fHumidity)
{
    return (short)((1.0f - (fHumidity - 0.6f) / 50.0f) *fObjY16);
}

/*函数功能：环温修正
输入参数：
float fObjTemp，未修正的目标温度
float fEnvirTemp，环境温度
返回参数：修正后的目标温度
by 03225px
by huangqiang (使用内联函数)
*/
inline static float EnvirTempCorr(float fObjTemp, float fEnvirTemp)
{
	float fDeltaT = 0.0f;


	if (fEnvirTemp < 18.0f)
	{
		fEnvirTemp = 18.0f;
	}
	else if (fEnvirTemp > 43.0f)
	{
		fEnvirTemp = 43.0f;
	}

	if ((fEnvirTemp >= 18.0f) && (fEnvirTemp < 31.0f))
	{
		float a1 = 0.2047f;
		float a2 = -6.45f;
		fDeltaT = a1*fEnvirTemp + a2;
		fDeltaT = gdmax(-3.0f, gdmin(1.8f, fDeltaT));
	}
	if ((fEnvirTemp >= 31.0f) && (fEnvirTemp <= 36.0f))
	{
		fDeltaT = 0.0f;
	}
	if ((fEnvirTemp >36.0f) && (fEnvirTemp <= 43.0f))
	{
		float a1 = 0.2286f;
		float a2 = -8.229f;
		fDeltaT = a1*fEnvirTemp + a2;
		fDeltaT = gdmax(-3.0f, gdmin(1.8f, fDeltaT));
	}
	//return (fObjTemp + fDeltaT);
	//return (fObjTemp + fDeltaT);
	return fObjTemp;
}

/*函数功能：构造函数
输入参数：无
返回参数：无
*/
CMeasureTempCore::CMeasureTempCore(MtParams *mtParams, int w, int h, short* allCurvesArray, int curvesLen)
{
	nImgWidth = w;                    
	nImgHeight = h;                               //图像宽高
	nImgLen = w * h;
	m_ucFpaGear = 0; // 切换测温范围时有几帧会用0档位测温，如果是-1，那么测温出现-1.5异常。
	fLensCorrIncrement = 0.0f;
	fShutterCorrIncrement = 0.0f;       //初始化参数
	pusCurve = allCurvesArray;
	g_fFocusTemp[G_NLEN] = { 0 };
	g_nFocusCnt = 0;
	smoothFocusT = 0;
	//mtParams->usCurveTempNumber = curvesLen / 4;
	/*mtParams在MTWrapper构造函数中完成初始化。在加载数据包loadData时载入数据。这些都在CMeasureTempCore实例化之前完成。*/
	if (mtParams->mtType == 0 || mtParams->mtType == 1)
	{
		m_fLensCorrK = mtParams->fLowLensCorrK;
		m_fShutterCorrK = mtParams->fLowShutterCorrCoff;
	}
	else if (mtParams->mtType == 2)
	{
		m_fLensCorrK = mtParams->fHighLensCorrK;
		m_fShutterCorrK = mtParams->fHighShutterCorrCoff;
	}
	else
	{
		m_fLensCorrK = mtParams->fLensCorrK;
		m_fShutterCorrK = mtParams->fShutterCorrCoff;
	}
	m_stMtParams = mtParams;
	m_pTempMap = NULL;
	m_logger = NULL;
	m_sortY16 = NULL;
	m_sortLen = 0;
}

/*函数功能：释放函数
输入参数：无
返回参数：无
*/
void CMeasureTempCore::FinalRelease()
{
	/*if (pusCurve)
	{
	free(pusCurve);
	}*/
}

/*函数功能：析构函数
输入参数：无
返回参数：无
*/
CMeasureTempCore::~CMeasureTempCore()
{
	if (m_pTempMap)
	{
		porting_free_mem(m_pTempMap);
		m_pTempMap = NULL;
	}
	if (m_sortY16)
	{
		porting_free_mem(m_sortY16);
		m_sortY16 = NULL;
		m_sortLen = 0;
	}
}

void CMeasureTempCore::reload(MtParams * mtParams, short * allCurvesArray, int curvesLen)
{
	pusCurve = allCurvesArray;
	m_stMtParams = mtParams;
}

/*函数功能：确定焦温档位和测温曲线
输入参数：无
返回参数：测温档位
*/
int CMeasureTempCore::GetFpaGear()
{
	//确定当前焦温所处的焦温档位
	int nFpaGear = -1;
	for (int i = 0; i < m_stMtParams->ucFpaTempNum; i++)
	{
		if (m_stMtParams->fRealTimeFpaTemp <= m_stMtParams->gFpaTempArray[i])
		{
			nFpaGear = i;
			break;
		}
		if (i == (m_stMtParams->ucFpaTempNum - 1))
		{
			nFpaGear = m_stMtParams->ucFpaTempNum;
		}
	}

	m_ucFpaGear = nFpaGear;
	/*每秒更新温传后调用本接口比较并切换档位。*/
	m_stMtParams->n_fpaGear = nFpaGear;
	return nFpaGear;
}

void CMeasureTempCore::setLogger(GuideLog * logger)
{
	m_logger = logger;
}

int CMeasureTempCore::ChangeRange()
{
	if (m_stMtParams->mtType == 0 || m_stMtParams->mtType == 1)
	{
		m_fLensCorrK = m_stMtParams->fLowLensCorrK;
		m_fShutterCorrK = m_stMtParams->fLowShutterCorrCoff;
	}
	else if (m_stMtParams->mtType == 2)
	{
		m_fLensCorrK = m_stMtParams->fHighLensCorrK;
		m_fShutterCorrK = m_stMtParams->fHighShutterCorrCoff;
	}
	else
	{
		m_fLensCorrK = m_stMtParams->fLensCorrK;
		m_fShutterCorrK = m_stMtParams->fShutterCorrCoff;
	}

	//判断焦温数组是否正常：当相邻焦温温差较小时，返回错误
	float FpaTempError = 0.0f;
	for (int i = 0; i < m_stMtParams->ucFpaTempNum - 1; i++)
	{
		FpaTempError = m_stMtParams->gFpaTempArray[i + 1] - m_stMtParams->gFpaTempArray[i];
		if (FpaTempError < 1.0f)
		{
			return ITA_FPA_TEMP_ARRAY_ERROR;
		}
	}
	return 0;
}

int CMeasureTempCore::RefreshCorrK()
{
	if (m_stMtParams->mtType == 0 || m_stMtParams->mtType == 1)
	{
		m_fLensCorrK = m_stMtParams->fLowLensCorrK;
		m_fShutterCorrK = m_stMtParams->fLowShutterCorrCoff;
	}
	else if (m_stMtParams->mtType == 2)
	{
		m_fLensCorrK = m_stMtParams->fHighLensCorrK;
		m_fShutterCorrK = m_stMtParams->fHighShutterCorrCoff;
	}
	else
	{
		m_fLensCorrK = m_stMtParams->fLensCorrK;
		m_fShutterCorrK = m_stMtParams->fShutterCorrCoff;
	}

	if (m_stMtParams->bLogPrint)
		m_logger->output(LOG_DEBUG, "RefreshCorrK %d %f %f %f %f %f %f %f %f", 
			m_stMtParams->mtType, m_fLensCorrK, m_fShutterCorrK, m_stMtParams->fLowLensCorrK, m_stMtParams->fLowShutterCorrCoff ,m_stMtParams->fHighLensCorrK, m_stMtParams->fHighShutterCorrCoff, m_stMtParams->fLensCorrK, m_stMtParams->fShutterCorrCoff);
	return 0;
}

/*函数功能：Y16单点测温
输入参数：目标Y16
返回参数：目标温度
*/
int CMeasureTempCore::GetTempByY16(float &fTemp, short nObjY16, float *kf, float *b)
{
	float fForwardNearTemp = 0.0f;                           
	float fBackNearTemp = 0.0f;
	float fForwardFarTemp = 0.0f;
	float fBackFarTemp = 0.0f;                     //4条曲线的测温结果

	float fForwardWeight = 0.0f;
	float fBackWeight = 0.0f;                      //焦温权重
	float fFarDistanceDif = 0.0f;
	float fNearDistanceDif = 0.0f;                 
	float fFarWeight = 0.0f;
	float fNearWeight = 0.0f;                    
	float fBackDif = 0.0f;
	float fForwardDif = 0.0;
	float fFarDistanceTemperature = 0.0f;
	float fNearDistanceTemperature = 0.0f;

	float fEnvironmentTemp = m_stMtParams->fEnvironmentTemp;
	float fReflectTemp = m_stMtParams->fReflectTemp;
	float fEmiss = m_stMtParams->fEmiss;
	float fTransmit = m_stMtParams->fTransmit;
	float fWindowTemperature = m_stMtParams->fWindowTemperature;
	float fHumidity = m_stMtParams->fHumidity;
	float fAtmosphereTransmittance;
	float fAtmosphereTemperature = m_stMtParams->fAtmosphereTemperature;

	if ((m_stMtParams->m_fieldType == 4) && (m_stMtParams->mtType == 0))       //当镜头类型为120模组90度镜头时，取第一个距离为自动校温点；
	{
		m_stMtParams->fCorrectDistance = m_stMtParams->gDistanceArray[0];
	}
	else                                     //当镜头类型为其他镜头时，取第二个距离为自动校温点；
	{
		m_stMtParams->fCorrectDistance = m_stMtParams->gDistanceArray[1];
	}


	//湿度修正
	if (m_stMtParams->bHumidityCorrection == 1)
	{
		nObjY16 = HumidityCorr(nObjY16, fHumidity);
	}

	//根据焦温档位，计算每条曲线的温度
	if (m_ucFpaGear == 0)
	{
		fForwardNearTemp = 0.0f;
		fForwardFarTemp = 0.0f;
		fBackNearTemp = GetSingleCurveTemperature(nObjY16, pusCurve, m_stMtParams->usCurveTempNumber, 0, kf, b);
		fBackFarTemp = GetSingleCurveTemperature(nObjY16, pusCurve + m_stMtParams->usCurveTempNumber, m_stMtParams->usCurveTempNumber, 1, kf, b);
	}
	else if (m_ucFpaGear == m_stMtParams->ucFpaTempNum)
	{
		fForwardNearTemp = GetSingleCurveTemperature(nObjY16, pusCurve, m_stMtParams->usCurveTempNumber, 0, kf, b);
		fForwardFarTemp = GetSingleCurveTemperature(nObjY16, pusCurve + m_stMtParams->usCurveTempNumber, m_stMtParams->usCurveTempNumber, 1, kf, b);
		fBackNearTemp = 0.0f;
		fBackFarTemp = 0.0f;
	}
	else
	{
		fForwardNearTemp = GetSingleCurveTemperature(nObjY16, pusCurve, m_stMtParams->usCurveTempNumber, 0, kf, b);
		fForwardFarTemp = GetSingleCurveTemperature(nObjY16, pusCurve + m_stMtParams->usCurveTempNumber, m_stMtParams->usCurveTempNumber, 1, kf, b);
		fBackNearTemp = GetSingleCurveTemperature(nObjY16, pusCurve + 2 * m_stMtParams->usCurveTempNumber, m_stMtParams->usCurveTempNumber, 0, kf, b);
		fBackFarTemp = GetSingleCurveTemperature(nObjY16, pusCurve + 3 * m_stMtParams->usCurveTempNumber, m_stMtParams->usCurveTempNumber, 1, kf, b);
	}

	//根据当前焦温确定对应焦温曲线的权重
	if (m_ucFpaGear == 0)
	{
		fBackWeight = 1.0f;
		fForwardWeight = 0.0f;
	}
	else if (m_ucFpaGear == m_stMtParams->ucFpaTempNum)
	{
		fBackWeight = 0.0f;
		fForwardWeight = 1.0f;
	}
	else
	{
		fBackDif = m_stMtParams->gFpaTempArray[m_ucFpaGear] - m_stMtParams->fRealTimeFpaTemp;
		fForwardDif = m_stMtParams->fRealTimeFpaTemp - m_stMtParams->gFpaTempArray[m_ucFpaGear - 1];
		fBackWeight = 1.0f - (fBackDif*1.0f) / (fBackDif + fForwardDif);
		fForwardWeight = 1.0f - (fForwardDif*1.0f) / (fBackDif + fForwardDif);
	}
	//根据当前距离确定对应距离的权重
	if (m_stMtParams->fDistance <= m_stMtParams->gDistanceArray[0])
	{
		fNearWeight = 1.0f;
		fFarWeight = 0.0f;
	}
	else if (m_stMtParams->fDistance >= m_stMtParams->gDistanceArray[1])
	{
		fNearWeight = 0.0f;
		fFarWeight = 1.0f;
	}
	else
	{
		fNearDistanceDif = m_stMtParams->fDistance - m_stMtParams->gDistanceArray[0];
		fFarDistanceDif = m_stMtParams->gDistanceArray[1] - m_stMtParams->fDistance;
		fNearWeight = 1.0f - (fNearDistanceDif*1.0f) / (fNearDistanceDif + fFarDistanceDif);
		fFarWeight = 1.0f - (fFarDistanceDif*1.0f) / (fNearDistanceDif + fFarDistanceDif);
	}

	fFarDistanceTemperature = fForwardFarTemp*fForwardWeight + fBackFarTemp*fBackWeight;		//1.5m远距离按照焦温加权
	fNearDistanceTemperature = fForwardNearTemp*fForwardWeight + fBackNearTemp*fBackWeight;  //0.5m近距离按照焦温加权

	if (m_stMtParams->m_fieldType == 3)      //当镜头类型为120模组标准镜头时，采用此距离校正方法
	{

		if (m_stMtParams->bDistanceCompensate == 1)
		{
			fTemp = fFarWeight * fFarDistanceTemperature + fNearWeight * fNearDistanceTemperature;
		}
		else
		{
			fTemp = fNearDistanceTemperature;
		}
	}
	else if ((m_stMtParams->m_fieldType == 4) &&( m_stMtParams->mtType == 0))   //当镜头类型为120模组90度镜头时，采用此距离校正方法
	{
		if (m_stMtParams->fDistance == m_stMtParams->fCorrectDistance)
		{
			fTemp = fNearDistanceTemperature;
		}
		if ((1 == m_stMtParams->bDistanceCompensate) && (m_stMtParams->fDistance != m_stMtParams->fCorrectDistance))
		{
			fTemp = DistCorr(fTemp, fNearDistanceTemperature, m_stMtParams->fDistance, m_stMtParams->fRealTimeShutterTemp,0);
		}
		else
		{
			fTemp = fNearDistanceTemperature; //不补偿
		}
	}
	else                                  //当镜头类型为其他镜头时，采用此距离校正方法
	{
		if (m_stMtParams->fDistance == m_stMtParams->fCorrectDistance)
		{
			fTemp = fFarDistanceTemperature;
		}
		if ((1 == m_stMtParams->bDistanceCompensate) && (m_stMtParams->fDistance != m_stMtParams->fCorrectDistance))
		{
			fTemp = DistCorr(fTemp, fFarDistanceTemperature, m_stMtParams->fDistance, m_stMtParams->fRealTimeShutterTemp , m_stMtParams->mtDisType);
		}
		else
		{
			fTemp = fFarDistanceTemperature;//不补偿
		}
	}
	if (m_stMtParams->bLogPrint)
		m_logger->output(LOG_DEBUG, "DistCorr=%.2f", fTemp);

	//窗口透过率校正
	if (m_stMtParams->bTransCorrection)
	{
		if (fTransmit <= 0.98)
		{
			fTemp = EmissCorrection(short(fTemp * 10.0f), (int)(fTransmit * 100.0f), GetY16FromT(int(fWindowTemperature * 10.0f)));
			fTemp = fTemp / 10.0f;
		}
	}

	//大气透过率校正
	if (m_stMtParams->bAtmosphereCorrection)
	{
		if (fAtmosphereTemperature >= 23.0f)
		{
			fAtmosphereTransmittance = 1 - (fAtmosphereTemperature - 23.0f) / (fAtmosphereTemperature * 15.0f);
		}
		else
		{
			fAtmosphereTransmittance = 1 - ( 23.0f - fAtmosphereTemperature) / ((46 - fAtmosphereTemperature) * 10.0f);
		}

		m_stMtParams->fAtmosphereTransmittance = fAtmosphereTransmittance;

		if (fAtmosphereTransmittance <= 0.98)
		{
			fTemp = EmissCorrection(short(fTemp * 10.0f), (int)(fAtmosphereTransmittance * 100.0f), GetY16FromT(int(fAtmosphereTemperature * 10.0f)));
			fTemp = fTemp / 10.0f;
		}
	}

	//发射率校正
	if (m_stMtParams->bEmissCorrection)
	{
		if (fEmiss <= 0.98)
		{
			fTemp = EmissCorrection(short(fTemp * 10.0f), (int)(fEmiss * 100.0f), GetY16FromT(int(fReflectTemp * 10.0f)));
			fTemp = fTemp / 10.0f;
		}
	}

	if (m_stMtParams->mtType != 0)
	{
		int nTemp;
		nTemp = int(fTemp * 10.0f);
		fTemp = nTemp / 10.0f;
	}

	if (m_stMtParams->bLogPrint)
		m_logger->output(LOG_DEBUG, "EmissCorr=%.2f", fTemp);

	//环温修正
	if (m_stMtParams->bEnvironmentCorrection)
	{
		fTemp = EnvirTempCorr(fTemp, fEnvironmentTemp);
	}

	//log打印
	if (m_stMtParams->bLogPrint)
	{
		m_logger->output(LOG_DEBUG, "\t%hhu\t%hhu\t%d\t%hd\t%.2f\t%hd\t%hd\t%hhu\t%d\t%d\t%d",
			m_stMtParams->bOrdinaryShutter,	//%d1
			m_stMtParams->bNucShutterFlag,		//%d2
			m_ucFpaGear,					//%d3
			nObjY16,						//%d4
			fTemp,							//%f5
			m_stMtParams->sCurrentShutterValue,//%d6
			m_stMtParams->iShutterValue,//%d7
			m_stMtParams->ucGearMark,			//%d8
			m_stMtParams->m_fieldType,          //%d9
			m_stMtParams->mtType,               //%d10
			m_stMtParams->mtDisType             //%d11
		);
		m_logger->output(LOG_DEBUG, " DIS \t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f",
			m_stMtParams->fCoefA1,
			m_stMtParams->fCoefA2,
			m_stMtParams->fCoefA3,
			m_stMtParams->fCoefB1,
			m_stMtParams->fCoefB2,
			m_stMtParams->fCoefB3,
			m_stMtParams->fCoefC1,
			m_stMtParams->fCoefC2,
			m_stMtParams->fCoefC3);

		m_logger->output(LOG_DEBUG, " TEPM TRANS \t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f",
			m_stMtParams->fRealTimeFpaTemp,	//%f
			m_stMtParams->fRealTimeShutterTemp,//%f
			m_stMtParams->fRealTimeLensTemp,	//%f
			m_stMtParams->fOrinalShutterTemp,	//%f15
			m_stMtParams->fCurrentShutterTemp,	//%f
			m_stMtParams->fCurrentLensTemp,	//%f
			m_stMtParams->fEnvironmentTemp);	//%f

		m_logger->output(LOG_DEBUG, "TEMP PARAM \t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f",
			m_fShutterCorrK,				//%f
			m_fLensCorrK,					//%f20
			fShutterCorrIncrement,			//%f
			fLensCorrIncrement,				//%f
			m_stMtParams->fDistance,			//%f
			m_stMtParams->fEmiss,				//%f
			m_stMtParams->fTransmit,			//%f25
			m_stMtParams->fReflectTemp,		//%f
			m_stMtParams->fHumidity,
			m_stMtParams->fWindowTemperature,
			m_stMtParams->fAtmosphereTransmittance,
			m_stMtParams->fAtmosphereTemperature);       //%f

		m_logger->output(LOG_DEBUG, " CORR \t%hhu\t%hhu\t%hhu\t%hhu\t%hhu\t%hhu\t%hhu\t%hhu",
			m_stMtParams->bEnvironmentCorrection,//%d
			m_stMtParams->bLensCorrection,		//%d
			m_stMtParams->bShutterCorrection,	//%d30
			m_stMtParams->bDistanceCompensate,	//%d
			m_stMtParams->bEmissCorrection,	//%d
			m_stMtParams->bTransCorrection,	//%d
			m_stMtParams->bHumidityCorrection,
			m_stMtParams->bAtmosphereCorrection);	//%d

		m_logger->output(LOG_DEBUG, " KF B \t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f",
			m_stMtParams->fNearKf,				//%f
			m_stMtParams->fNearB,				//%f
			m_stMtParams->fFarKf,				//%f10
			m_stMtParams->fFarB,				//%f
			m_stMtParams->fNearKf2,				//%f
			m_stMtParams->fNearB2,				//%f
			m_stMtParams->fFarKf2,				//%f10
			m_stMtParams->fFarB2);			//%f

		m_logger->output(LOG_DEBUG, "SINGEL TEMP \t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f",
			fForwardNearTemp,
			fForwardFarTemp,
			fBackNearTemp,
			fBackFarTemp,
			fNearWeight,
			fFarWeight,
			fBackWeight,
			fForwardWeight,
			fFarDistanceTemperature,
			fNearDistanceTemperature);
	}
	return 0;
}


void CMeasureTempCore::curveKfWithB(int distanceMark, float *fKf, float *fB)
{

#define TEMP_CURVE_KF_EXPR(kf1, kf2) (((kf1) / 10000.0f) * ((kf2) / 10000.0f))
#define TEM_CURVE_B_EXPR(b1, b2) ((b1) / 100.0f + (b2) / 100.0f)

    if (distanceMark && (nImgWidth == 256 && m_stMtParams->mtType == 0))
    {
        const float tempthres1 = 18.0f;
        const float tempthres2 = 26.0f;
        const float tempthres3 = 31.0f;
        const float a1 = 0.01818f;
        const float a2 = -0.5636f;

        if (m_stMtParams->fRealTimeShutterTemp < tempthres1)
        {
            *fKf = TEMP_CURVE_KF_EXPR(m_stMtParams->fNearKf, m_stMtParams->fNearKf2);
            *fB  = TEM_CURVE_B_EXPR(m_stMtParams->fNearB, m_stMtParams->fNearB2);
        }
        else if (m_stMtParams->fRealTimeShutterTemp >= tempthres1 &&
                 m_stMtParams->fRealTimeShutterTemp <= tempthres2)
        {
            *fKf = TEMP_CURVE_KF_EXPR(m_stMtParams->fNearKf, m_stMtParams->fNearKf2);

            float fB2 = TEM_CURVE_B_EXPR(m_stMtParams->fFarB, m_stMtParams->fFarB2);
            float fB1 = TEM_CURVE_B_EXPR(m_stMtParams->fNearB, m_stMtParams->fNearB2);
            float fB_K = (fB2 - fB1) / (tempthres2 - tempthres1);
            float fB_b = fB1 - fB_K * tempthres1;
            *fB = fB_K *m_stMtParams->fRealTimeShutterTemp + fB_b;
        }
        else if (m_stMtParams->fRealTimeShutterTemp > tempthres2 &&
                 m_stMtParams->fRealTimeShutterTemp <= tempthres3)
        {
            *fKf = TEMP_CURVE_KF_EXPR(m_stMtParams->fFarKf, m_stMtParams->fFarKf2);
            *fB  = TEM_CURVE_B_EXPR(m_stMtParams->fFarB, m_stMtParams->fFarB2);
        }
        else
        {
            float tmp = m_stMtParams->fRealTimeShutterTemp * a1 + a2;
            *fKf = TEMP_CURVE_KF_EXPR(m_stMtParams->fFarKf, m_stMtParams->fFarKf2);
            *fB  = TEM_CURVE_B_EXPR(m_stMtParams->fFarB, m_stMtParams->fFarB2);
            *fB += tmp;
        }
    }
    else if (distanceMark)
    {
        *fKf = TEMP_CURVE_KF_EXPR(m_stMtParams->fFarKf, m_stMtParams->fFarKf2);
        *fB  = TEM_CURVE_B_EXPR(m_stMtParams->fFarB, m_stMtParams->fFarB2);
    }
    else
    {
        *fKf = TEMP_CURVE_KF_EXPR(m_stMtParams->fNearKf, m_stMtParams->fNearKf2);
        *fB  = TEM_CURVE_B_EXPR(m_stMtParams->fNearB, m_stMtParams->fNearB2);
    }
#undef TEMP_CURVE_KF_EXPR
#undef TEM_CURVE_B_EXPR
}

struct TEMP_Y16_CURVE_FORMULA {
    short  y16;
    short *curve;
    int    curveLen;
    int    curveIdx;
    float  kf;
    float  b;
};

float CMeasureTempCore::curveY16Temp(const TEMP_Y16_CURVE_FORMULA *formula)
{
    float temp = 0;

    const short *pCurve = formula->curve;
    int iCurveLen       = formula->curveLen;
    short sCorrValue    = formula->y16;
    float fKf           = formula->kf;
    float fB            = formula->b;

    int idx = formula->curveIdx;
    int iShutterValue = pCurve[idx];
    
    //镜筒温漂修正
    if (m_stMtParams->bLensCorrection)
    {
        sCorrValue -= (short)fLensCorrIncrement;
    }

    //快门温漂修正
    if (m_stMtParams->bShutterCorrection)
    {
        sCorrValue += (short)fShutterCorrIncrement;
    }

    m_stMtParams->iShutterValue = iShutterValue;
    // 快门温度对应的y16值偏移sValue， sValue值是以快门为参考的
    int iMeasureValue = (int)(iShutterValue + sCorrValue * fKf);

    // 反查测温曲线,测温曲线是单调的
    float iMeasureTemp = 0.0f;
    if (iMeasureValue <= pCurve[0])
    {
        iMeasureTemp = 0.0f;
    }
    else if (iMeasureValue >= pCurve[iCurveLen - 1])
    {
        iMeasureTemp = ((float)(iCurveLen * 1.0))/ 10.0f;
    }
    else
    {
        for (int i = 0; i < iCurveLen; i++)
        {
            if (pCurve[i] > iMeasureValue)
            {
                iMeasureTemp = ((float)(i * 1.0)) / 10.0f;
                break;
            }
        }
    }

    temp = iMeasureTemp + m_stMtParams->fMinTemp * 1.0f + fB;

    return temp;
}

int CMeasureTempCore::GetAnalysisTempByY16(ITAAnalysisY16Info *y16Array, ITAAnalysisTemp *tempArray, int len)
{
    if (NULL == y16Array)
    {
        return ITA_NULL_PTR_ERR;
    }

    if (NULL == tempArray)
    {
        return ITA_NULL_PTR_ERR;
    }

    // float fKf = 0.0f;
    // float fB  = 0.0f;

    float fEnvironmentTemp = m_stMtParams->fEnvironmentTemp;
    float fReflectTemp = m_stMtParams->fReflectTemp;
    float fEmiss = m_stMtParams->fEmiss;
    float fTransmit = m_stMtParams->fTransmit;
    float fHumidity = m_stMtParams->fHumidity;

    m_stMtParams->fCorrectDistance = (m_stMtParams->m_fieldType == 4)
                    && (m_stMtParams->mtType == 0)
                    ? m_stMtParams->gDistanceArray[0]
                    : m_stMtParams->gDistanceArray[1];
    
    // 快门的曲线下标
    int iCurveLen  = m_stMtParams->usCurveTempNumber;
    int iTempIndex = (int)(m_stMtParams->fCurrentShutterTemp * 10 - m_stMtParams->fMinTemp * 10);
    if (iTempIndex < 0)
    {
        return ITA_INVALIDE_ENVIRTEMP;
    }
    int  curveIdx = iTempIndex < 0 ? 0 
               : iTempIndex > iCurveLen - 1 
               ? iCurveLen - 1 : iTempIndex;

    
    // Kf & B
    float fFarKf  = 0.0f;
    float fFarB   = 0.0f;
    float fNeerKf = 0.0f;
    float fNeerB  = 0.0f;
    curveKfWithB(0, &fNeerKf, &fNeerB);
    curveKfWithB(1, &fFarKf, &fFarB);
    if (0.0f == fNeerKf || 0.0f == fFarKf)
    {
        return ITA_INVALID_KF_ERROR;
    }

    // 镜筒温漂修正值
    // float fLensCorrIncrement = 0.0f;
    if (m_stMtParams->bLensCorrection)
    {
        LensTempDriftCorr(fLensCorrIncrement);
    }

    // 快门温漂修正值
    // float fShutterCorrIncrement = 0.0f;
    if (m_stMtParams->bShutterCorrection)
    {
        ShutterTempDriftCorr(fShutterCorrIncrement);
    }


#define TEMP_WEIGHT_EXPR_(n, d) (1.0f - (((n)) * 1.0f) / (d))
#define TEMP_WEIGHT_PART_EXPR(n1, n2) TEMP_WEIGHT_EXPR_(n1, ((n1) + (n2)))
    // 根据当前焦温确定对应焦温曲线的权重
    float fBackWeight    = 0.0f;
    float fForwardWeight = 0.0f;
    if (m_ucFpaGear == 0)
    {
        fBackWeight    = 1.0f;
        fForwardWeight = 0.0f;
    }
    else if (m_ucFpaGear == m_stMtParams->ucFpaTempNum)
    {
        fBackWeight    = 0.0f;
        fForwardWeight = 1.0f;
    }
    else
    {
        // float fBackDif    = m_stMtParams->gFpaTempArray[m_ucFpaGear] - m_stMtParams->fRealTimeFpaTemp;
        // float fForwardDif = m_stMtParams->fRealTimeFpaTemp - m_stMtParams->gFpaTempArray[m_ucFpaGear - 1];
        // fBackWeight = 1.0f - (fBackDif * 1.0f) / (fBackDif + fForwardDif);
        // fForwardWeight = 1.0f - (fForwardDif * 1.0f) / (fBackDif + fForwardDif);
        fBackWeight = TEMP_WEIGHT_PART_EXPR(
            (m_stMtParams->gFpaTempArray[m_ucFpaGear] - m_stMtParams->fRealTimeFpaTemp),
            (m_stMtParams->fRealTimeFpaTemp - m_stMtParams->gFpaTempArray[m_ucFpaGear - 1]));

         fForwardWeight = TEMP_WEIGHT_PART_EXPR(
            (m_stMtParams->fRealTimeFpaTemp - m_stMtParams->gFpaTempArray[m_ucFpaGear - 1]),
            (m_stMtParams->gFpaTempArray[m_ucFpaGear] - m_stMtParams->fRealTimeFpaTemp));
    }

    // 根据当前距离确定对应距离的权重
    float fNearWeight = 0.0f;
    float fFarWeight  = 0.0f;
    if (m_stMtParams->fDistance <= m_stMtParams->gDistanceArray[0])
    {
        fNearWeight = 1.0f;
        fFarWeight  = 0.0f;
    }
    else if (m_stMtParams->fDistance >= m_stMtParams->gDistanceArray[1])
    {
        fNearWeight = 0.0f;
        fFarWeight  = 1.0f;
    }
    else
    {
        // float fNearDistanceDif = m_stMtParams->fDistance - m_stMtParams->gDistanceArray[0];
        // float fFarDistanceDif  = m_stMtParams->gDistanceArray[1] - m_stMtParams->fDistance;
        // fNearWeight = 1.0f - (fNearDistanceDif * 1.0f) / (fNearDistanceDif + fFarDistanceDif);
        // fFarWeight  = 1.0f - (fFarDistanceDif * 1.0f) / (fNearDistanceDif + fFarDistanceDif);
        fNearWeight = TEMP_WEIGHT_PART_EXPR(
            (m_stMtParams->fDistance - m_stMtParams->gDistanceArray[0]),
            (m_stMtParams->gDistanceArray[1] - m_stMtParams->fDistance));

        fFarWeight = TEMP_WEIGHT_PART_EXPR(
            (m_stMtParams->gDistanceArray[1] - m_stMtParams->fDistance),
            (m_stMtParams->fDistance - m_stMtParams->gDistanceArray[0]));
    }
#undef TEMP_WEIGHT_EXPR_
#undef TEMP_WEIGHT_PART_EXPR

    // 反射值
    short reflectTemp = GetY16FromT((int)(fReflectTemp * 10.0f));

    float fForwardNearTemp = 0.0f;
    float fForwardFarTemp  = 0.0f;
    float fBackNearTemp    = 0.0f;
    float fBackFarTemp     = 0.0f;

    float fFarDistanceTemperature  = 0.0f;
    float fNearDistanceTemperature = 0.0f;

    
    short y16 = 0;
    float fTemp = 0.0f;
    float *pTemp = NULL;
    for (int i = 0; i < len; i++)
    {
        // y16 = y16Array[i];
        ITAAnalysisY16Info *itemY16  = y16Array + i;
        ITAAnalysisTemp    *itemTemp = tempArray + i;
        for (int k = 0; k < 3; k++)
        {
            switch(k)
            {
                case 0:
                    y16 = itemY16->maxY16;
                    pTemp = &(itemTemp->maxVal);
                break;
                case 1:
                    y16 = itemY16->minY16;
                    pTemp = &(itemTemp->minVal);
                break;
                case 2:
                    y16 = itemY16->avgY16;
                    pTemp = &(itemTemp->avgVal);
                break;
            }
        
            if (m_stMtParams->bHumidityCorrection == 1)
            {
                y16 = HumidityCorr(y16, m_stMtParams->fHumidity);
            }
            
            if (m_ucFpaGear == 0)
            {
                fForwardNearTemp = 0.0f;
                fForwardFarTemp  = 0.0f;
                
                TEMP_Y16_CURVE_FORMULA tf1 = { 0 };
                tf1.y16       = y16;
                tf1.curve     = pusCurve;
                tf1.curveLen  = iCurveLen;
                tf1.curveIdx  = curveIdx;
                tf1.kf        = fNeerKf;
                tf1.b         = fNeerB;
                fBackNearTemp = curveY16Temp(&tf1);

                TEMP_Y16_CURVE_FORMULA tf2 = { 0 };
                tf2.y16       = y16;
                tf2.curve     = pusCurve + iCurveLen;
                tf2.curveLen  = iCurveLen;
                tf2.curveIdx  = curveIdx;
                tf2.kf        = fFarKf;
                tf2.b         = fFarB;
                fBackFarTemp = curveY16Temp(&tf2);
            }
            else if (m_ucFpaGear == m_stMtParams->ucFpaTempNum)
            {
                TEMP_Y16_CURVE_FORMULA tf1 = { 0 };
                tf1.y16       = y16;
                tf1.curve     = pusCurve;
                tf1.curveLen  = iCurveLen;
                tf1.curveIdx  = curveIdx;
                tf1.kf        = fNeerKf;
                tf1.b         = fNeerB;
                fForwardNearTemp = curveY16Temp(&tf1);

                TEMP_Y16_CURVE_FORMULA tf2 = { 0 };
                tf2.y16       = y16;
                tf2.curve     = pusCurve + iCurveLen;
                tf2.curveLen  = iCurveLen;
                tf2.curveIdx  = curveIdx;
                tf2.kf        = fFarKf;
                tf2.b         = fFarB;
                fForwardFarTemp = curveY16Temp(&tf2);

                fBackNearTemp = 0.0f;
                fBackFarTemp  = 0.0f;
            }
            else
            {
                TEMP_Y16_CURVE_FORMULA tf1 = { 0 };
                tf1.y16       = y16;
                tf1.curve     = pusCurve;
                tf1.curveLen  = iCurveLen;
                tf1.curveIdx  = curveIdx;
                tf1.kf        = fNeerKf;
                tf1.b         = fNeerB;
                fForwardNearTemp = curveY16Temp(&tf1);

                TEMP_Y16_CURVE_FORMULA tf2 = { 0 };
                tf2.y16       = y16;
                tf2.curve     = pusCurve + iCurveLen;
                tf2.curveLen  = iCurveLen;
                tf2.curveIdx  = curveIdx;
                tf2.kf        = fFarKf;
                tf2.b         = fFarB;
                fForwardFarTemp = curveY16Temp(&tf2);

                TEMP_Y16_CURVE_FORMULA tf3 = { 0 };
                tf3.y16       = y16;
                tf3.curve     = pusCurve + iCurveLen * 2;
                tf3.curveLen  = iCurveLen;
                tf3.curveIdx  = curveIdx;
                tf3.kf        = fNeerKf;
                tf3.b         = fNeerB;
                fBackNearTemp = curveY16Temp(&tf3);

                TEMP_Y16_CURVE_FORMULA tf4 = { 0 };
                tf4.y16       = y16;
                tf4.curve     = pusCurve + iCurveLen * 3;
                tf4.curveLen  = iCurveLen;
                tf4.curveIdx  = curveIdx;
                tf4.kf        = fFarKf;
                tf4.b         = fFarB;
                fBackFarTemp = curveY16Temp(&tf4);
            }

            // 1.5m 远距离按照焦温加权
            fFarDistanceTemperature  = fForwardFarTemp * fForwardWeight + fBackFarTemp * fBackWeight;
            // 0.5m 近距离安装焦温加权
            fNearDistanceTemperature = fForwardNearTemp * fForwardWeight + fBackNearTemp * fBackWeight;


            // 距离校正
            if (m_stMtParams->m_fieldType == 3)
            {
                // 当镜头类型为120模组标准镜头时，采用此距离校正方法
                fTemp = m_stMtParams->bDistanceCompensate == 1 
                        ?  fFarWeight * fFarDistanceTemperature + fNearWeight * fNearDistanceTemperature 
                        : fNearDistanceTemperature;
            }
            else if ((m_stMtParams->m_fieldType == 4) &&( m_stMtParams->mtType == 0))
            {
                // 当镜头类型为120模组90度镜头时，采用此距离校正方法

                if (m_stMtParams->fDistance == m_stMtParams->fCorrectDistance)
                {
                    fTemp = fNearDistanceTemperature;
                }

                if ((1 == m_stMtParams->bDistanceCompensate) &&
                    (m_stMtParams->fDistance != m_stMtParams->fCorrectDistance))
                {
                    fTemp = DistCorr(fTemp, fNearDistanceTemperature, 
                            m_stMtParams->fDistance, 
                            m_stMtParams->fRealTimeShutterTemp,
                            0);
                }
                 else
                 {
                     fTemp = fNearDistanceTemperature; //不补偿
                 }
            }
            else
            {
                if (m_stMtParams->fDistance == m_stMtParams->fCorrectDistance)
                {
                    fTemp = fFarDistanceTemperature;
                }

                if ((1 == m_stMtParams->bDistanceCompensate) &&
                    (m_stMtParams->fDistance != m_stMtParams->fCorrectDistance))
                {
                    fTemp = DistCorr(fTemp, fFarDistanceTemperature,
                                m_stMtParams->fDistance,
                                m_stMtParams->fRealTimeShutterTemp,
                                m_stMtParams->mtDisType);
                }
                 else
                 {
                     fTemp = fFarDistanceTemperature;//不补偿
                 }
            }

            // 透过率校正
            if (m_stMtParams->bTransCorrection)
            {
                if (fTransmit <= 0.98f)
                {
                    fTemp = EmissCorrection((short)(fTemp * 10.0f),
                                (int)(fEmiss * 100.0f),
                                reflectTemp);
                    
                    fTemp = fTemp / 10.0f;
                }
            }

            // 发射率校正
            if (m_stMtParams->bEmissCorrection)
            {
                if (fEmiss <= 0.98f)
                {
                    fTemp = EmissCorrection((short)(fTemp * 10.0f),
                                (int)(fEmiss * 100.0f),
                                reflectTemp);
                    
                    fTemp = fTemp / 10.0f;
                }
            }

            // 非人体测温
            if (m_stMtParams->mtType != 0)
            {
                // 保留一位小数
                int nTemp = (int)(fTemp * 10.0f);
                fTemp = nTemp / 10.0f;
            }

            // 环温修正
            if (m_stMtParams->bEnvironmentCorrection)
            {
                // 环境温度暂时未使用到
                fTemp = EnvirTempCorr(fTemp, fEnvironmentTemp);

            }

            *pTemp = fTemp;
        }
    }

    return ITA_OK;
}


int CMeasureTempCore::autoChangeRange(short *pSrc, int width, int height, int range,
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

	int areaNum = range == 1 ? round(areaTh1 * width * height) : round((1 - areaTh2) * width * height);
	int tempTh = range == 1 ? low2high : high2low;
	if (m_stMtParams->bLogPrint)
	{
		m_logger->output(LOG_DEBUG, "AutoChange areaNum tempTh \t%d\t%d ", areaNum, tempTh);
	}
	
	for (int i = width * height - 1; i >= 0; i--)
	{
		if (num >= areaNum)
		{
			ifShiting = range == 1 ? 1 : 0;
			if (m_stMtParams->bLogPrint)
			{
				m_logger->output(LOG_DEBUG, "AutoChange Num Range ifShiting \t%d\t%d\t%d ", num, range, ifShiting);
			}
			break;
		}
		else
		{
			float temp;
			GetTempByY16(temp, m_sortY16[i]);
			if (temp <= tempTh)
			{
				ifShiting = range == 1 ? 0 : 1;
				if (m_stMtParams->bLogPrint)
				{
					m_logger->output(LOG_DEBUG, "AutoChange Temp Range  ifShiting \t%f\t%d\t%d ", temp, range, ifShiting);
				}
				break;
			}
			else
				num++;
		}
	}
	return ifShiting;
}

int CMeasureTempCore::autoMultiChangeRange(short* pSrc, int width, int height, int range,
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

	if (m_stMtParams->bLogPrint)
	{
		m_logger->output(LOG_DEBUG, "AutoChange Y16 \t%d\t%d", m_sortY16[0], m_sortY16[width * height - 1]);
	}

	float temp;
	for (int i = width * height - 1; i >= 0; i--)
	{

		GetTempByY16(temp, m_sortY16[i]);

		if (range == 1)
		{
			if (temp < low2mid)
			{
				if (m_stMtParams->bLogPrint)
				{
					m_logger->output(LOG_DEBUG, " AutoChange:low remain");
				}
				ifShiting = 0;
				break;
			}
			else
			{
				NumPasslow2mid++;
				if (NumPasslow2mid >= areaNumDownToUp)
				{
					if (m_stMtParams->bLogPrint)
					{
						m_logger->output(LOG_DEBUG, " AutoChange:low to mid");
					}
					ifShiting = 1;
					break;
				}
			}
		}
		else if (range == 2)
		{
			if (temp < mid2low)
			{
				if (m_stMtParams->bLogPrint)
				{
					m_logger->output(LOG_DEBUG, " AutoChange:mid to low");
				}
				ifShiting = -1;
				break;
			}
			else if ((temp < mid2high) && (temp >= mid2low))
			{
				NumPassmid2low++;
				if (NumPassmid2low >= areaNumUpToDown)
				{
					if (m_stMtParams->bLogPrint)
					{
						m_logger->output(LOG_DEBUG, " AutoChange:mid remain");
					}
					ifShiting = 0;
					break;
				}
			}
			else
			{
				NumPassmid2high++;
				if (NumPassmid2high >= areaNumDownToUp)
				{
					if (m_stMtParams->bLogPrint)
					{
						m_logger->output(LOG_DEBUG, " AutoChange:mid to high");
					}
					ifShiting = 1;
					break;
				}
			}
		}
		else
		{
			if (temp < high2mid)
			{
				if (m_stMtParams->bLogPrint)
				{
					m_logger->output(LOG_DEBUG, " AutoChange:high to mid");
				}
				ifShiting = -1;
				break;
			}
			else
			{
				NumPasshigh2mid++;
				if (NumPasshigh2mid >= areaNumDownToUp)
				{
					if (m_stMtParams->bLogPrint)
					{
						m_logger->output(LOG_DEBUG, " AutoChange:high remain");
					}
					ifShiting = 0;
					break;
				}
			}
		}
	}
	return ifShiting;
}

/*函数功能：反查Y16
输入参数：
float fTemperature，目标温度
unsigned short *pCurve，曲线数据
int distanceMark，距离标志位
返回参数：目标Y16
*/
short CMeasureTempCore::GetY16ByTemp(float fTemperature, short *pCurve, int distanceMark)
{
	short usIndex;
	short          sValue;
	float          fKf;
	float          fB;
	int iTempIndex = (int)(m_stMtParams->fCurrentShutterTemp * 10 - m_stMtParams->fMinTemp * 10);
	int iShutterValue;
	fShutterCorrIncrement = 0.0f;
	fLensCorrIncrement = 0.0f;

	if (distanceMark)
	{
		fKf = (m_stMtParams->fFarKf / 10000.0f)* (m_stMtParams->fFarKf2 / 10000.0f);
		fB = (m_stMtParams->fFarB / 100.0f) + (m_stMtParams->fFarB2 / 100.0f);
	}
	else
	{
		fKf = (m_stMtParams->fNearKf / 10000.0f)* (m_stMtParams->fNearKf2 / 10000.0f);
		fB = m_stMtParams->fNearB / 100.0f + m_stMtParams->fNearB2 / 100.0f;
	}

	usIndex = (fTemperature - m_stMtParams->fMinTemp - fB) * 10;

	if (usIndex < 0)
	{
		sValue = 0;
	}
	else if (usIndex >= m_stMtParams->usCurveTempNumber)
	{
		sValue = 16383;
	}
	else
	{
		sValue = pCurve[usIndex];
	}
    if (m_stMtParams->bLogPrint)
	{
		m_logger->output(LOG_DEBUG, "temp:%.2f,usIndex:%d,iTempIndex:%d,sValue:%d,fKf:%.2f,fB:%.2f", fTemperature, usIndex, iTempIndex, sValue, fKf,fB);
	}

	//根据快门温度查找对应的y16值
	if ((iTempIndex >= 0) && (iTempIndex < m_stMtParams->usCurveTempNumber))
	{
		iShutterValue = pCurve[iTempIndex];
	}
	else if (iTempIndex < 0)
	{
		iTempIndex = 0;
		iShutterValue = pCurve[iTempIndex];

	}
	else
	{
		iTempIndex = m_stMtParams->usCurveTempNumber - 1;
		iShutterValue = pCurve[iTempIndex];

	}
	sValue = sValue - short(iShutterValue);
    if (m_stMtParams->bLogPrint)
	{
		m_logger->output(LOG_DEBUG, "iShutterValue:%d,sValue:%d", iShutterValue, sValue);
	}

	// 反快门温漂
	if (m_stMtParams->bShutterCorrection)
	{

		ShutterTempDriftCorr(fShutterCorrIncrement);
		sValue -= short(fShutterCorrIncrement);
	}

	// 反镜筒温漂修正
	if (m_stMtParams->bLensCorrection)
	{

		LensTempDriftCorr(fLensCorrIncrement);
		sValue += short(fLensCorrIncrement);
	}
	if (m_stMtParams->bLogPrint)
	{
		m_logger->output(LOG_DEBUG, "fShutterCorrIncrement:%.2f,fLensCorrIncrement:%.2f,sValue:%d", fShutterCorrIncrement, fLensCorrIncrement, sValue);
	}

	sValue = short(sValue / fKf);

	return sValue;
}

/*函数功能：反查Y16
输入参数：
float fTemperature，目标温度
unsigned short *pCurve，曲线数据
int distanceMark，距离标志位
MtParams &stMtParams，测温参数
返回参数：目标Y16
*/
short CMeasureTempCore::CalY16ByTemp(short &y16, float fTemperature)
{
	short sForwardNearValue;
	short sBackNearValue;
	short sForwardFarValue;
	short sBackFarValue;
	short sRealValue = 200;
	float fForwardWeight = 0.0f;
	float fBackWeight = 0.0f;
	float fFarDistanceDif = 0.0f;
	float fNearDistanceDif = 0.0f;
	float fFarWeight = 0.0f;
	float fNearWeight = 0.0f;
	float fBackDif = 0.0f;
	float fForwardDif = 0.0f;
	short sFarDistanceValue = 0;
	short sNearDistanceValue = 0;
	short sY16Array[4] = { 0 };
	float ReverseTemp = 0.0f;
	float fEmiss = m_stMtParams->fEmiss;

	//反透过率校正
	if (m_stMtParams->bTransCorrection)
	{
		fEmiss = m_stMtParams->fEmiss * m_stMtParams->fTransmit;
	}

	//反发射率修正
	ReverseTemp = EmissionReverseCorrection(fTemperature , fEmiss);
	if (m_stMtParams->bLogPrint)
	{
		m_logger->output(LOG_DEBUG, "ReverseTemp:%.2f,fEmiss:%.2f", ReverseTemp, fEmiss);
	}


	//反距离修正
	
	ReverseTemp = DistanceReverseCorrection(ReverseTemp , m_stMtParams->mtDisType);
	if (m_stMtParams->bLogPrint)
	{
		m_logger->output(LOG_DEBUG, "ReverseTemp:%.2f", ReverseTemp);
	}
	sForwardNearValue = GetY16ByTemp(ReverseTemp, pusCurve, 0);
	sForwardFarValue = GetY16ByTemp(ReverseTemp, pusCurve + m_stMtParams->usCurveTempNumber, 1);
	sBackNearValue = GetY16ByTemp(ReverseTemp, pusCurve + 2 * m_stMtParams->usCurveTempNumber, 0);
	sBackFarValue = GetY16ByTemp(ReverseTemp, pusCurve + 3 * m_stMtParams->usCurveTempNumber, 1);

	if (m_stMtParams->bLogPrint)
	{
		m_logger->output(LOG_DEBUG, "FN:%d,FF:%d,BN:%d,BF:%d", sForwardNearValue, sForwardFarValue, sBackNearValue, sBackFarValue);
	}

	//确定距离权重和焦温权重
	if (m_ucFpaGear == 0)
	{
		fBackWeight = 1.0f;
		fForwardWeight = 0.0f;
	}
	else if (m_ucFpaGear == m_stMtParams->ucFpaTempNum)
	{
		fBackWeight = 0.0f;
		fForwardWeight = 1.0f;
	}
	else
	{
		fBackDif = m_stMtParams->gFpaTempArray[m_ucFpaGear] - m_stMtParams->fRealTimeFpaTemp;
		fForwardDif = m_stMtParams->fRealTimeFpaTemp - m_stMtParams->gFpaTempArray[m_ucFpaGear - 1];
		fBackWeight = 1.0f - (fBackDif*1.0f) / (fBackDif + fForwardDif);
		fForwardWeight = 1.0f - (fForwardDif*1.0f) / (fBackDif + fForwardDif);
	}
	//距离权重
	if (m_stMtParams->fDistance <= m_stMtParams->gDistanceArray[0])
	{
		fNearWeight = 1.0f;
		fFarWeight = 0.0f;
	}
	else if (m_stMtParams->fDistance >= m_stMtParams->gDistanceArray[1])
	{
		fNearWeight = 0.0f;
		fFarWeight = 1.0f;
	}
	else
	{
		fNearDistanceDif = m_stMtParams->fDistance - m_stMtParams->gDistanceArray[0];
		fFarDistanceDif = m_stMtParams->gDistanceArray[1] - m_stMtParams->fDistance;
		fNearWeight = 1.0f - (fNearDistanceDif*1.0f) / (fNearDistanceDif + fFarDistanceDif);
		fFarWeight = 1.0f - (fFarDistanceDif*1.0f) / (fNearDistanceDif + fFarDistanceDif);
	}
	if (m_stMtParams->bLogPrint)
	{
		m_logger->output(LOG_DEBUG, "BW:%.2f,FW:%.2f,NW:%.2f,FW:%.2f", fBackWeight, fForwardWeight, fNearWeight, fFarWeight);
	}

	if (1 == m_stMtParams->bDistanceCompensate)
	{
		sFarDistanceValue = short(sForwardFarValue*fForwardWeight + sBackFarValue*fBackWeight);
		sNearDistanceValue = short(sForwardNearValue*fForwardWeight + sBackNearValue*fBackWeight);
		sRealValue = sFarDistanceValue;
	}
	else
	{
		sRealValue = short(sForwardNearValue*fForwardWeight + sBackNearValue*fBackWeight);
	}
	y16 = sRealValue;
	if (m_stMtParams->bLogPrint)
	{
		m_logger->output(LOG_DEBUG, "y16:%d", y16);
	}
	return 0;
}

int CMeasureTempCore::CalY16MatrixByTempMatrix(float * pTempMatrix, short * pDst, int nImageHeight, int nImageWidth)
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

/*函数功能：坐标点测温
输入参数：
short *pSrcY16，图像Y16数据
unsigned short x，当前坐标x
unsigned short y，当前坐标y
MtParams &stMtParams，测温参数
返回参数：float &fTemp,坐标点温度
*/
int CMeasureTempCore::GetTempByCoord(float &fTemp, short *pSrcY16, unsigned short x, unsigned short y)
{
	short nObjY16 = 0;

	if ((x < 0) || (x > nImgWidth) || (y < 0) || (y > nImgHeight))
	{
		return ITA_INVALID_COORD;
	}

	if (x > 0 && x < nImgWidth - 1 && y > 0 && y < nImgHeight - 1)
	{
		nObjY16 = pSrcY16[(y - 1)*nImgWidth + x - 1] + pSrcY16[(y - 1)*nImgWidth + x] + pSrcY16[(y - 1)*nImgWidth + x + 1] +
			pSrcY16[y*nImgWidth + x - 1] + pSrcY16[y*nImgWidth + x] + pSrcY16[y*nImgWidth + x + 1] +
			pSrcY16[(y + 1)*nImgWidth + x - 1] + pSrcY16[(y + 1)*nImgWidth + x] + pSrcY16[(y + 1)*nImgWidth + x + 1];

		nObjY16 = short(nObjY16 / 9.0);
	}
	else
	{
		nObjY16 = pSrcY16[y*nImgWidth + x];
	}

	GetTempByY16(fTemp, nObjY16);

	return 0;
}

void CMeasureTempCore::refreshGear(int gear)
{
	m_ucFpaGear = gear;
}

/*函数功能：单条曲线测温
输入参数：
short nObjY16，目标Y16
unsigned short *pCurve，曲线数据
int nSignleCurveLen，单条曲线长度
int distanceMark，距离标志位
返回参数：温度
*/
float CMeasureTempCore::GetSingleCurveTemperature(short nObjY16, short *pCurve, int nSignleCurveLen, int distanceMark, float *kf, float *b)
{
	float fRealTemp = 0.0f;
	short sCorrValue = nObjY16;
	float fKf = 0.0f;
	float fB = 0.0f;
	float fB_K = 0.0f;
	float fB_b = 0.0f;
	float fB1 = 0.0f;
	float fB2 = 0.0f;
	int i;
	int iShutterValue = 0;
	int iMeasureValue = 0;
	float iMeasureTemp = 0.0f;
	int iCurveLen = nSignleCurveLen;

	float tempthres1 = 18.0f;
	float tempthres2 = 26.0f;
	float tempthres3 = 31.0f;
	float TTemp = 0.0f;
	float a1 = 0.01818f;
	float a2 = -0.5636f;

	int iTempIndex = (int)(m_stMtParams->fCurrentShutterTemp * 10 - m_stMtParams->fMinTemp * 10);

	if (iTempIndex < 0)
	{
		return ITA_INVALIDE_ENVIRTEMP;
	}

	//printf("fFarKf = %.2f, fFarB = %.2f, fNearKf = %.2f, fNearB = %.2f\n", m_stMtParams->fFarKf, m_stMtParams->fFarB, m_stMtParams->fNearKf, m_stMtParams->fNearB);

	if (distanceMark)
	{
			//256模组人体测温，kf和b分段
		if ((nImgWidth == 256) && (m_stMtParams->mtType == 0))
		{
			if (m_stMtParams->fRealTimeShutterTemp < tempthres1)
			{
				fKf = (m_stMtParams->fNearKf / 10000.0f) * (m_stMtParams->fNearKf2 / 10000.0f);
				fB = m_stMtParams->fNearB / 100.0f + m_stMtParams->fNearB2 / 100.0f;

			}
			else if (m_stMtParams->fRealTimeShutterTemp >= tempthres1 && m_stMtParams->fRealTimeShutterTemp <= tempthres2)
			{
				fKf = (m_stMtParams->fNearKf / 10000.0f) * (m_stMtParams->fNearKf2 / 10000.0f);
				fB2 = m_stMtParams->fFarB / 100.0f + m_stMtParams->fFarB2 / 100.0f;
				fB1 = m_stMtParams->fNearB / 100.0f + m_stMtParams->fNearB2 / 100.0f;
				fB_K = (fB2 - fB1) / (tempthres2 - tempthres1);
				fB_b = fB1 - fB_K * tempthres1;
				fB = fB_K * m_stMtParams->fRealTimeShutterTemp + fB_b;
			}
			else if (m_stMtParams->fRealTimeShutterTemp > tempthres2 && m_stMtParams->fRealTimeShutterTemp <= tempthres3)
			{
				fKf = (m_stMtParams->fFarKf / 10000.0f) * (m_stMtParams->fFarKf2 / 10000.0f);
				fB = m_stMtParams->fFarB / 100.0f + m_stMtParams->fFarB2 / 100.0f;
				if (kf != NULL && b != NULL)
				{
					*kf = fKf;
					*b = fB;
				}
			}
			else
			{
				TTemp = m_stMtParams->fRealTimeShutterTemp * a1 + a2;
				fKf = (m_stMtParams->fFarKf / 10000.0f) * (m_stMtParams->fFarKf2 / 10000.0f);
				fB = m_stMtParams->fFarB / 100.0f + m_stMtParams->fFarB2 / 100.0f + TTemp;
				if (kf != NULL && b != NULL)
				{
					*kf = fKf;
					*b = fB;
				}
			}
		}
		else
		{
			fKf = (m_stMtParams->fFarKf / 10000.0f) * (m_stMtParams->fFarKf2 / 10000.0f);
			fB = m_stMtParams->fFarB / 100.0f + m_stMtParams->fFarB2 / 100.0f;
		}
	}
	else
	{
		fKf = (m_stMtParams->fNearKf / 10000.0f) * (m_stMtParams->fNearKf2 / 10000.0f);
		fB = m_stMtParams->fNearB / 100.0f + m_stMtParams->fNearB2 / 100.0f;
	}

	if (fKf == 0.0f)
	{
		return ITA_INVALID_KF_ERROR;
	}
	//镜筒温漂修正
	if (m_stMtParams->bLensCorrection)
	{
		LensTempDriftCorr(fLensCorrIncrement);

		sCorrValue -= short(fLensCorrIncrement);
	}

	//快门温漂修正
	if (m_stMtParams->bShutterCorrection)
	{
		ShutterTempDriftCorr(fShutterCorrIncrement);
		sCorrValue += short(fShutterCorrIncrement);
	}

	//根据快门温度查找对应的y16值
	if (iTempIndex < 0)
	{
		iTempIndex = 0;
		iShutterValue = pCurve[iTempIndex];
	}
	else if (iTempIndex > iCurveLen - 1)
	{
		iTempIndex = iCurveLen - 1;
		iShutterValue = pCurve[iTempIndex];
	}
	else
	{
		iShutterValue = pCurve[iTempIndex];
	}
	m_stMtParams->iShutterValue = iShutterValue;
	//快门温度对应的y16值偏移sValue， sValue值是以快门为参考的
	iMeasureValue = (int)(iShutterValue + sCorrValue * fKf);

	if (m_stMtParams->bLogPrint)
	{
		m_logger->output(LOG_DEBUG, "SV=%d %0.2f %0.2f", iShutterValue, fLensCorrIncrement, fShutterCorrIncrement);
		m_logger->output(LOG_DEBUG, "pCurve=%d %d", pCurve[0], pCurve[iCurveLen - 1]);
	}
		

	//反查测温曲线,测温曲线是单调的
	if (iMeasureValue <= pCurve[0])
	{
		iMeasureTemp = 0.0f;
	}
	else if (iMeasureValue >= pCurve[iCurveLen - 1])
	{
		iMeasureTemp = float(iCurveLen * 1.0) / 10.0f;
	}
	else
	{
		for (i = 0; i < iCurveLen; i++)
		{
			if (pCurve[i] > iMeasureValue)
			{
				iMeasureTemp = float(i * 1.0) / 10.0f;
				break;
			}
		}
	}

	fRealTemp = iMeasureTemp + m_stMtParams->fMinTemp*1.0f + fB;


	return fRealTemp;
}

/* 20201013 by 03544cch
接口功能： 获取温度矩阵快速算法，加入ROI区域，距离信息
1. 输入参数: short* pSrcY16, int x, int y, int w, int h, float fDistance
2. 输出参数: float *pTempMatrix （需要在调用接口前预先申请pTempMatrix内存空间，H*W*sizeof(float)）
3. OK返回0，错误返回-1
4. 面测温矩阵精度与重复调用120*90次GetTempByY16，误差在±0.2℃以内
*/
int CMeasureTempCore::GetTempMatrixByY16(float *pObjTempMatrix, short *pSrcY16, int y16W, int y16H, int x, int y, int w, int h, float fDistance) {
	m_stMtParams->fDistance = fDistance;
	short sMin = 32767;
	short sMax = -32767;
	/*外部已检查过。
	if ((y > y16H) || (y < 0) || (x > y16W) || (x < 0))
	{
	return ITA_INVALID_COORD;
	}*/

	// 计算ROI区域最高AD值与最低AD值 
	for (int i = y; i < y + h; i++) {
		for (int j = x; j < x + w; j++) {
			sMin = gdmin(pSrcY16[i * y16W + j], sMin);
			sMax = gdmax(pSrcY16[i * y16W + j], sMax);
		}
	}

	// 分为 NSEPERATENUM 个小段，每一段调用一次测温接口 
	short sValue[NSEPERATENUM] = { 0 };
	float fTemp[NSEPERATENUM] = { 0 };
	bool changed = false;
	if (m_stMtParams->bLogPrint)
	{
		m_stMtParams->bLogPrint = 0;
		changed = true;
	}
	for (int i = 0; i < NSEPERATENUM; i++) {                                             //计算出NSEPERATENUM个温度
		sValue[i] = sMin + short(i * (sMax - sMin) * 1.0 / (NSEPERATENUM - 1));
		//只有高温点输出日志
		if (i == NSEPERATENUM - 1)
		{
			if (changed)
				m_stMtParams->bLogPrint = 1;
			GetTempByY16(fTemp[i], sValue[i]);
			if (changed)
				m_stMtParams->bLogPrint = 0;
		}
		else
			GetTempByY16(fTemp[i], sValue[i]);
	}
	if (changed)
	{
		m_stMtParams->bLogPrint = 1;
	}
	// 预先计算比例表   分段线性函数 
	float fK[NSEPERATENUM] = { 0 };
	for (int n = 0; n < NSEPERATENUM - 1; n++)
	{
		if (sValue[n + 1] - sValue[n] > 0)
		{
			fK[n] = (fTemp[n + 1] - fTemp[n]) * 1.0f / (sValue[n + 1] - sValue[n]);
		}
		else
		{
			fK[n] = 0;
		}
	}

	// ROI区域测温，返回ROI区域温度 
	for (int i = y; i < y + h; i++) {
		for (int j = x; j < x + w; j++) {
			int n = (i - y) * w + j - x; //以人脸框x,y为原点的坐标索引 
			int m = i * y16W + j;//以整幅图像（0,0）为原点的坐标索引 
			if (pSrcY16[m] >= sValue[NSEPERATENUM - 1]) {	//解决朗驰温度矩阵成像后有黑点区域，温度低了7、8°。
				pObjTempMatrix[n] = fTemp[NSEPERATENUM - 1];
			}
			else {
				for (int k = 0; k < NSEPERATENUM - 1; k++) {
					if (pSrcY16[m] >= sValue[k] && pSrcY16[m] < sValue[k + 1]) {
						pObjTempMatrix[n] = fTemp[k] + (pSrcY16[m] - sValue[k]) * fK[k];
						break;
					}
				}
			}
		}
	}
	return 0;
}

/*
接口功能：计算温度映射曲线 (每秒调用一次)
输入参数：	int sMax;		Y16最大值
int sMin;		Y16最小值
输出参数：	float *psTempMap 温度映射曲线，预先申请内存，长度为16384
*/
void CMeasureTempCore::calcTempMap(float *psTempMap, short sMax, short sMin)
{
	// 分为 nSeperateNum 个小段，每一段调用一次测温接口
	int i = 0;
	short sValue[NSEPERATENUM] = { 0 };
	float sTemp[NSEPERATENUM] = { 0 };
	float fK[NSEPERATENUM] = { 0.0f };
	for (i = 0; i < NSEPERATENUM; i++)
	{
		sValue[i] = sMin + short(i * (sMax - sMin) *1.0 / (NSEPERATENUM - 1));

		GetTempByY16(sTemp[i], sValue[i]);// 调用单点测温接口, 放大了100倍

		sValue[i] = sValue[i] + OFFSET;
	}
	// 预先计算比例表
	int n1 = 0;
	for (; n1 < NSEPERATENUM - 1; n1++)
	{
		if (sValue[n1 + 1] - sValue[n1] > 0)
		{
			fK[n1] = (sTemp[n1 + 1] - sTemp[n1]) * 1.0f / (sValue[n1 + 1] - sValue[n1]);
		}
		else
		{
			fK[n1] = 0;
		}
	}

	// 生成查找表
	int nMapMin = gdmax(0, sMin + OFFSET);
	int nMapMax = gdmin(Y16_RANGE, sMax + OFFSET);
	for (i = nMapMin; i < nMapMax; i++)
	{
		if (i >= sValue[NSEPERATENUM - 1])
		{
			psTempMap[i] = sTemp[NSEPERATENUM - 1];
		}
		else if (i <= sValue[0])
		{
			psTempMap[i] = sTemp[0];
		}
		else
		{
			int n = 0;
			for (; n < NSEPERATENUM - 1; n++)
			{
				if (i >= sValue[n] && i < sValue[n + 1])
				{
					psTempMap[i] = sTemp[n] + 1.0f * (i - sValue[n]) * fK[n];
					break;
				}
			}
		}
	}
}


void CMeasureTempCore::calcMatrixFromMap(float *psTempMatrix, float *psTempMap, short *psValue, int nLen)
{
	int i = 0;
	for (i = 0; i < nLen; i++)
	{
		psTempMatrix[i] = psTempMap[psValue[i] + OFFSET];
	}
}

/*  T256 2021.6.29 cch
接口功能：	计算温度映射曲线
输入参数：	int sMax;		Y16最大值
int sMin;		Y16最小值
输出参数：	float *psTempMap 温度映射曲线，预先申请内存，长度为16384
*/
void CMeasureTempCore::calcTempCurveOld(float *psTempMap, short sMax, short sMin)
{
	int i = 0;
	bool changed = false;
	if (m_stMtParams->bLogPrint)
	{
		m_stMtParams->bLogPrint = 0;
		changed = true;
	}
	for (i = sMin; i < sMax; i++)
	{
		int val = gdmin(Y16_RANGE, gdmax(0, i + OFFSET));
		GetTempByY16(psTempMap[val], (short)i);// 调用单点测温接口, 放大了100倍
	}
	float sTempMin;
	float sTempMax;
	GetTempByY16(sTempMin, sMin);
	if (changed)
		m_stMtParams->bLogPrint = 1;
	GetTempByY16(sTempMax, sMax);

	short imin = gdmin(Y16_RANGE, gdmax(0, sMin + OFFSET));//防止溢出
	int j;
	for (j = 0; j < imin; j++)
	{
		psTempMap[j] = sTempMin;
	}

	short imax = gdmin(Y16_RANGE, gdmax(0, sMax + OFFSET));//防止溢出
	int k;
	for (k = imax; k <= Y16_RANGE; k++)
	{
		psTempMap[k] = sTempMax;
	}
}


/*  T256 2021.6.29 cch
接口功能：	根据温度映射曲线，查找输出温度矩阵
输入参数：	float *psTempMap,	温度映射曲线
short *psValue,		Y16数据帧
int nLen;			像素总数 = Y16宽*Y16高
输出参数：	float *psTempMatrix 温度矩阵，预先申请内存，长度为nLen*sizeof(short)
*/
int CMeasureTempCore::calcMatrixFromCurveOld(float *psTempMatrix, short* pSrcY16, int y16W, int y16H, int x, int y, int w, int h, float fDistance)
{
	int i, j;
	short sMax = -30000, sMin = 30000, t, *start, *lineStart;

	if ((y > y16H) || (y < 0) || (x > y16W) || (x < 0))
	{
		return ITA_INVALID_COORD;
	}

	m_stMtParams->fDistance = fDistance;
	if (!m_pTempMap)
	{
		m_pTempMap = (float *)porting_calloc_mem(16384, sizeof(float), ITA_MT_MODULE);
	}
	/*找出指定区域的高温和低温，生成表。区域越大耗时越长。*/
	start = pSrcY16 + y*y16W + x;
	for (i = 0; i < h; i++)
	{
		lineStart = start + i*y16W;
		for (j = 0; j < w; j++)
		{
			t = *(lineStart + j);
			if (t > sMax)
				sMax = t;
			if (t < sMin)
				sMin = t;
		}
	}
	calcTempCurveOld(m_pTempMap, sMax, sMin);
	start = pSrcY16 + y*y16W + x;
	for (i = 0; i < h; i++)
	{
		lineStart = start + i*y16W;
		for (j = 0; j < w; j++)
		{
			t = *(lineStart + j);
			short val = gdmin(Y16_RANGE, gdmax(0, t + OFFSET));
			psTempMatrix[i*w + j] = m_pTempMap[val];
		}
	}
	return 0;
}


/*  T256 2021.8.16 cch
接口功能：	计算温度映射曲线
输入参数：	int sMax;		Y16最大值
int sMin;		Y16最小值
输出参数：	float *psTempMap 温度映射曲线，预先申请内存，长度为16384
*/
void CMeasureTempCore::calcTempCurve(float *psTempMap, short sMax, short sMin)
{
	//memset(psTempMap, 0, (Y16_RANGE + 1)*sizeof(short));
	int i = 0;
	int val = 0;
	for (i = sMin; i < sMax; i += 4)
	{
		val = gdmin(Y16_RANGE - 3, gdmax(0, i + OFFSET));
		//val = i + OFFSET;
		GetTempByY16(psTempMap[val], (short)i);			// 调用单点测温接口, 放大了100倍
		GetTempByY16(psTempMap[val + 1], (short)i + 1);	// 调用单点测温接口, 放大了100倍
		GetTempByY16(psTempMap[val + 2], (short)i + 2);	// 调用单点测温接口, 放大了100倍
		GetTempByY16(psTempMap[val + 3], (short)i + 3);	// 调用单点测温接口, 放大了100倍
	}

	float sTempMin;
	float sTempMax;
	GetTempByY16(sTempMin, sMin);
	GetTempByY16(sTempMax, sMax);

	short imin = gdmin(Y16_RANGE, gdmax(0, sMin + OFFSET));//防止溢出
	int j;
	for (j = 0; j < imin; j++)
	{
		psTempMap[j] = sTempMin;
	}

	short imax = gdmin(Y16_RANGE, gdmax(0, sMax + OFFSET));//防止溢出

	int k;
	for (k = imax; k <= Y16_RANGE; k++)
	{
		psTempMap[k] = sTempMax;
	}
}


/*  T256 2021.8.16 cch
接口功能：	根据温度映射曲线，查找输出温度矩阵
输入参数：	float *psTempMap,	温度映射曲线
short *psValue,		Y16数据帧
int nLen;			像素总数 = Y16宽*Y16高
输出参数：	float *psTempMatrix 温度矩阵，预先申请内存，长度为nLen*sizeof(short)
*/
void CMeasureTempCore::calcMatrixFromCurve(float *psTempMatrix, float *psTempMap, short *psValue, int nLen)
{
	int i = 0;
	short val = 0;
	//memset(psTempMatrix, 0, nLen * sizeof(short));
	for (i = 0; i < nLen; i += 4)
	{
		val = gdmin(Y16_RANGE, gdmax(0, psValue[i] + OFFSET));
		psTempMatrix[i] = psTempMap[val];
		val = gdmin(Y16_RANGE, gdmax(0, psValue[i + 1] + OFFSET));
		psTempMatrix[i + 1] = psTempMap[val];
		val = gdmin(Y16_RANGE, gdmax(0, psValue[i + 2] + OFFSET));
		psTempMatrix[i + 2] = psTempMap[val];
		val = gdmin(Y16_RANGE, gdmax(0, psValue[i + 3] + OFFSET));
		psTempMatrix[i + 3] = psTempMap[val];
	}
}

/*函数功能：快门温漂修正
输入参数：无
返回参数：快门温漂修正量
*/
short CMeasureTempCore::ShutterTempDriftCorr(float &fShutterCorrIncre)
{
	fShutterCorrIncre = m_fShutterCorrK * (m_stMtParams->fCurrentShutterTemp - m_stMtParams->fOrinalShutterTemp);
	return 0;
}

/*函数功能：镜筒温漂修正
输入参数：无
返回参数：镜筒温漂修正量
*/
short CMeasureTempCore::LensTempDriftCorr(float &fLensCorrIncre)
{
	fLensCorrIncre = m_fLensCorrK * (m_stMtParams->fRealTimeLensTemp - m_stMtParams->fCurrentLensTemp);
	return 0;
}

/*函数功能：体表体内温度映射
输入参数：
float fSurfaceTemp，目标体表温度
float fEnvirTemp，目标所处的环温
返回参数：目标体内温度
*/
short CMeasureTempCore::SurfaceTempToBody(float &fBodyTemp, float fSurfaceTemp, float fEnvirTemp)
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

/*函数功能：根据实时快门温估算环温
输入参数：无
返回参数：估算的环温
by 03225px
*/
float CMeasureTempCore::GetEnvirTemp(float &fEnvirTemp)
{
	int time = m_stMtParams->iStartingTime;
	float fTempDown;

	if (time < 3000)
	{
		fEnvirTemp = m_stMtParams->fOrinalShutterTemp;
	}
	else
	{
		if ((m_stMtParams->fRealTimeShutterTemp - m_stMtParams->fOrinalShutterTemp) > 10.0f)

			fTempDown = 10.0f;
		else if ((m_stMtParams->fRealTimeShutterTemp - m_stMtParams->fOrinalShutterTemp) < 1.5f)

			fTempDown = 1.5f;
		else
			fTempDown = m_stMtParams->fRealTimeShutterTemp - m_stMtParams->fOrinalShutterTemp;

		fEnvirTemp = m_stMtParams->fRealTimeShutterTemp - fTempDown;
	}
	return fEnvirTemp;
}

/*函数功能：透过率校正
输入参数：
int fTransmit 透过率
short nObjY16 目标Y16
返回参数：目标Y16
by gzy
*/
short CMeasureTempCore::TransmitCorr(short nObjY16, int fTransmit)
{
	fTransmit = 0;
	return nObjY16;
}

/*根据温度查辐射量
输入参数：
int ReflectT, 反射温度 * 10
返回参数：ReflectT对应的Y16 * 10
by gzy
*/
int CMeasureTempCore::GetY16FromT(int ReflectT)
{
	int nReflectY16;
	//得到未做发射率校正的温度对应的Y16
	/*short Y16NoEmiss = GetY16FromT(TNoEmiss, mBoschParam, NewCurveBuffer);*/
	if (ReflectT >= nEmissCurve[nEmissCurveLen - 1])
	{
		nReflectY16 = nEmissCurveLen - 1;
		return short(nReflectY16);
	}
	else if (ReflectT <= nEmissCurve[0])
	{
		nReflectY16 = 0;
		return short(nReflectY16);
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
	return nReflectY16;
}

/*发射率校正 校正后的温度nTEmiss在（-45,850）
输入参数：
short nTNoEmiss, 待修正温度 * 10
short nY16Reflect, 反射温度对应Y16 * 10
int nEmiss, 发射率 * 100
返回参数：nTEmiss经过发射率校正后的温度 * 10
by gzy
*/
float CMeasureTempCore::EmissCorrection(short nTNoEmiss, int nEmiss, int nY16Reflect)
{
	//小于0异常，直接返回最高温
	if (nEmiss <= 0)
		return nEmissCurve[nEmissCurveLen - 1];

	int nY16NoEmiss;
	int  nTEmissFloor, nTEmissCeil;
	int nEmissIndexFloor, nEmissIndexCeil;
	float nTEmiss, fEmissIndex, fY16NoEmiss;

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
			fY16NoEmiss = float(nY16NoEmiss + ((nTNoEmiss - nEmissCurve[nY16NoEmiss]) * 1.0 / (nEmissCurve[nY16NoEmiss + 1] - nEmissCurve[nY16NoEmiss])));
		else if (nDeltaT < 0)
			fY16NoEmiss = float(nY16NoEmiss - ((nEmissCurve[nY16NoEmiss] - nTNoEmiss) * 1.0 / (nEmissCurve[nY16NoEmiss] - nEmissCurve[nY16NoEmiss - 1])));
		else
			fY16NoEmiss = float(nY16NoEmiss);
	}
	else
		fY16NoEmiss = float(nY16NoEmiss);

	//发射率修正
	fEmissIndex = (fY16NoEmiss * 100.0f - (100.0f - nEmiss) * nY16Reflect) / nEmiss;
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
	nTEmiss = nTEmissFloor + (nTEmissCeil - nTEmissFloor) * (fEmissIndex - nEmissIndexFloor);

	return nTEmiss;

}



/*函数功能：距离修正
输入参数：
float objTemp，未修正的目标温度
float distance，目标距离
float EnvTemp，目标所处的环温
返回参数：距离修正后的温度
by 03225px
*/
float CMeasureTempCore::DistCorr(float &fTemp, float objTemp, float distance, float EnvTemp, int Param)
{
	if (m_stMtParams->mtType == 0)           //人体测温
	{
		if (m_stMtParams->m_fieldType == 4)    //120模组90度镜头距离修正算法
		{
			fTemp = objTemp + m_stMtParams->fCoefA1 * (distance - m_stMtParams->fCoefA2);
			return fTemp;
		}
		else                  //其他镜头距离修正方式，距离修正量受环境温度影响
		{
			float threshTemp1 = 18.0f;
			float threshTemp2 = 31.0f;
			float threshTemp3 = 36.0f;
			float threshTemp4 = 41.0f;
			float threshTemp5 = 42.5f;   //温度阈值    

			float fCorrTemp = 0.0f;
			float TempCorrection = 0.0f;
			float TempCorrectionK = 0.0f;

			fCorrTemp = (m_stMtParams->fCoefA1 * distance + m_stMtParams->fCoefA2) * objTemp * objTemp + (m_stMtParams->fCoefB1 * distance + m_stMtParams->fCoefB2) * objTemp + m_stMtParams->fCoefC1 * distance + m_stMtParams->fCoefC2;
			TempCorrection = fCorrTemp - objTemp;

			if (m_stMtParams->bUserTempCorrection == 1)
			{
				if (EnvTemp <threshTemp1)
				{
					TempCorrectionK = 1.4f;
				}
				if ((EnvTemp >= threshTemp1) && (EnvTemp < threshTemp2))
				{
					float a1 = -0.03077f;
					float a2 = 1.954f;
					TempCorrectionK = a1*EnvTemp + a2;

				}
				if ((EnvTemp >= threshTemp2) && (EnvTemp <= threshTemp3))
				{
					TempCorrectionK = 1.0f;
				}
				if ((EnvTemp >threshTemp3) && (EnvTemp <= threshTemp5))
				{
					float a1 = -0.1538f;
					float a2 = 6.538f;
					TempCorrectionK = a1*EnvTemp + a2;
				}

				if (m_stMtParams->fRealTimeShutterTemp >threshTemp5)
				{

					TempCorrectionK = 0.0f;
				}
				TempCorrectionK = TempCorrectionK *1.2f;

				fTemp = objTemp + TempCorrectionK * TempCorrection;
			}
			else
			{
				if (EnvTemp <threshTemp1)
				{
					TempCorrectionK = 1.8f;
				}
				if ((EnvTemp >= threshTemp1) && (EnvTemp < threshTemp2))
				{
					float a1 = -0.06154f;
					float a2 = 2.908f;
					TempCorrectionK = a1*EnvTemp + a2;

				}
				if ((EnvTemp >= threshTemp2) && (EnvTemp <= threshTemp3))
				{
					TempCorrectionK = 1.0f;
				}
				if ((EnvTemp >threshTemp3) && (EnvTemp <= threshTemp4))
				{
					float a1 = -0.2f;
					float a2 = 8.2f;
					TempCorrectionK = a1*EnvTemp + a2;
				}

				if (EnvTemp >threshTemp4)
				{

					TempCorrectionK = 0.0f;
				}

				fTemp = objTemp + TempCorrectionK * TempCorrection;

			}
		}

	}

	else
	{
		if (Param == 0)            //6参数距离修正
		{
			fTemp = (m_stMtParams->fCoefA1 * distance + m_stMtParams->fCoefA2) * objTemp * objTemp + (m_stMtParams->fCoefB1 * distance + m_stMtParams->fCoefB2) * objTemp + m_stMtParams->fCoefC1 * distance + m_stMtParams->fCoefC2;

		}
		else                      //9参数距离修正
		{
			fTemp = (m_stMtParams->fCoefA1 * distance * distance + m_stMtParams->fCoefA2 * distance + m_stMtParams->fCoefA3) * objTemp * objTemp + (m_stMtParams->fCoefB1 * distance * distance + m_stMtParams->fCoefB2 * distance + m_stMtParams->fCoefB3) * objTemp + m_stMtParams->fCoefC1 * distance * distance + m_stMtParams->fCoefC2 * distance + m_stMtParams->fCoefC3;

		}
	}
	return fTemp;
}

/*函数功能：厂内一次校温（256模组1.5米10℃环温人体测温使用，120模组0.5米常温使用）
输入参数：
AutoCorrectTempPara *autocorTpara 自动校温所需要的黑体等信息
返回参数：KF和B
by 03225px
*/
bool CMeasureTempCore::AutoCorrectTempNear(float &fNearKf, float &fNearB, AutoCorrectTempPara *autocorTpara)
{
	float fKf0 = m_stMtParams->fNearKf;
	float fB0 = m_stMtParams->fNearB;
	int nBlackBodyNumber = autocorTpara->nBlackBodyNumber;
	float tempErrorCorrect[10] = { 0 };
	float T1 = 0;
	short Y16_1 = 0;
	float T1_measure = 0;
	float fTemperatureDifferRatio = 1;//将不同温度段，温度误差归一化的参数
	float deltaT0 = 0;//初始化各个温度点的误差
	float deltaT_min = 65535;
	float deltaT = 0;//校温后各个温度点的误差

					 //计算初始误差
	for (int m = 0; m < nBlackBodyNumber; m++)
	{
		T1 = autocorTpara->TBlackBody[m];
		Y16_1 = autocorTpara->Y16[m];
		GetTempByY16(T1_measure, Y16_1);
		if (m_stMtParams->mtType == 0)
		{
			deltaT0 = deltaT0 + (T1_measure - T1)*(T1_measure - T1);
		}
		else
		{
			if (T1 <= 0)
			{
				fTemperatureDifferRatio = 0.5f;//<0℃  2/4
			}
			else if (T1 <= 100)
			{
				fTemperatureDifferRatio = 1.0f;//0-100℃时 2/2
			}
			else if (T1 > 100)
			{
				fTemperatureDifferRatio = 1.0f / (T1*0.01f);//>100℃时 2/(T*2%)
			}
			deltaT0 = deltaT0 + (T1_measure - T1)*(T1_measure - T1)*fTemperatureDifferRatio*fTemperatureDifferRatio;
		}
		autocorTpara->ErrorNoCorrect[m] = T1_measure - T1;//返回未校温时误差
	}

	//计算校温后误差，KF范围为7000~13000   B范围为-1000~1000    遍历kf b计算最小误差
	for (int i = 0; i < 61; i = i + 1)
	{
		for (int j = 0; j < 201; j = j + 1)
		{
			deltaT = 0;//不同kf、b，总误差需要归零

			if ((nImgWidth == 256) && (m_stMtParams->mtType == 0))   //256人体测温 Kf不变，只调整B；
			{
				m_stMtParams->fNearKf = 10000.0f;   //KF
				m_stMtParams->fNearB = (10.0f - float(j * 0.1f)) * 100.0f;  //B
			}
			else if ((m_stMtParams->m_fieldType == 3) && (m_stMtParams->mtType == 0))
			{
				m_stMtParams->fNearKf = (0.9f + float(0.01 * (i / 3))) * 10000.0f;    //KF
				m_stMtParams->fNearB = (3.3f - float((j / 3) * 0.1f)) * 100.0f; //B
			}
			else
			{
				m_stMtParams->fNearKf = (0.7f + float(0.01 * i)) * 10000.0f;   //KF        //其他调整kf和b；
				m_stMtParams->fNearB = (10.0f - float(j * 0.1f)) * 100.0f;  //B
			}

			for (int m = 0; m < nBlackBodyNumber; m++)
			{
				T1 = autocorTpara->TBlackBody[m];
				Y16_1 = autocorTpara->Y16[m];
				GetTempByY16(T1_measure, Y16_1);/*****************************************************************************************/

				if (m_stMtParams->mtType == 0)
				{
					deltaT = deltaT + (T1_measure - T1)*(T1_measure - T1);
				}
				else
				{
					if (T1 <= 100)
					{
						fTemperatureDifferRatio = 1.01f;
					}
					else if (T1 > 100)
					{
						fTemperatureDifferRatio = 2.0f / (T1*0.02f);
					}
					deltaT = deltaT + (T1_measure - T1)*(T1_measure - T1) * fTemperatureDifferRatio * fTemperatureDifferRatio;
				}
				tempErrorCorrect[m] = T1_measure - T1;//返回校温时的误差			
			}

			//printf("36:%f   KF:%f b:%f  deltaT:%f\n", T1_measure, m_stMtParams->fNearKf, m_stMtParams->fNearB, deltaT);

			if (deltaT < deltaT_min)
			{
				deltaT_min = deltaT;

				fNearKf = m_stMtParams->fNearKf;
				fNearB = m_stMtParams->fNearB;

				for (int m = 0; m < nBlackBodyNumber; m++)
				{
					autocorTpara->ErrorCorrect[m] = tempErrorCorrect[m];//总的误差减小时，返回校温时的误差
				}
			}
		}
	}

	//返回参数
	if (deltaT_min < deltaT0)
		return true;
	else
	{
		//校温失败，返回原始传进来的fKf0、fB0
		fNearKf = fKf0;
		fNearB = fB0;
		for (int m = 0; m < nBlackBodyNumber; m++)
		{
			autocorTpara->ErrorCorrect[m] = autocorTpara->ErrorNoCorrect[m];//用于返回的未校温时误差
		}
		return false;
	}
}

/*函数功能：客户二次校温（256模组1.5米10℃环温人体测温使用，120模组0.5米常温使用）
输入参数：
AutoCorrectTempPara *autocorTpara 自动校温所需要的黑体等信息
返回参数：KF和B
by 03225px
*/
bool CMeasureTempCore::AutoCorrectTempNear_ByUser(float &fNearKf2, float &fNearB2, AutoCorrectTempPara *autocorTpara)
{
	float fKf0 = m_stMtParams->fNearKf2;
	float fB0 = m_stMtParams->fNearB2;
	int nBlackBodyNumber = autocorTpara->nBlackBodyNumber;
	float tempErrorCorrect[10] = { 0 };
	float T1 = 0;
	short Y16_1 = 0;
	float T1_measure = 0;
	float fTemperatureDifferRatio = 1;//将不同温度段，温度误差归一化的参数
	float deltaT0 = 0;//初始化各个温度点的误差
	float deltaT_min = 65535;
	float deltaT = 0;//校温后各个温度点的误差

					 //计算初始误差
	for (int m = 0; m < nBlackBodyNumber; m++)
	{
		T1 = autocorTpara->TBlackBody[m];
		Y16_1 = autocorTpara->Y16[m];
		GetTempByY16(T1_measure, Y16_1);
		if (m_stMtParams->mtType == 0)
		{
			deltaT0 = deltaT0 + (T1_measure - T1)*(T1_measure - T1);
		}
		else
		{
			if (T1 <= 0)
			{
				fTemperatureDifferRatio = 0.5f;//<0℃  2/4
			}
			else if (T1 <= 100.0f)
			{
				fTemperatureDifferRatio = 1.0f;//0-100℃时 2/2
			}
			else if (T1 > 100.0f)
			{
				fTemperatureDifferRatio = 1.0f / (T1*0.01f);//>100℃时 2/(T*2%)
			}
			deltaT0 = deltaT0 + (T1_measure - T1)*(T1_measure - T1)*fTemperatureDifferRatio*fTemperatureDifferRatio;
		}
		autocorTpara->ErrorNoCorrect[m] = T1_measure - T1;//返回未校温时误差
	}

	//计算校温后误差，KF范围为7000~13000   B范围为-1000~1000
	for (int i = 0; i < 61; i = i + 1)
	{
		for (int j = 0; j < 201; j = j + 1)
		{
			deltaT = 0;//不同kf、b，总误差需要归零

			if ((nImgWidth == 256) && (m_stMtParams->mtType == 0))    			   //256人体测温 Kf不变，只调整B；
			{
				m_stMtParams->fNearKf2 = 10000.0f;   //KF
				m_stMtParams->fNearB2 = (10.0f - float(j * 0.1f)) * 100.0f;  //B
			}
			else if ((m_stMtParams->m_fieldType == 3) && (m_stMtParams->mtType == 0))
			{
				m_stMtParams->fNearKf2 = (0.9f + float(0.01 * (i / 3))) * 10000.0f;    //KF
				m_stMtParams->fNearB2 = (3.3f - float((j / 3) * 0.1f)) * 100.0f; //B
			}
			else
			{
				m_stMtParams->fNearKf2 = (0.7f + float(0.01 * i)) * 10000.0f;    //KF
				m_stMtParams->fNearB2 = (10.0f - float(j * 0.1f)) * 100.0f; //B
			}

			for (int m = 0; m < nBlackBodyNumber; m++)
			{
				T1 = autocorTpara->TBlackBody[m];
				Y16_1 = autocorTpara->Y16[m];
				GetTempByY16(T1_measure, Y16_1);/*****************************************************************************************/

				if (m_stMtParams->mtType == 0)
				{
					deltaT = deltaT + (T1_measure - T1)*(T1_measure - T1);
				}
				else
				{
					if (T1 <= 100.0f)
					{
						fTemperatureDifferRatio = 1.01f;
					}
					else if (T1 > 100.0f)
					{
						fTemperatureDifferRatio = 2.0f / (T1*0.02f);
					}
					deltaT = deltaT + (T1_measure - T1)*(T1_measure - T1) * fTemperatureDifferRatio * fTemperatureDifferRatio;
				}
				tempErrorCorrect[m] = T1_measure - T1;//返回校温时的误差			
			}

			//printf("36:%f   KF:%f b:%f  deltaT:%f\n", T1_measure, m_stMtParams->fNearKf, m_stMtParams->fNearB, deltaT);

			if (deltaT < deltaT_min)
			{
				deltaT_min = deltaT;

				fNearKf2 = m_stMtParams->fNearKf2;
				fNearB2 = m_stMtParams->fNearB2;

				for (int m = 0; m < nBlackBodyNumber; m++)
				{
					autocorTpara->ErrorCorrect[m] = tempErrorCorrect[m];//总的误差减小时，返回校温时的误差
				}
			}
		}
	}

	//返回参数
	if (deltaT_min < deltaT0)
		return true;
	else
	{
		//校温失败，返回原始传进来的fKf0、fB0
		fNearKf2 = fKf0;
		fNearB2 = fB0;
		for (int m = 0; m < nBlackBodyNumber; m++)
		{
			autocorTpara->ErrorCorrect[m] = autocorTpara->ErrorNoCorrect[m];//用于返回的未校温时误差
		}
		return false;
	}
}

/*函数功能：厂内一次校温（256模组1.5米23℃环温人体测温和工业测温均可使用，120模组1.2米常温使用）
输入参数：
AutoCorrectTempPara *autocorTpara 自动校温所需要的黑体等信息
返回参数：KF和B
by 03225px
*/
bool CMeasureTempCore::AutoCorrectTempFar(float &fFarKf, float &fFarB, AutoCorrectTempPara *autocorTpara)
{
	float fKf0 = m_stMtParams->fFarKf;
	float fB0 = m_stMtParams->fFarB;
	int nBlackBodyNumber = autocorTpara->nBlackBodyNumber;
	float tempErrorCorrect[10] = { 0 };
	float T1 = 0;
	short Y16_1 = 0;
	float T1_measure = 0;
	float fTemperatureDifferRatio = 1;//将不同温度段，温度误差归一化的参数
	float deltaT0 = 0;//初始化各个温度点的误差
	float deltaT_min = 65535;
	float deltaT = 0;//校温后各个温度点的误差

	m_logger->output(LOG_INFO, "actf %d %d %f", nImgWidth, m_stMtParams->m_fieldType, m_stMtParams->fFarB2);
	//计算初始误差
	for (int m = 0; m < nBlackBodyNumber; m++)
	{
		T1 = autocorTpara->TBlackBody[m];
		Y16_1 = autocorTpara->Y16[m];
		GetTempByY16(T1_measure, Y16_1);
		if (m_stMtParams->mtType == 0)
		{
			deltaT0 = deltaT0 + (T1_measure - T1)*(T1_measure - T1);
		}
		else
		{
			if (T1 <= 0)
			{
				fTemperatureDifferRatio = 0.5f;//<0℃  2/4
			}
			else if (T1 <= 100)
			{
				fTemperatureDifferRatio = 1.0f;//0-100℃时 2/2
			}
			else if (T1 > 100)
			{
				fTemperatureDifferRatio = 1.0f / (T1*0.01f);//>100℃时 2/(T*2%)
			}
			deltaT0 = deltaT0 + (T1_measure - T1)*(T1_measure - T1)*fTemperatureDifferRatio*fTemperatureDifferRatio;
		}
		autocorTpara->ErrorNoCorrect[m] = T1_measure - T1;//返回未校温时误差
	}

	//计算校温后误差，KF范围为7000~13000   B范围为-1000~1000
	for (int i = 0; i < 61; i = i + 1)
	{
		for (int j = 0; j < 201; j = j + 1)
		{
			deltaT = 0;//不同kf、b，总误差需要归零

			if ((nImgWidth == 256) && (m_stMtParams->mtType == 0))     //256人体测温 Kf不变，只调整B；
			{
				m_stMtParams->fFarKf = 10000.0f;   //KF
				m_stMtParams->fFarB = (10.0f - float(j * 0.1f)) * 100.0f;  //B
			}
			else if ((m_stMtParams->m_fieldType == 3) && (m_stMtParams->mtType == 0))
			{
				m_stMtParams->fFarKf = (0.9f + float(0.01 * (i / 3))) * 10000.0f;    //KF
				m_stMtParams->fFarB = (3.3f - float((j / 3) * 0.1f)) * 100.0f; //B
			}
			else
			{
				m_stMtParams->fFarKf = (0.7f + float(0.01 * i)) * 10000.0f;    //KF
				m_stMtParams->fFarB = (10.0f - float(j * 0.1f)) * 100.0f; //B
			}

			for (int m = 0; m < nBlackBodyNumber; m++)
			{
				T1 = autocorTpara->TBlackBody[m];
				Y16_1 = autocorTpara->Y16[m];
				GetTempByY16(T1_measure, Y16_1);/*****************************************************************************************/

				if (m_stMtParams->mtType == 0)
				{
					deltaT = deltaT + (T1_measure - T1)*(T1_measure - T1);
				}
				else
				{
					if (T1 <= 100.0f)
					{
						fTemperatureDifferRatio = 1.01f;
					}
					else if (T1 > 100.0f)
					{
						fTemperatureDifferRatio = 2.0f / (T1*0.02f);
					}
					deltaT = deltaT + (T1_measure - T1)*(T1_measure - T1) * fTemperatureDifferRatio * fTemperatureDifferRatio;
				}
				tempErrorCorrect[m] = T1_measure - T1;//返回校温时的误差			
			}

			//printf("36:%f   KF:%f b:%f  deltaT:%f\n", T1_measure, m_stMtParams->fNearKf, m_stMtParams->fNearB, deltaT);

			if (deltaT < deltaT_min)
			{
				deltaT_min = deltaT;

				fFarKf = m_stMtParams->fFarKf;
				fFarB = m_stMtParams->fFarB;

				for (int m = 0; m < nBlackBodyNumber; m++)
				{
					autocorTpara->ErrorCorrect[m] = tempErrorCorrect[m];//总的误差减小时，返回校温时的误差
				}
			}
		}
	}

	//返回参数
	if (deltaT_min < deltaT0)
		return true;
	else
	{
		//校温失败，返回原始传进来的fKf0、fB0
		fFarKf = fKf0;
		fFarB = fB0;
		for (int m = 0; m < nBlackBodyNumber; m++)
		{
			autocorTpara->ErrorCorrect[m] = autocorTpara->ErrorNoCorrect[m];//用于返回的未校温时误差
		}
		return false;
	}
}

/*函数功能：客户二次校温（256模组1.5米23℃环温人体测温和工业测温均可使用，120模组1.2米常温使用）
输入参数：
AutoCorrectTempPara *autocorTpara 自动校温所需要的黑体等信息
返回参数：KF和B
by 03225px
*/
bool CMeasureTempCore::AutoCorrectTempFar_ByUser(float &fFarKf2, float &fFarB2, AutoCorrectTempPara *autocorTpara)
{
	float fKf0 = m_stMtParams->fFarKf2;
	float fB0 = m_stMtParams->fFarB2;
	int nBlackBodyNumber = autocorTpara->nBlackBodyNumber;
	float tempErrorCorrect[10] = { 0 };
	float T1 = 0;
	short Y16_1 = 0;
	float T1_measure = 0;
	float fTemperatureDifferRatio = 1;//将不同温度段，温度误差归一化的参数
	float deltaT0 = 0;//初始化各个温度点的误差
	float deltaT_min = 65535;
	float deltaT = 0;//校温后各个温度点的误差

					 //计算初始误差
	for (int m = 0; m < nBlackBodyNumber; m++)
	{
		T1 = autocorTpara->TBlackBody[m];
		Y16_1 = autocorTpara->Y16[m];
		GetTempByY16(T1_measure, Y16_1);
		if (m_stMtParams->mtType == 0)
		{
			deltaT0 = deltaT0 + (T1_measure - T1)*(T1_measure - T1);
		}
		else
		{
			if (T1 <= 0)
			{
				fTemperatureDifferRatio = 0.5f;//<0℃  2/4
			}
			else if (T1 <= 100)
			{
				fTemperatureDifferRatio = 1.0f;//0-100℃时 2/2
			}
			else if (T1 > 100)
			{
				fTemperatureDifferRatio = 1.0f / (T1*0.01f);//>100℃时 2/(T*2%)
			}
			deltaT0 = deltaT0 + (T1_measure - T1)*(T1_measure - T1)*fTemperatureDifferRatio*fTemperatureDifferRatio;
		}
		autocorTpara->ErrorNoCorrect[m] = T1_measure - T1;//返回未校温时误差
	}

	//计算校温后误差，KF范围为7000~13000   B范围为-1000~1000
	for (int i = 0; i < 61; i = i + 1)
	{
		for (int j = 0; j < 201; j = j + 1)
		{
			deltaT = 0;//不同kf、b，总误差需要归零

			if ((nImgWidth == 256) && (m_stMtParams->mtType == 0))       //256人体测温 Kf不变，只调整B；
			{
				m_stMtParams->fFarKf2 = 10000.0f;   //KF
				m_stMtParams->fFarB2 = (10.0f - float(j * 0.1f)) * 100.0f;  //B
			}
			else if ((m_stMtParams->m_fieldType == 3) && (m_stMtParams->mtType == 0))
			{
				m_stMtParams->fFarKf2 = (0.9f + float(0.01 * (i / 3))) * 10000.0f;    //KF
				m_stMtParams->fFarB2 = (3.3f - float((j / 3) * 0.1f)) * 100.0f; //B
			}
			else
			{
				m_stMtParams->fFarKf2 = (0.7f + float(0.01 * i)) * 10000.0f;    //KF
				m_stMtParams->fFarB2 = (10.0f - float(j * 0.1f)) * 100.0f; //B
			}

			for (int m = 0; m < nBlackBodyNumber; m++)
			{
				T1 = autocorTpara->TBlackBody[m];
				Y16_1 = autocorTpara->Y16[m];
				GetTempByY16(T1_measure, Y16_1);/*****************************************************************************************/

				if (m_stMtParams->mtType == 0)
				{
					deltaT = deltaT + (T1_measure - T1)*(T1_measure - T1);
				}
				else
				{
					if (T1 <= 100.0f)
					{
						fTemperatureDifferRatio = 1.01f;
					}
					else if (T1 > 100.0f)
					{
						fTemperatureDifferRatio = 2.0f / (T1*0.02f);
					}
					deltaT = deltaT + (T1_measure - T1)*(T1_measure - T1) * fTemperatureDifferRatio * fTemperatureDifferRatio;
				}
				tempErrorCorrect[m] = T1_measure - T1;//返回校温时的误差			
			}

			//printf("36:%f   KF:%f b:%f  deltaT:%f\n", T1_measure, m_stMtParams->fNearKf, m_stMtParams->fNearB, deltaT);

			if (deltaT < deltaT_min)
			{
				deltaT_min = deltaT;

				fFarKf2 = m_stMtParams->fFarKf2;
				fFarB2 = m_stMtParams->fFarB2;

				for (int m = 0; m < nBlackBodyNumber; m++)
				{
					autocorTpara->ErrorCorrect[m] = tempErrorCorrect[m];//总的误差减小时，返回校温时的误差
				}
			}
		}
	}

	//返回参数
	if (deltaT_min < deltaT0)
		return true;
	else
	{
		//校温失败，返回原始传进来的fKf0、fB0
		fFarKf2 = fKf0;
		fFarB2 = fB0;
		for (int m = 0; m < nBlackBodyNumber; m++)
		{
			autocorTpara->ErrorCorrect[m] = autocorTpara->ErrorNoCorrect[m];//用于返回的未校温时误差
		}
		return false;
	}
}

/*函数功能：商汤客户二次校温
输入参数：
AutoCorrectTempPara *autocorTpara 自动校温所需要的黑体等信息
返回参数：KF和B
by 03225px
*/
/*bool CMeasureTempCore::AutoCorrectTempPRO(float &fPro_Kf, float &fPro_B, AutoCorrectTempPara *autocorTpara)
{
float T1 = autocorTpara->T_PRO;

short Y16_1 = autocorTpara->Y16_PRO;

//初始KF和B
float fPro_Kf0 = m_stMtParams->fPro_Kf;
float fPro_B0 = m_stMtParams->fPro_B;

unsigned char ucCurveNumber = 2 * m_stMtParams->ucDistanceNum;
float T1_measure;
//Y16_1测温准确性
GetTempByY16(T1_measure, Y16_1);

float deltaT0 = (T1_measure - T1) * (T1_measure - T1) ;

//用于返回的未校温时误差
autocorTpara->ErrorNoCorrect_T_PRO = T1_measure - T1;

float deltaT_min = 65535, deltaT;
//for (int i = 0; i < 61; i++)
for (int j = 0; j < 201; j++)
{
m_stMtParams->fPro_Kf = 1.0 * 10000;   //KF
m_stMtParams->fPro_B = (10 - j * 0.1) * 100;  //B
//Y16_1测温准确性
GetTempByY16(T1_measure, Y16_1);
//Y16_2测温准确性

deltaT = (T1_measure - T1) * (T1_measure - T1);
if (deltaT < deltaT_min)
{
deltaT_min = deltaT;
fPro_Kf = m_stMtParams->fPro_Kf;
fPro_B = m_stMtParams->fPro_B;

//用于返回的校温后误差
autocorTpara->ErrorCorrect_T_PRO = T1_measure - T1;
}
}
if (deltaT_min < deltaT0)
return true;
else
{
fPro_Kf = fPro_Kf0;
fPro_B = fPro_B0;

//相当于没校温
autocorTpara->ErrorCorrect_T_PRO = autocorTpara->ErrorNoCorrect_T_PRO;
return false;
}
}*/




short CMeasureTempCore::DeEmissCor(short nTEmiss, short nY16Reflect, int nEmiss)
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
			fY16Emiss = float(nY16Emiss + ((nTEmiss - nEmissCurve[nY16Emiss]) * 1.0 / (nEmissCurve[nY16Emiss + 1] - nEmissCurve[nY16Emiss])));
		else if (nDeltaT < 0)
			fY16Emiss = float(nY16Emiss - ((nEmissCurve[nY16Emiss] - nTEmiss) * 1.0 / (nEmissCurve[nY16Emiss] - nEmissCurve[nY16Emiss - 1])));
		else
			fY16Emiss = float(nY16Emiss);
	}
	else
		fY16Emiss = float(nY16Emiss);

	//发射率修正
	fNoEmissIndex = (fY16Emiss * nEmiss + (100 - nEmiss) * nY16Reflect) / 100;
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


float CMeasureTempCore::EmissionReverseCorrection(float temp ,float fEmiss)
{
	short nTNoEmiss = short(temp * 10);
	int ReflectT = int(m_stMtParams->fReflectTemp * 10);// 
	int nEmiss = int(fEmiss * 100);//
	short nY16Reflect = GetY16FromT(ReflectT);//反射温度发生了变化，才再次调用
	short DeTEmiss = DeEmissCor(nTNoEmiss, nY16Reflect, nEmiss);
	return DeTEmiss / 10.0f;
}

float CMeasureTempCore::DistanceReverseCorrection(float iTemperature , int param)
{
	if (m_stMtParams->mtType == 0 && m_stMtParams->m_fieldType == 4)
	{
		iTemperature = iTemperature - m_stMtParams->fCoefA1 * (m_stMtParams->fDistance - m_stMtParams->fCoefA2);
		return iTemperature;
	}
	float a, b, c;
	if (param == 0)
	{
		a = m_stMtParams->fCoefA1 * m_stMtParams->fDistance + m_stMtParams->fCoefA2;
		b = m_stMtParams->fCoefB1 * m_stMtParams->fDistance + m_stMtParams->fCoefB2;
		c = m_stMtParams->fCoefC1 * m_stMtParams->fDistance + m_stMtParams->fCoefC2;
	}
	else
	{	
			a = m_stMtParams->fCoefA1 * m_stMtParams->fDistance * m_stMtParams->fDistance + m_stMtParams->fCoefA2 * m_stMtParams->fDistance + m_stMtParams->fCoefA3;
			b = m_stMtParams->fCoefB1 * m_stMtParams->fDistance * m_stMtParams->fDistance + m_stMtParams->fCoefB2 * m_stMtParams->fDistance + m_stMtParams->fCoefB3;
			c = m_stMtParams->fCoefC1 * m_stMtParams->fDistance * m_stMtParams->fDistance + m_stMtParams->fCoefC2 * m_stMtParams->fDistance + m_stMtParams->fCoefC3;
	}

	float y = 0;

	if (m_stMtParams->fDistance == m_stMtParams->fCorrectDistance)
	{
		return iTemperature;
	}

	if (a == 0)
		if (b == 0)
		{
			return iTemperature;
		}
		else
		{
			return (iTemperature - c) / b;
		}
	else
	{
		y = float((pow(b * b + 4 * a * iTemperature - 4 * a * c, (float)0.5) - b) / a / 2);
	}
	return y;
}


/*  20200402
接口功能：对前端输出的焦温数据做异常滤波，提高测温稳定性
1. 将之前实时获取焦温改成1s获取一次焦温，并调用接口函数smoothFocusTemp得到smoothFocusT
2. 程序一旦启动，此接口每秒钟都会调用一次
3. 平滑后的焦温smoothFocusT用于测温
4. 若更换设备则重置全局变量g_fFocusTemp[g_nlen]，g_nFocusCnt，smoothFocusT
*/
float CMeasureTempCore::smoothFocusTemp(float jpmTemp)
{
	int i, j;
	float temp;
	float fFocusTemp[G_NLEN] = { 0 };
	if (g_nFocusCnt <= (G_NLEN - 1))
	{
		g_fFocusTemp[g_nFocusCnt] = jpmTemp;
		smoothFocusT = jpmTemp;
	}
	else
	{
		memcpy(g_fFocusTemp, g_fFocusTemp + 1, (G_NLEN - 1) * sizeof(float));  //灏??8涓???剁???涓?		
		g_fFocusTemp[(G_NLEN - 1)] = jpmTemp;

		memcpy(fFocusTemp, g_fFocusTemp, G_NLEN * sizeof(float));  //瀵瑰??ㄦ????搴?
																   //瀵瑰??ㄧ?娓╄?琛??搴?		
		for (i = 0; i < G_NLEN - 1; ++i)
		{
			for (j = i + 1; j < G_NLEN; ++j)
			{
				if (fFocusTemp[i] > fFocusTemp[j])
				{
					temp = fFocusTemp[i];
					fFocusTemp[i] = fFocusTemp[j];
					fFocusTemp[j] = temp;
				}
			}
		}
		smoothFocusT = fFocusTemp[(G_NLEN - 1) / 2];
	}
	g_nFocusCnt++;
	if (m_stMtParams->bLogPrint)
	{
		m_logger->output(LOG_DEBUG, "SFT %f %f", jpmTemp, smoothFocusT);
	}
	return smoothFocusT;
}

//更换设备后需要重置参数
void CMeasureTempCore::ResetFocusTempState()
{
	g_nFocusCnt = 0;
	smoothFocusT = 0;
	int i = 0;
	for (; i < G_NLEN; i++)
	{
		g_fFocusTemp[i] = 0;
	}
}

