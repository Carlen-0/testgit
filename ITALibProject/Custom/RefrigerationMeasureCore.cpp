#include "RefrigerationMeasureCore.h"
#include <math.h>
#include <string.h>

// ������ �ֳֺ�IPT���²�Ʒ���µ�λһ��ֻ�����������������Խ��²���仯�ܴ��ֳִ�TEC���豸������TEC�ѽ��¿�����������Ҫ���¶ȡ�
// �����豸�ǰ�̽�����Ľ��¿����ڽӽ�����0�㣬ֻ��һ�����ɱ��߼�ʮ����
#ifdef REFRIGER_EDITION

RefrigerationMeasureCore::RefrigerationMeasureCore(int w, int h, short *pusCurve, int curveLength)
{
	//��ȡͼ����
	ImageWidth = w;
	ImageHidth = h;
	//�������߳���
	curveDataLength = curveLength;
	//ͼ��Y16��ֵ
	//pSrcY16 = y16Image;
	//���߸�ֵ
	CurveData = pusCurve;
	//���²�������ֵ
	memset(&refrigerationTMparams, 0, sizeof(ITARefrigerMeasureParam));
	refrigerationTMparams.nKF = 100;
	refrigerationTMparams.nHumidity = 60;
	refrigerationTMparams.nAtmosphereTransmittance = 100;
	refrigerationTMparams.nCurrentB = 20.0;
	refrigerationTMparams.fEmiss = 1.0;
	refrigerationTMparams.fDistance = 3.0;
	refrigerationTMparams.fReflectT = 23.0;
	refrigerationTMparams.fAmbient = 23.0;
	refrigerationTMparams.fAtmosphereTemperature = 23.0;
	refrigerationTMparams.fWindowTransmittance = 1.0;
	refrigerationTMparams.fWindowTemperature = 23.0;
	refrigerationTMparams.bHumidityCorrection = true;
	refrigerationTMparams.bAmbientCorrection = true;
	refrigerationTMparams.bAtmosphereCorrection = true;
	refrigerationTMparams.bB1Correction = true;
	refrigerationTMparams.bDistanceCorrection = true;
	refrigerationTMparams.bEmissCorrection = true;
	refrigerationTMparams.bLensCorrection = true;
	refrigerationTMparams.bWindowTransmittanceCorrection = true;
	m_logger = new GuideLog();
	m_debug = false;
	psKMaxtrix = NULL;
	psStationaryB = NULL;
	m_matrixLen = 0;
}

/*�������ܣ���������
���������
��
���ز�����
��
*/
RefrigerationMeasureCore::~RefrigerationMeasureCore() 
{
	if (m_logger)
	{
		delete m_logger;
		m_logger = nullptr;
	}
}

void RefrigerationMeasureCore::setParams(ITARefrigerMeasureParam params)
{
	memcpy(&refrigerationTMparams, &params, sizeof(ITARefrigerMeasureParam));
}

void RefrigerationMeasureCore::getParams(ITARefrigerMeasureParam * params)
{
	memcpy(params, &refrigerationTMparams, sizeof(ITARefrigerMeasureParam));
}

void RefrigerationMeasureCore::setCurve(short * pusCurve, int curveLength)
{
	//�������߳���
	curveDataLength = curveLength;
	//���߸�ֵ
	CurveData = pusCurve;
}

/*�������ܣ����½ӿڣ���Y16����ת��Ϊ�¶�
���������
short y16value��	ԭʼY16
���ز�����
float &Temp��		Y16��Ӧ���¶�
*/
void RefrigerationMeasureCore::GetTempByY16(float &Temp, short y16value)
{

	int CurrentBTemp;
	//���鱾�׶�Ӧ��Y16
	CurrentBTemp = int(refrigerationTMparams.nCurrentB * 10);

	int TempIndex = 0;
	for (int i = 0; i < curveDataLength; i++)
	{
		if (CurveData[i] > CurrentBTemp)
		{
			TempIndex = i;
			break;
		}
	}
	for (int i = curveDataLength - 1; i > 0; i--)
	{
		if (CurveData[i] < CurrentBTemp)
		{
			refrigerationTMparams.nBCurveIndex = i;
			break;
		}
	}
	m_logger->output(LOG_INFO, "minCurve:%d,maxCurve:%d", CurveData[0], CurveData[16383]);

	m_logger->output(LOG_INFO, "y16value:%d, CurrentBTemp:%d, nBCurveIndexbig:%d, nCurrentB:%.2f", y16value, CurrentBTemp, refrigerationTMparams.nBCurveIndex, refrigerationTMparams.nCurrentB);
	refrigerationTMparams.nBCurveIndex = int((refrigerationTMparams.nBCurveIndex + TempIndex) / 2);
	m_logger->output(LOG_INFO, "nBCurveIndexsmall:%d,nBCurveIndex:%d", TempIndex,refrigerationTMparams.nBCurveIndex);
	//Y16����
	y16value += refrigerationTMparams.sY16Offset;
	m_logger->output(LOG_INFO, "Y16Offset:%d,y16value:%d", refrigerationTMparams.sY16Offset, y16value);

	//ʪ������
	m_logger->output(LOG_INFO, "bHumidityCorrection:%d", refrigerationTMparams.bHumidityCorrection);
	if (refrigerationTMparams.bHumidityCorrection)
	{
		y16value = HumidityCorrection(y16value, refrigerationTMparams.nHumidity);
	}
	m_logger->output(LOG_INFO, "nHumidity:%d,y16value:%d", refrigerationTMparams.nHumidity, y16value);

	//float fTemp;
	//��Ư����
	Temp = StandardY16ToTempWithY16Correction(y16value);
	m_logger->output(LOG_INFO, "WP Cor:%.2f", Temp);

	//B1����
	m_logger->output(LOG_INFO, "bB1Correction:%d", refrigerationTMparams.bB1Correction);
	if (refrigerationTMparams.bB1Correction)
	{
		Temp = Temp + ((refrigerationTMparams.nB1 + refrigerationTMparams.nB1Offset) / 100.0f);
	}
	m_logger->output(LOG_INFO, "B1:%d, B1Offset:%d, B1 Cor:%.2f", refrigerationTMparams.nB1, refrigerationTMparams.nB1Offset, Temp);

	//��������
	m_logger->output(LOG_INFO, "bDistanceCorrection:%d", refrigerationTMparams.bDistanceCorrection);
	if (refrigerationTMparams.bDistanceCorrection)
	{
		if (refrigerationTMparams.nDistance_a6 == -1 && refrigerationTMparams.nDistance_a7 == -1 && refrigerationTMparams.nDistance_a8 == -1)
		{
			Temp = DistanceCorrectionWithSixPara(Temp);
			m_logger->output(LOG_INFO, "dis Cor:%.2f in6", Temp);
		}
		else
		{
			Temp = DistanceCorrectionWithNinePara(Temp);
			m_logger->output(LOG_INFO, "dis Cor:%.2f in9", Temp);
		}
	}
	m_logger->output(LOG_INFO, "dis Cor:%.2f", Temp);

	//����������
	m_logger->output(LOG_INFO, "bEmissCorrection:%d", refrigerationTMparams.bEmissCorrection);
	if (refrigerationTMparams.bEmissCorrection)
	{
		Temp = EmissionCorrectionNew(Temp);
	}
	m_logger->output(LOG_INFO, "emiss:%.2f, emis Cor:%.2f", refrigerationTMparams.fEmiss, Temp);

	//��������
	m_logger->output(LOG_INFO, "bAmbientCorrection:%d", refrigerationTMparams.bAmbientCorrection);
	if (refrigerationTMparams.bAmbientCorrection)
	{
		Temp = EnvironmentCorrection(Temp);
	}
	m_logger->output(LOG_INFO, "ambient Cor:%.2f", Temp);

	//����͸��������
	m_logger->output(LOG_INFO, "bAtmosphereCorrection:%d", refrigerationTMparams.bAtmosphereCorrection);
	if (refrigerationTMparams.bAtmosphereCorrection)
	{
		Temp = AtmosphereCorrection(Temp);
	}
	m_logger->output(LOG_INFO, "AtmosphereTransmittance:%d, AtmosphereTemperature:%.2f, atos Cor:%.2f", refrigerationTMparams.nAtmosphereTransmittance, refrigerationTMparams.fAtmosphereTemperature, Temp);

	//����͸��������
	m_logger->output(LOG_INFO, "bWindowTransmittanceCorrection:%d", refrigerationTMparams.bWindowTransmittanceCorrection);
	if (refrigerationTMparams.bWindowTransmittanceCorrection)
	{
		Temp = WindowTransmittanceCorrection(Temp);
	}
	m_logger->output(LOG_INFO, "WindowTransmittance:%.2f, WindowTemperature:%.2f, windows Cor:%.2f", refrigerationTMparams.fWindowTransmittance, refrigerationTMparams.fWindowTemperature, Temp);
}


/*�������ܣ��¶ȷ���Y16�����¶�����ת��Ϊy16
���������
float &Temp��		ԭʼ�¶�
���ز�����
short y16value��	�¶ȶ�Ӧ��Y16
*/
void RefrigerationMeasureCore::CalY16ByTemp(short &y16, float fTemp)
{
	m_logger->output(LOG_INFO, "Input Temp:%f\n", fTemp);
	//����͸������������
	if (refrigerationTMparams.bWindowTransmittanceCorrection)
	{
		fTemp = WindowTransmittanceReverseCorrection(fTemp);
		m_logger->output(LOG_INFO, "Temp after WindowTransmittanceReverseCorrection:%f", fTemp);
	}

	//����͸������������
	if (refrigerationTMparams.bAtmosphereCorrection)
	{
		fTemp = AtmosphereReverseCorrection(fTemp);
		m_logger->output(LOG_INFO, "Temp after AtmosphereReverseCorrection:%f", fTemp);
	}

	//������������
	if (refrigerationTMparams.bAmbientCorrection)
	{
		fTemp = EnvironmentReverseCorrection(fTemp);
		m_logger->output(LOG_INFO, "Temp after EnvironmentReverseCorrection:%f", fTemp);
	}

	//��������������
	if (refrigerationTMparams.bEmissCorrection)
	{
		fTemp = EmissionReverseCorrectionNew(fTemp);
		m_logger->output(LOG_INFO, "Temp after EmissionReverseCorrectionNew:%f", fTemp);
	}

	//������������
	if (refrigerationTMparams.bDistanceCorrection)
	{
		if (refrigerationTMparams.nDistance_a6 == -1 && refrigerationTMparams.nDistance_a7 == -1 && refrigerationTMparams.nDistance_a8 == -1)
		{
			fTemp = DistanceReverseCorrectionWithSixPara(fTemp);
			m_logger->output(LOG_INFO, "Temp after DistanceReverseCorrectionWithSixPara:%f", fTemp);
		}
		else
		{
			fTemp = DistanceReverseCorrectionWithNinePara(fTemp);
			m_logger->output(LOG_INFO, "Temp after DistanceReverseCorrectionWithNinePara:%f", fTemp);
		}
	}

	//B1��������
	if (refrigerationTMparams.bB1Correction)
	{
		fTemp = fTemp - ((refrigerationTMparams.nB1 + refrigerationTMparams.nB1Offset) / 100.0f);
		m_logger->output(LOG_INFO, "Temp after bB1Correction:%f", fTemp);
	}

	//��Ư��������
	short y16_min = StandardTempToY16(fTemp, false);
	short y16_max = StandardTempToY16(fTemp, true);
	y16 = short((y16_min + y16_max) / 2);
	m_logger->output(LOG_INFO, "y16_min:%d,y16_max:%d,y16:%d", y16_min, y16_max, y16);


	//ʪ����������
	if (refrigerationTMparams.bHumidityCorrection)
	{
		y16 = HumidityReverseCorrection(y16, refrigerationTMparams.nHumidity);
		m_logger->output(LOG_INFO, "y16 after HumidityReverseCorrection:%d", y16);
	}
	y16 -= refrigerationTMparams.sY16Offset;
	m_logger->output(LOG_INFO, "y16 after sY16Offset:%d", y16);
}



/*�������ܣ�Y16����������������Ư��������ͷ��Ư������KF����
���������
short y16value��		ԭʼY16
���ز�����
float temp��			Y16��Ӧ���¶�
*/
float RefrigerationMeasureCore::StandardY16ToTempWithY16Correction(short y16value) 
{
	if (refrigerationTMparams.bLensCorrection)
	{
		y16value = y16value - (short)(refrigerationTMparams.nK2 / 100.f * (refrigerationTMparams.fRealTimeLensTemp - refrigerationTMparams.fOriginLensTemp));
	}
	if (refrigerationTMparams.bShutterCorrection)
	{
		y16value = y16value - (short)(refrigerationTMparams.nK1 / 100.f * (refrigerationTMparams.fRealShutterTemp - refrigerationTMparams.fOriShutterTemp));
	}

	int index = (int)(y16value * ((refrigerationTMparams.nKF + refrigerationTMparams.nKFOffset) / 100.f)) + refrigerationTMparams.nBCurveIndex;
	
	if (index < 0) 
	{
		index = 0;
	}
	else if (index >= curveDataLength) 
	{
		index = curveDataLength - 1;
	}

	m_logger->output(LOG_INFO, "WP in----KF:%d, nKFOffset:%d, K2:%d, RealTimeLens:%.2f, CurrentLensTemp:%.2f, index:%d, CurveMin:%d, CurveMax:%d, %d %d %.2f %.2f %.2f", refrigerationTMparams.nKF, refrigerationTMparams.nKFOffset, refrigerationTMparams.nK2,
		refrigerationTMparams.fRealTimeLensTemp, refrigerationTMparams.fCurrentLensTemp, index, CurveData[0], CurveData[16383], refrigerationTMparams.bShutterCorrection, refrigerationTMparams.nK1, refrigerationTMparams.fRealShutterTemp, refrigerationTMparams.fOriShutterTemp, refrigerationTMparams.fOriginLensTemp);
	int temp = CurveData[index];
	m_logger->output(LOG_INFO, "WP in----temp:%d", temp);

	return temp / 10.0f;

}


/*�������ܣ�ʪ������
���������
float fObjY16��			Ŀ��Y16
float fHumidity��		ʪ��ֵ
���ز���
short Y16��				������Y16
*/
short RefrigerationMeasureCore::HumidityCorrection(short fObjY16, int fHumidity)
{
	short Y16 = 0;
	Y16 = short((1.0f - (fHumidity - 60) / 5000.0f) * fObjY16);
	m_logger->output(LOG_INFO, "HumidityCorrection in----- fObjY16:%d,Y16:%d", fObjY16, Y16);
	return Y16;
}


/*�������ܣ���Ͳ��Ư�������Զ����㾵Ͳ��Ư
���������
��
���ز���
��
*/
void RefrigerationMeasureCore::LensTempDriftCorr()
{
	
}

/*�������ܣ�����������
���������
float iTemperature��		����������ǰ�¶�
���ز���
float Temp��				�������������¶�
*/
float RefrigerationMeasureCore::EmissionCorrectionNew(float iTemperature)
{
	short sY16Reflect = GetY16FromT(int(refrigerationTMparams.fReflectT * 10));
	short nTNoEmiss = short(iTemperature * 10);
	int nEmiss = int(refrigerationTMparams.fEmiss * 100);//
	short TEmiss = EmissCor(nTNoEmiss, sY16Reflect, nEmiss);
	//short DeTEmiss = DeEmissCor(nTNoEmiss, sY16Reflect, nEmiss);
	return TEmiss / 10.0f;
}

short RefrigerationMeasureCore::EmissCor(short nTNoEmiss, short sY16Reflect, int nEmiss)
{
	//С��0�쳣��ֱ�ӷ��������
	if (nEmiss <= 0)
		return nEmissCurve[nEmissCurveLen - 1];
	//�����ʴ���0.98������������У��
	if (nEmiss > 98)
		return nTNoEmiss;

	int nY16NoEmiss;
	int nTEmiss, nTEmissFloor, nTEmissCeil;
	int nEmissIndexFloor, nEmissIndexCeil;
	//float fEmissIndex, fY16NoEmiss;
	float fEmissIndex, fY16NoEmiss;
	/*short Y16NoEmiss = GetY16FromT(TNoEmiss, mBoschParam, NewCurveBuffer);*/

	//�õ�δ��������У�����¶ȶ�Ӧ�ķ�����
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

	//��߾��ȣ�������nY16NoEmissת�ɸ�����fY16NoEmiss //��ֵ��С�¶Ȳ����ķ�ֵ
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

	//����������
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

	//��ֵ��С�¶Ȳ����ķ�ֵ
	nTEmiss = int(nTEmissFloor + (nTEmissCeil - nTEmissFloor) * (fEmissIndex - nEmissIndexFloor));

	return nTEmiss;
}


/*�������ܣ����鷢��������
���������
int ReflectT��			�����¶�
���ز���
short nReflectY16��		�����¶ȶ�Ӧ��Y16
*/
short RefrigerationMeasureCore::GetY16FromT(int ReflectT)
{
	int nReflectY16;
	//�õ�δ��������У�����¶ȶ�Ӧ��Y16
	if (ReflectT >= nEmissCurve[nEmissCurveLen - 1])
	{
		nReflectY16 = nEmissCurveLen - 1;
		return nReflectY16;
	}
	else if (ReflectT <= nEmissCurve[0])
	{
		nReflectY16 = 0;
		return nReflectY16;
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


/*�������ܣ�����������6������
���������
float iTemperature��		��������ǰ�¶�
���ز���
float Temp��				�����������¶�
*/
float RefrigerationMeasureCore::DistanceCorrectionWithSixPara(float iTemperature)
{
	m_logger->output(LOG_INFO, "distance model:six, a0:%d, a1:%d, a2:%d, a3:%d, a4:%d, a5:%d", refrigerationTMparams.nDistance_a0, refrigerationTMparams.nDistance_a1, refrigerationTMparams.nDistance_a2,
		refrigerationTMparams.nDistance_a3, refrigerationTMparams.nDistance_a4, refrigerationTMparams.nDistance_a5);
	double a0 = refrigerationTMparams.nDistance_a0 / 1000000000.0;
	double a1 = refrigerationTMparams.nDistance_a1 / 1000000000.0;
	double b0 = refrigerationTMparams.nDistance_a2 / 1000000.0;
	double b1 = refrigerationTMparams.nDistance_a3 / 1000000.0;
	double c0 = refrigerationTMparams.nDistance_a4 / 1000.0;
	double c1 = refrigerationTMparams.nDistance_a5 / 1000.0;


	double a = a0 * refrigerationTMparams.fDistance + a1;
	double b = b0 * refrigerationTMparams.fDistance + b1;
	double c = c0 * refrigerationTMparams.fDistance + c1;

	if (a == 0 && b == 0) {
		return iTemperature;
	}
	else {
		float x = iTemperature + 273.5f;
		float y = float(a * x * x + b * x + c);

		return y / 10.0f - 273.5f;
	}
}

/*�������ܣ�����������9������
���������
float iTemperature��		��������ǰ�¶�
���ز���
float Temp��				�����������¶�
*/
float RefrigerationMeasureCore::DistanceCorrectionWithNinePara(float iTemperature)
{
	m_logger->output(LOG_INFO, "distance model:nine, a0:%d, a1:%d, a2:%d, a3:%d, a4:%d, a5:%d, a6:%d, a7:%d, a8:%d", refrigerationTMparams.nDistance_a0, refrigerationTMparams.nDistance_a1, refrigerationTMparams.nDistance_a2,
		refrigerationTMparams.nDistance_a3, refrigerationTMparams.nDistance_a4, refrigerationTMparams.nDistance_a5, refrigerationTMparams.nDistance_a6, refrigerationTMparams.nDistance_a7, refrigerationTMparams.nDistance_a8);
	double a0 = refrigerationTMparams.nDistance_a0 / 10000000000.0;
	double a1 = refrigerationTMparams.nDistance_a1 / 1000000000.0;
	double a2 = refrigerationTMparams.nDistance_a2 / 10000000.0;
	double b0 = refrigerationTMparams.nDistance_a3 / 100000.0;
	double b1 = refrigerationTMparams.nDistance_a4 / 100000.0;
	double b2 = refrigerationTMparams.nDistance_a5 / 10000.0;
	double c0 = refrigerationTMparams.nDistance_a6 / 1000.0;
	double c1 = refrigerationTMparams.nDistance_a7 / 100.0;
	double c2 = refrigerationTMparams.nDistance_a8 / 100.0;

	double a = a0 * refrigerationTMparams.fDistance * refrigerationTMparams.fDistance + a1 * refrigerationTMparams.fDistance + a2;
	double b = b0 * refrigerationTMparams.fDistance * refrigerationTMparams.fDistance + b1 * refrigerationTMparams.fDistance + b2;
	double c = c0 * refrigerationTMparams.fDistance * refrigerationTMparams.fDistance + c1 * refrigerationTMparams.fDistance + c2;

	if (a == 0 && b == 0) {
		return iTemperature;
	}
	else {
		float x = iTemperature + 273.5f;
		float y = float(a * x * x + b * x + c);

		return y / 10.0f - 273.5f;
	}
}

/*�������ܣ���������
���������
float iTemperature��		��������ǰ�¶�
���ز���
float Temp��				�����������¶�
*/
float RefrigerationMeasureCore::EnvironmentCorrection(float temp)
{
	m_logger->output(LOG_INFO, "K3:%d, B2:%d, Gear:%d, ambient:%.2f", refrigerationTMparams.nK3, refrigerationTMparams.nB2, refrigerationTMparams.nGear, refrigerationTMparams.fAmbient);
	float correction = refrigerationTMparams.nK3 / 10000.f * temp + refrigerationTMparams.nB2 / 10000.f;
	float delta = 0.0f;
	if (refrigerationTMparams.nGear == 0) {
		delta = (refrigerationTMparams.fAmbient - 3) * correction;
	}
	else if (refrigerationTMparams.nGear == 1) {
		delta = (refrigerationTMparams.fAmbient - 23) * correction;
	}
	return temp - delta;
}

/*�������ܣ���������������
���������
float iTemperature��		�������������¶�
���ز���
float Temp��				����������ǰ�¶�
*/
float RefrigerationMeasureCore::EmissionReverseCorrectionNew(float iTemperature)
{
	short sY16Reflect = GetY16FromT(int(refrigerationTMparams.fReflectT * 10));
	short nTNoEmiss = short(iTemperature * 10);
	int nEmiss = int(refrigerationTMparams.fEmiss * 100);//
	short DeTEmiss = DeEmissCor(nTNoEmiss, sY16Reflect, nEmiss);
	return DeTEmiss / 10.0f;
}

short RefrigerationMeasureCore::DeEmissCor(short nTEmiss, short sY16Reflect, int nEmiss)
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

	/******************20210810 �ķ����ʷ��㸡��ȡ������********************************/
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

	//����������
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

	//��ֵ��С�¶Ȳ����ķ�ֵ
	nTNoEmiss = int(nTNoEmissFloor + (nTNoEmissCeil - nTNoEmissFloor) * (fNoEmissIndex - nNoEmissIndexFloor));

	return nTNoEmiss;
	/******************20210810 �ķ����ʷ��㸡��ȡ������********************************/
}

//������������--�Ų���
float RefrigerationMeasureCore::DistanceReverseCorrectionWithNinePara(float iTemperature)
{
	if (refrigerationTMparams.fDistance > 0)
	{
		double a0 = refrigerationTMparams.nDistance_a0 / 10000000000.0;
		double a1 = refrigerationTMparams.nDistance_a1 / 1000000000.0;
		double a2 = refrigerationTMparams.nDistance_a2 / 10000000.0;
		double b0 = refrigerationTMparams.nDistance_a3 / 100000.0;
		double b1 = refrigerationTMparams.nDistance_a4 / 100000.0;
		double b2 = refrigerationTMparams.nDistance_a5 / 10000.0;
		double c0 = refrigerationTMparams.nDistance_a6 / 1000.0;
		double c1 = refrigerationTMparams.nDistance_a7 / 100.0;
		double c2 = refrigerationTMparams.nDistance_a8 / 100.0;

		double a = a0 * refrigerationTMparams.fDistance * refrigerationTMparams.fDistance + a1 * refrigerationTMparams.fDistance + a2;
		double b = b0 * refrigerationTMparams.fDistance * refrigerationTMparams.fDistance + b1 * refrigerationTMparams.fDistance + b2;
		double c = c0 * refrigerationTMparams.fDistance * refrigerationTMparams.fDistance + c1 * refrigerationTMparams.fDistance + c2;

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
			//���Ԫһ�η���
			y = float((pow(b * b + 4 * a * after - 4 * a * c, (float)0.5) - b) / a / 2);

			return y - 273.5f;
		}
	}
	else
	{
		return iTemperature;
	}
}


//������������--������
float RefrigerationMeasureCore::DistanceReverseCorrectionWithSixPara(float iTemperature)
{
	if (refrigerationTMparams.fDistance > 0)
	{
		double a0 = refrigerationTMparams.nDistance_a0 / 1000000000.0;
		double a1 = refrigerationTMparams.nDistance_a1 / 1000000000.0;
		double b0 = refrigerationTMparams.nDistance_a2 / 1000000.0;
		double b1 = refrigerationTMparams.nDistance_a3 / 1000000.0;
		double c0 = refrigerationTMparams.nDistance_a4 / 1000.0;
		double c1 = refrigerationTMparams.nDistance_a5 / 1000.0;

		double a = a0 * refrigerationTMparams.fDistance + a1;
		double b = b0 * refrigerationTMparams.fDistance + b1;
		double c = c0 * refrigerationTMparams.fDistance + c1;

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
			//���Ԫһ�η���
			y = float((pow(b * b + 4 * a * after - 4 * a * c, (float)0.5) - b) / a / 2);

			return y - 273.5f;
		}
	}
	else
	{
		return iTemperature;
	}
}

//ʪ����������
short RefrigerationMeasureCore::HumidityReverseCorrection(short y16, int fHumidity)
{
	short y = 0;
	y = short(y16 * (5000.0f / (5060 - fHumidity)));
	return y;
}

//������������
float  RefrigerationMeasureCore::EnvironmentReverseCorrection(float temp) 
{
	float tempz = 0.f;
	if (refrigerationTMparams.nGear == 0) {
		tempz = (temp + (refrigerationTMparams.nB2 / 10000.f)*(refrigerationTMparams.fAmbient - 3)) / (1 - (refrigerationTMparams.fAmbient - 3)*(refrigerationTMparams.nK3 / 10000.f));
		return tempz;
	}
	else if (refrigerationTMparams.nGear == 1) {
		tempz = (temp + (refrigerationTMparams.nB2 / 10000.f)* (refrigerationTMparams.fAmbient - 23)) / (1 - (refrigerationTMparams.fAmbient - 23)*(refrigerationTMparams.nK3 / 10000.f));
		return tempz;
	}
	else
	{
		return temp;
	}
	
}


//��Ư��������
short RefrigerationMeasureCore::StandardTempToY16(float iTemperature, bool fromSmallToBig)
{
	
	//��������
	int temp;
	temp = (int)(iTemperature * 10);

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

	int CurrentBTemp;
	//���鱾�׶�Ӧ��Y16
	CurrentBTemp = int(refrigerationTMparams.nCurrentB * 10);

	int TempIndex = 0;
	for (int i = 0; i < curveDataLength; i++)
	{
		if (CurveData[i] > CurrentBTemp)
		{
			TempIndex = i;
			break;
		}
	}
	for (int i = curveDataLength - 1; i > 0; i--)
	{
		if (CurveData[i] < CurrentBTemp)
		{
			refrigerationTMparams.nBCurveIndex = i;
			break;
		}
	}
	refrigerationTMparams.nBCurveIndex = int((refrigerationTMparams.nBCurveIndex + TempIndex) / 2);

	int y16value = (int)((index - refrigerationTMparams.nBCurveIndex) / ((refrigerationTMparams.nKF + refrigerationTMparams.nKFOffset) / 100.0));


	if (refrigerationTMparams.bLensCorrection) {
		y16value = y16value + (short)(refrigerationTMparams.nK2 / 100.f * (refrigerationTMparams.fRealTimeLensTemp - refrigerationTMparams.fOriginLensTemp));
	}

	if (refrigerationTMparams.bShutterCorrection)
	{
		y16value = y16value + (short)(refrigerationTMparams.nK1 / 100.f * (refrigerationTMparams.fRealShutterTemp - refrigerationTMparams.fOriShutterTemp));
	}

	if (y16value > (short)MAXSHORT)
	{
		y16value = (short)MAXSHORT;
	}
	else if (y16value < (short)MINSHORT)
	{
		y16value = (short)MINSHORT;
	}

	return y16value;
}

//����͸��������
float RefrigerationMeasureCore::AtmosphereCorrection(float iTemperature)
{
	short sY16Reflect = GetY16FromT(int(refrigerationTMparams.fAtmosphereTemperature * 10));
	short nTNoAtmosphere = short(iTemperature * 10);
	int nTransmittance = int(refrigerationTMparams.nAtmosphereTransmittance);//
	short TEmiss = EmissCor(nTNoAtmosphere, sY16Reflect, nTransmittance);
	//short DeTEmiss = DeEmissCor(nTNoEmiss, sY16Reflect, nEmiss);
	return TEmiss / 10.0f;
}

//����͸���ʷ���
float RefrigerationMeasureCore::AtmosphereReverseCorrection(float iTemperature)
{
	short sY16Reflect = GetY16FromT(int(refrigerationTMparams.fAtmosphereTemperature * 10));
	short nTNoAtmosphere = short(iTemperature * 10);
	int nTransmittance = int(refrigerationTMparams.nAtmosphereTransmittance);//
	short DeTEmiss = DeEmissCor(nTNoAtmosphere, sY16Reflect, nTransmittance);
	return DeTEmiss / 10.0f;
}


//����͸��������
float RefrigerationMeasureCore::WindowTransmittanceCorrection(float iTemperature)
{
	short sY16Windows = GetY16FromT(int(refrigerationTMparams.fWindowTemperature * 10));
	short nTNoEmiss = short(iTemperature * 10);
	int nWindowTransmittance = int(refrigerationTMparams.fWindowTransmittance * 100);//
	short TEmiss = EmissCor(nTNoEmiss, sY16Windows, nWindowTransmittance);
	return TEmiss / 10.0f;
}

//����͸������������
float RefrigerationMeasureCore::WindowTransmittanceReverseCorrection(float iTemperature)
{
	short sY16Windows = GetY16FromT(int(refrigerationTMparams.fWindowTemperature * 10));
	short nTNoEmiss = short(iTemperature * 10);
	int nWindowTransmittance = int(refrigerationTMparams.fWindowTransmittance * 100);//
	short DeTEmiss = DeEmissCor(nTNoEmiss, sY16Windows, nWindowTransmittance);
	return DeTEmiss / 10.0f;
}


//�����¶Ⱦ���ӿ�(ȫͼ�������)
int RefrigerationMeasureCore::GetTempMatrix(short *pSrc, float *pTempMatrix, int nImageWidth, int nImageHeight)
{
	short sMin = 32767;
	short sMax = -32767;
	int nLen = nImageWidth * nImageHeight;
	memset(pTempMatrix, 0, nLen * sizeof(float));

	// ����ȫͼ���ADֵ�����ADֵ
	for (int i = 0; i < nLen; i++)
	{
		sMin = gdmin(pSrc[i], sMin);
		sMax = gdmax(pSrc[i], sMax);
	}

	int tLen = sMax - sMin + 1;
	float *fTemp = (float*)malloc(tLen * sizeof(float));
	memset(fTemp, 0, tLen * sizeof(float));

	for (int i = sMin; i <= sMax; i++)
	{
		GetTempByY16(fTemp[i - sMin], i);
	}

	for (int i = 0; i < nLen; i++)
	{
		pTempMatrix[i] = fTemp[pSrc[i] - sMin];
	}

	if (fTemp)
	{
		free(fTemp);
	}
	fTemp = NULL;

	return 0;
}


//�����¶Ⱦ���ӿ�(ROI�����������)
int RefrigerationMeasureCore::GetTempMatrix(short *pSrc, float *pTempMatrix, int nImageWidth, int nImageHeight, int x, int y, int ROIWidth, int ROIHeight, float ROIDistance, float ROIEmiss)
{
	//��¼ԭ�еľ���ͷ�����
	float TempDistance = refrigerationTMparams.fDistance;
	float TempEmis = refrigerationTMparams.fEmiss;

	//�������еľ��롢�����ʽ��и���
	refrigerationTMparams.fDistance = ROIDistance;
	refrigerationTMparams.fEmiss = ROIEmiss;
	
	short sMin = 32767;
	short sMax = -32767;
	memset(pTempMatrix, 0, ROIWidth * ROIHeight * sizeof(float));

	// ����ROI�������ADֵ�����ADֵ
	for (int i = y; i < y + ROIHeight; i++)
	{
		for (int j = x; j < x + ROIWidth; j++)
		{
			sMin = gdmin(pSrc[i * nImageWidth + j], sMin);
			sMax = gdmax(pSrc[i * nImageWidth + j], sMax);
		}
	}

	int tLen = sMax - sMin + 1;
	float *fTemp = (float*)malloc(tLen * sizeof(float));
	memset(fTemp, 0, tLen * sizeof(float));

	for (long i = sMin; i <= sMax; i++)
	{
		GetTempByY16(fTemp[i - sMin], short(i));
	}

	for (int i = y; i < y + ROIHeight; i++)
	{
		for (int j = x; j < x + ROIWidth; j++)
		{
			pTempMatrix[(i - y) * ROIWidth + j - x] = fTemp[pSrc[i * nImageWidth + j] - sMin];
		}
	}

	//���롢�����ʻ�ԭ
	refrigerationTMparams.fDistance = TempDistance;
	refrigerationTMparams.fEmiss = TempEmis;

	if (fTemp)
	{
		free(fTemp);
	}
	fTemp = NULL;

	return 0;
}



//�����¶Ⱦ���ӿ�(ȫͼ���ټ���)
int RefrigerationMeasureCore::GetTempMatrixFast(short *pSrc, float *pTempMatrix, int nImageWidth, int nImageHeight)
{
	short sMin = 32767;
	short sMax = -32767;
	int nLen = nImageWidth * nImageHeight;
	memset(pTempMatrix, 0, nLen * sizeof(float));


	// ����ȫͼ���ADֵ�����ADֵ
	for (int i = 0; i < nLen; i++)
	{
		sMin = gdmin(pSrc[i], sMin);
		sMax = gdmax(pSrc[i], sMax);
	}

	// ��Ϊ NSEPERATENUM ��С�Σ�ÿһ�ε���һ�β��½ӿ�
	short sValue[NSEPERATENUM] = { 0 };
	float fTemp[NSEPERATENUM] = { 0 };

	for (int i = 0; i < NSEPERATENUM; i++)
	{
		sValue[i] = short(sMin + i * (sMax - sMin) *1.0f / (NSEPERATENUM - 1));

		GetTempByY16(fTemp[i], sValue[i]);
	}

	// Ԥ�ȼ��������
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

	// ȫͼ����
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


//�����¶Ⱦ���ӿ�(ROI������ټ���)
//x��y��ʼλ�ô�0,0��ʼ�����Ϊ�ֱ��ʣ�
int RefrigerationMeasureCore::GetTempMatrixFast(short *pSrc, float *pTempMatrix, int nImageWidth, int nImageHeight, int x, int y, int ROIWidth, int ROIHeight, float ROIDistance, float ROIEmiss)
{
	//��¼ԭ�еľ���ͷ�����
	float TempDistance = refrigerationTMparams.fDistance;
	float TempEmis = refrigerationTMparams.fEmiss;

	//�������еľ��롢�����ʽ��и���
	refrigerationTMparams.fDistance = ROIDistance;
	refrigerationTMparams.fEmiss = ROIEmiss;

	short sMin = 32767;
	short sMax = -32767;
	int nLen = ROIWidth * ROIHeight;
	memset(pTempMatrix, 0, nLen * sizeof(float));

	// ����ROI�������ADֵ�����ADֵ
	for (int i = y; i <= y + ROIHeight; i++)
	{
		for (int j = x; j <= x + ROIWidth; j++)
		{
			sMin = gdmin(pSrc[i * nImageWidth + j], sMin);
			sMax = gdmax(pSrc[i * nImageWidth + j], sMax);
		}
	}

	// ��Ϊ NSEPERATENUM ��С�Σ�ÿһ�ε���һ�β��½ӿ�
	short sValue[NSEPERATENUM] = { 0 };
	float fTemp[NSEPERATENUM] = { 0 };

	for (int i = 0; i < NSEPERATENUM; i++)
	{
		sValue[i] = short(sMin + i * (sMax - sMin) *1.0f / (NSEPERATENUM - 1));

		GetTempByY16(fTemp[i], sValue[i]);
	}

	// Ԥ�ȼ��������
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

	// �������
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

	//���롢�����ʻ�ԭ
	refrigerationTMparams.fDistance = TempDistance;
	refrigerationTMparams.fEmiss = TempEmis;

	return 0;
}


//ʹ���²�����ȡ�¶Ⱦ���
int RefrigerationMeasureCore::CalY16MatrixByTempMatrix(float *pTempMatrix, short *pDst, int nImageWidth, int nImageHeight)
{
	memset(pDst, 0, nImageHeight * nImageWidth * sizeof(short));
	//���������С�¶�
	float fMin = 2500.f;
	float fMax = -40.f;
	for (int i = 0; i < nImageHeight; i++)
	{
		for (int j = 0; j < nImageWidth; j++)
		{
			float tmp = pTempMatrix[i * nImageWidth + j];
			//��������
			if (tmp > 2500.f || tmp <-40.f)
			{
				return 1;
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

	//����Y16��
	int nNum = (int)((fMax - fMin) / 0.1f);
	short sValue[25400] = { 0 };
	for (int i = 0; i <= nNum; i++)
	{
		float fCurrTemp = fMin + 0.1f * i;
		short sCurrY16;
		CalY16ByTemp(sCurrY16, fCurrTemp);
		sValue[i] = sCurrY16;
	}

	//����ԭͼY16
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



//int low2high = 140;//���µ��и��µ��¶���ֵ
//int high2low = 130;//���µ��е��µ��¶���ֵ
//float areaTh1 = 0.0020;//���µ��и��µ����ռ����ֵ >140 ����> areaTh1*width*height
//float areaTh2 = 0.9995;//���µ��е��µ����ռ����ֵ >130 ����< (1-areaTh2)*width*height
int RefrigerationMeasureCore::autoChangeRange(short *pSrc, int width, int height, int range,
	float areaTh1, float areaTh2, int low2high, int high2low)
{
	int num = 0;
	int ifShiting = 0;

	short *sortY16 = (short *)malloc(width * height * sizeof(short));
	memcpy(sortY16, pSrc, width * height * sizeof(short));
	std::sort(sortY16, sortY16 + width * height);

	int areaNum = int(range == 0 ? round(areaTh1 * width * height) : round((1 - areaTh2) * width * height));
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
			float temp = StandardY16ToTempWithY16Correction(sortY16[i]);
			if (temp <= tempTh) 
			{
				ifShiting = range == 0 ? 0 : 1;
				break;
			}
			else
				num++;
		}
	}
	free(sortY16);
	return ifShiting;
}

short RefrigerationMeasureCore::Y16TransUnderDiffB(short sY16, short sStationaryB, short sRealBAvg, short realB, short sK, bool bMode)
{
	if (bMode)
	{
		short sTargetY16 = sY16 + short(sK * (realB - sStationaryB) / 8192.0f) - sRealBAvg;
		return sTargetY16;
	}
	else
	{
		short sTargetY16 = sY16 - short(sK * (realB - sStationaryB) / 8192.0f) + sRealBAvg;
		return sTargetY16;
	}
}

ITA_RESULT RefrigerationMeasureCore::Y16MatrixTransUnderDiffB(short* psY16, short* psTargetY16, short nAverageB, short* realB, short* stationaryB, short* kMatrix, int nImgHeight, int mImgWidth, bool bMode)
{
	int nLen = nImgHeight * mImgWidth;
	memset(psTargetY16, 0, nLen * 2);

	for (int i = 0; i < nLen; i++)
	{
		psTargetY16[i] = Y16TransUnderDiffB(psY16[i], stationaryB[i], nAverageB, realB[i], kMatrix[i], bMode);
	}
	return ITA_OK;
}

void RefrigerationMeasureCore::setMatrixData(short* stationaryB, short* kMatrix, int length)
{
	psKMaxtrix = kMatrix;
	psStationaryB = stationaryB;
	m_matrixLen = length;
}

ITA_RESULT RefrigerationMeasureCore::debugger(ITA_DEBUG_TYPE type, void* param)
{
	ITA_SWITCH* flag = (ITA_SWITCH*)param;
	ITA_RESULT ret = ITA_OK;
	if (!param && ITA_SNAPSHOT != type && ITA_START_RECORD != type && ITA_STOP_RECORD != type)
	{
		ret = ITA_NULL_PTR_ERR;
		m_logger->output(LOG_ERROR, "RefrigerationMeasureCore debugger ret=%d", ret);
		return ret;
	}
	switch (type)
	{
	case ITA_SET_DEBUGGING:			//�����Ƿ������ԡ��������ͣ�ITA_SWITCH��ITA_DISABLE���رյ��ԡ�ITA_ENABLE���������ԡ����δ������־·������ôĬ�ϱ����ڳ���ͬ��Ŀ¼��ITA.log��
		m_logger->output(LOG_INFO, "RefrigerationMeasureCore debugger op=%d %d", type, *flag);
		if (*flag<ITA_DISABLE || *flag>ITA_ENABLE)
		{
			return ITA_ARG_OUT_OF_RANGE;
		}
		if (ITA_DISABLE == *flag)
		{
			m_debug = false;
		}
		else
		{
			m_debug = true;
		}
		break;
	case ITA_DEBUGGING_PATH:		//������־���������·����ע��Ҫ��ITA_Init֮����á��������Ѵ��ڵ�Ŀ¼������/home/app/debug.log��Windows����"D:/Tools/debug.log"�����Ȳ�Ҫ����128���������ͣ��ַ������� \0 ������
	{
		ret = m_debugger.checkPath((const char*)param);
		if (ITA_OK == ret)
		{
			m_logger->setPath((const char*)param);
		}
		else
		{
			m_logger->output(LOG_ERROR, "ITA_DEBUGGING_PATH ret=%d", ret);
		}
		break;
	}
	case ITA_DEBUGGING_MODE:		//������־���ģʽ��1����־������ļ���Ĭ�������ʽ��2����־���������̨��֧���û��Զ��������־����ITARegistry��
	{
		int mode = *(int*)param;
		m_logger->output(LOG_INFO, "debugger op=%d %d", type, mode);
		if (mode < 1 || mode > 3)
		{
			return ITA_ARG_OUT_OF_RANGE;
		}
		m_logger->setMode((DBG_MODE_T)mode);
		break;
	}
	default:
		break;
	}
	return ret;
}

#endif // !REFRIGER_EDITION
