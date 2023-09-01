/*************************************************************
Copyright (C), 2021--2022, Wuhan Guide Sensmart Tech Co., Ltd
File name   : AutoFocus.h
Author      : Guide Sensmart RD. wangyan
Version     : 1.0
Date        : 2021/12/9
Description : ITA SDK AutoFocus.
*************************************************************/
#ifndef ITA_AUTOFOCUS_H
#define ITA_AUTOFOCUS_H

#include "../Module/GuideLog.h"
#include "../ITADTD.h"

typedef unsigned char           NI_U8;
typedef unsigned short          NI_U16;
typedef unsigned int            NI_U32;

typedef signed char             NI_S8;
typedef short                   NI_S16;
typedef int                     NI_S32;

typedef unsigned long long      NI_U64;
typedef long long               NI_S64;

#define NAME_FILE_AUTOFOCUS_C "AutoFocus.cpp"
#define NAME_FILE_EEPROM_C "eeprom.c"
//! -----------------------------------------------------------------------------------------------------
//�Զ���������״̬��
#define STATE_AUTOFOCUS_GET_NEAR            255 //������״̬
#define STATE_AUTOFOCUS_GET_NEAR_SWITCH     254 //�ӽ�����Զ���л��м�״̬
#define STATE_AUTOFOCUS_GET_NEAR_JUDGE      253 //������ͣ�жϣ��������������ֹͣ���߼����ص�������״̬
#define STATE_AUTOFOCUS_GET_NEAR_WILL_STOP  252 //������ͣ״̬�����ڵ�����ֵ�ص�
#define STATE_AUTOFOCUS_GET_NEAR_EDGE       251 //������ͣ״̬�����ڵ�����ֵ�ص�
#define STATE_AUTOFOCUS_GET_FAR             250 //��Զ��״̬
#define STATE_AUTOFOCUS_GET_FAR_SWITCH      249 //��Զ���������л��м�״̬
#define STATE_AUTOFOCUS_GET_FAR_JUDGE       248 //Զ����ͣ�жϣ��������������ֹͣ���߼����ص���Զ��״̬
#define STATE_AUTOFOCUS_GET_FAR_WILL_STOP   247 //Զ����ͣ״̬�����ڵ�����ֵ�ص�
#define STATE_AUTOFOCUS_GET_FAR_EDGE        246 //Զ����ͣ״̬�����ڵ�����ֵ�ص�
#define STATE_AUTOFOCUS_GET_STOP            0   //�˳�״̬

//! �Զ���������Ĭ��ֵ����------------------------------------------------------------------------------------
//�Զ�������������
#define MAX_AUTOFOCUS_TIME_CNT              300                                             //ms
#define MIN_AUTOFOCUS_TIME_CNT              43//41                                              //ms
#define RUN_AUTOFOCUS_TIME_CNT              10//20                                              //ms
#define WAIT_AUTOFOCUS_TIME_CNT             MIN_AUTOFOCUS_TIME_CNT-RUN_AUTOFOCUS_TIME_CNT   //ms
#define DIFF_MIN_AUTOFOCUS_SHARP_VALUE      10000

//�Զ����������ص�����
#define MAX_AUTOFOCUS_PARA_CAL_CNT          10  //У׼ģʽ�����Ѱλ����
#define CNT_AUTOFOCUS_GET_NEAR_WILL_STOP    3   //������ͣ����
#define CNT_AUTOFOCUS_GET_FAR_WILL_STOP     4   //Զ����ͣ����

#define FREQ_PWM            125000                      //PWMƵ�ʶ��壬125KHz
#define PWM_DURATION_UP     25//1*(1000000000/FREQ_PWM)/4   //ռ�ձȶ��壬25%
#define PWM_DURATION_DOWN   75//3*(1000000000/FREQ_PWM)/4   //ռ�ձȶ��壬75%

//  ģ�鳧��¼����������
#define IIC_EEPROM_REGADDR_BASE                                 0x0000                                                      // EEPROM��ַ����ַ
#define IIC_EEPROM_MOD_DATALEN_MAX                              1024                                                        // EEPROM���ݳ�������
#define IIC_EEPROM_REGADDR_DATA                                 IIC_EEPROM_REGADDR_BASE + 0x0000                            // EEPROM����������������ƫ�Ƶ�ַ
#define IIC_EEPROM_REGADDR_LEN                                  IIC_EEPROM_REGADDR_BASE + IIC_EEPROM_MOD_DATALEN_MAX        // EEPROM����������������ƫ�Ƶ�ַ
// 0000:length[7:0]
// 0001:length[15:8]

//  ģ���Զ�����������¼����������
#define IIC_EEPROM_AF_DATALEN_MAX                               16                                                          // EEPROM���ݳ�������
#define IIC_EEPROM_REGADDR_AFDATA                               IIC_EEPROM_REGADDR_BASE + 2048                              // EEPROM AF������������ƫ�Ƶ�ַ
#define IIC_EEPROM_REGADDR_AFDATA_LEN                           IIC_EEPROM_REGADDR_BASE + 2048 + IIC_EEPROM_AF_DATALEN_MAX  // EEPROM AF������������ƫ�Ƶ�ַ
#define IIC_EEPROM_REGADDR_MAX_AUTOFOCUS_TIME_CNT               0                                                           // EEPROM short ������ʱ��ƫ�Ƶ�ַ
#define IIC_EEPROM_REGADDR_MIN_AUTOFOCUS_TIME_CNT               2                                                           // EEPROM char  С������ʱ��ƫ�Ƶ�ַ
#define IIC_EEPROM_REGADDR_RUN_AUTOFOCUS_TIME_CNT               3                                                           // EEPROM char  С������ʱ��������ʱ��ƫ�Ƶ�ַ
#define IIC_EEPROM_REGADDR_DIFF_MIN_AUTOFOCUS_SHARP_VALUE       4                                                           // EEPROM short ����������С��ֵƫ�Ƶ�ַ
#define IIC_EEPROM_REGADDR_CNT_AUTOFOCUS_GET_NEAR_WILL_STOP     6                                                           // EEPROM char  ���������յ�ص�����ƫ�Ƶ�ַ
#define IIC_EEPROM_REGADDR_CNT_AUTOFOCUS_GET_FAR_WILL_STOP      7

typedef struct AUTOFOCUSPARA
{
	NI_U16 autofocus_time_cnt;              //������Ч����ʱ��
	NI_U16 max_autofocus_time_cnt;         //�����г��������ʱ��
	NI_U8  min_autofocus_time_cnt;         //����������С����ʱ��
	NI_U8  run_autofocus_time_cnt;         //����������С����ʱ����ʵ������ʱ��
	NI_U8  wait_autofocus_time_cnt;        //����������С����ʱ����ʵ�ʵȴ�ʱ��
	NI_U16 autofocus_time_cnt_at_af_max;    //�����У�����ֵ���ֵ���ֵ�����ʱ��

	NI_U32 autofocus_sharp_value;           //��ǰ֡����ֵ
	NI_U32 autofocus_sharp_value_pre0;      //֮ǰһ֡����ֵ
	NI_U32 autofocus_sharp_value_pre1;      //֮ǰ��֡����ֵ
	NI_U32 autofocus_sharp_value_max;       //����3������ֵ�����ֵ
	NI_U16 diff_min_autofocus_sharp_value; //�������С����ֵ��ֵ
	NI_U8  cnt_autofocus_get_near_will_stop;//��������ͣʱ�Ĳ���
	NI_U8  cnt_autofocus_get_far_will_stop;//��Զ����ͣʱ�Ĳ���

	NI_U8  autofocus_engage;               //�������б��
	NI_U8  autofocus_fail;                 //����ʧ�ܱ��
	NI_U8  autofocus_sharp_value_stable;   //����ֵ�ȶ����
	NI_U8  autofocus_getparastopsucess;    //������ͣʱ�Ĳ�����ȡ�ɹ����
}AUTOFOCUSPARA_T;

class AutoFocus
{
public:
	AutoFocus();
	~AutoFocus();
	//TODO ��Ҫ���Ӳ����Զ���ȡ
	//=================================================================
	//  ���÷�ʽ�� autofocusctl.af_auto(0) ��������,���Ϊ����
	//            autofocusctl.af_auto(1) ��������,���ΪԶ��
	//            autofocusctl.af_auto(2) У׼����,���Ϊ����
	//            autofocusctl.af_auto(3) У׼����,���ΪԶ��
	//=================================================================
	ITA_RESULT AFControl(ITA_BC_TYPE type, void *param);
	ITA_RESULT setRegister(ITARegistry *registry, int cameraID);
	void setLogger(GuideLog *logger);
	ITA_RESULT refreshFrame(short* y16Array, int w, int h);
	ITA_RESULT addInterceptor(PostHandle postHandle);
private:
#ifdef MULTI_THREAD_FUNCTION
	static void AFRunningFunc(void* param);
#endif // MULTI_THREAD_FUNCTION
	void AF_GetStepNear(int step_time);                             //�������������ڶ�׼�������֣���ͷ������
	void AF_GetStepFar(int step_time);                              //Զ�����������ڶ�׼Զ�����֣���ͷ������
	void AF_GetNear();
	void AF_GetFar();
	//=================================================================
	//  ���÷�ʽ�� autofocusctl.af_auto(0) ��������,���Ϊ����
	//            autofocusctl.af_auto(1) ��������,���ΪԶ��
	//            autofocusctl.af_auto(2) У׼����,���Ϊ����
	//            autofocusctl.af_auto(3) У׼����,���ΪԶ��
	//=================================================================
	void AF_Auto(NI_U8 para_stop_cal_mode);                         //�����Զ��������������ýṹ�巽ʽΪ autofocusctl.af_auto()
	void AF_Auto_Simple(NI_U8 para_stop_cal_mode);                  //�����Զ��������������ýṹ�巽ʽΪ autofocusctl.af_auto() ���װ�
	void AF_Init();
	void AF_Para_Init();                                            //�Զ�����������ʼ��
	NI_S32 AF_GetTotalValue();                                      //ͼ�����ֵ���㣬������ΪASICӲ���ṩ
	void AF_GetParaStop(NI_U8 current_loop, NI_U8 current_status);  //AFֹͣ����������ȡ
	void AF_VcmInit();
	int  AF_GetMax();
	void AF_VCM_Down();
	void AF_VcmSet();
	void AF_VcmSet1();

	void PWM_Stop();            //PWM���ƣ�ֹͣ
	void PWM_UpEn();            //PWM���ƣ���Զ��
	void PWM_DownEn();          //PWM���ƣ�������
	void PWM_Drive(int dutyration, int engage);	//PWM���ƣ�����Զ����������
	void PWM_Init();            //PWM���ƣ���ʼ��
	void PWM_TotalFar();
	void PWM_TotalNear();
	void PWM_TotalFar_1();
	void PWM_TotalNear_1();
	NI_U32 eeprom_iic_rd(NI_U16 addr);
	void eeprom_iic_wr(NI_U16 addr, NI_U8 data);
	void ReadData_AFPARA();
	void WriteData_AFPARA();
	int laplacianY16();
private:
	AUTOFOCUSPARA_T autofocuspara;
	GuideLog *m_logger;
	ITARegistry *m_registry;
	int m_cameraID;
#ifdef MULTI_THREAD_FUNCTION
	HANDLE_T hThreadHandle;
	HANDLE_T m_mutex;
	bool isRunning;
	ITA_BC_TYPE actionType;	//��ʼ״̬Ϊ0
	int actionParam; //0����������,���Ϊ������1����������,���ΪԶ����2��У׼����,���Ϊ������3��У׼����,���ΪԶ����
	short *m_y16Array;
	int m_width, m_height;
	PostHandle postCb;
#endif // MULTI_THREAD_FUNCTION
};

#endif // !ITA_AUTOFOCUS_H

