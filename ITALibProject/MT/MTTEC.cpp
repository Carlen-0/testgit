/*************************************************************
Copyright (C), 2021--2022, Wuhan Guide Sensmart Tech Co., Ltd
File name   : MTTEC.cpp
Author      : Guide Sensmart RD. wangyan
Version     : 1.0
Date        : 2022/1/15
Description : MT TEC Wrapper.
*************************************************************/

#include "MTTEC.h"
#include <limits>

#include "PolygonAnalysiser.h"

MTTEC::MTTEC(ITA_RANGE range, int w, int h, ITA_PRODUCT_TYPE type)
	: m_analysiser(new PolygonAnalysiser(w, h))
{
	m_w = w;
	m_h = h;
	m_y16Len = w * h * 2;
	m_coreTEC = NULL;
	/*m_lensTempWhenShutter = 0;
	m_shutterClosed = 0;*/
	m_paramsTEC.mtType = range;	//由用户设置。测温类型，0:人体测温   1：工业测温常温段    2：工业测温高温段
	//memset(&m_defaultConf, 0, sizeof(ITAConfig));
	memset(m_centralArray, 0, sizeof(short) * 8); //空域滤波中心点Y16数组
	m_centralCount = 0; //帧计数
	//默认配置成模组产品，读配置文件之后如果是非模组产品再配置手持和IPT测温类。
	m_product = type;
	memset(&m_paramsTEC, 0, sizeof(IMAGE_MEASURE_STRUCT));
	m_paramsTEC.nKF = 100;
	m_paramsTEC.fHumidity = 60;
	m_paramsTEC.nAtmosphereTransmittance = 100;
	m_paramsTEC.fEmiss = 1.0;
	m_paramsTEC.fDistance = 2.0;
	m_paramsTEC.fReflectT = 23;
	m_paramsTEC.fWindowTransmittance = 1.0;
	m_paramsTEC.fWindowTemperature = 23;
	m_paramsTEC.bHumidityCorrection = true;
	m_paramsTEC.bShutterCorrection = true;
	m_paramsTEC.bLensCorrection = true;
	m_paramsTEC.bEmissCorrection = true;
	m_paramsTEC.bDistanceCorrection = true;
	m_paramsTEC.bAmbientCorrection = true;
	m_paramsTEC.bB1Correction = true;
	m_paramsTEC.bAtmosphereCorrection = true;
	m_paramsTEC.bWindowTransmittanceCorrection = true;
}

MTTEC::~MTTEC()
{
	if (m_coreTEC)
	{
		delete m_coreTEC;
		m_coreTEC = NULL;
	}
	/*if (m_defaultConf.mtConfig)
	{
		porting_free_mem(m_defaultConf.mtConfig);
		m_defaultConf.mtConfig = NULL;
	}*/
}

int MTTEC::loadData(PackageHeader * ph, short * fpaTempArray, short * allCurvesArray)
{
	//多线程切换范围时销毁MeasureTempCoreTEC并重新创建，但在销毁之前MeasureTempCoreTEC有个接口IsChangeRange正在执行还没返回，访问指针出现死机。
	/*if (m_coreTEC)
	{
		delete m_coreTEC;
		m_coreTEC = NULL;
	}*/
	if (!m_coreTEC)
	{
		m_coreTEC = new MeasureTempCoreTEC(m_w, m_h, MAX_TEC_CURVE_SIZE, allCurvesArray, &m_paramsTEC);
		m_coreTEC->setLogger(m_logger);
	}
	else
	{
		m_coreTEC->reload(MAX_TEC_CURVE_SIZE, allCurvesArray, &m_paramsTEC);
	}
	return 0;
}

ITA_RESULT MTTEC::loadConfig(ITAConfig * config, ITA_RANGE range, ITA_FIELD_ANGLE lensType)
{
	ITAMTConfig *pConf;
	int i;
	if (!config->mtConfig || config->mtConfigCount <= 0)
	{
		m_logger->output(LOG_WARN, "MTTEC::loadConfig  ret = %d ", ITA_NO_MT_CONF);
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
			//if (pConf->correctDistanceS == 1)
			//	m_params.fCorrectDistance = pConf->correctDistance;		//自动校温距离
			//if (pConf->distanceS == 1)
			//	m_params.fDistance = pConf->distance;				//目标距离
			//if (pConf->lowLensCorrKS == 1)
			//	m_params.fLowLensCorrK = pConf->lowLensCorrK;			//测温常温档镜筒温漂系数
			//if (pConf->highLensCorrKS == 1)
			//	m_params.fHighLensCorrK = pConf->highLensCorrK;		//测温高温档镜筒温漂系数
			//if (pConf->lowShutterCorrCoffS == 1)
			//	m_params.fLowShutterCorrCoff = pConf->lowShutterCorrCoff;	//测温常温档快门温漂系数
			//if (pConf->highShutterCorrCoffS == 1)
			//	m_params.fHighShutterCorrCoff = pConf->highShutterCorrCoff;	//测温高温档快门温漂系数
			break;
		}
	}
	if (i >= config->mtConfigCount)
	{
		m_logger->output(LOG_WARN, "MTTEC::loadConfig  ret = %d ", ITA_NO_MT_CONF);
		return ITA_NO_MT_CONF;
	}
	else
		return ITA_OK;
}

ITA_RESULT MTTEC::refreshShutterStatus(short y16CenterValue, ITA_MODE m_mode)
{
	////第一帧时上次快门温度为0，赋值成当前快门温度。
	//if (!m_lensTempWhenShutter)
	//{
	//	m_lensTempWhenShutter = m_paramsTEC.fRealTimeLensTemp;
	//	//第一帧时开机快门温赋值
	//	m_paramsTEC.fOrinalShutterTemp = m_paramsTEC.fRealTimeShutterTemp;
	//}
	//if (m_paramsTEC.fOrinalShutterTemp < 1 || m_paramsTEC.fOrinalShutterTemp > 100)
	//{
	//	//开机首次获取的温传不一定是有效的，一段时间后温传数据才正确。此时开机快门温要更新成有效的值。
	//	m_paramsTEC.fOrinalShutterTemp = m_paramsTEC.fRealTimeShutterTemp;
	//}
	////打快门时更新上次温度。
	////本底更新标志位是保证现在快门已经闭合的最安全的标志，显示控制标志位是告诉你现在准备要打快门了。
	//if (m_paramsTEC.bNucShutterFlag)
	//{
	//	//快门刚闭合时记录温度，采样中心点Y16
	//	if (!m_shutterClosed)
	//	{
	//		m_lensTempWhenShutter = m_paramsTEC.fRealTimeLensTemp;
	//	}
	//}
	//m_shutterClosed = m_paramsTEC.bNucShutterFlag;
	//m_paramsTEC.fCurrentLensTemp = m_lensTempWhenShutter;
	return ITA_UNSUPPORT_OPERATION;
}

ITA_RESULT MTTEC::calcFPAGear(int * gear)
{
	//焦温档位由ASIC参数行传递，档位变化时更新参数和曲线
	ITA_RESULT result = ITA_UNSUPPORT_OPERATION;
	if (m_paramsTEC.nGear >= 1)
	{
		m_paramsTEC.fWindowTemperature = 23;
		m_paramsTEC.fReflectT = 23;
	}
	else
	{
		m_paramsTEC.fWindowTemperature = 3;
		m_paramsTEC.fReflectT = 3;
	}
	m_logger->output(LOG_INFO, "change FPAGear gear=%d old=%d", m_paramsTEC.nGear, *gear);
	return result;
}

ITA_RESULT MTTEC::changeRange(ITA_RANGE range, ITA_FIELD_ANGLE lensType)
{
	ITA_RESULT result = ITA_OK;
	m_paramsTEC.mtType = range;	//由用户设置。测温类型，0:人体测温   1：工业测温常温段    2：工业测温高温段
	//m_paramsTEC.m_fieldType = lensType;
	m_logger->output(LOG_INFO, "changeRange mtType=%d fieldType=%d", range, lensType);
	/*距离补偿系数，先设定一组默认数据。加载配置文件时更新成项目使用的值。*/
	/*if (ITA_HUMAN_BODY == range)
	{
		m_paramsTEC.bEmissCorrection = 0;
		m_paramsTEC.fEmiss = (float)0.98;
	}
	else if (ITA_INDUSTRY_LOW == range)
	{
		m_paramsTEC.bEmissCorrection = 1;
		m_paramsTEC.fEmiss = (float)0.95;
	}
	else
	{
		m_paramsTEC.bEmissCorrection = 1;
		m_paramsTEC.fEmiss = (float)0.95;
	}*/
	/*if (m_coreTEC)
		m_coreTEC->ChangeRange();
	else
		result = ITA_MT_NOT_READY;*/
	return result;
}

ITA_RESULT MTTEC::calcTempByY16(short y16, float *temp)
{
	float fTemp = 0;
	if (!m_coreTEC)
		return ITA_MT_NOT_READY;
	if (!temp)
		return ITA_NULL_PTR_ERR;
	m_coreTEC->GetTempByY16(fTemp, y16);
	*temp = fTemp;
	if (m_paramsTEC.bLogPrint)
	{
		m_logger->output(LOG_DEBUG, "MT:\t%d\t%.2f\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%hhu\t%hhu\t%d",
		y16, fTemp,
		m_paramsTEC.sY16Offset,
		m_paramsTEC.nKF,												//查曲线时Y16的缩放量(定点化100倍,1~100,默认值为100)
		m_paramsTEC.nB1,												//查曲线时Y16的偏移量(定点化100倍)
		m_paramsTEC.nDistance_a0,										//距离校正系数(定点化10000000000倍，默认0)
		m_paramsTEC.nDistance_a1,										//距离校正系数(定点化1000000000倍，默认0)
		m_paramsTEC.nDistance_a2,										//距离校正系数(定点化10000000倍，默认0)
		m_paramsTEC.nDistance_a3,										//距离校正系数(定点化100000倍，默认0)
		m_paramsTEC.nDistance_a4,										//距离校正系数(定点化100000倍，默认0)
		m_paramsTEC.nDistance_a5,										//距离校正系数(定点化10000倍，默认0)
		m_paramsTEC.nDistance_a6,										//距离校正系数(定点化1000倍，默认0)
		m_paramsTEC.nDistance_a7,										//距离校正系数(定点化100倍，默认0)
		m_paramsTEC.nDistance_a8,										//距离校正系数(定点化100倍，默认0)
		m_paramsTEC.nK1,												//快门温漂系数(定点化100倍，默认0)
		m_paramsTEC.nK2,												//镜筒温漂系数(定点化100倍，默认0)
		m_paramsTEC.nK3,												//环温修正系数(定点化10000倍，默认0)
		m_paramsTEC.nB2,												//环温修正偏移量(定点化10000倍，默认0)
		m_paramsTEC.nKFOffset,											//自动校温KF偏移量，置零后恢复出厂校温设置，默认0
		m_paramsTEC.nB1Offset,											//自动校温B1偏移量，置零后恢复出厂校温设置，默认0
		m_paramsTEC.nGear,												//测温档位, 0:低温档, 1:常温挡
		m_paramsTEC.fHumidity,											//湿度(定点化100倍，默认60)
		m_paramsTEC.nAtmosphereTransmittance,							//大气透过率(定点化100倍，范围0~100，默认100)
		m_paramsTEC.mtType,												//测温类型，0:人体测温   1：工业测温
		m_paramsTEC.fEmiss,											//发射率(0.01-1.0,默认1.0)
		m_paramsTEC.fDistance,										//测温距离 
		m_paramsTEC.fReflectT,										//反射温度（低温档默认3，常温档默认23）
		m_paramsTEC.fAmbient,											//环境温度（取开机快门温）
		m_paramsTEC.fWindowTransmittance,								//窗口透过率(范围0~1，默认1)
		m_paramsTEC.fWindowTemperature,								//窗口温度（低温档默认3，常温档默认23）
		m_paramsTEC.fRealTimeShutterTemp,								//实时快门温度
		m_paramsTEC.fOrinalShutterTemp,								//开机快门温
		m_paramsTEC.fRealTimeLensTemp,								//实时镜筒温
		m_paramsTEC.fCurrentLensTemp,									//最近一次打快门时的镜筒温
		m_paramsTEC.bHumidityCorrection,								//湿度修正开关，默认打开。
		m_paramsTEC.bShutterCorrection,								//快门修正开关，默认打开。
		m_paramsTEC.bLensCorrection,									//镜筒修正开关，默认打开。
		m_paramsTEC.bEmissCorrection,									//发射率修正开关，默认打开。
		m_paramsTEC.bDistanceCorrection,								//距离修正开关，默认打开。
		m_paramsTEC.bAmbientCorrection,								//环温修正开关，默认打开。
		m_paramsTEC.bB1Correction,										//B1修正开关，默认打开。
		m_paramsTEC.bAtmosphereCorrection,								//大气透过率修正开关，默认打开。
		m_paramsTEC.bWindowTransmittanceCorrection,					//窗口透过率开关，默认打开。
		m_paramsTEC.avgB,
		m_paramsTEC.bOrdinaryShutter,   //普通快门标志位
		m_paramsTEC.bNucShutterFlag,    //NUC快门标志位
		m_paramsTEC.lensType);
	}
	return ITA_OK;
}

ITA_RESULT MTTEC::calcBodyTemp(float surfaceTemp, float envirTemp, float * bodyTemp)
{
	float fTemp = 0;
	if (!m_coreTEC)
		return ITA_MT_NOT_READY;
	if (!bodyTemp)
		return ITA_NULL_PTR_ERR;
	m_coreTEC->SurfaceTempToBody(fTemp, surfaceTemp, envirTemp);
	*bodyTemp = fTemp;
	if (m_paramsTEC.bLogPrint)
	{
		m_logger->output(LOG_INFO, "\ttemp=\t%f\t, bodyTemp=\t%f\t envirTemp=\t%f\t", surfaceTemp, fTemp, envirTemp);
	}
	return ITA_OK;
}

ITA_RESULT MTTEC::calcY16ByTemp(float temp, short * y16)
{
	short Y16 = 0;
	if (!m_coreTEC)
		return ITA_MT_NOT_READY;
	if (!y16)
		return ITA_NULL_PTR_ERR;
	m_coreTEC->CalY16ByTemp(Y16, temp);
	*y16 = Y16;
	return ITA_OK;
}

ITA_RESULT MTTEC::calcY16MatrixByTemp(float * tempMatrix, int w, int h, short * y16Matrix, int matrixSize)
{
	if (!m_coreTEC)
		return ITA_MT_NOT_READY;
	if (!y16Matrix || !tempMatrix)
		return ITA_NULL_PTR_ERR;
	if (matrixSize <= 0)
	{
		return ITA_ARG_OUT_OF_RANGE;
	}
	return (ITA_RESULT)m_coreTEC->CalY16MatrixByTempMatrix(tempMatrix, y16Matrix, h, w);
}

ITA_RESULT MTTEC::calcTempMatrix(short * y16Array, int y16W, int y16H, float distance, int x, int y, int w, int h, ITA_MATRIX_TYPE type, float * tempMatrix)
{
	if (!m_coreTEC)
		return ITA_MT_NOT_READY;
	if (!y16Array || !tempMatrix)
		return ITA_NULL_PTR_ERR;
	if (distance <= 0 || distance > 10000)
	{
		return ITA_ARG_OUT_OF_RANGE;
	}
	//float myDis = m_paramsTEC.fDistance;
	/*检查区域是否有效或越界*/
	if (y16W <= 0 || y16H <= 0 || x < 0 || y < 0 || w <= 0 || h <= 0 || x + w > y16W || y + h > y16H)
	{
		m_logger->output(LOG_ERROR, "calcTempMatrix %d %d %d %d ret=%d", x, y, w, h, ITA_ARG_OUT_OF_RANGE);
		return ITA_ARG_OUT_OF_RANGE;
	}
	else
	{
		if (ITA_FAST_MATRIX == type)
			m_coreTEC->GetTempMatrixFast(y16Array, tempMatrix, y16W, y16H, x, y, w, h, distance, m_paramsTEC.fEmiss);
		else
			m_coreTEC->GetTempMatrix(y16Array, tempMatrix, y16W, y16H, x, y, w, h, distance, m_paramsTEC.fEmiss);
		/*距离恢复*/
		//m_paramsTEC.fDistance = myDis;
		if (m_paramsTEC.bLogPrint)
		{
			m_logger->output(LOG_DEBUG, "Matrix:\t%d\t%.2f\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%hhu\t%hhu\t%d",
				y16Array[y16W*(y+h/2)+x+w/2], tempMatrix[w*h/2+w/2],
				m_paramsTEC.sY16Offset,
				m_paramsTEC.nKF,												//查曲线时Y16的缩放量(定点化100倍,1~100,默认值为100)
				m_paramsTEC.nB1,												//查曲线时Y16的偏移量(定点化100倍)
				m_paramsTEC.nDistance_a0,										//距离校正系数(定点化10000000000倍，默认0)
				m_paramsTEC.nDistance_a1,										//距离校正系数(定点化1000000000倍，默认0)
				m_paramsTEC.nDistance_a2,										//距离校正系数(定点化10000000倍，默认0)
				m_paramsTEC.nDistance_a3,										//距离校正系数(定点化100000倍，默认0)
				m_paramsTEC.nDistance_a4,										//距离校正系数(定点化100000倍，默认0)
				m_paramsTEC.nDistance_a5,										//距离校正系数(定点化10000倍，默认0)
				m_paramsTEC.nDistance_a6,										//距离校正系数(定点化1000倍，默认0)
				m_paramsTEC.nDistance_a7,										//距离校正系数(定点化100倍，默认0)
				m_paramsTEC.nDistance_a8,										//距离校正系数(定点化100倍，默认0)
				m_paramsTEC.nK1,												//快门温漂系数(定点化100倍，默认0)
				m_paramsTEC.nK2,												//镜筒温漂系数(定点化100倍，默认0)
				m_paramsTEC.nK3,												//环温修正系数(定点化10000倍，默认0)
				m_paramsTEC.nB2,												//环温修正偏移量(定点化10000倍，默认0)
				m_paramsTEC.nKFOffset,											//自动校温KF偏移量，置零后恢复出厂校温设置，默认0
				m_paramsTEC.nB1Offset,											//自动校温B1偏移量，置零后恢复出厂校温设置，默认0
				m_paramsTEC.nGear,												//测温档位, 0:低温档, 1:常温挡
				m_paramsTEC.fHumidity,											//湿度(定点化100倍，默认60)
				m_paramsTEC.nAtmosphereTransmittance,							//大气透过率(定点化100倍，范围0~100，默认100)
				m_paramsTEC.mtType,												//测温类型，0:人体测温   1：工业测温
				m_paramsTEC.fEmiss,											//发射率(0.01-1.0,默认1.0)
				m_paramsTEC.fDistance,										//测温距离 
				m_paramsTEC.fReflectT,										//反射温度（低温档默认3，常温档默认23）
				m_paramsTEC.fAmbient,											//环境温度（取开机快门温）
				m_paramsTEC.fWindowTransmittance,								//窗口透过率(范围0~1，默认1)
				m_paramsTEC.fWindowTemperature,								//窗口温度（低温档默认3，常温档默认23）
				m_paramsTEC.fRealTimeShutterTemp,								//实时快门温度
				m_paramsTEC.fOrinalShutterTemp,								//开机快门温
				m_paramsTEC.fRealTimeLensTemp,								//实时镜筒温
				m_paramsTEC.fCurrentLensTemp,									//最近一次打快门时的镜筒温
				m_paramsTEC.bHumidityCorrection,								//湿度修正开关，默认打开。
				m_paramsTEC.bShutterCorrection,								//快门修正开关，默认打开。
				m_paramsTEC.bLensCorrection,									//镜筒修正开关，默认打开。
				m_paramsTEC.bEmissCorrection,									//发射率修正开关，默认打开。
				m_paramsTEC.bDistanceCorrection,								//距离修正开关，默认打开。
				m_paramsTEC.bAmbientCorrection,								//环温修正开关，默认打开。
				m_paramsTEC.bB1Correction,										//B1修正开关，默认打开。
				m_paramsTEC.bAtmosphereCorrection,								//大气透过率修正开关，默认打开。
				m_paramsTEC.bWindowTransmittanceCorrection,					//窗口透过率开关，默认打开。
				m_paramsTEC.avgB,
				m_paramsTEC.bOrdinaryShutter,   //普通快门标志位
				m_paramsTEC.bNucShutterFlag,    //NUC快门标志位
				m_paramsTEC.lensType);
		}
		return ITA_OK;
	}
}

ITA_RESULT MTTEC::parametersControl(ITA_MC_TYPE type, void * param)
{
	float fParam;
	ITA_SWITCH flag;
	ITA_RESULT ret = ITA_OK;
	if (!param)
	{
		ret = ITA_NULL_PTR_ERR;
		m_logger->output(LOG_ERROR, "MTTEC::parametersControl ret=%d", ret);
		return ret;
	}
	if (m_paramsTEC.bLogPrint)
		m_logger->output(LOG_INFO, "measureControl op=%d", type);
	switch (type)
	{
	case ITA_SET_DISTANCE:			//距离0.5~2.5米。可以不设置，默认1.5米。参数类型：浮点型。
		fParam = *(float *)param;
		if (fParam <= 0.0 || fParam > 10000.0)
		{
			return ITA_ARG_OUT_OF_RANGE;
		}
		m_paramsTEC.fDistance = fParam;
		break;
	case ITA_SET_EMISS:				//设置发射率(工业专用)0.01-1.0，默认0.95. 参数类型：浮点型。
		fParam = *(float *)param;
		//0.01-1.0
		if (fParam <= 0.0 || fParam > 1.0)
		{
			return ITA_ARG_OUT_OF_RANGE;
		}
		m_paramsTEC.fEmiss = fParam;
		break;
	case ITA_SET_TRANSMIT:			//透过率(工业专用，0.01-1, default 1)。参数类型：浮点型。
		fParam = *(float *)param;
		//0.01-1.0
		if (fParam <= 0.0 || fParam > 1.0)
		{
			return ITA_ARG_OUT_OF_RANGE;
		}
		m_paramsTEC.fWindowTransmittance = fParam;
		break;
	case ITA_REFLECT_TEMP:			//反射温度(工业专用)。参数类型：浮点型。
		fParam = *(float *)param;
		m_paramsTEC.fReflectT = fParam;
		break;
	case ITA_SET_ENVIRON:			//设置环境温度。在用户不设置的情况下，内部自动计算环温。如果用户设置，则使用设置的值。参数类型：浮点型。
		fParam = *(float *)param;
		if (fParam < -10000 || fParam > 10000)
		{
			return ITA_ARG_OUT_OF_RANGE;
		}
		m_paramsTEC.fAmbient = fParam;
		break;
	case ITA_GET_ENVIRON:			//获取环温。
		*(float *)param = m_paramsTEC.fAmbient;
		break;
	case ITA_GET_FPA_TEMP:			//获取焦温。
		if (param)
			*(float *)param = m_paramsTEC.fRealTimeShutterTemp;
		else
		{
			ret = ITA_NULL_PTR_ERR;
			m_logger->output(LOG_ERROR, "ITA_GET_FPA_TEMP ret=%d", ret);
		}
		break;
	case ITA_LENS_CORR:				//镜筒温漂校正开关。默认关闭。参数类型：ITA_SWITCH。
		flag = *(ITA_SWITCH *)param;
		if (flag > ITA_ENABLE || flag < ITA_DISABLE)
			ret = ITA_ARG_OUT_OF_RANGE;
		else
		{
			if(ITA_ENABLE == flag)
				m_paramsTEC.bLensCorrection = true;
			else
				m_paramsTEC.bLensCorrection = false;
		}			
		break;
	case ITA_SHUTTER_CORR:			//快门温漂校正开关。默认关闭。参数类型：ITA_SWITCH。
		flag = *(ITA_SWITCH *)param;
		if (flag > ITA_ENABLE || flag < ITA_DISABLE)
			ret = ITA_ARG_OUT_OF_RANGE;
		else
		{
			if(ITA_ENABLE == flag)
				m_paramsTEC.bShutterCorrection = true;
			else
				m_paramsTEC.bShutterCorrection = false;
		}
		break;
	case ITA_HIGH_LENS_CORR_K:		//实时高温档镜筒温漂修正系数。参数类型：浮点型。
		fParam = *(float *)param;
		m_paramsTEC.nK2 = (int)fParam;
		break;
	case ITA_LOW_LENS_CORR_K:		//实时常温档镜筒温漂修正系数。参数类型：浮点型。
		fParam = *(float *)param;
		m_paramsTEC.nK2 = (int)fParam;
		break;
	case ITA_HIGH_SHUTTER_CORR_K:	//高温档快门校正系数。参数类型：浮点型。
		fParam = *(float *)param;
		m_paramsTEC.nK1 = (int)fParam;
		break;
	case ITA_LOW_SHUTTER_CORR_K:	//常温档快门校正系数。参数类型：浮点型。
		fParam = *(float *)param;
		m_paramsTEC.nK1 = (int)fParam;
		break;
	case ITA_ENVIRON_CORR:			//环温修正开关。默认关闭。参数类型：ITA_SWITCH。
		flag = *(ITA_SWITCH *)param;
		if (flag > ITA_ENABLE || flag < ITA_DISABLE)
			ret = ITA_ARG_OUT_OF_RANGE;
		else {
			if(ITA_ENABLE == flag)
				m_paramsTEC.bAmbientCorrection = true;
			else
				m_paramsTEC.bAmbientCorrection = false;
		}
		break;
	case ITA_DISTANCE_COMPEN:		//距离补偿开关。默认打开。参数类型：ITA_SWITCH。
		flag = *(ITA_SWITCH *)param;
		if (flag > ITA_ENABLE || flag < ITA_DISABLE)
			ret = ITA_ARG_OUT_OF_RANGE;
		else
		{
			if(ITA_ENABLE == flag)
				m_paramsTEC.bDistanceCorrection = true;
			else
				m_paramsTEC.bDistanceCorrection = false;
		}
		break;
	case ITA_EMISS_CORR:			//发射率校正开关。默认打开。参数类型：ITA_SWITCH。
		flag = *(ITA_SWITCH *)param;
		if (flag > ITA_ENABLE || flag < ITA_DISABLE)
			ret = ITA_ARG_OUT_OF_RANGE;
		else
		{
			if(ITA_ENABLE == flag)
				m_paramsTEC.bEmissCorrection = true;
			else
				m_paramsTEC.bEmissCorrection = false;
		}			
		break;
	case ITA_TRANS_CORR:			//透过率校正开关。默认关闭。参数类型：ITA_SWITCH。
		flag = *(ITA_SWITCH *)param;
		if (flag > ITA_ENABLE || flag < ITA_DISABLE)
			ret = ITA_ARG_OUT_OF_RANGE;
		else
		{
			if(ITA_ENABLE == flag)
				m_paramsTEC.bWindowTransmittanceCorrection = true;
			else
				m_paramsTEC.bWindowTransmittanceCorrection = false;
		}	
		break;
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

IMAGE_MEASURE_STRUCT * MTTEC::getTECParams()
{
	return &m_paramsTEC;
}

MtParams * MTTEC::getMtParams()
{
	return 0;
}

void MTTEC::setMtParams(void * param, int paramLen)
{
	//设置测温参数
	memcpy(&m_paramsTEC, param, paramLen);
}

void MTTEC::getParamInfo(ITAParamInfo * info)
{
}

float MTTEC::smoothFocusTemp(float fpaTemp)
{
	return 0;
}

ITA_RESULT MTTEC::refreshFrame(short * y16Array, int w, int h)
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
	/*if (m_paramsTEC.bLogPrint)
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

bool MTTEC::AutoCorrectTempNear(float * fNearKf, float * fNearB, AutoCorrectTempPara * autocorTpara)
{
	return false;
}

bool MTTEC::AutoCorrectTempFar(float * fFarKf, float * fFarB, AutoCorrectTempPara * autocorTpara)
{
	return false;
}

bool MTTEC::AutoCorrectTempFarByUser(float * fFarKf, float * fFarB, AutoCorrectTempPara * autocorTpara)
{
	return false;
}

bool MTTEC::AutoCorrectTempNearByUser(float * fNearKf, float * fNearB, AutoCorrectTempPara * autocorTpara)
{
	return false;
}

void MTTEC::setLogger(GuideLog * logger)
{
	m_logger = logger;
    if (m_analysiser)
    {
        m_analysiser->setLogger(m_logger);
    }
}

void MTTEC::setProductType(ITA_PRODUCT_TYPE type)
{
	m_product = type;
}

ITA_RESULT MTTEC::isChangeRange(short * pSrc, int width, int height, int range, float areaTh1, float areaTh2, int low2high, int high2low, int * isChange)
{
	if (!m_coreTEC)
		return ITA_MT_NOT_READY;
	*isChange = m_coreTEC->autoChangeRange(pSrc, width, height, range, areaTh1, areaTh2, low2high, high2low);
	return ITA_OK;
}
ITA_RESULT MTTEC::isMultiChangeRange(short* pSrc, int width, int height, int range,
	float areaTh1, float areaTh2, int low2mid, int mid2low, int mid2high, int high2mid, int* isChange)
{
	if (!m_coreTEC)
		return ITA_MT_NOT_READY;
	*isChange = m_coreTEC->autoMultiChangeRange(pSrc, width, height, range, areaTh1, areaTh2, low2mid, mid2low, mid2high, high2mid);
	return ITA_OK;
}

int MTTEC::setKFB(ITA_CALIBRATE_TYPE type, float p)
{
	return 0;
}

ITA_RESULT MTTEC::refreshCoreParaInfo()
{
	return ITA_OK;
}

ITA_RESULT MTTEC::AutoCorrectTemp(float *pBlackBodyTemp, short *pY16, int arrLength)
{
	if (!pBlackBodyTemp || !pY16)
	{
		return ITA_NULL_PTR_ERR;
	}

	if (arrLength <= 0)
	{
		return ITA_ILLEGAL_PAPAM_ERR;
	}

	if (!m_coreTEC)
	{
		return ITA_MT_NOT_READY;
	}

	m_coreTEC->AutoCorrectTemp(pBlackBodyTemp, pY16, arrLength);

	return ITA_OK;
}

ITA_RESULT MTTEC::AnalysisAdd(ITAShape* shape, int size)
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

ITA_RESULT MTTEC::AnalysisClear()
{
    return m_analysiser->clear();
}

ITA_RESULT MTTEC::AnalysisTemp(short* y16Data, int width, int height, 
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

	if (m_analysiser->size() != size)
	{
		LOGW(m_logger, "analysis object count[%zu] inconsistent with TEMP array count[%d]",
			m_analysiser->size(), size);

		size = gdmin(m_analysiser->size(), size);
	}

	ITAY16InfoArray y16Info;
	y16Info.size = size;
	y16Info.y16 = (ITAAnalysisY16Info*)porting_calloc_mem(size, sizeof(ITAAnalysisY16Info), ITA_MT_MODULE);
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

	for (int i = 0; i < size; i++)
	{
		ITAAnalysisY16Info* itemY16 = y16Info.y16 + i;
		ITAAnalysisTemp* itemTemp = analysisTemp + i;

		m_coreTEC->GetTempByY16(itemTemp->maxVal, itemY16->maxY16 - avgB);
		m_coreTEC->GetTempByY16(itemTemp->minVal, itemY16->minY16 - avgB);
		m_coreTEC->GetTempByY16(itemTemp->avgVal, itemY16->avgY16 - avgB);
	}

	porting_free_mem(y16Info.y16);

	return ITA_OK;
}

ITA_RESULT MTTEC::AnalysisY16(short* y16Array, int width, int height,
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

ITA_RESULT MTTEC::GetRangeMask(ITARangeMask* rangeMask, int size)
{
	int objectSize = m_analysiser->size();
	if (size != objectSize || rangeMask->size != m_w * m_h)
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