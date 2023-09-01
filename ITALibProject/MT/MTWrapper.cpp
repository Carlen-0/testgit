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
		//�ʳ��Զ�У����ʱʧ�ܣ������趨������ֵ���㷨�齨����΢�ſ�������
		m_params.fFarB2 = 600;
	}
	m_params.fHighShutterCorrCoff = 0;
	m_params.fLowShutterCorrCoff = 0;
	m_params.fShutterCorrCoff = 0.0f;
	/*m_params.fHighLensCorrK = 500;
	m_params.fLowLensCorrK = 500;*/

	m_params.fDistance = 1.5;	//�û�����
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
	m_params.mtType = range;	//���û����á��������ͣ�0:�������   1����ҵ���³��¶�    2����ҵ���¸��¶�
	m_params.mtDisType = 0;
	/*���벹��ϵ�������趨һ��Ĭ�����ݡ����������ļ�ʱ���³���Ŀʹ�õ�ֵ��*/
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
	memset(m_centralArray, 0, sizeof(short) * 8); //�����˲����ĵ�Y16����
	m_centralCount = 0; //֡����
	//Ĭ�����ó�ģ���Ʒ���������ļ�֮������Ƿ�ģ���Ʒ�������ֳֺ�IPT�����ࡣ
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
	//���ݰ��еľ��벹��ϵ��ֵ�ǿյģ��������ļ��ж�ȡ��
	m_params.ucGearMark = ph->ucGearMark;	//0�����µ�  1�����µ�
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
	}*///��Ҫ���ٲ���ʵ����������̲߳���������
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
	//�������ļ����ҵ���Ӧ��һ������������µ�ֵ��
	for (i = 0; i < config->mtConfigCount; i++)
	{
		pConf = config->mtConfig + i;
		if (pConf->lensType == lensType && pConf->mtType == range)
		{
			//lensType��mtType���ж����������ø�ֵ��
			//m_params.m_fieldType = pConf->lensType;				//ITA_FIELD_ANGLE �ӳ������� 0:56�㣻1:25�㣻2:120�㣻3:50�㣻4:90�㣻5:33�㡣
			//pConf->mtDistanceRangeN;		//���¾��뷶Χ����
			//pConf->mtDistanceRangeF;		//���¾��뷶Χ����
			//m_params.mtType = pConf->mtType;					//����ģʽ 0�����壻1����ҵ���£�2����ҵ���¡�
			if (pConf->correctDistanceS == 1)
				m_params.fCorrectDistance = pConf->correctDistance;		//�Զ�У�¾���
			if (pConf->distanceS == 1)
				m_params.fDistance = pConf->distance;				//Ŀ�����
			if (pConf->lowLensCorrKS == 1)
				m_params.fLowLensCorrK = pConf->lowLensCorrK;			//���³��µ���Ͳ��Ưϵ��
			if (pConf->highLensCorrKS == 1)
				m_params.fHighLensCorrK = pConf->highLensCorrK;		//���¸��µ���Ͳ��Ưϵ��
			if (pConf->lowShutterCorrCoffS == 1)
				m_params.fLowShutterCorrCoff = pConf->lowShutterCorrCoff;	//���³��µ�������Ưϵ��
			if (pConf->highShutterCorrCoffS == 1)
				m_params.fHighShutterCorrCoff = pConf->highShutterCorrCoff;	//���¸��µ�������Ưϵ��
			if (pConf->mtDisTypeS == 1)
				m_params.mtDisType = pConf->mtDisType;			//���¾����������ͣ�0��6����; 1:9����
			if (pConf->coefA1S == 1)
				m_params.fCoefA1 = pConf->coefA1;				//���¾�����������
			if (pConf->coefA2S == 1)
				m_params.fCoefA2 = pConf->coefA2;				//���¾�����������
			if (pConf->coefB1S == 1)
				m_params.fCoefB1 = pConf->coefB1;				//���¾�����������
			if (pConf->coefB2S == 1)
				m_params.fCoefB2 = pConf->coefB2;				//���¾�����������
			if (pConf->coefC1S == 1)
				m_params.fCoefC1 = pConf->coefC1;				//���¾�����������
			if (pConf->coefC2S == 1)
				m_params.fCoefC2 = pConf->coefC2;				//���¾�����������
			if (pConf->coefA3S == 1)
				m_params.fCoefA3 = pConf->coefA3;				//���¾�����������
			if (pConf->coefB3S == 1)
				m_params.fCoefB3 = pConf->coefB3;				//���¾�����������
			if (pConf->coefC3S == 1)
				m_params.fCoefC3 = pConf->coefC3;				//���¾�����������
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
	//��һ֡ʱ�ϴο����¶�Ϊ0����ֵ�ɵ�ǰ�����¶ȡ�
	if (!m_shutterTempWhenShutter)
	{
		m_shutterTempWhenShutter = m_params.fRealTimeShutterTemp;
		m_lensTempWhenShutter = m_params.fRealTimeLensTemp;
		//��������������ô��һ֡ʱ���������¸�ʵʱ�´�ֵ������Ȼ������ȡ�ϴιػ�ʱ�洢�Ŀ��������¡�
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
		//�����״λ�ȡ���´���һ������Ч�ģ�һ��ʱ����´����ݲ���ȷ����ʱ����������Ҫ���³���Ч��ֵ��
		m_params.fOrinalShutterTemp = m_params.fRealTimeShutterTemp;
	}
	//�����ʱ�����ϴ��¶ȡ�
	//���׸��±�־λ�Ǳ�֤���ڿ����Ѿ��պϵ��ȫ�ı�־����ʾ���Ʊ�־λ�Ǹ���������׼��Ҫ������ˡ�������ģ���Ʒ��
	//��׼Э����bNucShutterFlag��ʾnuc����״̬��bOrdinaryShutter��ʾ��ͨ����״̬�����߶�Ҫ�����´���
	if (m_params.bNucShutterFlag || m_params.bOrdinaryShutter)
	{
		if (m_params.bLogPrint)
			m_logger->output(LOG_DEBUG, "shutter now. %d %d %d", m_params.bOrdinaryShutter, m_params.bNucShutterFlag, m_shutterClosed);
		//���Ÿձպ�ʱ��¼�¶ȣ��������ĵ�Y16
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
	//����ʱ�ο��������ò��ϡ�
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
	m_params.mtType = range;	//���û����á��������ͣ�0:�������   1����ҵ���³��¶�    2����ҵ���¸��¶�
	m_params.m_fieldType = lensType;
	m_logger->output(LOG_INFO, "changeRange mtType=%d fieldType=%d", range, lensType);
	/*���벹��ϵ�������趨һ��Ĭ�����ݡ����������ļ�ʱ���³���Ŀʹ�õ�ֵ��*/
	/*�û��л���ΧʱfEmiss�Ȳ�������֮ǰֵ*/
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
	/*�л����·�Χʱ���������ٹ���m_core����Ϊ��һ�ι���ʱ�Ѵ������õĵ�ַ��*/
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
	/*��������Ƿ���Ч��Խ��*/
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
		/*����ָ�*/
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
	case ITA_SET_DISTANCE:			//����0.5~2.5�ס����Բ����ã�Ĭ��1.5�ס��������ͣ������͡�
		fParam = *(float *)param;
		if (fParam <= 0.0 || fParam > 10000.0)
		{
			return ITA_ARG_OUT_OF_RANGE;
		}
		m_params.fDistance = fParam;
		break;
	case ITA_SET_EMISS:				//���÷�����(��ҵר��)0.01-1.0��Ĭ��0.95. �������ͣ������͡�
		fParam = *(float *)param;
		//0.01-1.0
		if (fParam <= 0.0 || fParam > 1.0)
		{
			return ITA_ARG_OUT_OF_RANGE;
		}
		m_params.fEmiss = fParam;
		break;
	case ITA_SET_HUMI:				//����ʪ��(0.01-1.0,Ĭ��0.6). �������ͣ������͡�
		fParam = *(float *)param;
		//0.01-1.0
		if (fParam <= 0.0 || fParam > 1.0)
		{
			return ITA_ARG_OUT_OF_RANGE;
		}
		m_params.fHumidity = fParam;
		break;
	case ITA_SET_TRANSMIT:			//͸����(��ҵר�ã�0.01-1, default 1)���������ͣ������͡�
		fParam = *(float *)param;
		//0.01-1.0
		if (fParam <= 0.0 || fParam > 1.0)
		{
			return ITA_ARG_OUT_OF_RANGE;
		}
		m_params.fTransmit = fParam;
		break;
	case ITA_REFLECT_TEMP:			//�����¶�(��ҵר��)���������ͣ������͡�
		fParam = *(float *)param;
		m_params.fReflectTemp = fParam;
		break;
	case ITA_SET_ENVIRON:			//���û����¶ȡ����û������õ�����£��ڲ��Զ����㻷�¡�����û����ã���ʹ�����õ�ֵ���������ͣ������͡�
		fParam = *(float *)param;
		if (fParam < -10000 || fParam > 10000)
		{
			return ITA_ARG_OUT_OF_RANGE;
		}
		m_params.fEnvironmentTemp = fParam;
		m_isUserEnviron = true;
		break;
	case ITA_GET_ENVIRON:			//��ȡ���¡�
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
	case ITA_GET_FPA_TEMP:			//��ȡ���¡�
		if(param)
			*(float *)param = m_params.fRealTimeFpaTemp;
		else
		{
			ret = ITA_NULL_PTR_ERR;
			m_logger->output(LOG_ERROR, "ITA_GET_FPA_TEMP ret=%d", ret);
		}
		break;
	case ITA_HUMI_CORR:				//ʪ��У�����ء�Ĭ�Ϲرա��������ͣ�ITA_SWITCH��
		flag = *(ITA_SWITCH *)param;
		if (flag > ITA_ENABLE || flag < ITA_DISABLE)
			ret = ITA_ARG_OUT_OF_RANGE;
		else
			m_params.bHumidityCorrection = (unsigned char)flag;
		break;
	case ITA_LENS_CORR:				//��Ͳ��ƯУ�����ء�Ĭ�Ϲرա��������ͣ�ITA_SWITCH��
		flag = *(ITA_SWITCH *)param;
		if (flag > ITA_ENABLE || flag < ITA_DISABLE)
			ret = ITA_ARG_OUT_OF_RANGE;
		else
			m_params.bLensCorrection = (unsigned char)flag;
		break;
	case ITA_SHUTTER_CORR:			//������ƯУ�����ء�Ĭ�Ϲرա��������ͣ�ITA_SWITCH��
		flag = *(ITA_SWITCH *)param;
		if (flag > ITA_ENABLE || flag < ITA_DISABLE)
			ret = ITA_ARG_OUT_OF_RANGE;
		else
			m_params.bShutterCorrection = (unsigned char)flag;
		break;
	case ITA_HIGH_LENS_CORR_K:		//ʵʱ���µ���Ͳ��Ư����ϵ�����������ͣ������͡�
		if (!m_core)
		{
			return ITA_MT_NOT_READY;
		}
		fParam = *(float *)param;
		m_params.fHighLensCorrK = fParam;
		m_core->RefreshCorrK();
		break;
	case ITA_OTHER_LENS_CORR_K:		//ʵʱ��������Ͳ��Ư����ϵ�����������ͣ������͡�
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
	case ITA_LOW_LENS_CORR_K:		//ʵʱ���µ���Ͳ��Ư����ϵ�����������ͣ������͡�
		if (!m_core)
		{
			return ITA_MT_NOT_READY;
		}
		fParam = *(float *)param;
		m_params.fLowLensCorrK = fParam;
		m_core->RefreshCorrK();
		break;
	case ITA_HIGH_SHUTTER_CORR_K:	//���µ�����У��ϵ�����������ͣ������͡�
		if (!m_core)
		{
			return ITA_MT_NOT_READY;
		}
		fParam = *(float *)param;
		m_params.fHighShutterCorrCoff = fParam;
		m_core->RefreshCorrK();
		break;
	case ITA_LOW_SHUTTER_CORR_K:	//���µ�����У��ϵ�����������ͣ������͡�
		if (!m_core)
		{
			return ITA_MT_NOT_READY;
		}
		fParam = *(float *)param;
		m_params.fLowShutterCorrCoff = fParam;
		m_core->RefreshCorrK();
		break;
	case ITA_OTHER_SHUTTER_CORR_K:	//����������У��ϵ�����������ͣ������͡�
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
	case ITA_ENVIRON_CORR:			//�����������ء�Ĭ�Ϲرա��������ͣ�ITA_SWITCH��
		flag = *(ITA_SWITCH *)param;
		if (flag > ITA_ENABLE || flag < ITA_DISABLE)
			ret = ITA_ARG_OUT_OF_RANGE;
		else
			m_params.bEnvironmentCorrection = (unsigned char)flag;
		break;
	case ITA_DISTANCE_COMPEN:		//���벹�����ء�Ĭ�ϴ򿪡��������ͣ�ITA_SWITCH��
		flag = *(ITA_SWITCH *)param;
		if (flag > ITA_ENABLE || flag < ITA_DISABLE)
			ret = ITA_ARG_OUT_OF_RANGE;
		else
			m_params.bDistanceCompensate = (unsigned char)flag;
		break;
	case ITA_EMISS_CORR:			//������У�����ء�Ĭ�ϴ򿪡��������ͣ�ITA_SWITCH��
		flag = *(ITA_SWITCH *)param;
		if (flag > ITA_ENABLE || flag < ITA_DISABLE)
			ret = ITA_ARG_OUT_OF_RANGE;
		else
			m_params.bEmissCorrection = (unsigned char)flag;
		break;
	case ITA_TRANS_CORR:			//͸����У�����ء�Ĭ�Ϲرա��������ͣ�ITA_SWITCH��
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
	case ITA_CENTRAL_TEMPER: 		//��ȡ�˲���������¡��������ͣ������͡�
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
		//������������²��Ҽ�ȥ���׾�ֵ����ôm_mtBase��û����Щ��Ϣ�޷����㡣
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
	//���ò��²���
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
	info->orinalShutterTemp = m_params.fOrinalShutterTemp;//���������¶�
	info->currentShutterTemp = m_params.fCurrentShutterTemp;//��ǰ�����ʱ�Ŀ����¶�
	info->realTimeShutterTemp = m_params.fRealTimeShutterTemp;//ʵʱ�����¶�
	info->realTimeLensTemp = m_params.fRealTimeLensTemp;//ʵʱ��Ͳ�¶�
	info->currentLensTemp = m_params.fCurrentLensTemp;//��ǰ�����ʱ�ľ�Ͳ�¶�
	info->realTimeFpaTemp = m_params.fRealTimeFpaTemp;//ʵʱ��ƽ���¶�
	info->humidity = m_params.fHumidity;//��ȡʪ�Ȳ���
}

float MTWrapper::smoothFocusTemp(float fpaTemp)
{
	if (!m_core)
		return 0;
	return m_core->smoothFocusTemp(fpaTemp);
}

ITA_RESULT MTWrapper::refreshFrame(short * y16Array, int w, int h)
{
	//���ص�Y16����δ���죬����ת��
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
	/*�ڲ����Զ��ӿ�ʱ��ѡȡ������Y16�Ƚ���֣����Զ�У�µ��߼������ҵ�һ��KF B������Y16���¶������С��
	�����滻��Kf��Bֵ����������KF��Bֵ����ʹ�����¶�ֵ���׼�����С��0.4������ʾУ��ʧ�ܡ�*/
	m_logger->output(LOG_INFO, "AutoCorrectTempFar result:kf=%f b=%f", farKf, farB);
	for (int i = 0; i < autocorTpara->nBlackBodyNumber; i++)
	{
		m_logger->output(LOG_INFO, "%d TBlackBody=%f Y16=%d ErrorNoCorrect=%f ErrorCorrect=%f",
			i, autocorTpara->TBlackBody[i], autocorTpara->Y16[i], autocorTpara->ErrorNoCorrect[i], autocorTpara->ErrorCorrect[i]);
	}
	return ret;
}
/*�û������Զ�У��*/
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
	//float Tt = (float)(1.25 * ad - 16100);   //����Ϊ0.01(��32��Ľ���TtֵΪ3200)
	float Tt = (float)(1.25 * ad - 15800);   //��ɼ��˱���һ�£�������µ�3������⡣
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
