/*************************************************************
Copyright (C), 2021--2022, Wuhan Guide Sensmart Tech Co., Ltd
File name   : ITADTD.h
Author      : Guide Sensmart RD. wangyan
Version     : 1.0
Date        : 2021/11/9
Description : ITA SDK data structure definition.
*************************************************************/
#ifndef ITA_DTD_H
#define ITA_DTD_H

#include "ERROR.h"

#ifdef WINDOWS_PLATFORM
#    define ITA_API __declspec(dllexport)
#else
#    define ITA_API
#endif // WINDOWS_PLATFORM

#ifdef WINDOWS_PLATFORM
typedef void *				HANDLE_T;	//����32bit��64bit����ϵͳ
#else
typedef unsigned long long	HANDLE_T;
#endif // WINDOWS_PLATFORM

typedef enum ITAMode
{
	ITA_X16,
	ITA_MCU_X16,
	ITA_Y16,
	ITA_FPGA_Y16
}ITA_MODE;

typedef enum ITARange
{
	ITA_HUMAN_BODY,     //������·�Χ
	ITA_INDUSTRY_LOW,   //��ҵ���³��¶�
	ITA_INDUSTRY_HIGH,  //��ҵ���¸��¶�
	ITA_CUSTOM_RANGE1,  //TEC���²�Ʒ�Զ��巶Χ1
	ITA_CUSTOM_RANGE2,  //TEC���²�Ʒ�Զ��巶Χ2
	ITA_CUSTOM_RANGE3,   //TEC���²�Ʒ�Զ��巶Χ3
	ITA_CUSTOM_RANGE4   //TEC���²�Ʒ�Զ��巶Χ4
}ITA_RANGE;

typedef enum ITASensorValue
{
	ITA_AD_VALUE,		/* �´�ADֵ��int�� */
	ITA_TEMP_VALUE		/* �´��¶�ֵ��float�� */
} ITA_SENSOR_VALUE;

/*��ITA_SENSOR_VALUE������ADֵ�����¶ȡ�*/
typedef enum ITATempType
{
	ITA_SHUTTER_TEMP,	/* ������ */
	ITA_FPA_TEMP,		/* ���� */
	ITA_LENS_TEMP		/* ��Ͳ�� */
} ITA_TEMP_TYPE;

typedef enum ITAShutterStatus
{
	ITA_STATUS_OPEN,		//���ŵ���״̬��
	ITA_STATUS_CLOSE,		//���űպ�״̬��
	ITA_STATUS_COMPENSATE	//���Ų���״̬��
}ITA_SHUTTER_STATUS;

/*�����û����õ�ͼ���ʽ�����ű�������ת��ʽ������Ľ����
* ��ת����ת��ͼ���Y16���ݶ���Ч������ֻ��ͼ����Ч��
*/
typedef struct ITA_IMG_INFO {
	int imgDataLen;			//�Ŵ���ͼ�����ݳ��ȣ���λByte��
	int imgW;				//��ת���Ŵ��ķֱ���
	int imgH;
	int y16Len;				//���ݳ��ȣ���λshort��
	int w;					//��ת��ķֱ���
	int h;
}ITAImgInfo;

/**
* ITA��ISPͼ�����������������
*/
typedef struct ITA_ISP_RES {
	unsigned char *imgDst;
	short *y16Data;
	ITAImgInfo info;
}ITAISPResult;

typedef enum ITAMatrixType
{
	ITA_NORMAL_MATRIX,	//�¶Ⱦ���
	ITA_FAST_MATRIX		//�����¶Ⱦ���ռ��CPU��Դ��Խ��٣��������ITA_NORMAL_MATRIX����1%���ڣ�����ڡ�0.2���ڡ�
}ITA_MATRIX_TYPE;

typedef enum ITASwitch
{
	ITA_DISABLE,		//�ر�
	ITA_ENABLE			//��
}ITA_SWITCH;

typedef enum ITADCType
{
	ITA_DETECTOR_VALID_MODE,       //0��̽����VSYNC��HSYNC����Ч��1��̽����VSYNC��HSYNC����Ч��Ĭ��0���������ͣ����͡�
	ITA_DETECTOR_DATA_MODE,        //0���ȷ�16λ���ݵĸ�8bits��1���ȷ�16λ���ݵĵ�8bits��Ĭ��1���������ͣ����͡�
	ITA_DETECTOR_FRAME_FREQUENCY   //����̽����֡Ƶ����Χ1-30����λfps������ʹ�����µ���֡Ƶ��9��15��20��25��30��Ĭ��25���������ͣ����͡�
}ITA_DC_TYPE;	//Detector control type

typedef enum ITABCType
{
	ITA_SHUTTER_NOW,			//�����
	ITA_AUTO_SHUTTER,			//�����Զ����ţ�Ĭ�ϴ򿪡��������ͣ�ITA_SWITCH��
	ITA_SHUTTER_TIME,			//�����Զ����ż��ʱ�䣬��λ�롣Ĭ��30�롣�������ͣ����͡�����û������Զ�����ʱ�䣬��ôĬ�ϵ��Զ����Ų��Բ�����Ч��
	ITA_SINGLE_STEP_TIME,		//���õ������ƿ��Ŵӱպϵ����������̵�ʱ�䣬��λms��Ĭ��Լ400ms���������ͣ����͡�
	ITA_STEP_DELAY_TIME,		//���õ������ƿ���ʱ������պϵ���ʱ����λms��Ĭ��100ms������USB���ӵĲ�Ʒ��������400ms���������ͣ����͡�
	ITA_B_DELAY,                //���òɼ�������ʱ����λ֡��Ĭ��5֡���û����Ը�����Ҫ������ʱ���������ͣ����͡�
	ITA_GET_SHUTTER_STATUS,		//��ȡ����״̬�����űպ�ʱ�û����Զ����档�������ͣ�ITA_SHUTTER_STATUS����X16ʹ�ÿ��Ų��Ե�������õ���
	ITA_FIRST_NUC_FINISH,		//��һ��NUC�Ƿ���ɡ��������ͣ�ITA_SWITCH��ITA_DISABLE��δ��ɣ�ITA_ENABLE������ɡ�
	ITA_DO_NUC,					//�ֶ�NUC
	ITA_AUTO_GEARS,				//�Զ��н��µ�λ���أ�Ĭ�ϴ򿪡��������ͣ�ITA_SWITCH���ֶ���֮ǰ�ȹر��Զ��л���
	ITA_CHANGE_GEAR,			//�ֶ��н��µ�λ���������ͣ����Σ�>= 0������ITA_Version��ȡ��λ�������ֶ���֮ǰ�ȹر��Զ��л���
	ITA_SHUTTER_POLICY,			//�����Զ����ź�NUC���ԵĿ��ء��������ͣ�ITA_SWITCH��Ĭ�ϴ򿪡�ͨ����X16ģʽ�´򿪣���Y16ģʽ�¹رգ�FPGA��Ʒ���⣩��
	ITA_CLOSE_SHUTTER,			//�������ƿ��űպϣ��޲�������disable ITA_SHUTTER_POLICY���ٵ������ƿ��š�
	ITA_OPEN_SHUTTER,			//�������ƿ��ŵ������޲�������disable ITA_SHUTTER_POLICY���ٵ������ƿ��š�
	ITA_IMAGE_PROCESS,			//����ͼ�����������ͣ�ITA_SWITCH��Ĭ�ϴ򿪡���X16ģʽ�²��ܹرգ���Y16ģʽ�¿��Թرա�����ĳ���Ʒͨ��оƬISP�Ѿ�����YUV��ʹ��ITA�����¡�
	ITA_SET_FRAMERATE,			//����֡�ʣ�1-25֮����Ч��ԭʼ֡��25fps������֡�ʿ��Ե���CPU���ء��������ͣ����͡�
	ITA_GET_FRAMERATE,			//��ȡʵ��֡�ʡ��������ͣ����͡�
	ITA_SET_CONSUME,			//����ִ��һ��ͼ�������ĵ�ƽ��ʱ�䣬ֵ���Բο�ITA_ISP_CONSUME���������ͣ����͡���λ��ms����ʹ��ITA_SET_FRAMERATE������£���ֵӰ��ʵ��֡�ʡ�
	ITA_NUC_REPEAT_CORRECTION,  //�ظ�NUC������ơ��������ͣ�ITA_SWITCH��Ĭ�Ϲرա�
	ITA_NUC_ANOTHER_SHUTTER,    //�ظ����ž�����ơ��������ͣ�ITA_SWITCH��Ĭ�Ϲرա�
	ITA_NUC_REPEAT_THRESHOLD,   //�ж�NUCʧЧ����ֵ���������ͣ����Ρ�Ĭ��400��������120ģ�顣�û����ݲ�Ʒ��Ҫ������ֵ��
	ITA_AUTO_FOCUS,				//�Զ��������������ͣ����Ρ�0����������,���Ϊ������1����������,���ΪԶ����2��У׼����,���Ϊ������3��У׼����,���ΪԶ����
	ITA_AF_NEAR,				//�������
	ITA_AF_FAR,					//����Զ��
	ITA_STEP_NEAR,				//����һ��ʱ�����ڲ�������
	ITA_STEP_FAR,				//����һ��ʱ�����ڲ���Զ��
	ITA_CH_DELAY,				//�������Ȼ�������ʱ���ж�
	ITA_SET_CHSTATUS			//�������Ȼ���״̬��1:��� 0:�Ȼ�;��������Ϊunsigned char,����ITA_Register֮ǰ����/*���������ʹ�ã��豸���ϱ����Ȼ���״̬�����������*/
}ITA_BC_TYPE;	//Base control type

typedef enum ITAMCType
{
	ITA_SET_DISTANCE,			//��Ч���뷶Χ�ɾ����Ʒ������Ĭ��1.5�ס��������ͣ������͡�
	ITA_SET_EMISS,				//���÷�����(��ҵר��)0.01-1.0��Ĭ��0.95. �������ͣ������͡�	
	ITA_SET_TRANSMIT,			//͸����(��ҵר�ã�0.01-1, default 1)���������ͣ������͡�
	ITA_REFLECT_TEMP,			//�����¶�(��ҵר��)���������ͣ������͡�
	ITA_SET_ENVIRON,			//���û����¶ȡ����û������õ�����£��ڲ��Զ����㻷�¡�����û����ã���ʹ�����õ�ֵ���������ͣ������͡�
	ITA_GET_ENVIRON,			//��ȡ���¡�
	ITA_GET_FPA_TEMP,			//��ȡ���¡�
	ITA_ENVIRON_CORR,			//�����������ء�Ĭ�Ϲرա��������ͣ�ITA_SWITCH��
	ITA_LENS_CORR,				//��Ͳ��ƯУ�����ء�Ĭ�ϴ򿪡��������ͣ�ITA_SWITCH��
	ITA_DISTANCE_COMPEN,		//���벹�����ء�Ĭ�ϴ򿪡��������ͣ�ITA_SWITCH��
	ITA_EMISS_CORR,				//������У�����ء�Ĭ�ϴ򿪡��������ͣ�ITA_SWITCH��
	ITA_TRANS_CORR,				//͸����У�����ء�Ĭ�Ϲرա��������ͣ�ITA_SWITCH��
	ITA_ATMOSPHERE_CORR,		//����͸����У�����ء�Ĭ�Ϲرա��������ͣ�ITA_SWITCH��
	ITA_LOW_LENS_CORR_K,		//ʵʱ���µ���Ͳ��Ư����ϵ�����������ͣ������͡�
	ITA_HIGH_LENS_CORR_K,		//ʵʱ���µ���Ͳ��Ư����ϵ�����������ͣ������͡�
	ITA_OTHER_LENS_CORR_K,		//ʵʱ��������Ͳ��Ư����ϵ�����������ͣ������͡�
	ITA_SHUTTER_CORR,			//������ƯУ�����ء�Ĭ�Ϲرա��������ͣ�ITA_SWITCH��
	ITA_LOW_SHUTTER_CORR_K,		//���µ�����У��ϵ�����������ͣ������͡�
	ITA_HIGH_SHUTTER_CORR_K,	//���µ�����У��ϵ�����������ͣ������͡�
	ITA_OTHER_SHUTTER_CORR_K,	//����������У��ϵ�����������ͣ������͡�
	ITA_SUB_AVGB,				//���¼�ȥ���׾�ֵ���ء�Ĭ�Ϲرա��������ͣ�ITA_SWITCH��
	ITA_GET_AVGB,				//��ȡ���׾�ֵ���������ͣ�short��
	ITA_CENTRAL_TEMPER,  		//��ȡ�˲���������¡��������ͣ������͡�
	ITA_SET_HUMI,				//����ʪ��(��ҵר��)0.01-1.0,Ĭ��0.6. �������ͣ������͡�
	ITA_HUMI_CORR,				//ʪ��У�����ء�Ĭ�Ϲرա��������ͣ�ITA_SWITCH��
	ITA_SET_WINDOWTEMP,			//���ô����¶�(Ĭ��23).�������ͣ������͡�
	ITA_SET_ATMOSPHTEMP,		//���ô����¶�(Ĭ��23).�������ͣ������͡�
	ITA_SET_ATMOSPHTRSMIT		//���ô���͸����(��Χ0.01-1.0,Ĭ��1.0).�������ͣ������͡�
}ITA_MC_TYPE;	//Measure control type

typedef enum ITAPixelFormat
{
	ITA_RGB888,			//Ĭ�����ظ�ʽ
	ITA_BGR888,
	ITA_RGBA8888,
	ITA_RGB565,
	ITA_YUV422_YUYV,
	ITA_YUV422_UYVY,
	ITA_YUV422_Plane,
	ITA_YVU422_Plane,
	ITA_YUV420_YUYV,//NV12
	ITA_YUV420_UYVY,//NV21
	ITA_YUV420_Plane,
	ITA_YVU420_Plane
}ITA_PIXEL_FORMAT;

/**
* 13��α�ʣ�0���ȣ�1���ң�2���죬3������4ҽ�ƣ�5������6�ʺ�1��7�ʺ�2��8���ȣ�9����ɸ�飬10��죬11���ȣ�12�����ȡ�
* ֧�����ӵ�26�֣��û�ʹ��ITA_Version��ѯα��������
*/
typedef enum ITAPaletteType
{
	ITA_WHITE_HEAT = 0,		//Ĭ�ϰ���
	ITA_LAVA = 1,			//����
	ITA_IRON_RED = 2,		//����
	ITA_HOT_IRON = 3,		//����
	ITA_MEDICAL_TREAT = 4,	//ҽ��
	ITA_ARCTIC = 5,			//����
	ITA_RAINBOW = 6,		//�ʺ�
	ITA_RAINBOW_EX = 7,		//�ʺ�2
	ITA_BLACK_HOT = 8,		//����
	ITA_BODY_SCREEN = 9,	//����ɸ��
	ITA_RED_TINT = 10,		//���
	ITA_BLUE_HOT = 11,		//����
	ITA_GREEN_HOT = 12,		//����
	ITA_MEDICAL_TREAT1 = 13,//ҽ��1
	ITA_MEDICAL_TREAT2 = 14,//ҽ��2
	ITA_MEDICAL_TREAT3 = 15,//ҽ��3
	ITA_RAINBOW3 = 16,		//�ʺ�3
	ITA_RED_TINT2 = 17,		//���2
	ITA_RED_TINT3 = 18,		//���3
	ITA_ICE_FIRE = 19,		//����
	ITA_REVERSE_ICEFIRE = 20,//������
	ITA_STEPS = 21,			//�ֲ�
	ITA_SEPIA = 22,			//���
	ITA_WINTER = 23,		//����
	ITA_HIGH_LIGHT = 24,	//����
	ITA_AMBER = 25			//����
}ITA_PALETTE_TYPE;

typedef enum ITARotateType
{
	ITA_ROTATE_NONE,	//����ת
	ITA_ROTATE_90,		//��ת90��
	ITA_ROTATE_180,
	ITA_ROTATE_270
}ITA_ROTATE_TYPE;

typedef enum ITAFlipType
{
	ITA_FLIP_NONE,			//����ת
	ITA_FLIP_HORIZONTAL,	//ˮƽ��ת
	ITA_FLIP_VERTICAL,		//��ֱ��ת 
	ITA_FLIP_HOR_VER		//ˮƽ+��ֱ��ת
}ITA_FLIP_TYPE;

typedef enum ITAZoomType
{
	ITA_ZOOM_NEAR,
	ITA_ZOOM_BILINEAR,
	ITA_ZOOM_BILINEARFAST
}ITA_ZOOM_TYPE;

typedef enum ITAPotCoverAlgoType
{
	ITA_POTCOVER_FIX,//�̶�����ϵ��
	ITA_POTCOVER_VAR1,//���ڿ��������¼���ϵ��
	ITA_POTCOVER_VAR2//������һ�ο����¼���ϵ��
}ITA_POTCOVER_ALGO_TYPE;

typedef struct ITA_POTCOVER_MODEL_DATA
{
	int width;
	int height;
	short* modelData;
}ITAPotcoverModelData;

typedef enum ITAPotCoverType
{
	ITA_POTCOVER_GET_MODEL,//��ȡ����ģ�����ݡ��û������Լ����б��棬�������ʹ�á��������ͣ�ITA_POTCOVER_MODEL_DATA
	ITA_POTCOVER_SET_MODEL,//���ù���ģ�����ݡ��������ͣ�ITA_POTCOVER_MODEL_DATA
	ITA_POTCOVER_SET_CALMODEL,//���ü������ģ�忪�أ��������ͣ�ITA_SWITCH,Ĭ��ֵ��ITA_DISABLE��
	ITA_POTCOVER_GET_CALMODEL,//��ȡ�������ģ�Ϳ��أ��������ͣ�ITA_SWITCH
	ITA_POTCOVER_SET_ALGOTYPE,//���ù����㷨���ͣ��������ͣ�ITA_POTCOVER_ALGO_TYPE��Ĭ��ֵ��ITA_POTCOVER_FIX��
	ITA_POTCOVER_GET_ALGOTYPE,//��ȡ�����㷨���ͣ��������ͣ�ITA_POTCOVER_ALGO_TYPE��
	ITA_POTCOVER_SET,//���ù����㷨���أ��������ͣ�ITA_SWITCH,Ĭ��ֵ��ITA_DISABLE��
	ITA_POTCOVER_GET//��ȡ�����㷨���أ��������ͣ�ITA_SWITCH��
}ITA_POTCOVER_OP_TYPE;

typedef enum ITAICType
{
	ITA_SET_FORMAT,			//���ó������ظ�ʽ��֧�ֶ�̬���á��������ͣ�ITA_PIXEL_FORMAT��
	ITA_SET_PALETTE,		//����α�ʣ�ͨ���ӿ�ITA_Version��ȡα�ʺŷ�Χ���������ͣ�ITA_PALETTE_TYPE��
	ITA_SET_SCALE,			//���ű�����֧��ͼ��̬ʵʱ���š��������ͣ����㡣
	ITA_SET_CONTRAST,		//���öԱȶȡ��Աȶ�0-511��Ĭ��255���������ͣ����Ρ�
	ITA_SET_BRIGHTNESS,		//�������ȡ�����0-255��Ĭ��70���������ͣ����Ρ�
	ITA_SET_ROTATE,			//������ת��ʽ��RGB��Y16���ݾ���ת���������ͣ�ITA_ROTATE_TYPE��
	ITA_SET_FLIP,			//���÷�ת��RGB��Y16���ݾ���ת���������ͣ�ITA_FLIP_TYPE��
	ITA_SET_REVERSE,		//�򿪻��߹رշ�����ITA_X16ģʽ��Ĭ�ϴ򿪡��������ͣ�bool��
	ITA_SET_HSM_WEIGHT,		//����������֡���ֵȨ�ء���Χ-1000-1000��Ĭ��100���������ͣ����Ρ�
	ITA_SET_HSM_STAYTHR,	//���������⾲̬��ֵ����Χ0-65535��Ĭ��30���������ͣ����Ρ�
	ITA_SET_HSM_STAYWEIGHT, //���������⾲̬֡���ֵȨ�ء���Χ-1000-1000��Ĭ��10���������ͣ����Ρ�
	ITA_HSM_INTERVAL,		//���������⻺������������Χ0-16383��Ĭ��50���������ͣ����Ρ�
	ITA_HSM_MIDFILTER,		//������������ֵ�˲����ء���Χ0��1��Ĭ��0���������ͣ�unsigned char��
	ITA_HSM_MIDFILTER_WINSIZE,//������������ֵ�˲����ڡ���Χ0-20��Ĭ��3���������ͣ�int��
	ITA_HSM_DELTA_UPTHR,	//����֡���ֵ����ֵ���ޡ���Χ0-16383��Ĭ��5000���������ͣ�int.
	ITA_HSM_DELTA_DOWNTHR,	//����֡���ֵ����ֵ���ޡ���Χ-16383-0��Ĭ��ֵ-5000.�������ͣ�int.
	ITA_HSM_POINT_THR,		//���õ���������ֵ����Χ0-16383��Ĭ��ֵ5.�������ͣ�int.
	ITA_TIME_FILTER,		//ʱ���˲����أ����Թرջ��ߴ򿪡��������ͣ�bool��
	ITA_SPATIAL_FILTER,		//�����˲����أ����룬���Թرջ��ߴ򿪡��������ͣ�bool��
	ITA_REMOVE_VERTICAL,	//ȥ���ƣ����Թرջ��ߴ򿪡��������ͣ�bool��
	ITA_REMOVE_HORIZONTAL,	//ȥ���ƣ����Թرջ��ߴ򿪡��������ͣ�bool��
	ITA_HSM,				//�����⣬ ���Թرջ��ߴ򿪡� �������ͣ�bool��
	ITA_SHARPENING,			//�񻯣����Թرջ��ߴ򿪡��������ͣ�bool��
	ITA_DETAIL_ENHANCE,		//ϸ����ǿ�����Թرջ��ߴ򿪡��������ͣ�bool��
	ITA_BLOCK_HISTOGRAM,	//�ֿ�ֱ��ͼ�����Թرջ��ߴ򿪡��������ͣ�bool���ù��ܷ�����
	ITA_GAMMA_CORRECTION,	//GammaУ�������Թرջ��ߴ򿪡��������ͣ�bool��
	ITA_Y8_ADJUSTBC,		//Y8��ƫ�����Թرջ��ߴ򿪡��������ͣ�bool��
	ITA_DRT_Y8,				//���⣬Ĭ�ϴ򿪡�����رգ���ô������Y16תY8����������imgDstͼ�����ݡ��������ͣ�bool��
	ITA_TFF_STD,            //ʱ���˲���׼�Ĭ��ֵ10���������ͣ����Ρ�
	ITA_RESTRAIN_RANGE,     //�������Ʒ�Χ��Ĭ��ֵ64���������ͣ����Ρ�
	ITA_DISCARD_UPRATIO,    //�������׵������Ĭ��ֵ1��ȡֵ��Χ0-10���������ͣ����Ρ�
	ITA_DISCARD_DOWNRATIO,    //�������׵������Ĭ��ֵ1��ȡֵ��Χ0-10���������ͣ����Ρ�
	ITA_MIX_THRLOW,			  //��ϵ������ֵ��Ĭ��ֵ100��ȡֵ��Χ0-16383.�������ͣ�int.
	ITA_MIX_THRHIGH,		  //��ϵ������ֵ��Ĭ��ֵ300��ȡֵ��Χ0-16383.�������ͣ�int.
	ITA_HEQ_PLAT_THRESH,     //ƽֱ̨��ͼ������ֵ������Ĭ��ֵ5��ȡֵ��Χ1-1000���������ͣ����Ρ�
	ITA_HEQ_RANGE_MAX,     //ƽֱ̨��ͼ����ӳ�䷶Χ������Ĭ��ֵ230��ȡֵ��Χ0-512���������ͣ����Ρ�
	ITA_HEQ_MIDVALUE,     //ƽֱ̨��ͼ����ӳ����ֵ������Ĭ��ֵ128��ȡֵ��Χ0-255���������ͣ����Ρ�
	ITA_IIE_ENHANCE_COEF,     //ϸ������ϵ����Ĭ��ֵ32��ȡֵ��Χ0-128���������ͣ����Ρ�
	ITA_IIE_GAUSS_STD,     //��ǿ�㷨������Ĭ��ֵ5��ȡֵ��Χ1-50���������ͣ����Ρ�
	ITA_GMC_GAMMA,     //Gamma�㷨������Ĭ��ֵ1.25��ȡֵ��Χ0-2���������ͣ������͡�
	ITA_GMC_TYPE,      //Gamma�㷨������Ĭ��ֵ0��ȡֵ��Χ0-1���������ͣ����Ρ�
	ITA_SET_ZOOM,	   //���÷Ŵ����ͣ��������ͣ�ITA_ZOOM_TYPE��
	ITA_SET_LAPLACE_WEIGHT, //�񻯲�����������ϵ�����������ͣ������͡�ȡֵ��Χ0-10��
	ITA_LINEAR_RESTRAIN_RANGETHR,  //����������
	ITA_AUTO_RPBDPS		//�Զ�ȥ���㣬�������ͣ�unsigned char��ȡֵ��Χ��0��1.Ŀǰ��ZM11A��ʹ�á�
}ITA_IC_TYPE;	//Image control type

/*Dynamic range transform type ��̬��Χת������*/
typedef enum ITADRTType
{
	ITA_DRT_LINEAR = 0,		//���Ե��⣬Ĭ��
	ITA_DRT_PLATHE,			//ƽֱ̨��ͼ����
	ITA_DRT_MIX,			//��ϵ���
	ITA_DRT_MANUAL			//�ֶ�����
}ITA_DRT_TYPE;

typedef struct ITA_DRT_PARAM
{
	//�ֶ��������
	short manltone_maxY16;
	short manltone_minY16;
}ITADRTParam;

typedef enum ITAEqualLineType
{
	ITA_EQUAL_NONE,		//�رյ����߹���
	ITA_EQUAL_HIGH,
	ITA_EQUAL_LOW,
	ITA_EQUAL_HIGHLOW,
	ITA_EQUAL_MIDDLE
}ITA_EQUAL_LINE_TYPE;

typedef struct ITA_EQUAL_LINE_PARAM
{
	short highY16;				//��Y16
	short lowY16;				//��Y16
	int color;					//�����ߵ�һ����ɫ��rgb888��ʽ��
	int otherColor;				//�����ߵ���һ����ɫ��rgb888��ʽ��
}ITAEqualLineParam;

typedef enum ITADebugType
{
	ITA_SET_DEBUGGING,			//�����Ƿ������ԡ��������ͣ�ITA_SWITCH��ITA_DISABLE���رյ��ԡ�ITA_ENABLE���������ԡ����δ������־·������ôĬ�ϱ����ڳ���ͬ��Ŀ¼��ITA.log��
	ITA_DEBUGGING_PATH,			//������־���������·����ע��Ҫ��ITA_Init֮����á��������Ѵ��ڵ�Ŀ¼������/home/app/debug.log��Windows����"D:/Tools/debug.log"�����Ȳ�Ҫ����128���������ͣ��ַ������� \0 ������
	ITA_DEBUGGING_MODE,			//������־���ģʽ���������ͣ����Ρ�1����־������ļ���Ĭ�������ʽ��2����־���������̨��3���û��Զ�����־����������£�ͬʱ������ļ���֧���û��Զ��������־����ITARegistry��
	ITA_SNAPSHOT,				//���ձ��浱ǰ֡ͼ�����������ITA_DEBUGGING_PATH����ô�洢�����õ�·��������洢�ڳ���Ŀ¼�¡�
	ITA_START_RECORD,			//��ʼ¼X16/Y16���ݡ�
	ITA_STOP_RECORD,			//ֹͣ¼X16/Y16���ݡ�
	ITA_MEMORY_USAGE,			//��ѯ��̬�ڴ�ʹ�������������ͣ����͡���λ��Byte��
	ITA_ISP_CONSUME				//��ѯִ��һ��ͼ�������ĵ�ʱ�䡣�������ͣ����͡���λ��ms��
}ITA_DEBUG_TYPE;	//Debug type

typedef enum ITACorrectType
{
	ITA_TRY_CORRECT_POINT,	//�ȳ��Ա궨���㣬ʵʱͼ��ȷ�ϱ궨�ɹ�������ӡ��������ͣ�ITA_POINT��
	ITA_CORRECT_POINT,		//��ӻ��㣬���Զ����ӡ��������ͣ�ITA_POINT��
	ITA_SAVE_POINTS 		//���滵�㡣�����������ɺ󣬱���ָ�����·�Χ�µĻ��㡣�������ͣ�ITA_RANGE��
}ITA_CORRECT_TYPE;

typedef struct ITA_POINT {
	int x;
	int y;
}ITAPoint;

typedef enum ITACollectType
{
	ITA_COLLECT_DISTANCE,		//������ΪУ�²�����
	ITA_COLLECT_ENVIRON 		//������ΪУ�²�����
}ITA_COLLECT_TYPE;

typedef struct ITA_COLLECT_PARAM
{
	ITA_COLLECT_TYPE type;	//��������
	float param;			//����ֵ
	float blackTemp;		//�����¶�
}ITACollectParam;

typedef struct ITA_CORRECT_PARAM
{
	int BlackBodyNumber;
	float TBlackBody[10];
	short Y16[10];
	float ErrorNoCorrect[10];
	float ErrorCorrect[10];
}ITACalibrateResult;

//�û��Զ���α�ʲ�������
typedef enum ITAPaletteOPType
{
	ITA_PALETTE_OP_ADD,			//�����������û��Զ���α�ʡ��������ͣ�ITAPaletteInfo��
	ITA_PALETTE_OP_UPDATE,		//������Ÿ����û��Զ���α�ʡ��������ͣ�ITAPaletteInfo��
	ITA_PALETTE_OP_DELETE 		//�������ɾ���û��Զ���α�ʡ��������ͣ�ITAPaletteInfo��
}ITA_PALETTE_OP_TYPE;

//�û��Զ���α����Ϣ
typedef struct ITA_PALETTE_INFO
{
	int paletteID;			//��ţ��û��Զ����α�����Ӧ��ITAVersionInfo��ȡ��totalNumberOfPalettes֮��ʼ��ӡ�
	unsigned char *data;	//�Զ���α�����ݡ�ÿ��α�����ݳ���256 * 4B��
	int dataLen;			//α�����ݳ��ȣ���λByte��
}ITAPaletteInfo;

typedef void(*ITACollectFinish)(int cameraID, void* param);

typedef void(*ITACollectBaseFinish)(int cameraID,int gear,unsigned short* baseData,int dataLen, void *param);

typedef void(*ITAReviewFinish)(int cameraID, bool status, float averTemp, float deviation, void* userParam);

/*У�²��������飬һ���Զ�У�µĲ�����һ���ֶ�У�µĲ�����*/
typedef enum ITACalibrateType
{
	ITA_NEAR_KF_AUTO,
	ITA_NEAR_B_AUTO,
	ITA_FAR_KF_AUTO,
	ITA_FAR_B_AUTO,
	ITA_NEAR_KF_MANUAL,
	ITA_NEAR_B_MANUAL,
	ITA_FAR_KF_MANUAL,
	ITA_FAR_B_MANUAL
}ITA_CALIBRATE_TYPE;

/*�����汾�ź͹�����Ϣ����ͬ�İ汾�����Ĺ��ܿ��ܴ��ڲ��졣*/
typedef struct ITA_VERSION_INFO
{
	char version[64];				//�汾��
	int totalNumberOfPalettes;		//α������
	int totalNumberOfGears;			//���µ�λ����
	char confVersion[64];           //�����ļ��汾��
	char moduleCode[24];            //ģ����
	float correctionFactors[9];     //�ڲξ��������У��ϵ��
	int productType;				//��Ʒ���� 0:��TEC��Ʒ 1��TEC��Ʒ
}ITAVersionInfo;

/*�����¼�֪ͨ�����е��¼�����*/
typedef enum ITAEventType
{
	ITA_NUC_BEGIN,		//��ʼ��NUC
	ITA_NUC_END,		//���һ��NUC
	ITA_SHUTTER_BEGIN,	//��ʼ����
	ITA_SHUTTER_END,	//���һ�δ����
	ITA_AF_BEGIN,		//��ʼ����
	ITA_AF_END			//��ɵ���������eventParamָʾ�����ɹ�����ʧ�ܡ��������ͣ�ITA_RESULT��
}ITA_EVENT_TYPE;


typedef enum ITAAFCalibrate
{
	ITA_AF_CALIBRATE_DIFF = 0,		//�س̲�У׼
	ITA_AF_CALIBRATE_LASER			//����У׼
}ITA_AF_CALIBRATE;

typedef enum ITAAFInfo
{
	ITA_AF_BASE_INFO = 0,			//������������
	ITA_AF_LASER_INFO,				//�����������
	ITA_AF_CONTRAST_INFO			//�ԱȶȲ�������
}ITA_AF_INFO;

typedef enum ITAFieldAngle
{
	ITA_ANGLE_56 = 0, //F1.1
	ITA_ANGLE_25 = 1, //F1.0
	ITA_ANGLE_120 = 2, //F1.1
	ITA_ANGLE_50 = 3, //F1.1
	ITA_ANGLE_90 = 4, //F1.0
	ITA_ANGLE_33 = 5, //F1.0
	ITA_ANGLE_25_F11 = 6,//F1.1���佹
	ITA_ANGLE_45 = 7, //F1.0
	ITA_ANGLE_15 = 8, //F1.2
	ITA_ANGLE_6 = 9, //F1.2
	ITA_CUSTOM_LENS1 = 10,  //��Ʒ�Զ��徵ͷ1��΢�࣬���¾�ͷ�ȡ�
	ITA_CUSTOM_LENS2 = 11,  //��Ʒ�Զ��徵ͷ2
	ITA_CUSTOM_LENS3 = 12   //��Ʒ�Զ��徵ͷ3
}ITA_FIELD_ANGLE;

typedef struct ITA_CURVE_PARAM
{
	short temperFreeLens;			//�Ƿ���У�¾�ͷ��1���ǣ�0����
}ITACurveParam;

/*ע����ƣ���ITA_Init֮����á�ITAʹ��HAL functions��ɳ�ʼ����*/
typedef struct ITA_REGISTRY
{
	/**
	* @brief:	�������ļ�ITA.conf����ѡ����û�������ļ��������ITAʹ��Ĭ�ϲ�����
	* @param:	int cameraID			��ʶ�豸��֧��ͬʱ�򿪶�̨�豸��
	* @param:	unsigned char *buf		�����׵�ַ�����������
	* @param:	int bufSize				�����С�����������
	* @param:	int *length				�����ļ����ȣ����������
	* @param:	void *userParam			�û�������ע��ʱ���û����롣ITA�����ע�ắ��ʱ�ٴ����û���
	* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
	* @note �����ļ���ֻ���ġ�δ����Ȩ���޸���Ч��
	* @see ITA_Init��camera id���û����롣
	**/
	ITA_RESULT(*ReadConfig)(int cameraID, unsigned char *buf, int bufSize, int *length, void *userParam);

	/**
	* Register log function. Output logs using registered function.
	* @param:	int cameraID			��ʶ�豸��֧��ͬʱ�򿪶�̨�豸��
	* @param:	const char *logInfo		һ����־��Ϣ��
	* @param:	void *userParam			�û�������ע��ʱ���û����롣ITA�����ע�ắ��ʱ�ٴ����û���
	* @note ���û�ע�������־������ITA�ڲ����������־�������û�����ITA_DEBUGGING_MODEΪ3��
	*/
	void(*OutputLog)(int cameraID, const char *logInfo, void *userParam);

	/*************************HAL�ӿڶ���**************************************************/
	/* Register HAL(Hardware Abstract Layer) functions. Users implement HAL functions 
	 * according to product requirements. Not every interface must be implemented. Please 
	 * refer to the integration documentation for which interfaces need to be implemented.*/
	/**************************************************************************************/
	/**
	* @brief:	���ƿ��š�
	* @param:	int cameraID				��ʶ�豸��֧��ͬʱ�򿪶�̨�豸��
	* @param:	ITA_SHUTTER_STATUS status	����״̬��
	* @param:	void *userParam				�û�������ע��ʱ���û����롣ITA�����ע�ắ��ʱ�ٴ����û���
	* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
	* @see ITA_Init��camera id���û����롣
	**/
	ITA_RESULT(*ShutterControl)(int cameraID, ITA_SHUTTER_STATUS status, void *userParam);

	/**
	* @brief:	��NUC����MCU�Ĳ�Ʒ��ʵ�֡�
	* @param:	int cameraID				��ʶ�豸��֧��ͬʱ�򿪶�̨�豸��
	* @param:	void *userParam				�û�������ע��ʱ���û����롣ITA�����ע�ắ��ʱ�ٴ����û���
	* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
	* @see ITA_Init��camera id���û����롣
	**/
	ITA_RESULT(*DoNUC)(int cameraID, void *userParam);

	/**
	* @brief:	��ȡ�¶ȴ�����ֵ����ITA_SENSOR_VALUE�������ADֵ�����¶�ֵ��
	* @param:	int cameraID					��ʶ�豸��֧��ͬʱ�򿪶�̨�豸�����������
	* @param:	ITA_TEMP_TYPE eType				�´����͡����������
	* @param:	ITA_SENSOR_VALUE *sensorValue	ֵ���͡�������������û�������
	* @param:	int *ADValue					ADֵ�������������ITA_AD_VALUE==sensorValueʱ��Ч��
	* @param:	float *tempValue				�¶�ֵ�������������ITA_TEMP_VALUE==sensorValueʱ��Ч��
	* @param:	void *userParam					�û�������ע��ʱ���û����롣ITA�����ע�ắ��ʱ�ٴ����û���
	* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
	* @see ITA_Init��camera id���û����롣
	**/
	ITA_RESULT(*GetSensorTemp)(int cameraID, ITA_TEMP_TYPE eType, ITA_SENSOR_VALUE *sensorValue, int *ADValue, float *tempValue, void *userParam);

	/**
	* @brief:	��I2C��Timo 256ͨ��I2C����̽������
	* @param:	int cameraID				��ʶ�豸��֧��ͬʱ�򿪶�̨�豸��
	* @param:	unsigned char u32DevAddr
	* @param:	void *userParam				�û�������ע��ʱ���û����롣ITA�����ע�ắ��ʱ�ٴ����û���
	* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
	* @see ITA_Init��camera id���û����롣
	**/
	ITA_RESULT(*I2COpen)(int cameraID, unsigned char u32DevAddr, void *userParam);

	/**
	* @brief:	�ر�I2C��Timo 256ͨ��I2C����̽������
	* @param:	int cameraID			��ʶ�豸��֧��ͬʱ�򿪶�̨�豸��
	* @param:	void *userParam			�û�������ע��ʱ���û����롣ITA�����ע�ắ��ʱ�ٴ����û���
	* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
	* @see ITA_Init��camera id���û����롣
	**/
	ITA_RESULT(*I2CClose)(int cameraID, void *userParam);

	/**
	* @brief:	I2C���Ĵ�����Timo 256ͨ��I2C����̽������
	* @param:	int cameraID			��ʶ�豸��֧��ͬʱ�򿪶�̨�豸��
	* @param:	unsigned char u8Addr	�Ĵ�����ַ��
	* @param:	unsigned char * pu8Val	�����ļĴ���ֵ��
	* @param:	void *userParam			�û�������ע��ʱ���û����롣ITA�����ע�ắ��ʱ�ٴ����û���
	* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
	* @see ITA_Init��camera id���û����롣
	**/
	ITA_RESULT(*I2CRead)(int cameraID, unsigned char u8Addr, unsigned char * pu8Val, void *userParam);

	/**
	* @brief:	I2Cд�Ĵ�����Timo 256ͨ��I2C����̽������
	* @param:	int cameraID			��ʶ�豸��֧��ͬʱ�򿪶�̨�豸��
	* @param:	unsigned char u8Addr	�Ĵ�����ַ��
	* @param:	unsigned char u8Val		д��ļĴ���ֵ��
	* @param:	void *userParam			�û�������ע��ʱ���û����롣ITA�����ע�ắ��ʱ�ٴ����û���
	* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
	* @see ITA_Init��camera id���û����롣
	**/
	ITA_RESULT(*I2CWrite)(int cameraID, unsigned char u8Addr, unsigned char u8Val, void *userParam);

	/**
	* @brief:	SPI�������ݣ���д���ٶ���Timo 120ͨ��SPI����̽������
	* @param:	int cameraID			 ��ʶ�豸��֧��ͬʱ�򿪶�̨�豸��
	* @param:	unsigned char *writeBuf  ��д������ݻ��档
	* @param:	unsigned char *readBuf   �����ݵĻ��档
	* @param:	int bufLen               ���泤�ȡ�
	* @param:	void *userParam			 �û�������ע��ʱ���û����롣ITA�����ע�ắ��ʱ�ٴ����û���
	* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
	* @see ITA_Init��camera id���û����롣
	**/
	ITA_RESULT(*SPITransmit)(int cameraID, unsigned char *writeBuf, unsigned char *readBuf, int bufLen, void *userParam);

	/**
	* @brief:	SPIд���ݡ�Timo 120ͨ��SPI����̽������
	* @param:	int cameraID			 ��ʶ�豸��֧��ͬʱ�򿪶�̨�豸��
	* @param:	unsigned char *writeData ��д������ݡ�
	* @param:	int writeLen             ���ݳ��ȡ�
	* @param:	void *userParam			 �û�������ע��ʱ���û����롣ITA�����ע�ắ��ʱ�ٴ����û���
	* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
	* @see ITA_Init��camera id���û����롣
	**/
	ITA_RESULT(*SPIWrite)(int cameraID, unsigned char *writeData, int writeLen, void *userParam);

	/**
	* @brief:	ģ������Flash��GPIO�����ͣ���ʼ���¼Ĵ���ֵ��ʹ������Flash��ģ���Ʒ��Ҫʵ�ִ˺�����
	* @param:	int cameraID			 ��ʶ�豸��֧��ͬʱ�򿪶�̨�豸��
	* @param:	void *userParam			 �û�������ע��ʱ���û����롣ITA�����ע�ắ��ʱ�ٴ����û���
	* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
	* @see ITA_Init��camera id���û����롣
	**/
	ITA_RESULT(*InternalFlashPrepare)(int cameraID, void *userParam);

	/**
	* @brief:	ģ������Flash��GPIO�����ߣ��������¼Ĵ���ֵ��ʹ������Flash��ģ���Ʒ��Ҫʵ�ִ˺�����
	* @param:	int cameraID			 ��ʶ�豸��֧��ͬʱ�򿪶�̨�豸��
	* @param:	void *userParam			 �û�������ע��ʱ���û����롣ITA�����ע�ắ��ʱ�ٴ����û���
	* @return:	����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
	* @see ITA_Init��camera id���û����롣
	**/
	ITA_RESULT(*InternalFlashDone)(int cameraID, void *userParam);

	/**
	* @brief:  �����ݰ���
	* @param:  int cameraID			��ʶ�豸��֧��ͬʱ�򿪶�̨�豸��
	* @param:  unsigned char *buf	���ݻ���
	* @param:  int offset			������ݰ�ͷ��ƫ�ƣ���ƫ�ƴ���ʼ��size���ȵ����ݡ�
	* @param:  int size				��ȡ�����ݳ���
	* @param:  ITA_RANGE range		���·�Χ����ͬ��Ʒ֧�ֵ��¶ȷ�Χ��������ͬ��һ�㲻����3����
	* @param:	void *userParam		�û�������ע��ʱ���û����롣ITA�����ע�ắ��ʱ�ٴ����û���
	* @return: ����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
	* @see ITA_Init��camera id���û����롣
	**/
	ITA_RESULT(*ReadPackageData)(int cameraID, unsigned char *buf, int offset, int size, ITA_RANGE range, void *userParam);

	/**
	* @brief:  ��һ�ַ�ʽ�����ݰ������ڴ��в�ͬ��ͷ�Ȳ�κ���У�¾�ͷ��ģ������²�Ʒ��ReadPackageData��ReadPackageDataEx�����û�ֻ��ע������һ�������ͬʱע����ôReadPackageData���ȡ�
	* @param:  int cameraID			��ʶ�豸��֧��ͬʱ�򿪶�̨�豸��
	* @param:  unsigned char *buf	���ݻ���
	* @param:  int offset			������ݰ�ͷ��ƫ�ƣ���ƫ�ƴ���ʼ��size���ȵ����ݡ�
	* @param:  int size				��ȡ�����ݳ���
	* @param:  ITA_RANGE range		���·�Χ����ͬ��Ʒ֧�ֵ��¶ȷ�Χ��������ͬ��һ�㲻����3����
	* @param:  ITA_FIELD_ANGLE lensType ��ͷ����
	* @param:  void *extendParam     ��չ������Ϣ�������ʽ��ITACurveParam��
	* @param:  int extendParamSize   ��չ������Ϣ���ȡ�
	* @param:	void *userParam		�û�������ע��ʱ���û����롣ITA�����ע�ắ��ʱ�ٴ����û���
	* @return: ����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
	* @see ITA_Init��camera id���û����롣
	**/
	ITA_RESULT(*ReadPackageDataEx)(int cameraID, unsigned char *buf, int offset, int size, ITA_RANGE range, 
		ITA_FIELD_ANGLE lensType, void *extendParam, int extendParamSize, void *userParam);

	/**
	* @brief:  д���ݰ���
	* @param:  int cameraID			��ʶ�豸��֧��ͬʱ�򿪶�̨�豸��
	* @param:  unsigned char *buf	���ݻ���
	* @param:  int offset			������ݰ�ͷ��ƫ�ƣ���ƫ�ƴ���ʼ��size���ȵ����ݡ�
	* @param:  int size				д�����ݳ���
	* @param:  ITA_RANGE range		���·�Χ������3����
	* @param:	void *userParam		�û�������ע��ʱ���û����롣ITA�����ע�ắ��ʱ�ٴ����û���
	* @return: ����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
	* @see ITA_Init��camera id���û����롣
	**/
	ITA_RESULT(*WritePackageData)(int cameraID, unsigned char *buf, int offset, int size, ITA_RANGE range, void *userParam);

	/**
	* @brief:  ��ȡУ�²�����
	* @param:  int cameraID				��ʶ�豸��֧��ͬʱ�򿪶�̨�豸��
	* @param:  ITA_RANGE range			���·�Χ������3����
	* @param:  ITA_CALIBRATE_TYPE type	�������͡�
	* @param:  float *p					����ָ�롣
	* @param:	void *userParam			�û�������ע��ʱ���û����롣ITA�����ע�ắ��ʱ�ٴ����û���
	* @return: ����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
	* @see ITA_Init��camera id���û����롣
	**/
	ITA_RESULT(*ReadCalibrateParam)(int cameraID, ITA_RANGE range, ITA_CALIBRATE_TYPE type, float *p, void *userParam);

	/**
	* @brief:  ����У�²�����
	* @param:  int cameraID				��ʶ�豸��֧��ͬʱ�򿪶�̨�豸��
	* @param:  ITA_RANGE range			���·�Χ������3����
	* @param:  ITA_CALIBRATE_TYPE type	�������͡�
	* @param:  float value				����ֵ��
	* @param:	void *userParam			�û�������ע��ʱ���û����롣ITA�����ע�ắ��ʱ�ٴ����û���
	* @return: ����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
	* @see ITA_Init��camera id���û����롣
	**/
	ITA_RESULT(*WriteCalibrateParam)(int cameraID, ITA_RANGE range, ITA_CALIBRATE_TYPE type, float value, void *userParam);
	
	/**
	* @brief:  �����߰��������ֲֳ��ºͻ�о���²�Ʒ��
	* @param:  int cameraID			��ʶ�豸��֧��ͬʱ�򿪶�̨�豸��
	* @param:  unsigned char *buf	���ݻ���
	* @param:  int bufSize			���ݻ���ĳ���
	* @param:  int *length			��ȡ��ʵ�ʳ���
	* @param:  ITA_FIELD_ANGLE lensType ��ͷ����
	* @param:  ITA_RANGE range		���·�Χ����ͬ��Ʒ֧�ֵ��¶ȷ�Χ��������ͬ��һ�㲻����3�������ITA_CUSTOM_RANGE1��ʼ��
	* @param:  int gear				���µ�λ���ֲֳ��ºͻ�о���²�Ʒͨ��������2����
	* @param:  void *userParam		�û�������ע��ʱ���û����롣ITA�����ע�ắ��ʱ�ٴ����û���
	* @return: ����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
	* @see ITA_Init��camera id���û����롣
	**/
	ITA_RESULT(*ReadCurveData)(int cameraID, unsigned char *buf, int bufSize, int *length, ITA_FIELD_ANGLE lensType, ITA_RANGE range, int gear, void *userParam);

	/**
	* @brief:  ��һ�ַ�ʽ�����߰������ڴ�����У�¾�ͷ���ֲֳ��ºͻ�о���²�Ʒ��ReadCurveData��ReadCurveDataEx�����û�ֻ��ע������һ�������ͬʱע����ôReadCurveData���ȡ�
	* @param:  int cameraID			��ʶ�豸��֧��ͬʱ�򿪶�̨�豸��
	* @param:  unsigned char *buf	���ݻ���
	* @param:  int bufSize			���ݻ���ĳ���
	* @param:  int *length			��ȡ��ʵ�ʳ���
	* @param:  ITA_FIELD_ANGLE lensType ��ͷ����
	* @param:  ITA_RANGE range		���·�Χ����ͬ��Ʒ֧�ֵ��¶ȷ�Χ��������ͬ��һ�㲻����3�������ITA_CUSTOM_RANGE1��ʼ��
	* @param:  int gear				���µ�λ���ֲֳ��ºͻ�о���²�Ʒͨ��������2����
	* @param:  void *extendParam     ��չ������Ϣ�������ʽ��ITACurveParam��
	* @param:  int extendParamSize   ��չ������Ϣ���ȡ�
	* @param:  void *userParam		�û�������ע��ʱ���û����롣ITA�����ע�ắ��ʱ�ٴ����û���
	* @return: ����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
	* @see ITA_Init��camera id���û����롣
	**/
	ITA_RESULT(*ReadCurveDataEx)(int cameraID, unsigned char *buf, int bufSize, int *length, 
		ITA_FIELD_ANGLE lensType, ITA_RANGE range, int gear, void *extendParam, int extendParamSize, void *userParam);

	/**
	* @brief:  ����PWM�����źš�
	* @param:  int cameraID     ��ʶ�豸��֧��ͬʱ�򿪶�̨�豸��
	* @param:  int dutyration	ռ�ձȣ�һ�����������ڣ��ߵ�ƽ��ʱ������������ʱ��ı�����
	* @param:  int engage		����ʹ�������1�������0���������
	* @param:  void *userParam	�û�������ע��ʱ���û����롣ITA�����ע�ắ��ʱ�ٴ����û���
	* @return: ����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
	* @see ITA_Init��camera id���û����롣
	**/
	ITA_RESULT(*PWMDrive)(int cameraID, int dutyration, int engage, void *userParam);

	/**
	* @brief:  дEEPROM�Զ�����������
	* @param:  int cameraID			��ʶ�豸��֧��ͬʱ�򿪶�̨�豸��
	* @param:  unsigned short addr	��ַ
	* @param:  unsigned char data	����
	* @param:  void *userParam		�û�������ע��ʱ���û����롣ITA�����ע�ắ��ʱ�ٴ����û���
	* @return: ����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
	* @see ITA_Init��camera id���û����롣
	**/
	ITA_RESULT(*EEPROMWrite)(int cameraID, unsigned short addr, unsigned char data, void *userParam);

	/**
	* @brief:  ��EEPROM�Զ�����������
	* @param:  int cameraID			��ʶ�豸��֧��ͬʱ�򿪶�̨�豸��
	* @param:  unsigned short addr	��ַ
	* @param:  unsigned char *data	����ָ��
	* @param:  void *userParam		�û�������ע��ʱ���û����롣ITA�����ע�ắ��ʱ�ٴ����û���
	* @return: ����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
	* @see ITA_Init��camera id���û����롣
	**/
	ITA_RESULT(*EEPROMRead)(int cameraID, unsigned short addr, unsigned char *data, void *userParam);

	/**
	* @brief:  �¼�֪ͨ���ơ�����NUC�����ź�AF���¼�����ʱ��֪ͨ�û�����Щ�¼����в�����
	* @param:  int cameraID				��ʶ�豸��֧��ͬʱ�򿪶�̨�豸��
	* @param:  ITA_EVENT_TYPE eventType	�¼�����
	* @param:  void *eventParam			�¼�����
	* @param:  void *userParam			�û�������ע��ʱ���û����롣ITA�����ע�ắ��ʱ�ٴ����û���
	* @return: ����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
	* @see ITA_Init��camera id���û����롣�û�ʵ���¼�֪ͨ����ʱ���������أ�����Ӱ��ITA����Ч�ʡ�
	**/
	ITA_RESULT(*NotifyEvent)(int cameraID, ITA_EVENT_TYPE eventType, void *eventParam, void *userParam);

	/**
	* @brief:  ���Ȼ��жϵȲ��ֹ�����Ҫ��д˽�����ݡ������Ʒ��Ҫ�˹��ܣ���ô���û�ʵ���ڴ洢�����϶�д˽�����ݡ�
	* @param:  int cameraID				��ʶ�豸��֧��ͬʱ�򿪶�̨�豸��
	* @param:  unsigned char *readBuf	���ݻ���
	* @param:  int readLen  			���泤��
	* @param:  void *userParam			�û�������ע��ʱ���û����롣ITA�����ע�ắ��ʱ�ٴ����û���
	* @return: ����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
	* @see ITA_Init��camera id���û����롣
	**/
	ITA_RESULT(*ReadPrivateData)(int cameraID, unsigned char *readBuf, int readLen, void *userParam);

	/**
	* @brief:  ���Ȼ��жϵȲ��ֹ�����Ҫ��д˽�����ݡ������Ʒ��Ҫ�˹��ܣ���ô���û�ʵ���ڴ洢�����϶�д˽�����ݡ�
	* @param:  int cameraID				��ʶ�豸��֧��ͬʱ�򿪶�̨�豸��
	* @param:  unsigned char *writeBuf	���ݻ���
	* @param:  int writeLen  			���泤��
	* @param:  void *userParam			�û�������ע��ʱ���û����롣ITA�����ע�ắ��ʱ�ٴ����û���
	* @return: ����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����ֵ<0���ο�ERROR.h��
	* @see ITA_Init��camera id���û����롣
	**/
	ITA_RESULT(*WritePrivateData)(int cameraID, unsigned char *writeBuf, int writeLen, void *userParam);

	/*�û�����������ע�ắ��ʱ�ٴ����û���*/
	void *userParam;
}ITARegistry;

typedef struct ITA_PARAM_INFO
{
	ITA_RANGE range;
	int fpaGear; //���µ�λ
	bool isAutoGear; //�Զ��е�����
	bool isReverse; //�Ƿ���Ŀ���
	bool isSubAvgB; //��ȥ���׾�ֵ�Ŀ���
	bool isDebug; //���Կ���
	ITA_DRT_TYPE drtType; //���ⷽʽ
	ITA_EQUAL_LINE_TYPE equalLineType; //����������
	int frameRate; //�û����õ�֡��
	float distance;
	float emiss;
	float transmit;
	float reflectTemp;
	ITA_SWITCH environCorr;
	ITA_SWITCH lensCorr;
	ITA_SWITCH distanceCompen;
	ITA_SWITCH emissCorr;
	ITA_SWITCH transCorr;
	float lowLensCorrK;		//���µ���ͷ����ϵ��
	float highLensCorrK;	//���µ���ͷ����ϵ��
	float lensCorrK;		//��������ͷ����ϵ��
	ITA_SWITCH shutterCorr;
	float lowShutterCorrK;	//���µ���������ϵ��
	float highShutterCorrK;	//���µ���������ϵ��
	float shutterCorrK;		//��������������ϵ��
	ITA_PIXEL_FORMAT format;
	ITA_PALETTE_TYPE palette;
	float scale;
	int contrast;
	int brightness;
	ITA_ROTATE_TYPE rotate;
	ITA_FLIP_TYPE flip;
	bool isTimeFilter;	//ʱ���˲����أ����Թرջ��ߴ򿪡�
	bool isSpatialFilter;	//�����˲����أ����룬���Թرջ��ߴ򿪡�
	bool isRemoveVertical;//ȥ���ƣ����Թرջ��ߴ򿪡�
	bool isRemoveHorizontal;//ȥ���ƣ����Թرջ��ߴ򿪡�
	bool isSharpening;	//�񻯣����Թرջ��ߴ򿪡�
	bool isDetailEnhance;	//ϸ����ǿ�����Թرջ��ߴ򿪡�
	bool isBlockHistogram;//�ֿ�ֱ��ͼ�����Թرջ��ߴ򿪡��ù��ܷ�����
	bool isGammaCorrection;//GammaУ�������Թرջ��ߴ򿪡�
	bool isY8AdjustBC;	//Y8��ƫ�����Թرջ��ߴ򿪡�
	bool isHsm;			//�����⿪�أ����Թرջ��ߴ򿪡�
	unsigned char isAutoRpBdPs;	//�Զ�ȥ���㿪�أ����Թرջ��ߴ򿪡�
	int tffStd;	//ʱ���˲���׼��
	int restrainRange; //�������Ʒ�Χ
	int discardUpratio;  //�������׵����
	int mixThrLow;		//��ϵ������ֵ
	int mixThrHigh;		//��ϵ������ֵ
	int discardDownratio;  //�������׵����
	int heqPlatThresh;  //ƽֱ̨��ͼ�������
	int heqRangeMax;  //ƽֱ̨��ͼ�������
	int heqMidvalue;  //ƽֱ̨��ͼ�������
	int iieEnhanceCoef; //ϸ������ϵ��
	int iieGaussStd;  //��ǿ�㷨����
	float gmcGamma;  //Gamma�㷨����
	int gmcType;  //Gamma�㷨����
	int hsmWeight;//�������㷨����
	int hsmStayThr;//�����⾲̬��ֵ
	int hsmStayWeight; //�����⾲̬֡���ֵȨ��
	int hsmInterval;//����������
	int hsmMidfilterWinSize;//��ֵ�˲�����
	int hsmDeltaUpthr;//֡���ֵ����ֵ����
	int hsmDeltaDownthr;//֡���ֵ����ֵ����
	int hsmPointThr; //����������ֵ
	bool isHsmMidfilter;//��������ֵ�˲�����
	float nearKFAuto;
	float nearBAuto;
	float farKFAuto;
	float farBAuto;
	float nearKFManual;
	float nearBManual;
	float farKFManual;
	float farBManual;
	float orinalShutterTemp;//���������¶�
	float currentShutterTemp;//��ǰ�����ʱ�Ŀ����¶�
	float realTimeShutterTemp;//ʵʱ�����¶�
	float realTimeLensTemp;//ʵʱ��Ͳ�¶�
	float currentLensTemp;//��ǰ�����ʱ�ľ�Ͳ�¶�
	float realTimeFpaTemp;//ʵʱ��ƽ���¶�
	float humidity;		//ʪ��
	int zoomType;//�Ŵ�����,0:Near,1:Bilinear,2:BilinearFast
	unsigned char coldHotStatus;//����Ȼ�״̬��1:��� 0:�Ȼ�
	float laplaceWeight;
	int linearRestrainRangethr; //���������Ʋ���
}ITAParamInfo;

typedef struct ITA_TEC_INFO
{
	int mtType;			//���·�Χ��ʹ��ITA_SetMeasureRange���á�
	int nGear;			//���µ�λ, 0:���µ�, 1:���µ���ʹ��ITA_CHANGE_GEAR���á�
	short sY16Offset;					//Y16ƫ����(Ĭ��0)
	int nKF;							//������ʱY16��������(���㻯100����Ĭ��ֵΪ100)
	int nB1;							//������ʱY16��ƫ����(���㻯100��)
	int nDistance_a0;					//����У��ϵ��(���㻯10000000000����Ĭ��0)
	int nDistance_a1;					//����У��ϵ��(���㻯1000000000����Ĭ��0)
	int nDistance_a2;					//����У��ϵ��(���㻯10000000����Ĭ��0)
	int nDistance_a3;					//����У��ϵ��(���㻯100000����Ĭ��0)
	int nDistance_a4;					//����У��ϵ��(���㻯100000����Ĭ��0)
	int nDistance_a5;					//����У��ϵ��(���㻯10000����Ĭ��0)
	int nDistance_a6;					//����У��ϵ��(���㻯1000����Ĭ��0)
	int nDistance_a7;					//����У��ϵ��(���㻯100����Ĭ��0)
	int nDistance_a8;					//����У��ϵ��(���㻯100����Ĭ��0)
	int nK1;							//������Ưϵ��(���㻯100����Ĭ��0)
	int nK2;							//��Ͳ��Ưϵ��(���㻯100����Ĭ��0)
	int nK3;							//��������ϵ��(���㻯10000����Ĭ��0)
	int nB2;							//��������ƫ����(���㻯10000����Ĭ��0)
	int nKFOffset;						//�Զ�У��KFƫ�����������ָ�����У�����ã�Ĭ��0
	int nB1Offset;						//�Զ�У��B1ƫ�����������ָ�����У�����ã�Ĭ��0
	int fHumidity;						//ʪ��(���㻯100����Ĭ��60)
	int nAtmosphereTransmittance;		//����͸����(���㻯100������Χ0~100)
	float fEmiss;						//������(0.01-1.0,Ĭ��1.0)
	float fDistance;					//���¾��� 
	float fReflectT;					//�����¶ȣ����µ�Ĭ��3�����µ�Ĭ��23��
	float fAmbient;						//�����¶ȣ�ȡ���������£�
	float fAtmosphereTemperature;		//�����¶�
	float fWindowTransmittance;			//����͸����(��Χ0~1)
	float fWindowTemperature;			//�����¶ȣ����µ�Ĭ��3�����µ�Ĭ��23��
	bool bHumidityCorrection;			//ʪ���������أ�Ĭ�ϴ򿪡�
	bool bShutterCorrection;			//�����������أ�Ĭ�ϴ򿪡�
	bool bLensCorrection;				//��Ͳ�������أ�Ĭ�ϴ򿪡�
	bool bEmissCorrection;				//�������������أ�Ĭ�ϴ򿪡�
	bool bDistanceCorrection;			//�����������أ�Ĭ�ϴ򿪡�
	bool bAmbientCorrection;			//�����������أ�Ĭ�ϴ򿪡�
	bool bB1Correction;					//B1�������أ�Ĭ�ϴ򿪡�
	bool bAtmosphereCorrection;			//����͸�����������أ�Ĭ�ϴ򿪡�
	bool bWindowTransmittanceCorrection;//����͸���ʿ��أ�Ĭ�ϴ򿪡�
	unsigned char burnSwitch;//�����տ���
	unsigned char burnFlag;//���ձ�־
	short burnDelayTime;//���ճ���ʱ��
	int burnThreshold;//������ֵ
}ITATECInfo;

typedef struct ITA_MEASURE_INFO
{
	unsigned char          bDistanceCompensate;//���벹������
	unsigned char          bEmissCorrection;//������У������
	unsigned char          bTransCorrection;//͸����У������
	unsigned char          bHumidityCorrection;//ʪ��У������

	float         fNearKf;	//������kfϵ��
	float         fNearB;	//������ƫ��ϵ��
	float         fFarKf;	//Զ����kfϵ��
	float         fFarB;	//Զ����ƫ��ϵ��

	float         fNearKf2;	//������kfϵ��
	float         fNearB2;	//������ƫ��ϵ��
	float         fFarKf2;	//Զ����kfϵ��
	float         fFarB2;	//Զ����ƫ��ϵ��

	float         fHighShutterCorrCoff;		//���µ�����У��ϵ��(��ҵר��)
	float         fLowShutterCorrCoff;		//���µ�����У��ϵ��(��ҵר��)
	float         fShutterCorrCoff;		    //�������µ�λ����У��ϵ��(��ҵר��)

	float         fHighLensCorrK;	//���µ���Ͳ��Ư����ϵ��
	float         fLowLensCorrK;	//���µ���Ͳ��Ư����ϵ��
	float         fLensCorrK;	//��������Ͳ��Ư����ϵ��

	float         fDistance;	//Ŀ�����
	float		  fEmiss;		//������(��ҵר��)
	float		  fTransmit;	//͸����(��ҵר��)
	float         fHumidity;	//ʪ��(0.01-1.0,Ĭ��0.6)
	float		  fReflectTemp;	//�����¶�(��ҵר��)
	float         fCorrectDistance;//У�¾���
}ITAMeasureInfo;

/*����������Ҫ����Androidƽ̨�߳���ͨ��postHandle֪ͨӦ�ò��ͷ���Դ��*/
typedef void(*PostHandle)(int cameraID);

typedef struct ITA_SCENE_PARAM 
{
	void *mtParam;            //���²���
	int paramLen;             //���²������ݳ���
	unsigned char *curveData; //��������
	int curveLen;             //�������ݳ��ȣ���λByte��
	void *reservedData;       //�������ݣ�ģ���Ʒʹ�á�
	int reservedLen;          //�������ݳ��ȡ��������0����ô�û���Ҫ���汣�����ݡ�
}ITASceneParam;

typedef enum ITADataType
{
	ITA_DATA_Y8,		  //��������Ϊunsigned char��
	ITA_DATA_Y16		  //��������Ϊshort��
}ITA_DATA_TYPE;

typedef enum ITAY8DataMode
{
	ITA_Y8_NORMAL,		//Y8���ݷ�Χ��0-255
	ITA_Y8_XINSHENG		//Y8��������о�ɵ�ASIC,Y8���ݷ�Χ��16-235
}ITA_Y8_DATA_MODE;

typedef struct ITA_FUNCTION_CONFIG {
	char versionNumber[64];
	int productType;						//��Ʒ�ͺ�
	float shutterTempUpper;					//���Ų�����������
	float nucTempUpper;						//NUC������������
	int nucIntervalBottom;					//��������NUC��С���ʱ��
	int shutterIntervalBottom;				//�������ο�����С���ʱ�䣨����ʱ����Ƶ������������壩
	int isSubAvgB;							//Y16��ȥ���׾�ֵ�Ŀ���
	int shutterCollectInterval;				//�ϴο���֮���������뿪ʼ�ɼ�
	int collectInterval;					//�������βɼ����ʱ��
	int collectNumberOfTimes;				//һ�ο���֮����Ҫ�ɼ��Ĵ���
	int bootTimeToNow1;						//����60������
	int shutterPeriod1;						//�Զ���������30��
	int bootTimeToNow2;						//����60������120����֮��
	int shutterPeriod2;						//�Զ���������60��
	int bootTimeToNow3;						//����120����֮��
	int shutterPeriod3;						//�Զ���������90��
	int maxFramerate;						//̽�������õ�֡��
	int internalFlash;                      //0��ģ��������Flash��1��120 90��ģ������P25Q Flash��
	unsigned char versionNumberS;
	unsigned char productTypeS;							//��Ʒ�ͺ�
	unsigned char shutterTempUpperS;					//���Ų�����������
	unsigned char nucTempUpperS;						//NUC������������
	unsigned char nucIntervalBottomS;					//��������NUC��С���ʱ��
	unsigned char shutterIntervalBottomS;				//�������ο�����С���ʱ�䣨����ʱ����Ƶ������������壩
	unsigned char isSubAvgBS;							//Y16��ȥ���׾�ֵ�Ŀ���
	unsigned char shutterCollectIntervalS;				//�ϴο���֮���������뿪ʼ�ɼ�
	unsigned char collectIntervalS;						//�������βɼ����ʱ��
	unsigned char collectNumberOfTimesS;				//һ�ο���֮����Ҫ�ɼ��Ĵ���
	unsigned char bootTimeToNow1S;						//����60������
	unsigned char shutterPeriod1S;						//�Զ���������30��
	unsigned char bootTimeToNow2S;						//����60������120����֮��
	unsigned char shutterPeriod2S;						//�Զ���������60��
	unsigned char bootTimeToNow3S;						//����120����֮��
	unsigned char shutterPeriod3S;						//�Զ���������90��
	unsigned char maxFramerateS;						//̽�������õ�֡��
	unsigned char internalFlashS;                       //ģ��Flash��Ϣ
}ITAFunctionConfig;

typedef struct ITA_MT_CONFIG {
	int	lensType;				//ITA_FIELD_ANGLE �ӳ������� 0:56�㣻1:25�㣻2:120�㣻3:50�㣻4:90�㣻5:33�㡣
	float mtDistanceRangeN;		//���¾��뷶Χ����
	float mtDistanceRangeF;		//���¾��뷶Χ����
	int	mtType;					//����ģʽ 0�����壻1����ҵ���£�2����ҵ���¡�
	float correctDistance;		//�Զ�У�¾���
	float distance;				//Ŀ�����
	float lowLensCorrK;			//���³��µ���Ͳ��Ưϵ��
	float highLensCorrK;		//���¸��µ���Ͳ��Ưϵ��
	float lowShutterCorrCoff;	//���³��µ�������Ưϵ��
	float highShutterCorrCoff;	//���¸��µ�������Ưϵ��
	int mtDisType;				//���¾����������ͣ�0��6����; 1:9����
	float coefA1;				//���¾�����������
	float coefA2;				//���¾�����������
	float coefB1;				//���¾�����������
	float coefB2;				//���¾�����������
	float coefC1;				//���¾�����������
	float coefC2;				//���¾�����������
	float coefA3;				//���¾�����������
	float coefB3;				//���¾�����������
	float coefC3;				//���¾�����������
	unsigned char correctDistanceS;		//�Զ�У�¾���
	unsigned char distanceS;				//Ŀ�����
	unsigned char lowLensCorrKS;			//���³��µ���Ͳ��Ưϵ��
	unsigned char highLensCorrKS;		//���¸��µ���Ͳ��Ưϵ��
	unsigned char lowShutterCorrCoffS;	//���³��µ�������Ưϵ��
	unsigned char highShutterCorrCoffS;	//���¸��µ�������Ưϵ��
	unsigned char mtDisTypeS;			//���¾�����������
	unsigned char coefA1S;				//���¾�����������
	unsigned char coefA2S;				//���¾�����������
	unsigned char coefB1S;				//���¾�����������
	unsigned char coefB2S;				//���¾�����������
	unsigned char coefC1S;				//���¾�����������
	unsigned char coefC2S;				//���¾�����������
	unsigned char coefA3S;				//���¾�����������
	unsigned char coefB3S;				//���¾�����������
	unsigned char coefC3S;				//���¾�����������
}ITAMTConfig;
typedef struct ITA_ISP_CONFIG {
	int	lensType;				//�ӳ������� 0��25�㣻1:33�㣻2��50�㣻3��56�㣻4��90�㣻5��120�㡣
	int	mtType;					//����ģʽ 0�����壻1����ҵ���£�2����ҵ���¡�
	int	tffStd;					//ʱ���˲���׼�����ʱ���˲��̶ȣ�����Ӧ��Ӱ��ܴ󣬾����Ʒ�������á�
	int	vStripeWinWidth;		//ԽС����Ƶͼ����Խ�٣�ȥ���Ƹ�ǳԽ�󣬸�Ƶͼ����Խ�࣬ȥ���Ƹ���
	int	vStripeStd;				//�Ҷȱ�׼��Խ��ȥ����������Խ����
	int	vStripeWeightThresh;	//ֵ���󣬲���������ض࣬����ȥ����ֵ��С��������������٣������Ƽ���
	int	vStripeDetailThresh;	//ֵ��С����������Ʒ�ֵԽС������ȥ����ֵ���󣬼�������Ʒ�ֵԽ�󣬷����Ƽ���
	int	hStripeWinWidth;		//ԽС����Ƶͼ����Խ�٣�ȥ���Ƹ�ǳԽ�󣬸�Ƶͼ����Խ�࣬ȥ���Ƹ���
	int	hStripeStd;				//�Ҷȱ�׼��Խ��ȥ����������Խ����
	int	hStripeWeightThresh;	//ֵ���󣬲���������ض࣬����ȥ����ֵ��С��������������٣������Ƽ���
	int	hStripeDetailThresh;	//ֵ��С����������Ʒ�ֵԽС������ȥ����ֵ���󣬼�������Ʒ�ֵԽ�󣬷����Ƽ���
	int	rnArithType;			//0Ϊ��˹�����˲��� 1Ϊ��˹�Ҷ��˲�
	int	distStd;				//stdԽ���˲�Ч��Խǿ��ͼ��Խƽ��
	int	grayStd;				//stdԽ���˲�Ч��Խǿ��ͼ��Խƽ��
	int	discardUpratio;			//�������׵����
	int	discardDownratio;		//�������׵����
	int	linearBrightness;		//ֵԽ������Խ��
	int	linearContrast;			//ֵԽ��Աȶ�Խ��
	int	linearRestrainRangethre; //ֱ��Ӱ��С��̬��Χ�ĶԱȶȲ���(����������)
	int	heqPlatThresh;			//Ӱ��ͼ������Աȶ�
	int	heqRangeMax;			//ֵԽ��ӳ�䷶ΧԽ������Աȶ�Խ��
	int	heqMidvalue;			//Ӱ��ͼ����������
	int	iieEnhanceCoef;			//ϸ����ǿϵ����ֵԽ�󣬸�Ƶ����Ч��Խ���ԣ�������Խ����
	int	iieGrayStd;				//ֵԽ��ƽ��Ч��Խ����
	int	iieGaussStd;			//ֵԽ�󣬸�Ƶ����Խ��
	int	iieDetailThr;			//ϸ����ֵ��ֵԽ����ǿЧ��Խ���ԣ�������Խ����
	int	claheCliplimit;			//��ֵԽ��ͼ��Աȶ�Խǿ
	int	claheHistMax;			//ֵԽ��ֱ��ͼ��ӳ������ֵԽ��
	int	claheBlockWidth;		//�з���ֿ�����
	int	claheBlockHeight;		//�з���ֿ�����
	float spLaplaceWeight;		//ֵԽ����Ч��Խ���ԣ���������������
	int	gmcType;				//0��GammaУ����1˫GammaУ����
	float gmcGamma;				//GammaУ��ֵ
	int	adjustbcBright;			//����Y8ͼ������������ֵԽ������Խ��
	int	adjustbcContrast;		//����Y8ͼ��Աȶ�������ֵԽ��Աȶ�Խ��
	int	zoomType;				//0Ϊ���ڽ���ֵ��1Ϊ˫���Բ�ֵ��
	int mixThrLow;				//��ϵ������ֵ
	int mixThrHigh;				//��ϵ������ֵ
	unsigned char tffStdS;					//ʱ���˲���׼�����ʱ���˲��̶ȣ�����Ӧ��Ӱ��ܴ󣬾����Ʒ�������á�
	unsigned char vStripeWinWidthS;		//ԽС����Ƶͼ����Խ�٣�ȥ���Ƹ�ǳԽ�󣬸�Ƶͼ����Խ�࣬ȥ���Ƹ���
	unsigned char vStripeStdS;				//�Ҷȱ�׼��Խ��ȥ����������Խ����
	unsigned char vStripeWeightThreshS;	//ֵ���󣬲���������ض࣬����ȥ����ֵ��С��������������٣������Ƽ���
	unsigned char vStripeDetailThreshS;	//ֵ��С����������Ʒ�ֵԽС������ȥ����ֵ���󣬼�������Ʒ�ֵԽ�󣬷����Ƽ���
	unsigned char hStripeWinWidthS;		//ԽС����Ƶͼ����Խ�٣�ȥ���Ƹ�ǳԽ�󣬸�Ƶͼ����Խ�࣬ȥ���Ƹ���
	unsigned char hStripeStdS;				//�Ҷȱ�׼��Խ��ȥ����������Խ����
	unsigned char hStripeWeightThreshS;	//ֵ���󣬲���������ض࣬����ȥ����ֵ��С��������������٣������Ƽ���
	unsigned char hStripeDetailThreshS;	//ֵ��С����������Ʒ�ֵԽС������ȥ����ֵ���󣬼�������Ʒ�ֵԽ�󣬷����Ƽ���
	unsigned char rnArithTypeS;			//0Ϊ��˹�����˲��� 1Ϊ��˹�Ҷ��˲�
	unsigned char distStdS;				//stdԽ���˲�Ч��Խǿ��ͼ��Խƽ��
	unsigned char grayStdS;				//stdԽ���˲�Ч��Խǿ��ͼ��Խƽ��
	unsigned char discardUpratioS;			//�������׵����
	unsigned char discardDownratioS;		//�������׵����
	unsigned char linearBrightnessS;		//ֵԽ������Խ��
	unsigned char linearContrastS;			//ֵԽ��Աȶ�Խ��
	unsigned char linearRestrainRangethreS; //ֱ��Ӱ��С��̬��Χ�ĶԱȶȲ���(����������)
	unsigned char heqPlatThreshS;			//Ӱ��ͼ������Աȶ�
	unsigned char heqRangeMaxS;			//ֵԽ��ӳ�䷶ΧԽ������Աȶ�Խ��
	unsigned char heqMidvalueS;			//Ӱ��ͼ����������
	unsigned char iieEnhanceCoefS;			//ϸ����ǿϵ����ֵԽ�󣬸�Ƶ����Ч��Խ���ԣ�������Խ����
	unsigned char iieGrayStdS;				//ֵԽ��ƽ��Ч��Խ����
	unsigned char iieGaussStdS;			//ֵԽ�󣬸�Ƶ����Խ��
	unsigned char iieDetailThrS;			//ϸ����ֵ��ֵԽ����ǿЧ��Խ���ԣ�������Խ����
	unsigned char claheCliplimitS;			//��ֵԽ��ͼ��Աȶ�Խǿ
	unsigned char claheHistMaxS;			//ֵԽ��ֱ��ͼ��ӳ������ֵԽ��
	unsigned char claheBlockWidthS;		//�з���ֿ�����
	unsigned char claheBlockHeightS;		//�з���ֿ�����
	unsigned char spLaplaceWeightS;		//ֵԽ����Ч��Խ���ԣ���������������
	unsigned char gmcTypeS;				//0��GammaУ����1˫GammaУ����
	unsigned char gmcGammaS;				//GammaУ��ֵ
	unsigned char adjustbcBrightS;			//����Y8ͼ������������ֵԽ������Խ��
	unsigned char adjustbcContrastS;		//����Y8ͼ��Աȶ�������ֵԽ��Աȶ�Խ��
	unsigned char zoomTypeS;				//0Ϊ���ڽ���ֵ��1Ϊ˫���Բ�ֵ��
	unsigned char mixThrLowS;				//��ϵ��������ֵ
	unsigned char mixThrHighS;				//��ϵ��������ֵ
}ITAISPConfig;

typedef struct ITA_CONFIG {
	ITAFunctionConfig* functionConfig;
	int mtConfigCount;
	ITAMTConfig* mtConfig;
	int ispConfigCount;
	ITAISPConfig* ispConfig;
}ITAConfig;

typedef enum ITAShapeType
{
    ITA_SHAPE_TYPE_NONE     = 0,  // �Ƿ���״
    ITA_SHAPE_TYPE_POINT    = 1,  // �� (һ����)
    ITA_SHAPE_TYPE_LINE     = 2,  // �߶� (�߶ε����˵�)
    ITA_SHAPE_TYPE_TRIANGLE = 3,  // ������ (ÿ�����������,˳ʱ�뷽��)
    ITA_SHAPE_TYPE_RETANGE  = 4,  // ���� (���Ϻ����¶�������)
    ITA_SHAPE_TYPE_PENTAGON = 5,  // ����� (ÿ�����������,˳ʱ�뷽��)
    ITA_SHAPE_TYPE_HEXAGON  = 6,  // ������ (ÿ�����������,˳ʱ�뷽��)
    ITA_SHAPE_TYPE_POLYGON  = 7,  // �ֻ����� (ÿ�����������,˳ʱ�뷽��)
    ITA_SHAPE_TYPE_CIRCLE   = 8,  // Բ�� (������Ӿ��ε����Ϻ����¶�������)
    ITA_SHAPE_TYPE_ELIPSE   = 9,  // ��Բ (ˮƽ���õ���Բ,������Բ��Ӿ��ε����Ϻ����¶�������)
    ITA_SHAPE_TYPE_CONTOUR  = 10  // �ֻ����� (��������Ϊ�����ϵĵ�)

} ITA_SHAPE_TYPE;

typedef struct ITA_Shape
{
    ITAPoint* point;            // һ������ζ���Ķ��������б�(����������˳ʱ�뷽������)
    int            size;        // һ������ζ���Ķ����б�size
    ITA_SHAPE_TYPE type;        // ����ζ��������
} ITAShape;

typedef struct ITA_RangeMask
{
	unsigned char* maskArray;	// һ�������������������
	int size;					// һ�������������������ĳ���
}ITARangeMask;


typedef struct ITA_AnalysisTemp {
	float maxVal;
	float minVal;
	float avgVal;
} ITAAnalysisTemp;

typedef struct ITA_TempArray
{
	ITAAnalysisTemp* temp;
	int              size;
} ITATempArray;

typedef struct ITA_AnalysisY16Info {
	short maxY16;
	short minY16;
	short avgY16;
}ITAAnalysisY16Info;

typedef struct ITA_AnalysisY16Array {
	ITAAnalysisY16Info* y16;
	int                size;
} ITAY16InfoArray;

#endif // !ITA_DTD_H