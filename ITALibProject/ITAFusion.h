/*************************************************************
Copyright (C), 2022--2023, Wuhan Guide Sensmart Tech Co., Ltd
File name   : ITARefriger.h
Author      : Guide Sensmart RD. wangyan
Version     : 1.0
Date        : 2022/10/29
Description : Temperature measurement model of refrigeration detector.
*************************************************************/
#ifndef ITA_FUSION_H
#define ITA_FUSION_H

#ifdef __cplusplus
#    define ITA_FUSION_START  extern "C" {
#    define ITA_FUSION_END    };
#else
#    define ITA_REFRIGER_START
#    define ITA_REFRIGER_END
#endif

ITA_FUSION_START

#include "ERROR.h"
#include "ITADTD.h"

//����ͼ������
typedef enum ITAImageType {
	ITA_VL = 0,
	ITA_IR
}ITA_IMAGE_TYPE;

typedef struct ITA_RECTANGLE {
	ITA_POINT leftTop;
	ITA_POINT rightDown;
}ITARectangle;


/**
* @brief:	�ں�ģ���ʼ����
* @param:	ITA_PIXEL_FORMAT format			�ɼ��⡢����ͼ������ݸ�ʽ��
* * @param:	ITA_FIELD_ANGLE lensType		ͼ��ľ�ͷ����
* * @param:	int irWidth,int irHeight		����ͼ��Ŀ��ߡ�
* * @param:	int vlWidth,int vlHeight		�ɼ���ͼ��Ŀ��ߡ�
* @param:	unsigned char detail			��Ե����ǿ�ȡ�
* @param:	const char* fileDir				csv�ļ��Ĵ洢·��
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
* @note:
**/
ITA_API HANDLE_T ITA_FusionInit(ITA_PIXEL_FORMAT format, ITA_FIELD_ANGLE lensType,int irWidth,int irHeight,int vlWidth,int vlHeight, unsigned char detail, const char* fileDir);

/**
* @brief:	�ͷ�һ���ںϣ�ITA����ʵ����Դ��
* @param:	HANDLE_T instance	ITAʵ����
* @return:	�ޡ�
**/
ITA_API void ITA_FusionUninit(HANDLE_T instance);

/**
* @brief:	ͼ���ں�,���������ɼ����YUVͼ�����ݣ�ʵ��ͼ�����ݵ��ں�Ч��
* @param:	HANDLE_T instance	 			ʵ�������
* @param:	unsigned char* irData			YUV��ʽ�ĺ���ͼ�����ݡ��ڴ����û�������ͷš�
* @param:	unsigned char* visData			YUV��ʽ�Ŀɼ���ͼ�����ݡ��ڴ����û�������ͷš�
* @param:	int irWidth					    ����ͼ�����ݵĿ�ȡ�
* @param:	int irHeight					����ͼ�����ݵĸ߶ȡ�
* @param:	int visWidth					�ɼ���ͼ�����ݵĿ�ȡ�
* @param:	int visHeight					�ɼ���ͼ�����ݵĸ߶ȡ�
* @param:	float distance					�������,�ɼ��������ͼ��ѡ���ݾ�����·�����󡣷�ΧΪ1-25�ס�
* @param:	unsigned char* fusionImg		�����YUV�ںϵ�ͼ�����ݡ��ڴ����û�������ͷš�
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
* @note:
**/
ITA_API ITA_RESULT ITA_ImageFusion(HANDLE_T instance, unsigned char* irData, unsigned char* visData, int irWidth, int irHeight,
	int visWidth, int visHeight, float distance, unsigned char* fusionImg);

/**
* @brief:	Բ����ȡ��
* @param:	HANDLE_T instance	 			ʵ�������
* @param:	unsigned char* data			    ̽�����ݡ�
* @param:	int width,int height			ͼ��Ŀ��ߡ�
* @param:	ITA_PIXEL_FORMAT format			ͼ��ĸ�ʽ��
* @param:	const ITARectangle rect        ̽��ľ�������
* @param:	int index						ͼ���������
* @param:	ITA_Image_TYPE imgType		    ͼ������͡�
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
* @note:
**/
ITA_API ITA_RESULT ITA_DetectPoint(HANDLE_T instance,unsigned char* data,int width,int height, ITA_PIXEL_FORMAT format, ITARectangle rect, int index, ITA_IMAGE_TYPE imgType);

/**
* @brief:	���������㡣
* @param:	HANDLE_T instance	 			ʵ�������
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
* @note:
**/
ITA_API ITA_RESULT ITA_AffineTransTable(HANDLE_T instance);

/**
* @brief:	���������ת�����ŷ���ƽ��ֵ��
* @param:	HANDLE_T instance	 			ʵ�������
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
* @note:
**/
ITA_API ITA_RESULT ITA_AverageAllMatrix(HANDLE_T instance);

/**
* @brief:	����������X������Y����ƫ����
* @param:	HANDLE_T instance	 		ʵ�������
* @param:	int index					����ͼ�������ֵ��
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
* @note:
**/
ITA_API ITA_RESULT ITA_ReverseCalAffineTransTable(HANDLE_T instance,int index);

/**
* @brief:	���㲢��ȡ���б궨�����µ�X Y ƫ������100mmΪ��ࡣ
* @param:	HANDLE_T instance	 		ʵ�������
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
* @note:
**/
ITA_API ITA_RESULT ITA_Getinterp1HermitePchip(HANDLE_T instance);

/**
* @brief:	���㻭�л������ϽǺ����½����ꡣ
* @param:	HANDLE_T instance	 		ʵ�������
* @param:	float distance				�������,�ɼ��������ͼ��ѡ���ݾ�����·�����󡣷�ΧΪ1-25�ס�
* @param:	ITARectangle* rect			�������Ļ��л������ϽǺ����½����ꡣ
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
* @note:
**/
ITA_API ITA_RESULT ITA_CalculateRect(HANDLE_T instance, float distance, ITARectangle* rect);

/**
* @brief:	������־���������·����ע��Ҫ��ITA_FusionInit֮����á�
* @param:	HANDLE_T instance	 	  ʵ�������
* @param:	const char* path		  ���뱣����־������·����
* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
* @note:
**/
ITA_API ITA_RESULT ITA_SetLoggerPath(HANDLE_T instance,const char* path);

ITA_FUSION_END

#endif // !ITA_FUSION_H