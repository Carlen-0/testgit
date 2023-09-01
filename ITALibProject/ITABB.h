/*************************************************************
Copyright (C), 2022--2023, Wuhan Guide Sensmart Tech Co., Ltd
File name   : ITABB.h
Author      : Guide Sensmart RD. wangyan
Version     : 1.0
Date        : 2022/10/29
Description : ITA SDK interface definition for blackbody temperature measurement products.
*************************************************************/
#ifndef ITA_BB_H
#define ITA_BB_H

#ifdef __cplusplus
#    define ITA_BB_START  extern "C" {
#    define ITA_BB_END    };
#else
#    define ITA_BB_START
#    define ITA_BB_END
#endif

ITA_BB_START

#include "ERROR.h"
#include "ITADTD.h"

//���νṹ��
typedef struct ITA_BB_RECTANGLE_INFO
{
	int nX;												//���Ͻ�x
	int nY;												//���Ͻ�y
	int nWidth;											//���ο�
	int nHeight;										//���θ�
}ITABBRectangleInfo;

//������ز����ṹ��
typedef struct ITA_BB_MEASURE_PARAM
{
	int nKf;											//KF,�Ŵ�100����Ĭ��100
	int nB0;											//B0,�Ŵ�10����Ĭ��0
	int nMaxMeasuLimit;									//����¶ȣ��Ŵ�10��.Ĭ��650
	int nMinMeasuLimit;									//��С�¶ȣ��Ŵ�10����Ĭ��-150
	int nDistance;										//���¾��룬�Ŵ�10����Ĭ��2
	int nHumidity;										//ʪ�ȣ��Ŵ�100����Ĭ��70
	int nEmissivity;									//�����ʣ��Ŵ�100����Ĭ��100
	int nDistance_a0;									//����У��ϵ��(���㻯1000000000����Ĭ��0)
	int nDistance_a1;									//����У��ϵ��(���㻯1000000000����Ĭ��0)
	int nDistance_a2;									//����У��ϵ��(���㻯1000000����Ĭ��0)
	int nDistance_a3;									//����У��ϵ��(���㻯1000000����Ĭ��0)
	int nDistance_a4;									//����У��ϵ��(���㻯1000����Ĭ��0)
	int nDistance_a5;									//����У��ϵ��(���㻯1000����Ĭ��0)
	float fEnvironmentT;								//�����¶�
	float fBlackBodyT;									//�����¶�
	float fManualAdjustT;								//�û��ֶ������¶�
	bool bIfDistanceCorrection;							//������������
	bool bIfEmissCorrection;							//��������������
	ITABBRectangleInfo BlackBodyRectArea;
}ITABBMeasureParam;

/**
* @brief:	��ʼ��һ��ITA�������ģ��ʵ����
* @param:	int w  Y16��
* @param:	int h  Y16�ߡ�
* @param:	short *pusCurve  �������ݵ�ַ��
* @param:	int curveLength  �������ݳ��ȡ�
* @return:	�ɹ�������ʵ�������ʧ�ܣ�����NULL��
**/
ITA_API HANDLE_T ITA_BBInit(int w, int h, short *pusCurve, int curveLength);

/**
* @brief:	�ͷ�һ��ITA�������ģ��ʵ����Դ��
* @param:	HANDLE_T instance	ITA�������ģ��ʵ����
* @return:	�ޡ�
**/
ITA_API void ITA_BBUninit(HANDLE_T instance);

/**
* @brief:	���ò��²�����
* @param:	HANDLE_T instance           ITA�������ģ��ʵ����
* @param:	ITABBMeasureParam params    ���²�����
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_BBSetParams(HANDLE_T instance, ITABBMeasureParam params);

/**
* @brief:	��ѯ���²�����Ϣ��
* @param:	HANDLE_T instance           ITA�������ģ��ʵ����
* @param:	ITABBMeasureParam *params   ������²�����Ϣ��
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_BBGetParams(HANDLE_T instance, ITABBMeasureParam *params);

/**
* @brief:	�����������ݡ�
* @param:	HANDLE_T instance			ITA�������ģ��ʵ����
* @param:	short *pusCurve 	        �������ݵ�ַ��
* @param:	int curveLength				�������ݳ��ȡ�
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_BBSetCurve(HANDLE_T instance, short *pusCurve, int curveLength);

/**
* @brief:	������¡�
* @param:	HANDLE_T instance           ITA�������ģ��ʵ����
* @param:	short y16value				����Y16��
* @param:	float *temp					����¶ȡ�
* @param:	short *y16Image				�����������ǰ֡Y16�����ַ��
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_BBCalcTemp(HANDLE_T instance, short y16value, short *y16Image, float *temp);

/**
* @brief:	�����¶ȷ���Y16��
* @param:	HANDLE_T instance			ITA�������ģ��ʵ����
* @param:	float temp				    �����¶ȡ�
* @param:	short *y16value	            ���y16ֵ��
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_BBCalcY16(HANDLE_T instance, float temp, short *y16value);

ITA_BB_END

#endif // !ITA_BB_H
