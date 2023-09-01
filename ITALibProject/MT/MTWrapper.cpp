/*************************************************************
Copyright (C), 2021--2022, Wuhan Guide Sensmart Tech Co., Ltd
File name   : MTWrapper.cpp
Author      : Guide Sensmart RD. wangyan
Version     : 1.0
Date        : 2021/11/9
Description : MT TIMO Wrapper.
*************************************************************/
#include "MTWrapper.h"
#include <string.h>
#include "MeasureTempCore.h"
#include "../Module/GuideLog.h"
#include "PolygonAnalysiser.h"

MTWrapper::MTWrapper(ITA_RANGE range, int w, int h, ITA_PRODUCT_TYPE type)
	: m_analysiser(new PolygonAnalysiser(w, h))
{
	m_w = w;
	m_h = h;
	m_y16Len = w * h * 2;
	m_core = NULL;
	memset(&m_params, 0, sizeof(MtParams));
	m_params.bEnvironmentCorrection = 0;
	m_params.bLensCorrection = 1;
	m_params.bShutterCorrection = 0;
	m_params.bDistanceCompensate = 1;
	m_params.bEmissCorrection = 1;
	m_params.bTransCorrection = 0;
	m_params.bHumidityCorrection = 0;
	m_params.bAtmosphereCorrection = 0;
	m_params.bLogPrint = 0;
	m_params.fNearKf = 10000;
	m_params.fNearB = 0;
	m_params.fFarKf = 10000;
	m_params.fFarB = 0;
	m_params.fNearKf2 = 10000;
	m_params.fNearB2 = 0;
	m_params.fFarKf2 = 10000;
	m_params.fFarB2 = 0;
	if (256 == w && 192 == h)
	{
		//朗驰自动校温有时失败，低于设定的条件值。算法组建议稍微放宽条件。
		m_params.fFarB2 = 600;
	}
	m_params.fHighShutterCorrCoff = 0;
	m_params.fLowShutterCorrCoff = 0;
	m_params.fShutterCorrCoff = 0.0f;
	/*m_params.fHighLensCorrK = 500;
	m_params.fLowLensCorrK = 500;*/

	m_params.fDistance = 1.5;	//用户设置
	m_params.fEmiss = (float)0.95;
	m_params.fTransmit = 1;
	m_params.fHumidity = (float)0.6;
	m_params.fReflectTemp = 23;
	m_params.fEnvironmentTemp = 25;
	m_params.fWindowTemperature = 23.0f;
	m_params.fAtmosphereTemperature = 23.0f;
	m_params.fAtmosphereTransmittance = 1.0f;
	m_isUserEnviron = false;

	m_lensTempWhenShutter = 0;
	m_shutterTempWhenShutter = 0;
	m_shutterClosed = 0;
	m_x16WhenShutter = 0;
	m_params.mtType = range;	//由用户设置。测温类型，0:人体测温   1：工业测温常温段    2：工业测温高温段
	m_params.mtDisType = 0;
	/*距离补偿系数，先设定一组默认数据。加载配置文件时更新成项目使用的值。*/
	if (ITA_HUMAN_BODY == range)
	{
		m_params.fCoefA1 = (float)0.002647719980613;
		m_params.fCoefA2 = (float)-0.003469829249016;
		m_params.fCoefB1 = (float)-0.135885628369434;
		m_params.fCoefB2 = (float)1.168639396560524;
		m_params.fCoefC1 = (float)2.048427662684918;
		m_params.fCoefC2 = (float)-2.530146001650383;
		m_params.fCoefA3 = (float)3.246970343144496e-04;
		m_params.fCoefB3 = (float)0.868820530321706;
		m_params.fCoefC3 = (float)4.032032909613879;
		m_params.bEmissCorrection = 0;
		m_params.fEmiss = (float)0.98;
		m_params.fHighLensCorrK = 500;
		m_params.fLowLensCorrK = 500;
		m_params.fLensCorrK = 500;
	}
	else if (ITA_INDUSTRY_LOW == range)
	{
		m_params.fCoefA1 = (float)-1.153993367594985e-04;
		m_params.fCoefA2 = (float)1.896140188520671e-04;
		m_params.fCoefB1 = (float)0.053514786526522;
		m_params.fCoefB2 = (float)0.914778373691867;
		m_params.fCoefC1 = (float)-1.920382323272305;
		m_params.fCoefC2 = (float)3.085370548537257;
		m_params.fCoefA3 = (float)3.246970343144496e-04;
		m_params.fCoefB3 = (float)0.868820530321706;
		m_params.fCoefC3 = (float)4.032032909613879;
		m_params.bEmissCorrection = 1;
		m_params.fEmiss = (float)0.95;
		m_params.fHighLensCorrK = 0;
		m_params.fLowLensCorrK = 110;
		m_params.fLensCorrK = 110;
	}
	else
	{
		m_params.fCoefA1 = (float)1.219498322836064e-05;
		m_params.fCoefA2 = (float)-2.428526357615574e-05;
		m_params.fCoefB1 = (float)0.030329551458164;
		m_params.fCoefB2 = (float)0.955956218237682;
		m_params.fCoefC1 = (float)-0.715050251156294;
		m_params.fCoefC2 = (float)0.523718991720748;
		m_params.fCoefA3 = (float)-1.455628053613217e-05;
		m_params.fCoefB3 = (float)0.932616605341318;
		m_params.fCoefC3 = (float)0.449407757207951;
		m_params.bEmissCorrection = 1;
		m_params.fEmiss = (float)0.95;
		m_params.fHighLensCorrK = 0;
		m_params.fLowLensCorrK = 110;
		m_params.fLensCorrK = 110;
	}
	//memset(&m_defaultConf, 0, sizeof(ITAConfig));
	memset(m_centralArray, 0, sizeof(short) * 8); //空域滤波中心点Y16数组
	m_centralCount = 0; //帧计数
	//默认配置成模组产品，读配置文件之后如果是非模组产品再配置手持和IPT测温类。
	m_product = type;
}

MTWrapper::~MTWrapper()
{
	if (m_core)
	{
		delete m_core;
		m_core = NULL;
	}
	/*if (m_defaultConf.mtConfig)
	{
		porting_free_mem(m_defaultConf.mtConfig);
		m_defaultConf.mtConfig = NULL;
	}*/
}

int MTWrapper::loadData(PackageHeader *ph, short* fpaTempArray, short* allCurvesArray)
{
	//数据包中的距离补偿系数值是空的，从配置文件中读取。
	m_params.ucGearMark = ph->ucGearMark;	//0：常温档  1：高温档
	m_params.ucFpaTempNum = ph->ucFocusNumber;
	m_params.ucDistanceNum = ph->ucDistanceNumber;
	m_params.m_fieldType = ph->lensType;
	m_params.m_focusType = ph->focusType;
	m_logger->output(LOG_INFO, "fieldType=%d,focusType=%d,gearMark=%d", m_params.m_fieldType, m_params.m_focusType, m_params.ucGearMark);
	for (int i = 0; i < ph->ucFocusNumber; i++)
	{
		m_params.gFpaTempArray[i] = (float)(fpaTempArray[i]) / 100;
		m_logger->output(LOG_INFO,"gFpaTempArray[%d] = %f ", i, m_params.gFpaTempArray[i]);
	}
	for (int j = 0; j < ph->ucDistanceNumber; j++)
	{
		m_params.gDistanceArray[j] = (float)(ph->usDistanceArray[j]) / 10;
		m_logger->output(LOG_INFO,"gDistanceArray[%d] = %f ", j, m_params.gDistanceArray[j]);
	}
	m_params.fMinTemp = (float)ph->sTMin;
	m_params.fMaxTemp = (float)ph->sTMax;
	m_params.usCurveTempNumber = ph->usCurveTemperatureNumber;
	/*if (m_core)
	{
		delete m_core;
		m_core = NULL;
	}*///不要销毁测温实例，避免多线程操作死机。
	if (!m_core)
	{
		m_core = new CMeasureTempCore(&m_params, m_w, m_h, allCurvesArray, ph->usCurveTemperatureNumber * ph->ucDistanceNumber * 2);
		m_core->setLogger(m_logger);
	}
	else
	{
		m_core->reload(&m_params, allCurvesArray, ph->usCurveTemperatureNumber * ph->ucDistanceNumber * 2);
	}
	return 0;
}

ITA_RESULT MTWrapper::loadConfig(ITAConfig * config, ITA_RANGE range, ITA_FIELD_ANGLE lensType)
{
	ITAMTConfig *pConf;
	int i;
	if (!config->mtConfig || config->mtConfigCount <= 0)
	{
		m_logger->output(LOG_WARN, "MTWrapper::loadConfig  ret = %d ", ITA_NO_MT_CONF);
		return ITA_NO_MT_CONF;
	}
	//从配置文件中找到对应的一组参数，设置新的值。
	for (i = 0; i < config->mtConfigCount; i++)
	{
		pConf = config->mtConfig + i;
		if (pConf->lensType == lensType && pConf->mtType == range)
		{
			//lensType和mtType是判断条件，不用赋值。
			//m_params.m_fieldType = pConf->lensType;				//ITA_FIELD_ANGLE 视场角类型 0:56°；1:25°；2:120°；3:50°；4:90°；5:33°。
			//pConf->mtDistanceRangeN;		//测温距离范围下限
			//pConf->mtDistanceRangeF;		//测温距离范围上限
			//m_params.mtType = pConf->mtType;					//测温模式 0：人体；1：工业低温；2：工业高温。
			if (pConf->correctDistanceS == 1)
				m_params.fCorrectDistance = pConf->correctDistance;		//自动校温距离
			if (pConf->distanceS == 1)
				m_params.fDistance = pConf->distance;				//目标距离
			if (pConf->lowLensCorrKS == 1)
				m_params.fLowLensCorrK = pConf->lowLensCorrK;			//测温常温档镜筒温漂系数
			if (pConf->highLensCorrKS == 1)
				m_params.fHighLensCorrK = pConf->highLensCorrK;		//测温高温档镜筒温漂系数
			if (pConf->lowShutterCorrCoffS == 1)
				m_params.fLowShutterCorrCoff = pConf->lowShutterCorrCoff;	//测温常温档快门温漂系数
			if (pConf->highShutterCorrCoffS == 1)
				m_params.fHighShutterCorrCoff = pConf->highShutterCorrCoff;	//测温高温档快门温漂系数
			if (pConf->mtDisTypeS == 1)
				m_params.mtDisType = pConf->mtDisType;			//测温距离修正类型，0：6参数; 1:9参数
			if (pConf->coefA1S == 1)
				m_params.fCoefA1 = pConf->coefA1;				//测温距离修正参数
			if (pConf->coefA2S == 1)
				m_params.fCoefA2 = pConf->coefA2;				//测温距离修正参数
			if (pConf->coefB1S == 1)
				m_params.fCoefB1 = pConf->coefB1;				//测温距离修正参数
			if (pConf->coefB2S == 1)
				m_params.fCoefB2 = pConf->coefB2;				//测温距离修正参数
			if (pConf->coefC1S == 1)
				m_params.fCoefC1 = pConf->coefC1;				//测温距离修正参数
			if (pConf->coefC2S == 1)
				m_params.fCoefC2 = pConf->coefC2;				//测温距离修正参数
			if (pConf->coefA3S == 1)
				m_params.fCoefA3 = pConf->coefA3;				//测温距离修正参数
			if (pConf->coefB3S == 1)
				m_params.fCoefB3 = pConf->coefB3;				//测温距离修正参数
			if (pConf->coefC3S == 1)
				m_params.fCoefC3 = pConf->coefC3;				//测温距离修正参数
			if (m_core)
			{
				m_core->RefreshCorrK();
			}
			else {
				m_logger->output(LOG_INFO, "MTWrapper::loadConfig ITA_MT_NOT_READY");
			}
			break;
		}
	}
	m_logger->output(LOG_INFO, "MTWrapper::loadConfig  %f %f %f %f %f %f %d %f %f %f %f %f %f %f %f %f %d %d", 
		m_params.fCorrectDistance, m_params.fDistance, m_params.fLowLensCorrK, m_params.fHighLensCorrK, m_params.fLowShutterCorrCoff, m_params.fHighShutterCorrCoff, m_params.mtDisType,
		m_params.fCoefA1, m_params.fCoefA2, m_params.fCoefB1, m_params.fCoefB2, m_params.fCoefC1, m_params.fCoefC2,m_params.fCoefA3,m_params.fCoefB3,m_params.fCoefC3, range, lensType);
	if (i >= config->mtConfigCount)
	{
		m_logger->output(LOG_WARN, "MTWrapper::loadConfig  ret = %d ", ITA_NO_MT_CONF);
		return ITA_NO_MT_CONF;
	}
	else
		return ITA_OK;
}

ITA_RESULT MTWrapper::refreshShutterStatus(short y16CenterValue,ITA_MODE mode)
{
	//第一帧时上次快门温度为0，赋值成当前快门温度。
	if (!m_shutterTempWhenShutter)
	{
		m_shutterTempWhenShutter = m_params.fRealTimeShutterTemp;
		m_lensTempWhenShutter = m_params.fRealTimeLensTemp;
		//如果冷机开机，那么第一帧时开机快门温赋实时温传值。如果热机，则读取上次关机时存储的开机快门温。
		if (!m_params.fOrinalShutterTemp)
		{
			m_params.fOrinalShutterTemp = m_params.fRealTimeShutterTemp;
			m_logger->output(LOG_INFO, "Cold start.%f", m_params.fOrinalShutterTemp);
		}
		else
		{
			m_logger->output(LOG_INFO, "Hot start.%f %f", m_params.fOrinalShutterTemp, m_params.fRealTimeShutterTemp);
		}
	}
	if (m_params.fOrinalShutterTemp < 1 || m_params.fOrinalShutterTemp > 100)
	{
		//开机首次获取的温传不一定是有效的，一段时间后温传数据才正确。此时开机快门温要更新成有效的值。
		m_params.fOrinalShutterTemp = m_params.fRealTimeShutterTemp;
	}
	//打快门时更新上次温度。
	//本底更新标志位是保证现在快门已经闭合的最安全的标志，显示控制标志位是告诉你现在准备要打快门了。这是裸模组产品。
	//标准协议中bNucShutterFlag表示nuc快门状态，bOrdinaryShutter表示普通快门状态。二者都要更新温传。
	if (m_params.bNucShutterFlag || m_params.bOrdinaryShutter)
	{
		if (m_params.bLogPrint)
			m_logger->output(LOG_DEBUG, "shutter now. %d %d %d", m_params.bOrdinaryShutter, m_params.bNucShutterFlag, m_shutterClosed);
		//快门刚闭合时记录温度，采样中心点Y16
		if (!m_shutterClosed)
		{
			m_shutterTempWhenShutter = m_params.fRealTimeShutterTemp;
			m_lensTempWhenShutter = m_params.fRealTimeLensTemp;
			/*if(x16Array)
				m_x16WhenShutter = *(x16Array + m_w * m_h / 2 - m_w / 2);*/
			m_x16WhenShutter = y16CenterValue;
		}
		m_shutterClosed = 1;
	}
	else
	{
		m_shutterClosed = 0;
	}
	if (m_params.fCurrentShutterTemp > 1 && m_params.fCurrentShutterTemp < 100 && mode == ITA_Y16) {
		if (m_params.bLogPrint)
		{
			m_logger->output(LOG_DEBUG, "refreshShutterStatus1 %.2f %.2f %.2f %.2f %d", m_params.fCurrentShutterTemp, m_params.fCurrentLensTemp, m_params.fRealTimeShutterTemp, m_params.fRealTimeLensTemp, m_params.bOrdinaryShutter);
		}
	}
	else {
		m_params.fCurrentShutterTemp = m_shutterTempWhenShutter;
		m_params.fCurrentLensTemp = m_lensTempWhenShutter;
		if (m_params.bLogPrint)
		{
			m_logger->output(LOG_DEBUG, "refreshShutterStatus2 %.2f %.2f %.2f %.2f %d", m_params.fCurrentShutterTemp, m_params.fCurrentLensTemp, m_params.fRealTimeShutterTemp, m_params.fRealTimeLensTemp, m_params.bOrdinaryShutter);
		}
	}
	m_params.fLastShutterTemp = m_params.fCurrentShutterTemp;
	//调试时参考，测温用不上。
	m_params.sCurrentShutterValue = m_x16WhenShutter;
	return ITA_OK;
}

ITA_RESULT MTWrapper::calcFPAGear(int *gear)
{
	ITA_RESULT result = ITA_OK;
	if (m_core)
		*gear = m_core->GetFpaGear();
	else
	{
		result = ITA_MT_NOT_READY;
		if (m_params.bLogPrint)
			m_logger->output(LOG_WARN, "calcFPAGear error. ITA_MT_NOT_READY ret=%d", result);
	}
	return result;
}

ITA_RESULT MTWrapper::changeRange(ITA_RANGE range, ITA_FIELD_ANGLE lensType)
{
	ITA_RESULT result = ITA_OK;
	m_params.mtType = range;	//由用户设置。测温类型，0:人体测温   1：工业测温常温段    2：工业测温高温段
	m_params.m_fieldType = lensType;
	m_logger->output(LOG_INFO, "changeRange mtType=%d fieldType=%d", range, lensType);
	/*距离补偿系数，先设定一组默认数据。加载配置文件时更新成项目使用的值。*/
	/*用户切换范围时fEmiss等参数保留之前值*/
	/*if (ITA_HUMAN_BODY == range)
	{
		m_params.fCoefA1 = (float)0.002647719980613;
		m_params.fCoefA2 = (float)-0.003469829249016;
		m_params.fCoefB1 = (float)-0.135885628369434;
		m_params.fCoefB2 = (float)1.168639396560524;
		m_params.fCoefC1 = (float)2.048427662684918;
		m_params.fCoefC2 = (float)-2.530146001650383;
		m_params.bEmissCorrection = 0;
		m_params.fEmiss = (float)0.98;
		m_params.fHighLensCorrK = 500;
		m_params.fLowLensCorrK = 500;
	}
	else if (ITA_INDUSTRY_LOW == range)
	{
		m_params.fCoefA1 = (float)-1.153993367594985e-04;
		m_params.fCoefA2 = (float)1.896140188520671e-04;
		m_params.fCoefB1 = (float)0.053514786526522;
		m_params.fCoefB2 = (float)0.914778373691867;
		m_params.fCoefC1 = (float)-1.920382323272305;
		m_params.fCoefC2 = (float)3.085370548537257;
		m_params.bEmissCorrection = 1;
		m_params.fEmiss = (float)0.95;
		m_params.fHighLensCorrK = 0;
		m_params.fLowLensCorrK = 110;
	}
	else
	{
		m_params.fCoefA1 = (float)1.219498322836064e-05;
		m_params.fCoefA2 = (float)-2.428526357615574e-05;
		m_params.fCoefB1 = (float)0.030329551458164;
		m_params.fCoefB2 = (float)0.955956218237682;
		m_params.fCoefC1 = (float)-0.715050251156294;
		m_params.fCoefC2 = (float)0.523718991720748;
		m_params.bEmissCorrection = 1;
		m_params.fEmiss = (float)0.95;
		m_params.fHighLensCorrK = 0;
		m_params.fLowLensCorrK = 110;
	}*/
	/*切换测温范围时不用析构再构造m_core，因为第一次构造时已传入引用的地址。*/
	if (m_core)
		m_core->ChangeRange();
	else
		result = ITA_MT_NOT_READY;
	return result;
}

ITA_RESULT MTWrapper::calcTempByY16(short y16, float * temp)
{
	float fTemp = 0;
	if (!m_core)
		return ITA_MT_NOT_READY;
	if (!temp)
		return ITA_NULL_PTR_ERR;
	m_core->GetTempByY16(fTemp, y16);
	*temp = fTemp;
	/*if (m_params.bLogPrint)
	{
		m_logger->output(LOG_INFO, "\ttemp=\t%f\t, y16=\t%d\t",	fTemp, y16);
	}*/
	return ITA_OK;
}

ITA_RESULT MTWrapper::calcBodyTemp(float surfaceTemp, float envirTemp, float * bodyTemp)
{
	float fTemp = 0;
	if (!m_core)
		return ITA_MT_NOT_READY;
	if (!bodyTemp)
		return ITA_NULL_PTR_ERR;
	m_core->SurfaceTempToBody(fTemp, surfaceTemp, envirTemp);
	*bodyTemp = fTemp;
	if (m_params.bLogPrint)
	{
		m_logger->output(LOG_INFO, "\ttemp=\t%f\t, bodyTemp=\t%f\t envirTemp=\t%f\t", surfaceTemp, fTemp, envirTemp);
	}
	return ITA_OK;
}

ITA_RESULT MTWrapper::calcY16ByTemp(float temp, short * y16)
{
	short Y16 = 0;
	if (!m_core)
		return ITA_MT_NOT_READY;
	if (!y16)
		return ITA_NULL_PTR_ERR;
	m_core->CalY16ByTemp(Y16, temp);
	*y16 = Y16;
	return ITA_OK;
}

ITA_RESULT MTWrapper::calcY16MatrixByTemp(float * tempMatrix, int w, int h, short * y16Matrix, int matrixSize)
{

	if (!m_core)
		return ITA_MT_NOT_READY;
	if (!y16Matrix || !tempMatrix)
		return ITA_NULL_PTR_ERR;
	if (matrixSize <= 0)
	{
		return ITA_ARG_OUT_OF_RANGE;
	}
	return (ITA_RESULT)m_core->CalY16MatrixByTempMatrix(tempMatrix, y16Matrix, h, w);
}

//ITA_RESULT MTWrapper::calcTempMatrix(short* y16Array, int y16W, int y16H, int x, int y, int w, int h, float * tempMatrix)
//{
//	if (!m_core)
//		return ITA_MT_NOT_READY;
//	m_core->GetTempMatrixByY16(tempMatrix, y16Array, y16W, y16H, x, y, w, h, m_params.fDistance);
//	return ITA_OK;
//}

ITA_RESULT MTWrapper::calcTempMatrix(short * y16Array, int y16W, int y16H, float distance, int x, int y, int w, int h, ITA_MATRIX_TYPE type, float * tempMatrix)
{
	if (!m_core)
		return ITA_MT_NOT_READY;
	if (!y16Array || !tempMatrix)
		return ITA_NULL_PTR_ERR;
	if (distance <= 0 || distance > 10000)
	{
		return ITA_ARG_OUT_OF_RANGE;
	}
	float myDis = m_params.fDistance;
	/*检查区域是否有效或越界*/
	if (y16W <= 0 || y16H <= 0 || x < 0 || y < 0 || w <= 0 || h <= 0 || x + w > y16W || y + h > y16H)
	{
		m_logger->output(LOG_ERROR, "calcTempMatrix %d %d %d %d ret=%d", x, y, w, h, ITA_ARG_OUT_OF_RANGE);
		return ITA_ARG_OUT_OF_RANGE;
	}
	else
	{
		if(ITA_FAST_MATRIX == type)
			m_core->GetTempMatrixByY16(tempMatrix, y16Array, y16W, y16H, x, y, w, h, distance);
		else
			m_core->calcMatrixFromCurveOld(tempMatrix, y16Array, y16W, y16H, x, y, w, h, distance);
		/*距离恢复*/
		m_params.fDistance = myDis;
		return ITA_OK;
	}
}

ITA_RESULT MTWrapper::parametersControl(ITA_MC_TYPE type, void * param)
{
	float fParam;
	ITA_SWITCH flag;
	ITA_RESULT ret = ITA_OK;
	if (!param)
	{
		ret = ITA_NULL_PTR_ERR;
		m_logger->output(LOG_ERROR, "MTWrapper::parametersControl ret=%d", ret);
		return ret;
	}
	if (m_params.bLogPrint)
		m_logger->output(LOG_INFO, "measureControl op=%d", type);
	switch (type)
	{
	case ITA_SET_DISTANCE:			//距离0.5~2.5米。可以不设置，默认1.5米。参数类型：浮点型。
		fParam = *(float *)param;
		if (fParam <= 0.0 || fParam > 10000.0)
		{
			return ITA_ARG_OUT_OF_RANGE;
		}
		m_params.fDistance = fParam;
		break;
	case ITA_SET_EMISS:				//设置发射率(工业专用)0.01-1.0，默认0.95. 参数类型：浮点型。
		fParam = *(float *)param;
		//0.01-1.0
		if (fParam <= 0.0 || fParam > 1.0)
		{
			return ITA_ARG_OUT_OF_RANGE;
		}
		m_params.fEmiss = fParam;
		break;
	case ITA_SET_HUMI:				//设置湿度(0.01-1.0,默认0.6). 参数类型：浮点型。
		fParam = *(float *)param;
		//0.01-1.0
		if (fParam <= 0.0 || fParam > 1.0)
		{
			return ITA_ARG_OUT_OF_RANGE;
		}
		m_params.fHumidity = fParam;
		break;
	case ITA_SET_TRANSMIT:			//透过率(工业专用，0.01-1, default 1)。参数类型：浮点型。
		fParam = *(float *)param;
		//0.01-1.0
		if (fParam <= 0.0 || fParam > 1.0)
		{
			return ITA_ARG_OUT_OF_RANGE;
		}
		m_params.fTransmit = fParam;
		break;
	case ITA_REFLECT_TEMP:			//反射温度(工业专用)。参数类型：浮点型。
		fParam = *(float *)param;
		m_params.fReflectTemp = fParam;
		break;
	case ITA_SET_ENVIRON:			//设置环境温度。在用户不设置的情况下，内部自动计算环温。如果用户设置，则使用设置的值。参数类型：浮点型。
		fParam = *(float *)param;
		if (fParam < -10000 || fParam > 10000)
		{
			return ITA_ARG_OUT_OF_RANGE;
		}
		m_params.fEnvironmentTemp = fParam;
		m_isUserEnviron = true;
		break;
	case ITA_GET_ENVIRON:			//获取环温。
		if (m_isUserEnviron)
		{
			*(float *)param = m_params.fEnvironmentTemp;
		}
		else
		{
			if (!m_core)
			{
				return ITA_MT_NOT_READY;
			}
			*(float *)param = m_core->GetEnvirTemp(fParam);
		}
		break;
	case ITA_GET_FPA_TEMP:			//获取焦温。
		if(param)
			*(float *)param = m_params.fRealTimeFpaTemp;
		else
		{
			ret = ITA_NULL_PTR_ERR;
			m_logger->output(LOG_ERROR, "ITA_GET_FPA_TEMP ret=%d", ret);
		}
		break;
	case ITA_HUMI_CORR:				//湿度校正开关。默认关闭。参数类型：ITA_SWITCH。
		flag = *(ITA_SWITCH *)param;
		if (flag > ITA_ENABLE || flag < ITA_DISABLE)
			ret = ITA_ARG_OUT_OF_RANGE;
		else
			m_params.bHumidityCorrection = (unsigned char)flag;
		break;
	case ITA_LENS_CORR:				//镜筒温漂校正开关。默认关闭。参数类型：ITA_SWITCH。
		flag = *(ITA_SWITCH *)param;
		if (flag > ITA_ENABLE || flag < ITA_DISABLE)
			ret = ITA_ARG_OUT_OF_RANGE;
		else
			m_params.bLensCorrection = (unsigned char)flag;
		break;
	case ITA_SHUTTER_CORR:			//快门温漂校正开关。默认关闭。参数类型：ITA_SWITCH。
		flag = *(ITA_SWITCH *)param;
		if (flag > ITA_ENABLE || flag < ITA_DISABLE)
			ret = ITA_ARG_OUT_OF_RANGE;
		else
			m_params.bShutterCorrection = (unsigned char)flag;
		break;
	case ITA_HIGH_LENS_CORR_K:		//实时高温档镜筒温漂修正系数。参数类型：浮点型。
		if (!m_core)
		{
			return ITA_MT_NOT_READY;
		}
		fParam = *(float *)param;
		m_params.fHighLensCorrK = fParam;
		m_core->RefreshCorrK();
		break;
	case ITA_OTHER_LENS_CORR_K:		//实时其他档镜筒温漂修正系数。参数类型：浮点型。
	{
		if (!m_core)
		{
			return ITA_MT_NOT_READY;
		}
		fParam = *(float*)param;
		m_params.fLensCorrK = fParam;
		m_core->RefreshCorrK();
		break;
	}
	case ITA_LOW_LENS_CORR_K:		//实时常温档镜筒温漂修正系数。参数类型：浮点型。
		if (!m_core)
		{
			return ITA_MT_NOT_READY;
		}
		fParam = *(float *)param;
		m_params.fLowLensCorrK = fParam;
		m_core->RefreshCorrK();
		break;
	case ITA_HIGH_SHUTTER_CORR_K:	//高温档快门校正系数。参数类型：浮点型。
		if (!m_core)
		{
			return ITA_MT_NOT_READY;
		}
		fParam = *(float *)param;
		m_params.fHighShutterCorrCoff = fParam;
		m_core->RefreshCorrK();
		break;
	case ITA_LOW_SHUTTER_CORR_K:	//常温档快门校正系数。参数类型：浮点型。
		if (!m_core)
		{
			return ITA_MT_NOT_READY;
		}
		fParam = *(float *)param;
		m_params.fLowShutterCorrCoff = fParam;
		m_core->RefreshCorrK();
		break;
	case ITA_OTHER_SHUTTER_CORR_K:	//其他档快门校正系数。参数类型：浮点型。
	{
		if (!m_core)
		{
			return ITA_MT_NOT_READY;
		}
		fParam = *(float*)param;
		m_params.fShutterCorrCoff = fParam;
		m_core->RefreshCorrK();
		break;
	}
	case ITA_ENVIRON_CORR:			//环温修正开关。默认关闭。参数类型：ITA_SWITCH。
		flag = *(ITA_SWITCH *)param;
		if (flag > ITA_ENABLE || flag < ITA_DISABLE)
			ret = ITA_ARG_OUT_OF_RANGE;
		else
			m_params.bEnvironmentCorrection = (unsigned char)flag;
		break;
	case ITA_DISTANCE_COMPEN:		//距离补偿开关。默认打开。参数类型：ITA_SWITCH。
		flag = *(ITA_SWITCH *)param;
		if (flag > ITA_ENABLE || flag < ITA_DISABLE)
			ret = ITA_ARG_OUT_OF_RANGE;
		else
			m_params.bDistanceCompensate = (unsigned char)flag;
		break;
	case ITA_EMISS_CORR:			//发射率校正开关。默认打开。参数类型：ITA_SWITCH。
		flag = *(ITA_SWITCH *)param;
		if (flag > ITA_ENABLE || flag < ITA_DISABLE)
			ret = ITA_ARG_OUT_OF_RANGE;
		else
			m_params.bEmissCorrection = (unsigned char)flag;
		break;
	case ITA_TRANS_CORR:			//透过率校正开关。默认关闭。参数类型：ITA_SWITCH。
		flag = *(ITA_SWITCH *)param;
		if (flag > ITA_ENABLE || flag < ITA_DISABLE)
			ret = ITA_ARG_OUT_OF_RANGE;
		else
			m_params.bTransCorrection = (unsigned char)flag;
		break;
	case ITA_ATMOSPHERE_CORR:
	{
		flag = *(ITA_SWITCH*)param;
		if (flag > ITA_ENABLE || flag < ITA_DISABLE)
		{
			ret = ITA_ARG_OUT_OF_RANGE;
		}
		else {
			m_params.bAtmosphereCorrection = (unsigned char)flag;
		}
		break;
	}
	case ITA_SET_WINDOWTEMP:
	{
		if (!m_core)
		{
			return ITA_MT_NOT_READY;
		}
		fParam = *(float*)param;
		m_params.fWindowTemperature = fParam;
		break;
	}
	case ITA_SET_ATMOSPHTEMP:
	{
		if (!m_core)
		{
			return ITA_MT_NOT_READY;
		}
		fParam = *(float*)param;
		m_params.fAtmosphereTemperature = fParam;
		break;
	}
	case ITA_SET_ATMOSPHTRSMIT:
	{
		if (!m_core)
		{
			return ITA_MT_NOT_READY;
		}
		fParam = *(float*)param;
		m_params.fAtmosphereTransmittance = fParam;
		break;
	}
	case ITA_CENTRAL_TEMPER: 		//获取滤波后的中心温。参数类型：浮点型。
	{
		int count = 0, sum = 0;
		short y16Value;
		for (int i = 0; i < 8; i++)
		{
			if (m_centralArray[i])
			{
				sum += m_centralArray[i];
				count++;
			}
		}
		if (!count)
		{
			ret = ITA_INACTIVE_CALL;
			break;
		}
		y16Value = (short)(sum / count);
		//如果计算中心温并且减去本底均值，那么m_mtBase内没有这些信息无法计算。
		//ret = calcTempByY16(y16Value, (float *)param);
		*(short *)param = y16Value;
		break;
	}
	default:
		break;
	}
	return ret;
}

IMAGE_MEASURE_STRUCT * MTWrapper::getTECParams()
{
	return 0;
}

MtParams *MTWrapper::getMtParams()
{
	return &m_params;
}

void MTWrapper::setMtParams(void * param, int paramLen)
{
	//设置测温参数
	memcpy(&m_params, param, paramLen);
}

void MTWrapper::getParamInfo(ITAParamInfo * info)
{
	info->fpaGear = m_params.n_fpaGear;
	info->distance = m_params.fDistance;
	info->emiss = m_params.fEmiss;
	info->transmit = m_params.fTransmit;
	info->reflectTemp = m_params.fReflectTemp;
	info->environCorr = (ITA_SWITCH)m_params.bEnvironmentCorrection;
	info->lensCorr = (ITA_SWITCH)m_params.bLensCorrection;
	info->distanceCompen = (ITA_SWITCH)m_params.bDistanceCompensate;
	info->emissCorr = (ITA_SWITCH)m_params.bEmissCorrection;
	info->transCorr = (ITA_SWITCH)m_params.bTransCorrection;
	info->lowLensCorrK = m_params.fLowLensCorrK;
	info->highLensCorrK = m_params.fHighLensCorrK;
	info->lensCorrK = m_params.fLensCorrK;
	info->shutterCorr = (ITA_SWITCH)m_params.bShutterCorrection;
	info->lowShutterCorrK = m_params.fLowShutterCorrCoff;
	info->highShutterCorrK = m_params.fHighShutterCorrCoff;
	info->shutterCorrK = m_params.fShutterCorrCoff;
	info->nearKFAuto = m_params.fNearKf;
	info->nearBAuto = m_params.fNearB;
	info->farKFAuto = m_params.fFarKf;
	info->farBAuto = m_params.fFarB;
	info->nearKFManual = m_params.fNearKf2;
	info->nearBManual = m_params.fNearB2;
	info->farKFManual = m_params.fFarKf2;
	info->farBManual = m_params.fFarB2;
	info->orinalShutterTemp = m_params.fOrinalShutterTemp;//开机快门温度
	info->currentShutterTemp = m_params.fCurrentShutterTemp;//当前打快门时的快门温度
	info->realTimeShutterTemp = m_params.fRealTimeShutterTemp;//实时快门温度
	info->realTimeLensTemp = m_params.fRealTimeLensTemp;//实时镜筒温度
	info->currentLensTemp = m_params.fCurrentLensTemp;//当前打快门时的镜筒温度
	info->realTimeFpaTemp = m_params.fRealTimeFpaTemp;//实时焦平面温度
	info->humidity = m_params.fHumidity;//获取湿度参数
}

float MTWrapper::smoothFocusTemp(float fpaTemp)
{
	if (!m_core)
		return 0;
	return m_core->smoothFocusTemp(fpaTemp);
}

ITA_RESULT MTWrapper::refreshFrame(short * y16Array, int w, int h)
{
	//返回的Y16数据未拉伸，有旋转。
	int x, y, sum;
	x = w / 2 - 1;
	y = h / 2 - 1;
	sum = y16Array[y*w + x];
	sum += y16Array[y*w + x - 1];
	sum += y16Array[y*w + x + 1];
	sum += y16Array[(y - 1)*w + x];
	sum += y16Array[(y - 1)*w + x - 1];
	sum += y16Array[(y - 1)*w + x + 1];
	sum += y16Array[(y + 1)*w + x];
	sum += y16Array[(y + 1)*w + x - 1];
	sum += y16Array[(y + 1)*w + x + 1];
	m_centralArray[m_centralCount % 8] = (short)(sum / 9);
	/*if (m_params.bLogPrint)
		m_logger->output(LOG_DEBUG, "[%d %d %d %d %d %d %d %d %d] %d avg=%d",
			y16Array[y*w + x],
			y16Array[y*w + x - 1],
			y16Array[y*w + x + 1],
			y16Array[(y - 1)*w + x],
			y16Array[(y - 1)*w + x - 1],
			y16Array[(y - 1)*w + x + 1],
			y16Array[(y + 1)*w + x],
			y16Array[(y + 1)*w + x - 1],
			y16Array[(y + 1)*w + x + 1],
			m_centralCount % 8,
			m_centralArray[m_centralCount % 8]);*/
	m_centralCount++;
	return ITA_OK;
}

bool MTWrapper::AutoCorrectTempNear(float * fNearKf, float * fNearB, AutoCorrectTempPara * autocorTpara)
{
	bool ret;
	float nearKf, nearB;
	if (!m_core)
		return false;
	ret = m_core->AutoCorrectTempNear(nearKf, nearB, autocorTpara);
	*fNearKf = nearKf;
	*fNearB = nearB;
	m_logger->output(LOG_INFO, "AutoCorrectTempNear result:kf=%f b=%f", nearKf, nearB);
	for (int i = 0; i < autocorTpara->nBlackBodyNumber; i++)
	{
		m_logger->output(LOG_INFO, "%d TBlackBody=%f Y16=%d ErrorNoCorrect=%f ErrorCorrect=%f", 
			i, autocorTpara->TBlackBody[i], autocorTpara->Y16[i], autocorTpara->ErrorNoCorrect[i], autocorTpara->ErrorCorrect[i]);
	}
	return ret;
}

bool MTWrapper::AutoCorrectTempFar(float * fFarKf, float * fFarB, AutoCorrectTempPara * autocorTpara)
{
	bool ret;
	float farKf, farB;
	if (!m_core)
		return false;
	ret = m_core->AutoCorrectTempFar(farKf, farB, autocorTpara);
	*fFarKf = farKf;
	*fFarB = farB;
	/*在测试自动接口时，选取的三个Y16比较奇怪，在自动校温的逻辑中能找到一组KF B让三个Y16的温度误差最小，
	所以替换了Kf和B值，但是这组KF和B值不能使三个温度值与基准的误差小于0.4所以显示校温失败。*/
	m_logger->output(LOG_INFO, "AutoCorrectTempFar result:kf=%f b=%f", farKf, farB);
	for (int i = 0; i < autocorTpara->nBlackBodyNumber; i++)
	{
		m_logger->output(LOG_INFO, "%d TBlackBody=%f Y16=%d ErrorNoCorrect=%f ErrorCorrect=%f",
			i, autocorTpara->TBlackBody[i], autocorTpara->Y16[i], autocorTpara->ErrorNoCorrect[i], autocorTpara->ErrorCorrect[i]);
	}
	return ret;
}
/*用户二次自动校温*/
bool MTWrapper::AutoCorrectTempFarByUser(float * fFarKf, float * fFarB, AutoCorrectTempPara * autocorTpara)
{
	bool ret;
	float farKf, farB;
	if (!m_core)
		return false;
	ret = m_core->AutoCorrectTempFar_ByUser(farKf, farB, autocorTpara);
	*fFarKf = farKf;
	*fFarB = farB;
	m_logger->output(LOG_INFO, "AutoCorrectTempFarByUser result:kf=%f b=%f", farKf, farB);
	for (int i = 0; i < autocorTpara->nBlackBodyNumber; i++)
	{
		m_logger->output(LOG_INFO, "%d TBlackBody=%f Y16=%d ErrorNoCorrect=%f ErrorCorrect=%f",
			i, autocorTpara->TBlackBody[i], autocorTpara->Y16[i], autocorTpara->ErrorNoCorrect[i], autocorTpara->ErrorCorrect[i]);
	}
	return ret;
}

bool MTWrapper::AutoCorrectTempNearByUser(float * fNearKf, float * fNearB, AutoCorrectTempPara * autocorTpara)
{
	bool ret;
	float nearKf, nearB;
	if (!m_core)
		return false;
	ret = m_core->AutoCorrectTempNear_ByUser(nearKf, nearB, autocorTpara);
	*fNearKf = nearKf;
	*fNearB = nearB;
	m_logger->output(LOG_INFO, "AutoCorrectTempNearByUser result:kf=%f b=%f", nearKf, nearB);
	for (int i = 0; i < autocorTpara->nBlackBodyNumber; i++)
	{
		m_logger->output(LOG_INFO, "%d TBlackBody=%f Y16=%d ErrorNoCorrect=%f ErrorCorrect=%f",
			i, autocorTpara->TBlackBody[i], autocorTpara->Y16[i], autocorTpara->ErrorNoCorrect[i], autocorTpara->ErrorCorrect[i]);
	}
	return ret;
}

void MTWrapper::setLogger(GuideLog * logger)
{
	m_logger = logger;
    if (m_analysiser)
    {
        m_analysiser->setLogger(m_logger);
    }
}

int MTWrapper::setKFB(ITA_CALIBRATE_TYPE type, float p)
{
	switch (type)
	{
	case ITA_NEAR_KF_AUTO:
		m_params.fNearKf = p;
		break;
	case ITA_NEAR_B_AUTO:
		m_params.fNearB = p;
		break;
	case ITA_FAR_KF_AUTO:
		m_params.fFarKf = p;
		break;
	case ITA_FAR_B_AUTO:
		m_params.fFarB = p;
		break;
	case ITA_NEAR_KF_MANUAL:
		m_params.fNearKf2 = p;
		break;
	case ITA_NEAR_B_MANUAL:
		m_params.fNearB2 = p;
		break;
	case ITA_FAR_KF_MANUAL:
		m_params.fFarKf2 = p;
		break;
	case ITA_FAR_B_MANUAL:
		m_params.fFarB2 = p;
		break;
	default:
		break;
	}
	return 0;
}

void MTWrapper::setProductType(ITA_PRODUCT_TYPE type)
{
	m_product = type;
}

ITA_RESULT MTWrapper::isChangeRange(short * pSrc, int width, int height, int range, float areaTh1, float areaTh2, int low2high, int high2low, int * isChange)
{
	if (!m_core)
		return ITA_MT_NOT_READY;
	*isChange = m_core->autoChangeRange(pSrc, width, height, range, areaTh1, areaTh2, low2high, high2low);
	return ITA_OK;
}

ITA_RESULT MTWrapper::isMultiChangeRange(short* pSrc, int width, int height, int range,
	float areaTh1, float areaTh2, int low2mid, int mid2low, int mid2high, int high2mid, int* isChange)
{
	if (!m_core)
		return ITA_MT_NOT_READY;
	*isChange = m_core->autoMultiChangeRange(pSrc, width, height, range, areaTh1, areaTh2, low2mid, mid2low, mid2high, high2mid);
	return ITA_OK;
}

ITA_RESULT MTWrapper::refreshCoreParaInfo()
{
	if (!m_core)
	{
		return ITA_MT_NOT_READY;
	}
	m_core->refreshGear(m_params.n_fpaGear);
	m_core->RefreshCorrK();
	return ITA_OK;
}

ITA_RESULT MTWrapper::setMeasureInfo(ITAMeasureInfo info)
{
	if (!m_core)
	{
		return ITA_MT_NOT_READY;
	}
	m_params.bDistanceCompensate = info.bDistanceCompensate;
	m_params.bEmissCorrection = info.bEmissCorrection;
	m_params.bTransCorrection = info.bTransCorrection;
	m_params.bHumidityCorrection = info.bHumidityCorrection;
	m_params.fNearKf = info.fNearKf;
	m_params.fNearB = info.fNearB;
	m_params.fFarKf = info.fFarKf;
	m_params.fFarB = info.fFarB;
	m_params.fNearKf2 = info.fNearKf2;
	m_params.fNearB2 = info.fNearB2;
	m_params.fFarKf2 = info.fFarKf2;
	m_params.fFarB2 = info.fFarB2;
	m_params.fHighShutterCorrCoff = info.fHighShutterCorrCoff;
	m_params.fLowShutterCorrCoff = info.fLowShutterCorrCoff;
	m_params.fShutterCorrCoff = info.fShutterCorrCoff;
	m_params.fHighLensCorrK = info.fHighLensCorrK;
	m_params.fLowLensCorrK = info.fLowLensCorrK;
	m_params.fLensCorrK = info.fLensCorrK;
	m_params.fDistance = info.fDistance;
	m_params.fEmiss = info.fEmiss;
	m_params.fTransmit = info.fTransmit;
	m_params.fHumidity = info.fHumidity;
	m_params.fReflectTemp = info.fReflectTemp;
	m_params.fCorrectDistance = info.fCorrectDistance;
	m_core->RefreshCorrK();
	return ITA_OK;
}

float MTWrapper::getTemp(short ad)
{
	double temp1 = -(0.000000000040235 * ad * ad * ad);

	double temp2 = (0.000001218402729 * ad * ad);

	double temp3 = -(0.018218076216914 * ad);

	double y = temp1 + temp2 + temp3 + 127.361304901973000;

	return (float)((y * 100) / 100.0f);
}

float MTWrapper::getJPMTemp(short ad)
{
	/*double y = -0.0201 * ad + 371.29;
	return (y * 100) / 100.0f;*/
	//float Tt = (float)(1.25 * ad - 16100);   //精度为0.01(即32℃的焦温Tt值为3200)
	float Tt = (float)(1.25 * ad - 15800);   //与采集端保持一致，解决焦温低3℃的问题。
	return Tt / 100;
}

unsigned int MTWrapper::getBitsValue(unsigned char* pFrameParam, int nStartIndex, int nLen, int nStartBitIndex, int nBitLen)
{
	if (nLen >= 1 && nLen <= 4 && nBitLen >= 1 && nBitLen <= 32 && nStartBitIndex >= 0 && nStartBitIndex < 32 && (nStartBitIndex + nBitLen) <= 32)
	{
		unsigned int n1 = 0;
		unsigned int nWeight = 1;
		for (int i = 0; i < nLen; ++i)
		{
			unsigned int n2 = pFrameParam[i + nStartIndex] * nWeight;
			nWeight *= 256;
			n1 += n2;
		}

		n1 = n1 >> nStartBitIndex;

		unsigned int nMod = 0xFFFFFFFF;

		nMod = nMod >> (32 - nBitLen);

		n1 = n1 & nMod;

		return n1;
	}
	return 0;
}

ITA_RESULT MTWrapper::AutoCorrectTemp(float *pBlackBodyTemp, short *pY16, int arrLength)
{
	return ITA_UNSUPPORT_OPERATION;
}
ITA_RESULT MTWrapper::AnalysisAdd(ITAShape* shape, int size)
{
    if (NULL == shape)
    {
        return ITA_NULL_PTR_ERR;
    }

    if (size <= 0)
    {
        return ITA_ILLEGAL_PAPAM_ERR;
    }

    ITA_RESULT ret = ITA_OK;

    PolygonAnalysiser::PolygonPool items;
    for (int i = 0; i < size; i++)
    {
        ITAShape* node = shape + i;
        if (NULL == node)
        {
            LOGW(m_logger, "analysis object[%d] is null, size=%d", i, size);
            return ITA_NULL_PTR_ERR;
        }
        PolygonAnalysiser::Polygon item;
        ret = m_analysiser->analysiser(node->type,
            PolygonAnalysiser::convert(node), item);

        if (ITA_OK != ret)
        {
            LOGW(m_logger, "analysis object[%d] add fail, type=%d, count: %d, err=%d",
                i, node->type, node->size, ret);

            return ret;
        }

#if __cplusplus >= 201103L
        items.push_back(std::move(item));
#else
        items.push_back(item);
#endif

    }

    m_analysiser->add(items);

    return ITA_OK;
}

ITA_RESULT MTWrapper::AnalysisClear()
{
    return m_analysiser->clear();
}

ITA_RESULT MTWrapper::AnalysisTemp(short* y16Data, int width, int height, 
                    ITAAnalysisTemp* analysisTemp, int size, short avgB)
{
	if (NULL == y16Data)
	{
		return ITA_NULL_PTR_ERR;
	}
	if (NULL == analysisTemp)
	{
		return ITA_NULL_PTR_ERR;
	}
    if (NULL == m_core)
    {
        return ITA_MT_NOT_READY;
    }

	if (m_analysiser->size() != size)
	{
		LOGW(m_logger, "analysis object count[%zu] inconsistent with TEMP array count[%d]",
			m_analysiser->size(), size);

		size = gdmin(m_analysiser->size(), size);
	}

	ITAY16InfoArray y16Info;
	y16Info.size = size;
	y16Info.y16 = (ITAAnalysisY16Info*)porting_calloc_mem(size,
                    sizeof(ITAAnalysisY16Info), ITA_MT_MODULE);
	if (NULL == y16Info.y16)
	{
		LOGE(m_logger, "alloc mem fail: size=%zu", size * sizeof(ITAAnalysisY16Info));
		return ITA_OUT_OF_MEMORY;
	}

	ITA_RESULT ret = AnalysisY16(y16Data, width, height, y16Info.y16, size);
	if (ITA_OK != ret)
	{
		porting_free_mem(y16Info.y16);

		LOGW(m_logger, "AnalysisY16 fail. err=%d", ret);

		return ret;
	}

	if (0 != avgB)
	{
		for (int i = 0; i < size; i++)
		{
			ITAAnalysisY16Info* item = y16Info.y16 + i;
			item->maxY16 -= avgB;
			item->minY16 -= avgB;
			item->avgY16 -= avgB;
		}
	}

	ret = (ITA_RESULT)m_core->GetAnalysisTempByY16(y16Info.y16, analysisTemp, size);

	porting_free_mem(y16Info.y16);

	return ret;
}

ITA_RESULT MTWrapper::GetRangeMask(ITARangeMask* rangeMask, int size)
{
	int objectSize = m_analysiser->size();
	if (size != objectSize || rangeMask->size != m_w* m_h)
	{
		return ITA_ERROR;
	}

	for (int i = 0; i < objectSize; i++)
	{
		const auto& node = m_analysiser->at(i);
		if (node.size() != 0) {
			for (int j = 0; j < node.size(); j++)
			{
				const PolygonAnalysiser::Polygon::Line& line = node.at(j);
				int y = line.y;
				int index = 0;
				for (int k = line.x1; k < line.x2; k++) {
					index = y * m_w + k;
					rangeMask[i].maskArray[index] = 255;
				}
			}
		}
	}
	return ITA_OK;
}

ITA_RESULT MTWrapper::AnalysisY16(short* y16Array, int width, int height, 
                        ITAAnalysisY16Info* analysisY16Info, int size)
{
	if (NULL == y16Array)
	{
		return ITA_NULL_PTR_ERR;
	}

	if (NULL == analysisY16Info)
	{
		return ITA_NULL_PTR_ERR;
	}

	if (m_analysiser->size() != size)
	{
		LOGW(m_logger, "analysis object count[%zu] inconsistent with Y16 array count[%d]",
			m_analysiser->size(), size);

		size = gdmin(m_analysiser->size(), size);
	}

	int len = width * height;


	for (int i = 0; i < size; i++)
	{
		ITAAnalysisY16Info* item = analysisY16Info + i;
		if (NULL == item)
		{
			LOGE(m_logger, "analysis object[%d] Y16 array[%d] is null, size=%d",
				i, i, size);

			return ITA_NULL_PTR_ERR;
		}

		const auto& node = m_analysiser->at(i);
		if (node.size() == 0)
		{
			LOGW(m_logger, "analysis object[%d] contain point number[%zu]",
				i, node.size());

			// set zero
			memset(item, 0, sizeof(ITAAnalysisY16Info));

			continue;
		}

		//short maxY16 = std::numeric_limits<short>::min();
		//short minY16 = std::numeric_limits<short>::max();
		short maxY16 = -32768;
		short minY16 = 32767;
		long long avgY16 = 0; // declare 'long long' type prevention overflow

		int n = 0;
		int index = 0;
		for (int j = 0; j < node.size(); j++)
		{
			const PolygonAnalysiser::Polygon::Line& line = node.at(j);
			for (int x = line.x1; x <= line.x2; x++)
			{
				index = x + line.y * width;

				const int& y16 = y16Array[index];

				maxY16 = gdmax(y16, maxY16);
				minY16 = gdmin(y16, minY16);
				avgY16 += y16;

				n++;
			}
		}

		// check shape contain valid point
		if (n <= 0)
		{
			LOGW(m_logger, "analysis object[%d] no valid point", i);

			// set zero
			memset(item, 0, sizeof(ITAAnalysisY16Info));

			continue;
		}

		item->maxY16 = maxY16;
		item->minY16 = minY16;
		item->avgY16 = static_cast<short>(avgY16 / n);
	}

	return ITA_OK;
}
