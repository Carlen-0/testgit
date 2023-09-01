/*************************************************************
Copyright (C), 2022--2023, Wuhan Guide Sensmart Tech Co., Ltd
File name   : BBMeasureCore.cpp
Author      : Guide Sensmart RD. wangyan
Version     : 1.0
Date        : 2022/10/29
Description : BBMeasureCore.
*************************************************************/
#include "BBMeasureCore.h"
#include <string.h>

#ifdef BLACKBODY_EDITION

BBTempMeasureCore::BBTempMeasureCore(int w, int h, short *pusCurve, int curveLength)
{
	nImgWidth = w;
	nImgHeight = h;
	//�������߳���
	nCurveLength = curveLength;
	//���߸�ֵ
	sCurveData = pusCurve;
	//���²�������ֵ
	memset(&BBTMparams, 0, sizeof(ITABBMeasureParam));
	BBTMparams.nKf = 100;											//KF,�Ŵ�100����Ĭ��100
	BBTMparams.nMaxMeasuLimit = 650;									//����¶ȣ��Ŵ�10��.Ĭ��650
	BBTMparams.nMinMeasuLimit = -150;									//��С�¶ȣ��Ŵ�10����Ĭ��-150
	BBTMparams.nDistance = 2;										//���¾��룬�Ŵ�10����Ĭ��2
	BBTMparams.nHumidity = 70;										//ʪ�ȣ��Ŵ�100����Ĭ��70
	BBTMparams.nEmissivity = 100;									//�����ʣ��Ŵ�100����Ĭ��100
	BBTMparams.fEnvironmentT = 23;								//�����¶�
	BBTMparams.fBlackBodyT = 35;									//�����¶�
	BBTMparams.fManualAdjustT = 0;								//�û��ֶ������¶�
	BBTMparams.bIfDistanceCorrection = true;							//������������
	BBTMparams.bIfEmissCorrection = true;							//��������������
	BBTMparams.BlackBodyRectArea.nX = 0;
	BBTMparams.BlackBodyRectArea.nY = 0;
	BBTMparams.BlackBodyRectArea.nWidth = 3;
	BBTMparams.BlackBodyRectArea.nHeight = 3;
}

BBTempMeasureCore::~BBTempMeasureCore()
{
}

void BBTempMeasureCore::setParams(ITABBMeasureParam params)
{
	memcpy(&BBTMparams, &params, sizeof(ITABBMeasureParam));
}

void BBTempMeasureCore::getParams(ITABBMeasureParam * params)
{
	memcpy(params, &BBTMparams, sizeof(ITABBMeasureParam));
}

void BBTempMeasureCore::setCurve(short * pusCurve, int curveLength)
{
	//�������߳���
	nCurveLength = curveLength;
	//���߸�ֵ
	sCurveData = pusCurve;
}

//������º���
void BBTempMeasureCore::GetTempByY16(float &Temp, short y16value, short *y16Image)
{
	//ͼ��Y16��ֵ
	pSrcY16 = y16Image;
	//���㵱ǰ֡���������Y16
	short sBBY16;
	GetBlackBodyY16(sBBY16);

	//���ݺ����¶ȷ���Y16
	short nInvertY16Index = TempToCurveY16(BBTMparams.fBlackBodyT);

	//�����߻�ȡ�¶�
	short nMeasureY16 = nInvertY16Index + (short)((y16value - sBBY16) * BBTMparams.nKf / 100.0f);

	if (nMeasureY16 < 0)
	{
		nMeasureY16 = 0;
	}
	if (nMeasureY16 > nCurveLength)
	{
		nMeasureY16 = nCurveLength - 1;
	}
	Temp = sCurveData[nMeasureY16] / 10.0f;

	float Tmax = BBTMparams.nMaxMeasuLimit / 10.0f;
	float Tmin = BBTMparams.nMinMeasuLimit / 10.0f;
	if (Temp > Tmax)
	{
		Temp = Tmax;
	}
	if (Temp < Tmin)
	{
		Temp = Tmin;
	}

	//B1����
	Temp += BBTMparams.nB0 / 10.0f;

	//��������
	if (BBTMparams.bIfDistanceCorrection)
	{
		Temp = DistanceCorrectionWithSixPara(Temp);
	}

	//����������
	if (BBTMparams.bIfEmissCorrection)
	{
		Temp = EmissionCorrection(Temp);
	}

	//�û��ֶ��¶ȵ���
	Temp += BBTMparams.fManualAdjustT;
}


//�¶ȷ���Y16
void BBTempMeasureCore::GetY16ByTemp(short &y16value, float Temp)
{
	//�û��ֶ��¶ȵ�������
	Temp -= BBTMparams.fManualAdjustT;

	//��������������
	if (BBTMparams.bIfEmissCorrection)
	{
		Temp = EmissionCorrection(Temp);
	}

	//������������
	if (BBTMparams.bIfDistanceCorrection)
	{
		Temp = DistanceCorrectionWithSixPara(Temp);
	}

	//B1��������
	Temp -= BBTMparams.nB0 / 10.0f;

	//�����������
	y16value = TempToCurveY16(Temp);

	//���㵱ǰ֡���������Y16
	short sBBY16;
	GetBlackBodyY16(sBBY16);

	//���ݺ����¶ȷ���Y16
	short nInvertY16Index = TempToCurveY16(BBTMparams.fBlackBodyT);

	y16value = (short)(((y16value - nInvertY16Index) / (BBTMparams.nKf / 100.0f)) + sBBY16);
}


//���㵱ǰ֡���������Y16
int BBTempMeasureCore::GetBlackBodyY16(short &sBBY16)
{
	if (!bIfBBAreaRight())
	{
		return -1;
	}
	else
	{
		//640*512*16384��2^33
		long lSum = 0;
		short sCnt = 0;
		for (int i = BBTMparams.BlackBodyRectArea.nX; i < BBTMparams.BlackBodyRectArea.nX + BBTMparams.BlackBodyRectArea.nWidth; i++)
		{
			for (int j = BBTMparams.BlackBodyRectArea.nY; j < BBTMparams.BlackBodyRectArea.nY + BBTMparams.BlackBodyRectArea.nHeight; j++)
			{
				short sCurrPos = j * nImgWidth + i;
				if (sCurrPos >= 0 && sCurrPos < nImgWidth * nImgHeight)
				{
					lSum += pSrcY16[sCurrPos];
					sCnt++;
				}
			}
		}
		sBBY16 = (short)(double(lSum) / sCnt);
		return 0;
	}
}

//�жϺ��������Ƿ���ȷ
bool BBTempMeasureCore::bIfBBAreaRight()
{
	if (BBTMparams.BlackBodyRectArea.nX >= 0 && BBTMparams.BlackBodyRectArea.nY >= 0 && BBTMparams.BlackBodyRectArea.nWidth >= 0 && BBTMparams.BlackBodyRectArea.nHeight >= 0 && 
		(BBTMparams.BlackBodyRectArea.nX + BBTMparams.BlackBodyRectArea.nWidth) <= nImgWidth && (BBTMparams.BlackBodyRectArea.nY + BBTMparams.BlackBodyRectArea.nHeight) <= nImgHeight)
		return true;
	else
		return false;
}



//�¶ȷ�������
short BBTempMeasureCore::TempToCurveY16(float Temp)
{
	short sTemp = short(Temp * 10);
	//��������¶�Ӧ��Y16
	short sInvY16Big = 0;
	short sInvY16Small = 0;
	for (int i = 0; i < nCurveLength; i++)
	{
		if (sCurveData[i] > sTemp)
		{
			sInvY16Small = i;
			break;
		}
	}
	for (int i = nCurveLength - 1; i > 0; i--)
	{
		if (sCurveData[i] < sTemp)
		{
			sInvY16Big = i;
			break;
		}
	}
	short sInvY16 = short((sInvY16Small + sInvY16Big) / 2);
	return sInvY16;
}


/*�������ܣ�����������6������
���������
float iTemperature��		��������ǰ�¶�
���ز���
float Temp��				�����������¶�
*/
float BBTempMeasureCore::DistanceCorrectionWithSixPara(float iTemperature)
{
	double a0 = BBTMparams.nDistance_a0 / 1000000000.0;
	double a1 = BBTMparams.nDistance_a1 / 1000000000.0;
	double b0 = BBTMparams.nDistance_a2 / 1000000.0;
	double b1 = BBTMparams.nDistance_a3 / 1000000.0;
	double c0 = BBTMparams.nDistance_a4 / 1000.0;
	double c1 = BBTMparams.nDistance_a5 / 1000.0;


	double a = a0 * BBTMparams.nDistance + a1;
	double b = b0 * BBTMparams.nDistance + b1;
	double c = c0 * BBTMparams.nDistance + c1;

	if (a == 0 && b == 0) {
		return iTemperature;
	}
	else {
		float x = iTemperature + 273.5f;
		float y = float(a * x * x + b * x + c);

		return y / 10.0f - 273.5f;
	}
}


/*�������ܣ�����������
���������
float iTemperature��		����������ǰ�¶�
���ز���
float Temp��				�������������¶�
*/
float BBTempMeasureCore::EmissionCorrection(float iTemperature)
{
	short sY16Reflect = GetY16FromT(int(BBTMparams.fEnvironmentT * 10));
	short nTNoEmiss = short(iTemperature * 10);
	int nEmiss = int(BBTMparams.nEmissivity * 100);
	short TEmiss = EmissCor(nTNoEmiss, sY16Reflect, nEmiss);
	//short DeTEmiss = DeEmissCor(nTNoEmiss, sY16Reflect, nEmiss);
	return TEmiss / 10.0f;
}

/*�������ܣ����鷢��������
���������
int ReflectT��			�����¶�
���ز���
short nReflectY16��		�����¶ȶ�Ӧ��Y16
*/
short BBTempMeasureCore::GetY16FromT(int ReflectT)
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


short BBTempMeasureCore::EmissCor(short nTNoEmiss, short sY16Reflect, int nEmiss)
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

#endif // !BLACKBODY_EDITION
