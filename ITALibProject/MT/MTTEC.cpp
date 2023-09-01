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
	m_paramsTEC.mtType = range;	//���û����á��������ͣ�0:�������   1����ҵ���³��¶�    2����ҵ���¸��¶�
	//memset(&m_defaultConf, 0, sizeof(ITAConfig));
	memset(m_centralArray, 0, sizeof(short) * 8); //�����˲����ĵ�Y16����
	m_centralCount = 0; //֡����
	//Ĭ�����ó�ģ���Ʒ���������ļ�֮������Ƿ�ģ���Ʒ�������ֳֺ�IPT�����ࡣ
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
	//���߳��л���Χʱ����MeasureTempCoreTEC�����´�������������֮ǰMeasureTempCoreTEC�и��ӿ�IsChangeRange����ִ�л�û���أ�����ָ�����������
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
			//if (pConf->correctDistanceS == 1)
			//	m_params.fCorrectDistance = pConf->correctDistance;		//�Զ�У�¾���
			//if (pConf->distanceS == 1)
			//	m_params.fDistance = pConf->distance;				//Ŀ�����
			//if (pConf->lowLensCorrKS == 1)
			//	m_params.fLowLensCorrK = pConf->lowLensCorrK;			//���³��µ���Ͳ��Ưϵ��
			//if (pConf->highLensCorrKS == 1)
			//	m_params.fHighLensCorrK = pConf->highLensCorrK;		//���¸��µ���Ͳ��Ưϵ��
			//if (pConf->lowShutterCorrCoffS == 1)
			//	m_params.fLowShutterCorrCoff = pConf->lowShutterCorrCoff;	//���³��µ�������Ưϵ��
			//if (pConf->highShutterCorrCoffS == 1)
			//	m_params.fHighShutterCorrCoff = pConf->highShutterCorrCoff;	//���¸��µ�������Ưϵ��
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
	////��һ֡ʱ�ϴο����¶�Ϊ0����ֵ�ɵ�ǰ�����¶ȡ�
	//if (!m_lensTempWhenShutter)
	//{
	//	m_lensTempWhenShutter = m_paramsTEC.fRealTimeLensTemp;
	//	//��һ֡ʱ���������¸�ֵ
	//	m_paramsTEC.fOrinalShutterTemp = m_paramsTEC.fRealTimeShutterTemp;
	//}
	//if (m_paramsTEC.fOrinalShutterTemp < 1 || m_paramsTEC.fOrinalShutterTemp > 100)
	//{
	//	//�����״λ�ȡ���´���һ������Ч�ģ�һ��ʱ����´����ݲ���ȷ����ʱ����������Ҫ���³���Ч��ֵ��
	//	m_paramsTEC.fOrinalShutterTemp = m_paramsTEC.fRealTimeShutterTemp;
	//}
	////�����ʱ�����ϴ��¶ȡ�
	////���׸��±�־λ�Ǳ�֤���ڿ����Ѿ��պϵ��ȫ�ı�־����ʾ���Ʊ�־λ�Ǹ���������׼��Ҫ������ˡ�
	//if (m_paramsTEC.bNucShutterFlag)
	//{
	//	//���Ÿձպ�ʱ��¼�¶ȣ��������ĵ�Y16
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
	//���µ�λ��ASIC�����д��ݣ���λ�仯ʱ���²���������
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
	m_paramsTEC.mtType = range;	//���û����á��������ͣ�0:�������   1����ҵ���³��¶�    2����ҵ���¸��¶�
	//m_paramsTEC.m_fieldType = lensType;
	m_logger->output(LOG_INFO, "changeRange mtType=%d fieldType=%d", range, lensType);
	/*���벹��ϵ�������趨һ��Ĭ�����ݡ����������ļ�ʱ���³���Ŀʹ�õ�ֵ��*/
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
		m_paramsTEC.nKF,												//������ʱY16��������(���㻯100��,1~100,Ĭ��ֵΪ100)
		m_paramsTEC.nB1,												//������ʱY16��ƫ����(���㻯100��)
		m_paramsTEC.nDistance_a0,										//����У��ϵ��(���㻯10000000000����Ĭ��0)
		m_paramsTEC.nDistance_a1,										//����У��ϵ��(���㻯1000000000����Ĭ��0)
		m_paramsTEC.nDistance_a2,										//����У��ϵ��(���㻯10000000����Ĭ��0)
		m_paramsTEC.nDistance_a3,										//����У��ϵ��(���㻯100000����Ĭ��0)
		m_paramsTEC.nDistance_a4,										//����У��ϵ��(���㻯100000����Ĭ��0)
		m_paramsTEC.nDistance_a5,										//����У��ϵ��(���㻯10000����Ĭ��0)
		m_paramsTEC.nDistance_a6,										//����У��ϵ��(���㻯1000����Ĭ��0)
		m_paramsTEC.nDistance_a7,										//����У��ϵ��(���㻯100����Ĭ��0)
		m_paramsTEC.nDistance_a8,										//����У��ϵ��(���㻯100����Ĭ��0)
		m_paramsTEC.nK1,												//������Ưϵ��(���㻯100����Ĭ��0)
		m_paramsTEC.nK2,												//��Ͳ��Ưϵ��(���㻯100����Ĭ��0)
		m_paramsTEC.nK3,												//��������ϵ��(���㻯10000����Ĭ��0)
		m_paramsTEC.nB2,												//��������ƫ����(���㻯10000����Ĭ��0)
		m_paramsTEC.nKFOffset,											//�Զ�У��KFƫ�����������ָ�����У�����ã�Ĭ��0
		m_paramsTEC.nB1Offset,											//�Զ�У��B1ƫ�����������ָ�����У�����ã�Ĭ��0
		m_paramsTEC.nGear,												//���µ�λ, 0:���µ�, 1:���µ�
		m_paramsTEC.fHumidity,											//ʪ��(���㻯100����Ĭ��60)
		m_paramsTEC.nAtmosphereTransmittance,							//����͸����(���㻯100������Χ0~100��Ĭ��100)
		m_paramsTEC.mtType,												//�������ͣ�0:�������   1����ҵ����
		m_paramsTEC.fEmiss,											//������(0.01-1.0,Ĭ��1.0)
		m_paramsTEC.fDistance,										//���¾��� 
		m_paramsTEC.fReflectT,										//�����¶ȣ����µ�Ĭ��3�����µ�Ĭ��23��
		m_paramsTEC.fAmbient,											//�����¶ȣ�ȡ���������£�
		m_paramsTEC.fWindowTransmittance,								//����͸����(��Χ0~1��Ĭ��1)
		m_paramsTEC.fWindowTemperature,								//�����¶ȣ����µ�Ĭ��3�����µ�Ĭ��23��
		m_paramsTEC.fRealTimeShutterTemp,								//ʵʱ�����¶�
		m_paramsTEC.fOrinalShutterTemp,								//����������
		m_paramsTEC.fRealTimeLensTemp,								//ʵʱ��Ͳ��
		m_paramsTEC.fCurrentLensTemp,									//���һ�δ����ʱ�ľ�Ͳ��
		m_paramsTEC.bHumidityCorrection,								//ʪ���������أ�Ĭ�ϴ򿪡�
		m_paramsTEC.bShutterCorrection,								//�����������أ�Ĭ�ϴ򿪡�
		m_paramsTEC.bLensCorrection,									//��Ͳ�������أ�Ĭ�ϴ򿪡�
		m_paramsTEC.bEmissCorrection,									//�������������أ�Ĭ�ϴ򿪡�
		m_paramsTEC.bDistanceCorrection,								//�����������أ�Ĭ�ϴ򿪡�
		m_paramsTEC.bAmbientCorrection,								//�����������أ�Ĭ�ϴ򿪡�
		m_paramsTEC.bB1Correction,										//B1�������أ�Ĭ�ϴ򿪡�
		m_paramsTEC.bAtmosphereCorrection,								//����͸�����������أ�Ĭ�ϴ򿪡�
		m_paramsTEC.bWindowTransmittanceCorrection,					//����͸���ʿ��أ�Ĭ�ϴ򿪡�
		m_paramsTEC.avgB,
		m_paramsTEC.bOrdinaryShutter,   //��ͨ���ű�־λ
		m_paramsTEC.bNucShutterFlag,    //NUC���ű�־λ
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
	/*��������Ƿ���Ч��Խ��*/
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
		/*����ָ�*/
		//m_paramsTEC.fDistance = myDis;
		if (m_paramsTEC.bLogPrint)
		{
			m_logger->output(LOG_DEBUG, "Matrix:\t%d\t%.2f\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%hhu\t%hhu\t%d",
				y16Array[y16W*(y+h/2)+x+w/2], tempMatrix[w*h/2+w/2],
				m_paramsTEC.sY16Offset,
				m_paramsTEC.nKF,												//������ʱY16��������(���㻯100��,1~100,Ĭ��ֵΪ100)
				m_paramsTEC.nB1,												//������ʱY16��ƫ����(���㻯100��)
				m_paramsTEC.nDistance_a0,										//����У��ϵ��(���㻯10000000000����Ĭ��0)
				m_paramsTEC.nDistance_a1,										//����У��ϵ��(���㻯1000000000����Ĭ��0)
				m_paramsTEC.nDistance_a2,										//����У��ϵ��(���㻯10000000����Ĭ��0)
				m_paramsTEC.nDistance_a3,										//����У��ϵ��(���㻯100000����Ĭ��0)
				m_paramsTEC.nDistance_a4,										//����У��ϵ��(���㻯100000����Ĭ��0)
				m_paramsTEC.nDistance_a5,										//����У��ϵ��(���㻯10000����Ĭ��0)
				m_paramsTEC.nDistance_a6,										//����У��ϵ��(���㻯1000����Ĭ��0)
				m_paramsTEC.nDistance_a7,										//����У��ϵ��(���㻯100����Ĭ��0)
				m_paramsTEC.nDistance_a8,										//����У��ϵ��(���㻯100����Ĭ��0)
				m_paramsTEC.nK1,												//������Ưϵ��(���㻯100����Ĭ��0)
				m_paramsTEC.nK2,												//��Ͳ��Ưϵ��(���㻯100����Ĭ��0)
				m_paramsTEC.nK3,												//��������ϵ��(���㻯10000����Ĭ��0)
				m_paramsTEC.nB2,												//��������ƫ����(���㻯10000����Ĭ��0)
				m_paramsTEC.nKFOffset,											//�Զ�У��KFƫ�����������ָ�����У�����ã�Ĭ��0
				m_paramsTEC.nB1Offset,											//�Զ�У��B1ƫ�����������ָ�����У�����ã�Ĭ��0
				m_paramsTEC.nGear,												//���µ�λ, 0:���µ�, 1:���µ�
				m_paramsTEC.fHumidity,											//ʪ��(���㻯100����Ĭ��60)
				m_paramsTEC.nAtmosphereTransmittance,							//����͸����(���㻯100������Χ0~100��Ĭ��100)
				m_paramsTEC.mtType,												//�������ͣ�0:�������   1����ҵ����
				m_paramsTEC.fEmiss,											//������(0.01-1.0,Ĭ��1.0)
				m_paramsTEC.fDistance,										//���¾��� 
				m_paramsTEC.fReflectT,										//�����¶ȣ����µ�Ĭ��3�����µ�Ĭ��23��
				m_paramsTEC.fAmbient,											//�����¶ȣ�ȡ���������£�
				m_paramsTEC.fWindowTransmittance,								//����͸����(��Χ0~1��Ĭ��1)
				m_paramsTEC.fWindowTemperature,								//�����¶ȣ����µ�Ĭ��3�����µ�Ĭ��23��
				m_paramsTEC.fRealTimeShutterTemp,								//ʵʱ�����¶�
				m_paramsTEC.fOrinalShutterTemp,								//����������
				m_paramsTEC.fRealTimeLensTemp,								//ʵʱ��Ͳ��
				m_paramsTEC.fCurrentLensTemp,									//���һ�δ����ʱ�ľ�Ͳ��
				m_paramsTEC.bHumidityCorrection,								//ʪ���������أ�Ĭ�ϴ򿪡�
				m_paramsTEC.bShutterCorrection,								//�����������أ�Ĭ�ϴ򿪡�
				m_paramsTEC.bLensCorrection,									//��Ͳ�������أ�Ĭ�ϴ򿪡�
				m_paramsTEC.bEmissCorrection,									//�������������أ�Ĭ�ϴ򿪡�
				m_paramsTEC.bDistanceCorrection,								//�����������أ�Ĭ�ϴ򿪡�
				m_paramsTEC.bAmbientCorrection,								//�����������أ�Ĭ�ϴ򿪡�
				m_paramsTEC.bB1Correction,										//B1�������أ�Ĭ�ϴ򿪡�
				m_paramsTEC.bAtmosphereCorrection,								//����͸�����������أ�Ĭ�ϴ򿪡�
				m_paramsTEC.bWindowTransmittanceCorrection,					//����͸���ʿ��أ�Ĭ�ϴ򿪡�
				m_paramsTEC.avgB,
				m_paramsTEC.bOrdinaryShutter,   //��ͨ���ű�־λ
				m_paramsTEC.bNucShutterFlag,    //NUC���ű�־λ
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
	case ITA_SET_DISTANCE:			//����0.5~2.5�ס����Բ����ã�Ĭ��1.5�ס��������ͣ������͡�
		fParam = *(float *)param;
		if (fParam <= 0.0 || fParam > 10000.0)
		{
			return ITA_ARG_OUT_OF_RANGE;
		}
		m_paramsTEC.fDistance = fParam;
		break;
	case ITA_SET_EMISS:				//���÷�����(��ҵר��)0.01-1.0��Ĭ��0.95. �������ͣ������͡�
		fParam = *(float *)param;
		//0.01-1.0
		if (fParam <= 0.0 || fParam > 1.0)
		{
			return ITA_ARG_OUT_OF_RANGE;
		}
		m_paramsTEC.fEmiss = fParam;
		break;
	case ITA_SET_TRANSMIT:			//͸����(��ҵר�ã�0.01-1, default 1)���������ͣ������͡�
		fParam = *(float *)param;
		//0.01-1.0
		if (fParam <= 0.0 || fParam > 1.0)
		{
			return ITA_ARG_OUT_OF_RANGE;
		}
		m_paramsTEC.fWindowTransmittance = fParam;
		break;
	case ITA_REFLECT_TEMP:			//�����¶�(��ҵר��)���������ͣ������͡�
		fParam = *(float *)param;
		m_paramsTEC.fReflectT = fParam;
		break;
	case ITA_SET_ENVIRON:			//���û����¶ȡ����û������õ�����£��ڲ��Զ����㻷�¡�����û����ã���ʹ�����õ�ֵ���������ͣ������͡�
		fParam = *(float *)param;
		if (fParam < -10000 || fParam > 10000)
		{
			return ITA_ARG_OUT_OF_RANGE;
		}
		m_paramsTEC.fAmbient = fParam;
		break;
	case ITA_GET_ENVIRON:			//��ȡ���¡�
		*(float *)param = m_paramsTEC.fAmbient;
		break;
	case ITA_GET_FPA_TEMP:			//��ȡ���¡�
		if (param)
			*(float *)param = m_paramsTEC.fRealTimeShutterTemp;
		else
		{
			ret = ITA_NULL_PTR_ERR;
			m_logger->output(LOG_ERROR, "ITA_GET_FPA_TEMP ret=%d", ret);
		}
		break;
	case ITA_LENS_CORR:				//��Ͳ��ƯУ�����ء�Ĭ�Ϲرա��������ͣ�ITA_SWITCH��
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
	case ITA_SHUTTER_CORR:			//������ƯУ�����ء�Ĭ�Ϲرա��������ͣ�ITA_SWITCH��
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
	case ITA_HIGH_LENS_CORR_K:		//ʵʱ���µ���Ͳ��Ư����ϵ�����������ͣ������͡�
		fParam = *(float *)param;
		m_paramsTEC.nK2 = (int)fParam;
		break;
	case ITA_LOW_LENS_CORR_K:		//ʵʱ���µ���Ͳ��Ư����ϵ�����������ͣ������͡�
		fParam = *(float *)param;
		m_paramsTEC.nK2 = (int)fParam;
		break;
	case ITA_HIGH_SHUTTER_CORR_K:	//���µ�����У��ϵ�����������ͣ������͡�
		fParam = *(float *)param;
		m_paramsTEC.nK1 = (int)fParam;
		break;
	case ITA_LOW_SHUTTER_CORR_K:	//���µ�����У��ϵ�����������ͣ������͡�
		fParam = *(float *)param;
		m_paramsTEC.nK1 = (int)fParam;
		break;
	case ITA_ENVIRON_CORR:			//�����������ء�Ĭ�Ϲرա��������ͣ�ITA_SWITCH��
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
	case ITA_DISTANCE_COMPEN:		//���벹�����ء�Ĭ�ϴ򿪡��������ͣ�ITA_SWITCH��
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
	case ITA_EMISS_CORR:			//������У�����ء�Ĭ�ϴ򿪡��������ͣ�ITA_SWITCH��
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
	case ITA_TRANS_CORR:			//͸����У�����ء�Ĭ�Ϲرա��������ͣ�ITA_SWITCH��
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
	//���ò��²���
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