#include "MeasureTempCoreTEC.h"
#include <float.h>
#include <algorithm>

/*�������ܣ����캯��
���������
int w:							ͼ���
int h:							ͼ���
int curveLength:				�����ܳ���
short *pusCurve:				��������
IMAGE_MEASURE_STRUCT *params:	���½ṹ��
short *y16Image:				ͼƬԭʼY16����
���ز�����
��
*/
MeasureTempCoreTEC::MeasureTempCoreTEC(int w, int h, int curveLength, short *pusCurve,IMAGE_MEASURE_STRUCT *params)
{
	////��ȡSDK�汾
	//GetSDKVerision();

	//��ȡͼ����
	/*ImageWidth = w;
	ImageHidth = h;*/

	//�������߳���
	curveDataLength = curveLength;

	//���߸�ֵ
	CurveData = pusCurve;

	//���½ṹ�帳ֵ
	TMparams = params;
	m_fTempMatrix = NULL;
	m_tempLen = 0;
	m_logger = NULL;
	m_sortY16 = NULL;
	m_sortLen = 0;
	////���������Ư����Ͳ��Ư
	//ShutterTempDriftCorr();
	//LensTempDriftCorr();

	////��ʼ�������¶ȶ�Ӧ��Y16
	//TMparams->sY16Reflect = GetY16FromT(TMparams->fReflectT * 10);

	////��ʼ�������¶ȶ�Ӧ��Y16
	//TMparams->sY16Windows = GetY16FromT(TMparams->fWindowTemperature * 10);
}


/*�������ܣ���������
���������
��
���ز�����
��
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

//// ��ȡ����SDK�汾
//char* TempMeasureCore::GetSDKVerision()
//{
//	printf("MtVersion: %s\n", VERSION);
//	return VERSION;
//}

//// ˢ��Y16�������С����²��²���
//void TempMeasureCore::RefreshMeasureParam(IMAGE_MEASURE_STRUCT &Params, short *SrcY16)
//{
//	memcpy(&TMparams, &Params, sizeof(IMAGE_MEASURE_STRUCT));
//
//	//ˢ��Y16
//	memcpy(pSrcY16, SrcY16, ImageWidth * ImageHidth * sizeof(short));
//
//	//ˢ�¿�����Ư����Ͳ��Ư
//	ShutterTempDriftCorr();
//	LensTempDriftCorr();
//
//	//��������¶�Ӧ��Y16
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
//	//�����¶ȷ����˱仯�����ٴε���
//	if (TMparams->bIsReflectTChange)
//	{
//		TMparams->sY16Reflect = GetY16FromT(TMparams->fReflectT * 10);
//	}
//
//	//�����¶ȷ����˱仯�����ٴε���
//	if (TMparams->bIsWindowsTChange)
//	{
//		TMparams->sY16Windows = GetY16FromT(TMparams->fWindowTemperature * 10);
//	}
//
//	//�жϾ����������
//	if (TMparams->nDistance_a6 == -1 && TMparams->nDistance_a7 == -1 && TMparams->nDistance_a8 == -1)
//	{
//		TMparams->disType = 0;
//	}
//	else
//	{
//		TMparams->disType = 1;
//	}
//}


//ˢ�����ߣ��л���λ����·�Χʱ����
//void TempMeasureCore::RefreshCurve(short *curve)
//{
//	memcpy(CurveData, curve, curveDataLength * sizeof(short));
//}


void MeasureTempCoreTEC::reload(int curveLength, short * pusCurve, IMAGE_MEASURE_STRUCT * params)
{
	//�������߳���
	curveDataLength = curveLength;
	//���߸�ֵ
	CurveData = pusCurve;
	//���½ṹ�帳ֵ
	TMparams = params;
}

/*�������ܣ����½ӿڣ���Y16����ת��Ϊ�¶�
���������
short y16value��	ԭʼY16
���ز�����
float &Temp��		Y16��Ӧ���¶�
*/
void MeasureTempCoreTEC::GetTempByY16(float &Temp, short y16value)
{

	//��������¶�Ӧ��Y16
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

	////�����¶ȷ����˱仯�����ٴε���
	//if (TMparams->bIsReflectTChange)
	//{
	//	TMparams->sY16Reflect = GetY16FromT(TMparams->fReflectT * 10);
	//}

	////�����¶ȷ����˱仯�����ٴε���
	//if (TMparams->bIsWindowsTChange)
	//{
	//	TMparams->sY16Windows = GetY16FromT(TMparams->fWindowTemperature * 10);
	//}

	//Y16����
	y16value += TMparams->sY16Offset;

	//ʪ������
	if (TMparams->bHumidityCorrection)
	{
		y16value = HumidityCorrection(y16value, TMparams->fHumidity);
	}

	//float fTemp;
	//��Ư����
	Temp = StandardY16ToTempWithY16Correction(y16value);

	//B1����
	if (TMparams->bB1Correction)
	{
		Temp = Temp + ((TMparams->nB1 + TMparams->nB1Offset) / 100.0f);
	}

	//��������
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

	//����������
	if (TMparams->bEmissCorrection)
	{
		Temp = EmissionCorrectionNew(Temp);
	}


	if (TMparams->mtType)
	{
		//��������
		if (TMparams->bAmbientCorrection)
		{
			Temp = EnvironmentCorrection(Temp);
		}

		//����͸��������
		if (TMparams->bAtmosphereCorrection)
		{
			Temp = AtmosphereCorrection(Temp);
		}

		//����͸��������
		if (TMparams->bWindowTransmittanceCorrection)
		{
			Temp = WindowTransmittanceCorrection(Temp);
		}
	}
}

//// ��������
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


/*�������ܣ��¶ȷ���Y16�����¶�����ת��Ϊy16
���������
float &Temp��		ԭʼ�¶�
���ز�����
short y16value��	�¶ȶ�Ӧ��Y16
*/
void MeasureTempCoreTEC::CalY16ByTemp(short &y16, float fTemp)
{
	if (TMparams->bLogPrint)
	{
		m_logger->output(LOG_DEBUG, "CalY16ByTemp:%d %.2f %d %d", y16, fTemp, TMparams->mtType, TMparams->nGear);
	}
	if (TMparams->mtType)
	{
		//����͸������������
		if (TMparams->bWindowTransmittanceCorrection)
		{
			fTemp = WindowTransmittanceReverseCorrection(fTemp);
		}
		//����͸������������
		if (TMparams->bAtmosphereCorrection)
		{
			fTemp = AtmosphereReverseCorrection(fTemp);
		}
		//������������
		if (TMparams->bAmbientCorrection)
		{
			fTemp = EnvironmentReverseCorrection(fTemp);
		}
	}

	//��������������
	if (TMparams->bEmissCorrection)
	{
		fTemp = EmissionReverseCorrectionNew(fTemp);
	}
	//������������
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
	//B1��������
	if (TMparams->bB1Correction)
	{
		fTemp = fTemp - ((TMparams->nB1 + TMparams->nB1Offset) / 100.0f);
	}
	//��������¶�Ӧ��Y16
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
	//��Ư��������
	short y16_min = StandardTempToY16(fTemp, false);
	short y16_max = StandardTempToY16(fTemp, true);
	y16 = short((y16_min + y16_max) / 2);


	//ʪ����������
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

/*�������ܣ�Y16����������������Ư��������ͷ��Ư������KF����
���������
short y16value��		ԭʼY16
���ز�����
float temp��			Y16��Ӧ���¶�
*/
float MeasureTempCoreTEC::StandardY16ToTempWithY16Correction(short y16value)
{

	//ˢ�¿�����Ư����Ͳ��Ư
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


/*�������ܣ�ʪ������
���������
float fObjY16��			Ŀ��Y16
float fHumidity��		ʪ��ֵ
���ز���
short Y16��				������Y16
*/
short MeasureTempCoreTEC::HumidityCorrection(short fObjY16, int fHumidity)
{
	short Y16 = 0;
	Y16 = short((1.0f - (fHumidity - 60) / 5000.0f) * fObjY16);
	return Y16;
}


/*�������ܣ�������Ư�������Զ����������Ư
���������
��
���ز���
��
*/
void MeasureTempCoreTEC::ShutterTempDriftCorr()
{
	fCorrection1 = TMparams->nK1 / 100.f * (TMparams->fRealTimeShutterTemp - TMparams->fOrinalShutterTemp);
}

/*�������ܣ���Ͳ��Ư�������Զ����㾵Ͳ��Ư
���������
��
���ز���
��
*/
void MeasureTempCoreTEC::LensTempDriftCorr()
{
	fCorrection2 = TMparams->nK2 / 100.f * (TMparams->fRealTimeLensTemp - TMparams->fCurrentLensTemp);
}

/*�������ܣ�����������
���������
float iTemperature��		����������ǰ�¶�
���ز���
float Temp��				�������������¶�
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

	return (short)nTEmiss;
}


/*�������ܣ����鷢��������
���������
int ReflectT��			�����¶�
���ز���
short nReflectY16��		�����¶ȶ�Ӧ��Y16
*/
short MeasureTempCoreTEC::GetY16FromT(int ReflectT)
{
	int nReflectY16;
	//�õ�δ��������У�����¶ȶ�Ӧ��Y16
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


/*�������ܣ�����������6������
���������
float iTemperature��		��������ǰ�¶�
���ز���
float Temp��				�����������¶�
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

/*�������ܣ�����������9������
���������
float iTemperature��		��������ǰ�¶�
���ز���
float Temp��				�����������¶�
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

/*�������ܣ���������
���������
float iTemperature��		��������ǰ�¶�
���ز���
float Temp��				�����������¶�
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

////�ڼκͻ�������
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
////�ڼκͻ�����������
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


/*�������ܣ���������������
���������
float iTemperature��		�������������¶�
���ز���
float Temp��				����������ǰ�¶�
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

	return (short)nTNoEmiss;
	/******************20210810 �ķ����ʷ��㸡��ȡ������********************************/
}

//������������--�Ų���
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
short MeasureTempCoreTEC::HumidityReverseCorrection(short y16, int fHumidity)
{
	short y = 0;
	y = short(y16 * (5000.0f / (5060 - fHumidity)));
	return y;
}

//������������
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


//��Ư��������
short MeasureTempCoreTEC::StandardTempToY16(float iTemperature, bool fromSmallToBig)
{
	//ˢ�¿�����Ư����Ͳ��Ư
	ShutterTempDriftCorr();
	LensTempDriftCorr();
	
	//��������
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

	//KF��������
	int y16value = (int)((index - nShutterCurveIndex) / ((TMparams->nKF + TMparams->nKFOffset) / 100.0));

	//��Ư��������
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

//����͸��������
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

//����͸���ʷ���
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


//����͸��������
float MeasureTempCoreTEC::WindowTransmittanceCorrection(float iTemperature)
{
	short sY16Windows = GetY16FromT(int(TMparams->fWindowTemperature * 10));
	short nTNoEmiss = short(iTemperature * 10);
	int nWindowTransmittance = int(TMparams->fWindowTransmittance * 100);//
	short TEmiss = EmissCor(nTNoEmiss, sY16Windows, nWindowTransmittance);
	return TEmiss / 10.0f;
}

//����͸������������
float MeasureTempCoreTEC::WindowTransmittanceReverseCorrection(float iTemperature)
{
	short sY16Windows = GetY16FromT(int(TMparams->fWindowTemperature * 10));
	short nTNoEmiss = short(iTemperature * 10);
	int nWindowTransmittance = int(TMparams->fWindowTransmittance * 100);//
	short DeTEmiss = DeEmissCor(nTNoEmiss, sY16Windows, nWindowTransmittance);
	return DeTEmiss / 10.0f;
}



//����У��
void MeasureTempCoreTEC::SecondCorrectByOnePoint(float bb_temperature, short bb_y16)
{
	bb_y16 += TMparams->sY16Offset;

	float AftreB1Temp = 0.f;
	float BeforeB1Temp = 0.f;

	if (TMparams->mtType)
	{
		//����͸������������
		if (TMparams->bWindowTransmittanceCorrection)
		{
			AftreB1Temp = WindowTransmittanceReverseCorrection(bb_temperature);
		}

		//����͸������������
		if (TMparams->bAtmosphereCorrection)
		{
			AftreB1Temp = AtmosphereReverseCorrection(AftreB1Temp);
		}

		//������������
		if (TMparams->bAmbientCorrection)
		{
			AftreB1Temp = EnvironmentReverseCorrection(AftreB1Temp);
		}
	}
	
	//��������������
	if (TMparams->bEmissCorrection)
	{
		AftreB1Temp = EmissionReverseCorrectionNew(AftreB1Temp);
	}

	//������������
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

//����У��
void MeasureTempCoreTEC::SecondCorrectByTwoPoint(float bb_temperature_1, float bb_temperature_2, short bb_y16_1, short bb_y16_2, int* coff)
{
	bb_y16_1 += TMparams->sY16Offset;
	bb_y16_2 += TMparams->sY16Offset;

	float AftreB1Temp1 = 0.f;
	float AftreB1Temp2 = 0.f;



	if (TMparams->mtType)
	{
		//����͸������������
		if (TMparams->bWindowTransmittanceCorrection)
		{
			AftreB1Temp1 = WindowTransmittanceReverseCorrection(bb_temperature_1);
			AftreB1Temp2 = WindowTransmittanceReverseCorrection(bb_temperature_2);
		}

		//����͸������������
		if (TMparams->bAtmosphereCorrection)
		{
			AftreB1Temp1 = AtmosphereReverseCorrection(AftreB1Temp1);
			AftreB1Temp2 = AtmosphereReverseCorrection(AftreB1Temp2);
		}

		//������������
		if (TMparams->bAmbientCorrection)
		{
			AftreB1Temp1 = EnvironmentReverseCorrection(AftreB1Temp1);
			AftreB1Temp2 = EnvironmentReverseCorrection(AftreB1Temp2);
		}
	}
	
	//��������������
	if (TMparams->bEmissCorrection)
	{
		AftreB1Temp1 = EmissionReverseCorrectionNew(AftreB1Temp1);
		AftreB1Temp2 = EmissionReverseCorrectionNew(AftreB1Temp2);
	}

	//������������
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

	//���ڲ��Ĵ��ڣ��޷��������KF,B1,��˲�ȡС��Χѭ��������ʽ
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

//�����¶Ⱦ���ӿ�(ȫͼ�������)
int MeasureTempCoreTEC::GetTempMatrix(short *pSrc, float *pTempMatrix, int nImageWidth, int nImageHeight)
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


//�����¶Ⱦ���ӿ�(ROI�����������)
int MeasureTempCoreTEC::GetTempMatrix(short *pSrc, float *pTempMatrix, int nImageWidth, int nImageHeight, int x, int y, int ROIWidth, int ROIHeight, float ROIDistance, float ROIEmiss)
{
	//��¼ԭ�еľ���ͷ�����
	float TempDistance = TMparams->fDistance;
	float TempEmis = TMparams->fEmiss;

	//�������еľ��롢�����ʽ��и���
	TMparams->fDistance = ROIDistance;
	TMparams->fEmiss = ROIEmiss;
	
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
	//Y16ֵ�쳣��������¶Ⱦ���������ѭ���޷�������short����ѭ������������¡�
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

	//���롢�����ʻ�ԭ
	TMparams->fDistance = TempDistance;
	TMparams->fEmiss = TempEmis;
	return 0;
}



//�����¶Ⱦ���ӿ�(ȫͼ���ټ���)
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
int MeasureTempCoreTEC::GetTempMatrixFast(short *pSrc, float *pTempMatrix, int nImageWidth, int nImageHeight, int x, int y, int ROIWidth, int ROIHeight, float ROIDistance, float ROIEmiss)
{
	//��¼ԭ�еľ���ͷ�����
	float TempDistance = TMparams->fDistance;
	float TempEmis = TMparams->fEmiss;

	//�������еľ��롢�����ʽ��и���
	TMparams->fDistance = ROIDistance;
	TMparams->fEmiss = ROIEmiss;

	short sMin = 32767;
	short sMax = -32767;
	int nLen = ROIWidth * ROIHeight;
	memset(pTempMatrix, 0, nLen * sizeof(float));

	// ����ROI�������ADֵ�����ADֵ
	for (int i = y; i < y + ROIHeight; i++)
	{
		for (int j = x; j < x + ROIWidth; j++)
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
	TMparams->fDistance = TempDistance;
	TMparams->fEmiss = TempEmis;

	return 0;
}


/*�������ܣ���������¶�ӳ��
���������
float fSurfaceTemp��Ŀ������¶�
float fEnvirTemp��Ŀ�������Ļ���
���ز�����Ŀ�������¶�
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

	const double fMapTempMin = 35.8;//ӳ������� 
	const double fMapTempMax = 42.0;//ӳ������� 
	fSurfaceTemp = float(fmin(40, fmax(fSurfaceTemp, 28)));//����¶���ֵ 

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

//int low2high = 140;//���µ��и��µ��¶���ֵ
//int high2low = 130;//���µ��е��µ��¶���ֵ
//float areaTh1 = 0.0020;//���µ��и��µ����ռ����ֵ >140 ����> areaTh1*width*height
//float areaTh2 = 0.9995;//���µ��е��µ����ռ����ֵ >130 ����< (1-areaTh2)*width*height
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
	int ifShiting = 0;                 //�е���־λ
	int NumPasslow2mid = 0;           //�¶ȳ���140������
	int NumPassmid2low = 0;           //�¶ȵ͹�130������
	int NumPassmid2high = 0;          //�¶ȳ���600������
	int NumPasshigh2mid = 0;          //�¶ȵ���580������

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

	int areaNumDownToUp = round(areaTh1 * width * height);                     //�����е���������ֵ  
	int areaNumUpToDown = round((1 - areaTh2) * width * height);              //���е���������ֵ 

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

//�Զ�У�½ӿ�
void MeasureTempCoreTEC::AutoCorrectTemp(float* fBlackBodyTemps, short* sY16s, int nLength)
{
	//�Դ������ĺ����¶��Լ���Ӧ��y16ֵ��������
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
			//����͸������������
			if (TMparams->bWindowTransmittanceCorrection)
			{
				pTmpBlackBodyTemp[i] = WindowTransmittanceReverseCorrection(pTmpBlackBodyTemp[i]);
			}

			//����͸������������
			if (TMparams->bAtmosphereCorrection)
			{
				pTmpBlackBodyTemp[i] = AtmosphereReverseCorrection(pTmpBlackBodyTemp[i]);
			}

			//������������
			if (TMparams->bAmbientCorrection)
			{
				pTmpBlackBodyTemp[i] = EnvironmentReverseCorrection(pTmpBlackBodyTemp[i]);
			}
		}

		//��������������
		if (TMparams->bEmissCorrection)
		{
			pTmpBlackBodyTemp[i] = EmissionReverseCorrectionNew(pTmpBlackBodyTemp[i]);
		}

		//������������
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

	//���ڲ��Ĵ��ڣ��޷��������KF,B1,��˲�ȡС��Χѭ��������ʽ
	float fDiffMax = FLT_MAX;
	int nBestKF = 0;
	int nBestB1 = 0;
	//KF ��ԭʼֵ��0.8~1.2����Χ�ڵ���
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