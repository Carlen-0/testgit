/*************************************************************
Copyright (C), 2021--2022, Wuhan Guide Sensmart Tech Co., Ltd
File name   : Config.h
Author      : Guide Sensmart RD. wangyan
Version     : 1.0
Date        : 2021/11/9
Description : Function macro definition.
*************************************************************/
#ifndef ITA_CONFIG_H
#define ITA_CONFIG_H

/*********************���ܺ궨������******************/
#define MULTI_THREAD_FUNCTION			//֧�ֶ��̹߳��ܣ�AF������Ҫ��
//#define PALETTE_TIMO					//�汾����TIMO��Ʒ��13��α�ʡ�
#define PALETTE_ALL					//�汾�������в�Ʒ26��α�ʡ�
//#define EMISS_CURVES_800				//���������߳���16KB short
#define EMISS_CURVES_2100				//���������߳���40KB short
//#define CONTINUE_FRAME_DROP			//����������֡�������û�ָ����֡��
//#define SOURCE_COPY

/*�汾���ƺ꣬ÿ���汾���Ե���������Ҳ֧���콢��+���ư�ͬʱ������һ���汾�С�*/
#define ULTIMATE_EDITION                //�콢��
//#define COIN_EDITION                    //COIN��о���ư棬ָ����Ŀ����ʹ�ô˺꣬������رա�
#define BLACKBODY_EDITION				//������²�Ʒ������IR236X��
#define FIRE_DRC_EDITION				//����������⣬����ZC17A��
#define REFRIGER_EDITION				//����̽��������ģ�ͣ�����ZC12A��
//#define IMAGE_FUSION_EDITION			//ͼ���ں���궨������ZC12A��ͼ���ں���궨����OPENCV�⣬���򿪸ú���Ҫ����������OPENCV��
//#define CALIBRATE_EDITION				//�Ƿ���У�º����������ͻ�����У�¾�����������Ҫ�򿪴˺꣬�ڲ�������Ҫ�رմ˺ꡣ
//#define SHIHUI_EDITION				//�ӻԿͻ�TEC��Ʒ�ڿ��űպ�״̬��������¡�
//#define MEASURETOOL_EDITION

/*���ܼ��ٺ�*/
#define COPY_ACCELERATE //��������


/*********************�����������Ͷ�������******************/
typedef enum ITAProductType
{
	ITA_256_TIMO,
	ITA_120_TIMO,
	ITA_HANDHELD_TEC, //2���ֳ����Ʒ��B\C\Dϵ��
	ITA_IPT_TEC, //3��IPT���Ʒ��IPT\IPM\PLUG
	ITA_UNDEFINED_PRODUCT
}ITA_PRODUCT_TYPE;

//���ֲ����㷨�õ��Ĺ�����
//���������߳���
#define	nEmissCurveLen			40960
#define gdmin(a, b)				((a)<(b)?(a):(b))
#define gdmax(a, b)				((a)>(b)?(a):(b))
#define	MAXSHORT				16384
#define	MINSHORT				-16383

#endif // !ITA_CONFIG_H
