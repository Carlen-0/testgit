/*************************************************************
Copyright (C), 2022--2023, Wuhan Guide Sensmart Tech Co., Ltd
File name   : ITAFireDRC.h
Author      : Guide Sensmart RD. wangyan
Version     : 1.0
Date        : 2022/10/29
Description : ITA SDK interface definition for fire imaging.
*************************************************************/
#ifndef ITA_FIRE_DRC_H
#define ITA_FIRE_DRC_H

#ifdef __cplusplus
#    define ITA_FIRE_DRC_START  extern "C" {
#    define ITA_FIRE_DRC_END    };
#else
#    define ITA_FIRE_DRC_START
#    define ITA_FIRE_DRC_END
#endif

ITA_FIRE_DRC_START

#include "ERROR.h"
#include "ITADTD.h"

typedef enum ITAFirePaletteType
{
	ITA_FIRE_WHITEHOT = 0,	//����
	ITA_FIRE_BLACKHOT,		//����
	ITA_FIRE_IRONRED,		//����
	ITA_FIRE_ARCTIC,		//����
	ITA_FIRE_HOTIRON,		//����
	ITA_FIRE_RAINBOW1,		//�ʺ�1
	ITA_FIRE_FULGURITE,		//����
	ITA_FIRE_TINT1,			//���1
	ITA_FIRE_TINT3,			//���3�����ȼ��ģʽʹ�ã�
	ITA_FIRE_FIRE			//����α�ʣ�Ĭ������ģʽʹ�ã�����ѡ��
}ITA_FIRE_PALETTE_TYPE;

typedef struct ITA_Y16_TEMP_MAP{
	int y16minus20;
	int y16_0;				
	int y16_100;
	int y16_125;
	int y16_150;
	int y16_250;
	int y16_300;
	int y16_400;
	int y16_450;
	int y16_500;
	int y16_650;
	int y16_2000;
}ITAY16TempMap;

//���������������ṹ��
typedef struct ITA_FIRE_DRC_PARAM
{
	int mixContrastExp;		//��ϵ��������Ե���Աȶȣ�Ĭ��ֵ128����Χ0-255��
	int mixBrightExp;		//��ϵ��������Ե������ȣ�Ĭ��ֵ255����Χ0-512��
	int restrainRangeThre;	//��ϵ��������Ե�����������ƣ�Ĭ��ֵ128����Χ0-256��
	int mixRange;			//��ϵ�����ֱ��ͼ����ӳ�䷶Χ��Ĭ��ֵ255����Χ0-512��
	int midValue;			//��ϵ�����ֱ��ͼ����ӳ����ֵ��Ĭ��ֵ128����Χ0-255��
	int platThresh;			//��ϵ�����ֱ��ͼ����ƽ̨��ֵ��Ĭ��ֵ200����Χ1-1000��
	int upDiscardRatio;		//��ϵ��������׵������ǧ��֮һ����Ĭ��ֵ1����Χ0-100��
	int downDiscardRatio;	//��ϵ��������׵������ǧ��֮һ����Ĭ��ֵ1����Χ1-100��
	int enThr;				//ϸ����ǿ��ֵ��Ĭ��ֵ20����Χ0-50��
	int enCoef;				//ϸ����ǿϵ����Ĭ��ֵ32����Χ0-128��
	int filterStd;			//ϸ����ǿ���˲�ϵ����׼�Ĭ��ֵ10����Χ1-50��
	ITA_FIRE_PALETTE_TYPE pseudoNo;			//α�ʺţ�Ŀǰ֧��10�֡�
	ITA_PIXEL_FORMAT imgType;   //���ͼ���ʽѡ��
}ITAFireDRCParam;

typedef enum ITA_FIRE_WORKMODE
{
	BASE_FIRE_MODE = 0,
	BLACK_FIRE_EXTINGUISHING_MODE = 1,
	FIRE_DANGER_MODE = 2,
	RESCUE_MODE = 3,
	HEAT_DETECT_MODE = 4,
	STANDART_FIRE_MODE = 5,
}ITAFireWorkMode;

/**
* @brief:	��ʼ��һ��ITA��������ģ��ʵ����
* @param:	int w  Y16��
* @param:	int h  Y16�ߡ�
* @param:	ITA_PIXEL_FORMAT imgFormat ͼ���ʽ,Ŀǰ��֧��ITA_BGR888ͼ���ʽ��
* @return:	�ɹ�������ʵ�������ʧ�ܣ�����NULL��
**/
ITA_API HANDLE_T ITA_FireDRCInit(int w, int h, ITA_PIXEL_FORMAT imgFormat);

/**
* @brief:	�ͷ�һ��ITA��������ģ��ʵ����Դ��
* @param:	HANDLE_T instance	ITA��������ģ��ʵ����
* @return:	�ޡ�
**/
ITA_API void ITA_FireDRCUninit(HANDLE_T instance);

/**
* @brief:	��������������������
* @param:	HANDLE_T instance           ITA��������ģ��ʵ����
* @param:	ITABBMeasureParam params    ����������������
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_FireDRCSetParams(HANDLE_T instance, ITAFireDRCParam params);

/**
* @brief:	��ѯ����������������Ϣ��
* @param:	HANDLE_T instance           ITA��������ģ��ʵ����
* @param:	ITABBMeasureParam *params   �������������������Ϣ��
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_FireDRCGetParams(HANDLE_T instance, ITAFireDRCParam *params);

/**
* @brief:	��������ģʽ���Ͳ��·�Χʹ�ã�����������ͼ�����ݵ��ڴ�����û�������ͷš�
* @param:	HANDLE_T instance			ITA��������ģ��ʵ����
* @param:	short *srcData 	            ����Y16�����ڴ��׵�ַ��
* @param:	unsigned char* dstData	    ���ͼ���ڴ��׵�ַ���ڴ��С��ITA_FireDRCInit�е�imgFormat��ͼ���߾�����
* @param:	int width					����ͼ�����ݵĿ�ȡ�
* @param:	int height					����ͼ�����ݵĸ߶ȡ�
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_FireDRCGeneral(HANDLE_T instance, short *srcData, unsigned char* dstData, int width, int height);

/**
* @brief:	��������ģʽ���ֶε��⣩����������ͼ�����ݵ��ڴ�����û�������ͷš�
* @param:	HANDLE_T instance           ITA��������ģ��ʵ����
* @param:	short *srcData 	            ����Y16�����ڴ��׵�ַ��
* @param:	unsigned char* dstData	    ���ͼ���ڴ��׵�ַ���ڴ��С��ITAFireDRCParam�е�imgType��ͼ���߾�����
* @param:	int width					����ͼ�����ݵĿ�ȡ�
* @param:	int height					����ͼ�����ݵĸ߶ȡ�
* @param:	int upY16					������·����Y16��
* @param:	int downY16					������·����Y16��
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_FireDRCSection(HANDLE_T instance, short *srcData, unsigned char* dstData, int width, int height, int upY16, int downY16);

/**
* @brief:	����ͼ��Center�¶ȶ�Ӧ�Ŀ̶ȳ�λ��
* @param:	HANDLE_T instance           ITA��������ģ��ʵ����
* @param:	int centerY16				�������ĵ�Y16��
* @param:	ITAY16TempMap map		����Y16�¶�ӳ�����������Ϊ��ITA_Y16_TEMP_MAP��	
* @param:	ITAFireWorkMode workMode	ͼ����ʾģʽ(���������֡��ȼ��)��
* @param:	float* barHeight			�������Ϊ�߶�ֵ����ʾռ�̶ܿȳߵı�����
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_CalcROITemperBarHeightSimplify(HANDLE_T instance, int centerY16, ITAY16TempMap map, ITAFireWorkMode workMode, float* barHeight);

/**
* @brief:	zc17a�����ֹ���ģʽ�����㷨����
* @param:	HANDLE_T instance           ITA��������ģ��ʵ����
* @param:	short* srcY16				����ԭʼY16���ݡ��ڴ����û����롣
* @param:	unsigned char* dst			�����α�ʵ�y8ͼ�����ݡ��ڴ����û����롣
* @param:	int width					Y16ͼ�����ݵĿ�
* @param:	int height			        Y16ͼ�����ݵĸߡ�
* @param:	int maxY16Temp				��ǰ����ģʽ����¶�Ӧ��Y16��
* @param:	int threshY16				���Ե�����Զ�����ֽ�Y16��ֵ��
* @param:	ITAFireWorkMode work_mode	ͼ����ʾģʽ(���������֡��ȼ��)��
* @param:	ITA_FIRE_PALETTE_TYPE pesudoͼ��α�ʺš�
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
**/
ITA_API ITA_RESULT ITA_DrcFireAdaptive2DifferentMode(HANDLE_T instance, short* srcY16, unsigned char* dst, int width, int height, int maxY16Temp, int threshY16, ITAFireWorkMode work_mode, ITA_FIRE_PALETTE_TYPE pesudo);


ITA_FIRE_DRC_END

#endif // !ITA_FIRE_DRC_H
