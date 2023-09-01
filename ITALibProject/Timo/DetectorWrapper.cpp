/*************************************************************
Copyright (C), 2021--2022, Wuhan Guide Sensmart Tech Co., Ltd
File name   : DetectorWrapper.cpp
Author      : Guide Sensmart RD. wangyan
Version     : 1.0
Date        : 2021/11/9
Description : Configure detector.
*************************************************************/
#include "DetectorWrapper.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "../Module/GuideLog.h"

#define IR256_I2C_DEVADDR (0x8A)	//8位地址。如果客户的平台是7位地址，那么右移一位后是0x45.

SENSOR_TYPE g_sensorType;
//extern int g_sdk_debug;

DetectorWrapper::DetectorWrapper(ITARegistry * registry, int cameraID, ITA_RANGE range)
{
	g_sensorType = SENSOR_V1C;	//SENSOR_V1B = 1, SENSOR_V1C = 2
	m_range = range;
	m_debug = false;
	
	m_cameraID = cameraID;
	m_registry = registry;
	m_clkHz = 12000000;
}

DetectorWrapper::~DetectorWrapper()
{
}

ITA_RESULT DetectorWrapper::DetectorInit(int clkHz, unsigned char validMode, unsigned char dataMode, unsigned char frameFrequency)
{
	//ClkHz,reserved to do something,not use now,use public varible to record this value
	unsigned char regTest[21];
	unsigned short ADC_CALIB_NMIDDLE_USER = 12000;
	unsigned char tmp = 0;
	unsigned char userValue = 0x05;
	m_clkHz = clkHz;
	m_logger->output(LOG_INFO, "DetectorInit clock=%d sensorType=%d %d %d %d", clkHz, g_sensorType, validMode, dataMode, frameFrequency);
	//256 0x10寄存器  05 先低后高，场高有效  01 先高后低，场高有效  07 先低后高，场低有效  03先高后低，场低有效
	/*用户设置的探测器参数
	validMode = 0;//0：探测器VSYNC和HSYNC高有效；1：探测器VSYNC和HSYNC低有效。默认为0。
	dataMode = 1;//0：先发16位数据的高8bits；1：先发16位数据的低8bits。默认为1。
	frameFrequency = 25;//探测器帧频，单位fps。默认25。*/
	if (!validMode)
	{
		if (!dataMode)
		{
			userValue = 0x01;
		}
		else
		{
			userValue = 0x05;
		}
	}
	else
	{
		if (!dataMode)
		{
			userValue = 0x03;
		}
		else
		{
			userValue = 0x07;
		}
	}
	//根据帧率计算几个强相关的寄存器值
	//MC_frequency=2×Frame_rate×[ (256+x_blank) *(200+y_blank)+reg_ypixblank]
	//x_win = reg_03<<8 | reg_02;
	//y_win = reg_05<<8 | reg_04;
	//x_blank = reg_07<<8| reg_06;
	//y_blank = reg_09<<8|reg_08;
	//reg_ypixblank =  reg_0b<<8|reg_0a;

	unsigned char reg_0c = frameFrequency << 4 & 0xff;
	unsigned char reg_0d = frameFrequency >> 4 & 0x03;
	unsigned char reg_06, reg_07, reg_08, reg_09, reg_0a, reg_0b;
	int x_blank, y_blank, reg_ypixblank;
	float tmp2;
	unsigned int tmp1;
	if (frameFrequency <= 25)
	{
		x_blank = 128;
		tmp2 = (float)m_clkHz / (2 * frameFrequency * 384);
		tmp1 = (unsigned int)floor(tmp2);
		y_blank = tmp1 - 200;
		reg_ypixblank = m_clkHz / (2* frameFrequency) - (256 + x_blank) *(200 + y_blank);

		reg_07 = x_blank >> 8 & 0xff;
		reg_06 = x_blank & 0xff;
		reg_09 = y_blank >> 8 & 0xff;
		reg_08 = y_blank & 0xff;
		reg_0b = reg_ypixblank >> 8 & 0xff;
		reg_0a = reg_ypixblank & 0xff;
	}
	/*256模组 50HZ参考aisc的进行配置,asic的userValue = 0x01*/
	else if (frameFrequency == 50)
	{
		reg_06 = 0x80;
		reg_07 = 0x00;
		reg_08 = 0x70;
		reg_09 = 0x00;
		reg_0a = 0xc0;
		reg_0b = 0x00;
		reg_0c = 0x20;
		reg_0d = 0x03;
	}
	else
	{
		y_blank = 425;
		tmp2 = (float)m_clkHz / (2* frameFrequency*625);
		tmp1 = (unsigned int)floor(tmp2);
		x_blank = tmp1 - 256;
		reg_ypixblank = m_clkHz / (2* frameFrequency) - (256 + x_blank) *(200 + y_blank);

		reg_09 = y_blank >> 8 & 0xff;
		reg_08 = y_blank & 0xff;
		reg_07 = x_blank >> 8 & 0xff;
		reg_06 = x_blank & 0xff;
		reg_0b = reg_ypixblank >> 8 & 0xff;
		reg_0a = reg_ypixblank & 0xff;
	}
	m_logger->output(LOG_INFO, "DetectorInit userValue=%#x %#x %#x %#x %#x %#x %#x %#x %#x %d %d %f %d %d",
		userValue, reg_06, reg_07, reg_08, reg_09, reg_0a, reg_0b, reg_0c, reg_0d, x_blank, y_blank, tmp2, tmp1, reg_ypixblank);
	//解析数据包Sensor Verion	0X3143	ASCII码格式1C
	if (m_registry->ReadPackageData && ITA_OK == m_registry->ReadPackageData(m_cameraID, (unsigned char *)&m_params, 216, sizeof(SENSORCONFIG_STATUS), m_range, m_registry->userParam))
	{
		if (0 == SensorParamisValid(&m_params) && 0 == CheckOtherSensorParamValid(&m_params))
		{
			if (0x3143 == m_params.SensorVerion)
			{
				g_sensorType = SENSOR_V1C;
			}
			else
			{
				g_sensorType = SENSOR_V1B;
			}
			//printf("DetectorWrapper SensorVerion=%#x sensorType=%d\n", m_params.SensorVerion, g_sensorType);
			//如果此时日志路径还未设置，输出到默认路径后，再设置路径无效。
			m_logger->output(LOG_INFO, "DetectorWrapper SensorVerion=%#x sensorType=%d", m_params.SensorVerion, g_sensorType);
			if (0 == m_params.ADC_CALIB_NMIDDLE)
				ADC_CALIB_NMIDDLE_USER = 12000;
			else
				ADC_CALIB_NMIDDLE_USER = m_params.ADC_CALIB_NMIDDLE;
			m_logger->output(LOG_INFO, "DetectorInit ADC_CALIB_NMIDDLE=%d", ADC_CALIB_NMIDDLE_USER);
		}
		else
		{
			//printf("DetectorWrapper SensorParam check failed!\n");
			m_logger->output(LOG_ERROR, "DetectorWrapper SensorParam check failed!");
		}
	}
	else
	{
		//printf("DetectorWrapper ReadPackageData failed!\n");
		m_logger->output(LOG_ERROR, "DetectorWrapper ReadPackageData failed!");
		//如果数据包没有读成功，那么使用一组默认的参数。
		//TBD
	}
	ITA_RESULT ret = m_registry->I2COpen(m_cameraID, IR256_I2C_DEVADDR, m_registry->userParam);
	if (ret != ITA_OK)
	{
		return ret;
	}
	int flag = SetNucAll(0x40);

	flag = m_registry->I2CWrite(m_cameraID, 0x7c, 0xa9, m_registry->userParam);  //to page1;
	flag = m_registry->I2CWrite(m_cameraID, 0x1c, 0x04, m_registry->userParam); //ADC_CALIB_ENABLE_FLAG[2] = 1;

	tmp = ADC_CALIB_NMIDDLE_USER & 0x00ff;
	flag = m_registry->I2CWrite(m_cameraID, 0x0b, tmp, m_registry->userParam);

	tmp = (ADC_CALIB_NMIDDLE_USER >> 8) & 0x00ff;
	flag = m_registry->I2CWrite(m_cameraID, 0x0c, tmp, m_registry->userParam);

	flag = m_registry->I2CWrite(m_cameraID, 0x7c, 0xa8, m_registry->userParam);  //to page0;
	int len = 20;
	if (12000000 == m_clkHz)
	{
		//12MHZ的配置
		DetectorConf pConf[20] = {
		{ 0x02, 0x00 },
		{ 0x03, 0x01 },
		{ 0x04, 0xc8 },
		{ 0x05, 0x00 },
		{ 0x06, reg_06}, //0x80 },
		{ 0x07, reg_07 }, //0x00 },
		{ 0x08, reg_08 }, //0xa9 },
		{ 0x09, reg_09 }, //0x01 },
		{ 0x0a, reg_0a }, //0x00 },
		{ 0x0b, reg_0b }, //0x00 },
		{ 0x0c, reg_0c }, //0x90 },
		{ 0x0d, reg_0d }, //0x01 },
		{ 0x0e, 0x00 },
		{ 0x10, userValue },
		{ 0x16, 0x00 },
		{ 0x2f, 0x00 },
		{ 0x30, 0x24 },
		{ 0x31, 0x02 },
		{ 0x33, 0x00 },
		{ 0x01, 0x01 }
		};
		for (int i = 0; i < len; i++)
		{
			flag = m_registry->I2CWrite(m_cameraID, pConf[i].addr, pConf[i].regValue, m_registry->userParam);
			porting_thread_sleep(1);
			flag = m_registry->I2CRead(m_cameraID, pConf[i].addr, &regTest[i], m_registry->userParam);
			porting_thread_sleep(1);
			if (m_debug)
			{
				m_logger->output(LOG_DEBUG, "I2CWrite i=%d addr=0x%x val=0x%x read=0x%x", i, pConf[i].addr, pConf[i].regValue, regTest[i]);
			}
		}
	}
	else
	{
		//6MHZ的配置
		DetectorConf pConf[20] = {
			{ 0x02, 0x00 },
			{ 0x03, 0x01 },
			{ 0x04, 0xc8 },
			{ 0x05, 0x00 },
			{ 0x06, reg_06 }, //0x80 },
			{ 0x07, reg_07 }, //0x00 },
			{ 0x08, reg_08 }, //0xa9 },
			{ 0x09, reg_09 }, //0x01 },
			{ 0x0a, reg_0a }, //0x00 },
			{ 0x0b, reg_0b }, //0x00 },
			{ 0x0c, reg_0c }, //0x90 },
			{ 0x0d, reg_0d }, //0x01 },
			{ 0x0e, 0x00 },
			{ 0x10, userValue }, //05
			{ 0x16, 0x00 },
			{ 0x2f, 0x00 },
			{ 0x30, 0x3d },
			{ 0x31, 0x02 },
			{ 0x33, 0x00 },
			{ 0x01, 0x01 }
		};
		for (int i = 0; i < len; i++)
		{
			flag = m_registry->I2CWrite(m_cameraID, pConf[i].addr, pConf[i].regValue, m_registry->userParam);
			porting_thread_sleep(1);
			flag = m_registry->I2CRead(m_cameraID, pConf[i].addr, &regTest[i], m_registry->userParam);
			porting_thread_sleep(1);
			if (m_debug)
			{
				m_logger->output(LOG_DEBUG, "I2CWrite i=%d addr=0x%x val=0x%x read=0x%x", i, pConf[i].addr, pConf[i].regValue, regTest[i]);
			}
		}
	}
	int seed_ready_cnt = 0;
	unsigned char rd_data = 1;
	porting_thread_sleep(5);

	while ((rd_data == 1) && (seed_ready_cnt < 10000))
	{
		flag = m_registry->I2CRead(m_cameraID, 0x01, &rd_data, m_registry->userParam);
		rd_data = rd_data & 0x01;
		seed_ready_cnt++;
		if (m_debug)
		{
			m_logger->output(LOG_DEBUG, "I2CRead 0x01 rd_data=%d seed_ready_cnt=%d", rd_data, seed_ready_cnt);
		}
	}

	flag = m_registry->I2CWrite(m_cameraID, 0x01, 0x80, m_registry->userParam); //Capture_start = 1;
	porting_thread_sleep(1);

	flag = m_registry->I2CWrite(m_cameraID, 0x7c, 0xA8, m_registry->userParam);  //page0;
	flag = m_registry->I2CWrite(m_cameraID, 0x1f, 0x00, m_registry->userParam);
	flag = m_registry->I2CWrite(m_cameraID, 0x35, 0x15, m_registry->userParam); //***********20200630**********//  
	flag = m_registry->I2CWrite(m_cameraID, 0x7C, 0xAB, m_registry->userParam);//page3
	flag = m_registry->I2CWrite(m_cameraID, 0x20, 0xA5, m_registry->userParam);//
	flag = m_registry->I2CWrite(m_cameraID, 0x7C, 0xA8, m_registry->userParam);//page0

	porting_thread_sleep(1);
	flag = m_registry->I2CWrite(m_cameraID, 0x7C, 0xAB, m_registry->userParam);//page3
	flag = m_registry->I2CWrite(m_cameraID, 0x0B, 0x88, m_registry->userParam);
	flag = m_registry->I2CWrite(m_cameraID, 0x10, 0x61, m_registry->userParam);
	flag = m_registry->I2CWrite(m_cameraID, 0x15, 0x48, m_registry->userParam);//bit[0] : 1 GFID???? 0:GFID???
	flag = m_registry->I2CWrite(m_cameraID, 0x1D, 0x05, m_registry->userParam);
	flag = m_registry->I2CWrite(m_cameraID, 0x7C, 0xA8, m_registry->userParam);//page0

	/*	GSK?GFID??????	*/
	flag = m_registry->I2CWrite(m_cameraID, 0x7C, 0xAB, m_registry->userParam);//page3
	flag = m_registry->I2CWrite(m_cameraID, 0x0E, 0x3e, m_registry->userParam);//bit[1] : 0 GFID????? 1: GFID????
	flag = m_registry->I2CWrite(m_cameraID, 0x10, 0xe1, m_registry->userParam);
	flag = m_registry->I2CWrite(m_cameraID, 0x7C, 0xA8, m_registry->userParam);//page0

	flag = m_registry->I2CWrite(m_cameraID, 0x2C, 0x05, m_registry->userParam);//bit[2]: 0 : GSK????? 1:GSK????
	flag = m_registry->I2CWrite(m_cameraID, 0x24, 0x20, m_registry->userParam);//Gcode_user[5:0];
	flag = m_registry->I2CWrite(m_cameraID, 0x25, 0x20, m_registry->userParam);//OScode_user[5:0];

	//////////////////////////////////////////////////////////////////
	///////*****  20200402????         ///////////////////////////
	//////////////////////////////////////////////////////////////////

	flag = m_registry->I2CWrite(m_cameraID, 0x7C, 0xAB, m_registry->userParam);//page
	flag = m_registry->I2CWrite(m_cameraID, 0x1f, 0x60, m_registry->userParam);//***********20200817**********//
	flag = m_registry->I2CWrite(m_cameraID, 0x17, 0x07, m_registry->userParam);//RA_sel
	flag = m_registry->I2CWrite(m_cameraID, 0x18, 0xff, m_registry->userParam);
	flag = m_registry->I2CWrite(m_cameraID, 0x19, 0xff, m_registry->userParam);
	flag = m_registry->I2CWrite(m_cameraID, 0x1a, 0xff, m_registry->userParam);
	flag = m_registry->I2CWrite(m_cameraID, 0x1b, 0xff, m_registry->userParam);
	flag = m_registry->I2CWrite(m_cameraID, 0x12, 0xF0, m_registry->userParam);//***********20200817**********//HSSD
	flag = m_registry->I2CWrite(m_cameraID, 0x1c, 0x02, m_registry->userParam);//***********20200817**********//
	flag = m_registry->I2CWrite(m_cameraID, 0x0c, 0x88, m_registry->userParam);//***********20200817**********//

	flag = m_registry->I2CWrite(m_cameraID, 0x14, 0x80, m_registry->userParam);//***********20200817**********//
	if (g_sensorType == SENSOR_V1C)
		flag = m_registry->I2CWrite(m_cameraID, 0x0f, 0xda, m_registry->userParam);//***********20200817**********//
	else
		flag = m_registry->I2CWrite(m_cameraID, 0x0f, 0x42, m_registry->userParam);//***********20200817**********//

	flag = m_registry->I2CWrite(m_cameraID, 0x0a, 0x88, m_registry->userParam);//***********20200817**********//
	flag = m_registry->I2CWrite(m_cameraID, 0x07, 0x20, m_registry->userParam);//***********20200817**********//
	flag = m_registry->I2CWrite(m_cameraID, 0x13, 0x00, m_registry->userParam);//***********20200810**********//
	flag = m_registry->I2CWrite(m_cameraID, 0x7C, 0xA9, m_registry->userParam);//page1
	flag = m_registry->I2CWrite(m_cameraID, 0x3E, 0x80, m_registry->userParam);//***********20200702**********//
	flag = m_registry->I2CWrite(m_cameraID, 0x3F, 0x80, m_registry->userParam);//***********202011218**********//
	flag = m_registry->I2CWrite(m_cameraID, 0x40, 0x74, m_registry->userParam);//***********20200630**********//
	flag = m_registry->I2CWrite(m_cameraID, 0x7C, 0xA8, m_registry->userParam);//page0

	flag = m_registry->I2CWrite(m_cameraID, 0x7C, 0xA9, m_registry->userParam);//page1
	flag = m_registry->I2CWrite(m_cameraID, 0x57, 0x5a, m_registry->userParam);//bit3 PD_MIPI_HS_BIAS : 1 off; 0: on
	flag = m_registry->I2CWrite(m_cameraID, 0x7C, 0xA8, m_registry->userParam);//page0

	//NUC????????
	flag = m_registry->I2CWrite(m_cameraID, 0x3E, 0x00, m_registry->userParam); //reg_nuc_diapause[7:0] = 2;
	/*******************************************************************
	??????????:???25Hz?,????20??
	?????AD????8000(16383????????,????)??,???
	??????16383-(8000 - 1200)= 9583;
	??????16383-(8000 + 1200)= 7183;
	??????16383-(8000 - 500 )= 8883;
	??????16383-(8000 - 500 )= 7883;
	*///****************************************************************
	//????:9583;
	flag = m_registry->I2CWrite(m_cameraID, 0x41, 0x6f, m_registry->userParam);//?????8bit
	flag = m_registry->I2CWrite(m_cameraID, 0x42, 0x25, m_registry->userParam);//?????6bit
	//????:7183;
	flag = m_registry->I2CWrite(m_cameraID, 0x43, 0x0f, m_registry->userParam);//?????8bit
	flag = m_registry->I2CWrite(m_cameraID, 0x44, 0x1C, m_registry->userParam);//?????6bit
	//????:8883;
	flag = m_registry->I2CWrite(m_cameraID, 0x45, 0xb3, m_registry->userParam);//?????8bit
	flag = m_registry->I2CWrite(m_cameraID, 0x46, 0x22, m_registry->userParam);//?????6bit
	//????:7883;
	flag = m_registry->I2CWrite(m_cameraID, 0x47, 0xcb, m_registry->userParam);//?????8bit
	flag = m_registry->I2CWrite(m_cameraID, 0x48, 0x1e, m_registry->userParam);//?????6bit

	flag = m_registry->I2CWrite(m_cameraID, 0x49, 0x41, m_registry->userParam);//??SRAM????

	if (12000000 == m_clkHz)
	{
		//12MHZ的配置
		DetectorConf pConf[20] = {
			{ 0x02, 0x00 },
			{ 0x03, 0x01 },
			{ 0x04, 0xc8 },
			{ 0x05, 0x00 },
			{ 0x06, reg_06 }, //0x80 },
			{ 0x07, reg_07 }, //0x00 },
			{ 0x08, reg_08 }, //0xa9 },
			{ 0x09, reg_09 }, //0x01 },
			{ 0x0a, reg_0a }, //0x00 },
			{ 0x0b, reg_0b }, //0x00 },
			{ 0x0c, reg_0c }, //0x90 },
			{ 0x0d, reg_0d }, //0x01 },
			{ 0x0e, 0x00 },
			{ 0x10, userValue },
			{ 0x16, 0x00 },
			{ 0x2f, 0x00 },
			{ 0x30, 0x24 },
			{ 0x31, 0x02 },
			{ 0x33, 0x00 },
			{ 0x01, 0x01 }
		};
		for (int i = 0; i < len; i++)
		{
			flag = m_registry->I2CRead(m_cameraID, pConf[i].addr, &regTest[i], m_registry->userParam);
			porting_thread_sleep(1);  //zjh add
			if (m_debug)
			{
				m_logger->output(LOG_DEBUG, "I2CRead i=%d addr=0x%x read=0x%x", i, pConf[i].addr, regTest[i]);
			}
		}
	}
	else
	{
		//6MHZ的配置
		DetectorConf pConf[20] = {
			{ 0x02, 0x00 },
			{ 0x03, 0x01 },
			{ 0x04, 0xc8 },
			{ 0x05, 0x00 },
			{ 0x06, reg_06 }, //0x80 },
			{ 0x07, reg_07 }, //0x00 },
			{ 0x08, reg_08 }, //0xa9 },
			{ 0x09, reg_09 }, //0x01 },
			{ 0x0a, reg_0a }, //0x00 },
			{ 0x0b, reg_0b }, //0x00 },
			{ 0x0c, reg_0c }, //0x90 },
			{ 0x0d, reg_0d }, //0x01 },
			{ 0x0e, 0x00 },
			{ 0x10, userValue }, //05
			{ 0x16, 0x00 },
			{ 0x2f, 0x00 },
			{ 0x30, 0x3d },
			{ 0x31, 0x02 },
			{ 0x33, 0x00 },
			{ 0x01, 0x01 }
		};
		for (int i = 0; i < len; i++)
		{
			flag = m_registry->I2CRead(m_cameraID, pConf[i].addr, &regTest[i], m_registry->userParam);
			porting_thread_sleep(1);  //zjh add
			if (m_debug)
			{
				m_logger->output(LOG_DEBUG, "I2CRead i=%d addr=0x%x read=0x%x", i, pConf[i].addr, regTest[i]);
			}
		}
	}
	m_registry->I2CClose(m_cameraID, m_registry->userParam);
	// need to add some log output to debug
	m_logger->output(LOG_INFO, "DetectorInit return ok.sensorType=%d", g_sensorType);
	return ITA_OK;
}

int DetectorWrapper::DoNUC()
{
	/*ITA支持事件通知机制。在一些情况下用户期望知道NUC、快门和AF等事件的发生。当用户有此需求时，注
	册NotifyEvent函数，ITA在这些事件发生时会主动调用NotifyEvent通知用户。用户在收到事件后做出响应。
    图像分层现象是两帧不完整图像的上下或左右拼接。为什么从底层缓存读取到不完整的原始数据帧？排除代
	码逻辑错误或漏洞外，还有一个可能的原因是在写探测器寄存器过程中时序不稳，读取到不完整的数据帧。
	平台ISP硬件采集Camera数据有两种方式：帧中断（逐帧拷贝）和行（逐行拷贝）中断。如果是行中断方式，
	时序不稳可能会漏掉若干行。或者打静电产生假的行中断信号，这样后面按行拼接的帧数据都是错位的。帧
	中断方式则不存在此问题。ITA做NUC须配置探测器寄存器。安全的做法是用户在收到ITA_NUC_BEGIN事件时关
	闭DVP数据通道停止接收数据，在收到ITA_NUC_END事件后再打开DVP数据通道继续收取数据，保证读取的是完
	整数据帧。*/
	unsigned char rd_data, wr_data;
	ITA_RESULT flag = ITA_OK;
	if (m_registry->NotifyEvent)
		m_registry->NotifyEvent(m_cameraID, ITA_NUC_BEGIN, 0, m_registry->userParam);
	int ret = m_registry->I2COpen(m_cameraID, IR256_I2C_DEVADDR, m_registry->userParam);
	if (ret != ITA_OK)
	{
		if (m_registry->NotifyEvent)
			m_registry->NotifyEvent(m_cameraID, ITA_NUC_END, 0, m_registry->userParam);
		return ret;
	}
	flag = m_registry->I2CWrite(m_cameraID, 0x7C, 0xA8, m_registry->userParam);//page0 
	flag = m_registry->I2CRead(m_cameraID, 0x40, &rd_data, m_registry->userParam);
	wr_data = (rd_data & 0xfc) | 0x03;
	flag = m_registry->I2CWrite(m_cameraID, 0x40, wr_data, m_registry->userParam);

	flag = m_registry->I2CRead(m_cameraID, 0x40, &rd_data, m_registry->userParam);

	if (m_debug)
	{
		m_logger->output(LOG_DEBUG, "state nuc write start 0x%x.", wr_data);
	}
	porting_thread_sleep(1000);

	flag = m_registry->I2CWrite(m_cameraID, 0x7C, 0xA8, m_registry->userParam);//page0 
	flag = m_registry->I2CRead(m_cameraID, 0x40, &rd_data, m_registry->userParam);
	wr_data = rd_data & 0xfe;
	flag = m_registry->I2CWrite(m_cameraID, 0x40, wr_data, m_registry->userParam);

	flag = m_registry->I2CRead(m_cameraID, 0x40, &rd_data, m_registry->userParam);
	m_registry->I2CClose(m_cameraID, m_registry->userParam);
	if (m_registry->NotifyEvent)
		m_registry->NotifyEvent(m_cameraID, ITA_NUC_END, 0, m_registry->userParam);
	if (m_debug)
	{
		m_logger->output(LOG_DEBUG, "state nuc write stop 0x%x.", rd_data);
	}
	return ITA_OK;
}

int DetectorWrapper::ChangeGears(ITA_RANGE range)
{
	// need to add some log output to debug
	// if no correct data in the machine,use this defalut vale to config detect
	m_logger->output(LOG_INFO, "Detector ChangeGears range=%d sensorType=%d", range, g_sensorType);
	SENSORCONFIG_STATUS params;
	if (!m_registry || !m_registry->ReadPackageData)
	{
		return ITA_HAL_UNREGISTER;
	}
	if (ITA_OK == m_registry->ReadPackageData(m_cameraID, (unsigned char *)&params, 216, sizeof(SENSORCONFIG_STATUS), range, m_registry->userParam))
	{
		if (0 == SensorParamisValid(&params) && 0 == CheckOtherSensorParamValid(&params))
		{
			int ret = m_registry->I2COpen(m_cameraID, IR256_I2C_DEVADDR, m_registry->userParam);
			if (ret != ITA_OK)
			{
				return ret;
			}
			Sensor_INT_Set(params.INT);
			Sensor_gHssd_Set((BYTE_T)params.Hssd);  //18
			Sensor_Gain_Set((BYTE_T)params.Gain);
			Sensor_gRaSel((BYTE_T)params.Rasel);
			Sensor_NUCParamSet((BYTE_T)params.NUC_Step, params.NUC_high, params.NUC_low);
			Sensor_PNMOS_Set((BYTE_T)params.gPorNmos, (BYTE_T)params.gStb_en, (BYTE_T)params.gStbNum);
			Sensor_Poll_Set((BYTE_T)params.Poll);
			Sensor_REF_POLL((BYTE_T)params.Ref_poll);

			Sensor_Gsk_Set((BYTE_T)params.GSK);
			Sensor_VCM_Set((BYTE_T)params.VCM);
			Sensor_VRD_Set((BYTE_T)params.VRD);
			Sensor_RC_Set((BYTE_T)params.RD_RC);
			if (g_sensorType == SENSOR_V1C)
			{
				if (m_debug)
				{
					//printf("ChangeGears AD_STEP=%d, NUC_HIGH_fine=%d, NUC_LOW_fine=%d\n", params.AD_STEP, params.NUC_HIGH_fine, params.NUC_LOW_fine);
					m_logger->output(LOG_INFO, "ChangeGears SENSOR_V1C AD_STEP=%d, NUC_HIGH_fine=%d, NUC_LOW_fine=%d", params.AD_STEP, params.NUC_HIGH_fine, params.NUC_LOW_fine);
				}
				if (params.AD_STEP > 0 && params.AD_STEP < 255 && params.NUC_HIGH_fine != 0xffff && params.NUC_LOW_fine != 0xffff)	//防止读错数据包
				{
					Sensor_Ad_Step_Set((BYTE_T)params.AD_STEP);	//should to read from raw file
					NUC_Fine_Set(params.NUC_HIGH_fine, params.NUC_LOW_fine);
				}
				else
				{
					Sensor_Ad_Step_Set(128);
					NUC_Fine_Set(12683, 11683);
				}
			}	
			m_registry->I2CClose(m_cameraID, m_registry->userParam);
			if (m_debug)
			m_logger->output(LOG_INFO, 
				"ChangeGears Detect_Tmp=%d INT=%d, Gain=%d, Rasel=%d Hssd=%d NUC_Step=%d, NUC_low=%d, NUC_high=%d Vsk_ad_low=%d Vsk_ad_high=%d, Vsk_Thres=%d, gPorNmos=%d gStb_en=%d gStbNum=%d, Ref_poll=%d, Poll=%d, NUCValue=%d, GSK=%d, VCM=%d, VRD=%d, RD_RC=%d, PMOS_Loop_High=%d, PMOS_Loop_Low=%d, Rasel_Loop_High=%d, Rasel_Loop_Low=%d", 
				params.Detect_Tmp, params.INT, params.Gain, params.Rasel, params.Hssd, params.NUC_Step,
				params.NUC_low, params.NUC_high, params.Vsk_ad_low, params.Vsk_ad_high, params.Vsk_Thres, params.gPorNmos,
				params.gStb_en, params.gStbNum, params.Ref_poll, params.Poll, params.NUCValue, params.GSK,
				params.VCM, params.VRD, params.RD_RC, params.PMOS_Loop_High, params.PMOS_Loop_Low, params.Rasel_Loop_High, params.Rasel_Loop_Low);
			return ITA_OK;
		}
		else
		{
			m_logger->output(LOG_ERROR, "ChangeGears SensorParam check failed!");
		}
	}
	else
	{
		m_logger->output(LOG_ERROR, "ChangeGears ReadPackageData failed!");
	}
	// 1.need to complete get detect param func by index first
	// 2.check param is valid by call SensorParamisValid()&CheckOtherSensorParamValid(),
	//otherwise to use default value
	int ret = m_registry->I2COpen(m_cameraID, IR256_I2C_DEVADDR, m_registry->userParam);
	if (ret != ITA_OK)
	{
		return ret;
	}
	if (g_sensorType == SENSOR_V1C)
	{
		//默认理想值 
		Sensor_INT_Set(40);
		Sensor_gHssd_Set(16);
		Sensor_Gain_Set(4);
		Sensor_gRaSel(37);
		Sensor_NUCParamSet(32, 12683, 11683);
		Sensor_PNMOS_Set(0, 1, 8);
		Sensor_Poll_Set(0);
		Sensor_REF_POLL(0);

		Sensor_Gsk_Set(218);
		Sensor_VCM_Set(21);
		Sensor_VRD_Set(32);
		Sensor_RC_Set(116);
		Sensor_Ad_Step_Set(128);
		NUC_Fine_Set(12683, 11683);
	}
	else
	{
		Sensor_INT_Set(35);
		Sensor_gHssd_Set(16);  //18
		Sensor_Gain_Set(3);
		Sensor_gRaSel(36);
		Sensor_NUCParamSet(32, 12683, 11683);
		Sensor_PNMOS_Set(0, 1, 8);
		Sensor_Poll_Set(0);
		Sensor_REF_POLL(0);

		Sensor_Gsk_Set(0X42);
		Sensor_VCM_Set(0X11);
		Sensor_VRD_Set(0X68);
		Sensor_RC_Set(0XBD);
	}
	m_registry->I2CClose(m_cameraID, m_registry->userParam);
	//3.need to do nuc and shutter to update B
	return ITA_OK;
}

void DetectorWrapper::setLogger(GuideLog * logger)
{
	m_logger = logger;
}

void DetectorWrapper::setDebug(bool debug)
{
	m_debug = debug;
}

int DetectorWrapper::SensorParamisValid(SENSORCONFIG_STATUS *pcConfig)
{
	if ((pcConfig->Gain >= 0 && pcConfig->Gain <= 7) && \
		(pcConfig->INT >= 0 && pcConfig->INT <= 100) && \
		(pcConfig->Hssd >= 0 && pcConfig->Hssd < 32) &&
		(pcConfig->Rasel >= 0 && pcConfig->Rasel <= 40) && \
		(pcConfig->Poll >= 0 && pcConfig->Poll <= 255) && \
		(pcConfig->Ref_poll >= 0 && pcConfig->Poll <= 255) && \
		pcConfig->NUC_high != 0xffff && pcConfig->NUC_low != 0xffff && pcConfig->NUC_Step != 0xffff && \
		pcConfig->Vsk_ad_high != 0xffff && pcConfig->Vsk_ad_low != 0xffff && pcConfig->Vsk_Thres != 0xffff && \
		(pcConfig->gStb_en == 0 || pcConfig->gStb_en == 1) && \
		(pcConfig->gPorNmos == 0 || pcConfig->gPorNmos == 1) && \
		(pcConfig->gStbNum >= 0 && pcConfig->gStbNum <= 15))
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

int DetectorWrapper::CheckOtherSensorParamValid(SENSORCONFIG_STATUS *pSensorConfig)
{
	if ((pSensorConfig->GSK == 0x00 && pSensorConfig->VCM == 0x00 && pSensorConfig->VRD == 0x00 && pSensorConfig->RD_RC == 0x00) \
		|| (pSensorConfig->GSK == 0xffff && pSensorConfig->VCM == 0xffff && pSensorConfig->VRD == 0xffff && pSensorConfig->RD_RC == 0xffff))
	{
		return -1;
	}
	else if ((pSensorConfig->GSK >= 0 && pSensorConfig->GSK <= 255) && \
		(pSensorConfig->VCM >= 0 && pSensorConfig->VCM <= 255) && \
		(pSensorConfig->VRD >= 0 && pSensorConfig->VRD <= 255) && \
		(pSensorConfig->RD_RC >= 0 && pSensorConfig->RD_RC <= 255)
		)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

int DetectorWrapper::SetNucAll(unsigned char NucData)
{
	unsigned char rd_data, wr_nuc_data = 0x42;
	ITA_RESULT flag = ITA_OK;
	wr_nuc_data = NucData;
	flag = m_registry->I2CWrite(m_cameraID, 0x7c, 0xa8, m_registry->userParam);
	if (flag != ITA_OK)
	{
		return flag;
	}
	flag = m_registry->I2CWrite(m_cameraID, 0x49, wr_nuc_data, m_registry->userParam);
	if (flag != ITA_OK)
	{
		return flag;
	}

	flag = m_registry->I2CRead(m_cameraID, 0x40, &rd_data, m_registry->userParam);
	if (flag != ITA_OK)
	{
		return flag;
	}

	rd_data = rd_data | 0x10;
	flag = m_registry->I2CWrite(m_cameraID, 0x40, rd_data, m_registry->userParam);
	if (flag != ITA_OK)
	{
		return flag;
	}
	return flag;
}

// set int time (us) 0-100
void DetectorWrapper::Sensor_INT_Set(UINT16_T value)
{
	UINT16_T INT_Value = 0;
	unsigned short gctia = 300;
	unsigned char  gctia_l = 24;
	unsigned char  gctia_h = 1;
	ITA_RESULT flag = ITA_OK;

	INT_Value = value;

	if (INT_Value > 60)
	{
		INT_Value = 60;
	}
	if (12000000 == m_clkHz)
	{
		gctia = 64 - INT_Value;
		gctia = gctia * 4;
		m_logger->output(LOG_INFO, "12MHZ Sensor_INT_Set value=%d gctia=%d", value, gctia);
	}
	else
	{
		//硬件晶振6MHZ的配置
		gctia = 1010 - INT_Value * 10;
		gctia = gctia * 2 / 5;
		m_logger->output(LOG_INFO, "6MHZ Sensor_INT_Set value=%d gctia=%d", value, gctia);
	}
	gctia_l = gctia & 0xff;
	gctia_h = (gctia >> 8) & 0xff;
	flag = m_registry->I2CWrite(m_cameraID, 0x7c, 0xa8, m_registry->userParam);
	flag = m_registry->I2CWrite(m_cameraID, 0x2d, gctia_l, m_registry->userParam);
	flag = m_registry->I2CWrite(m_cameraID, 0x2e, gctia_h, m_registry->userParam);
}

// set gHssd 0-31
void DetectorWrapper::Sensor_gHssd_Set(BYTE_T value)
{
	unsigned char  gHssd_rd_reg_data = 0;
	ITA_RESULT flag = ITA_OK;
	BYTE_T gHssd_Value = 0;

	gHssd_Value = value;
	if (gHssd_Value > 31)
	{
		gHssd_Value = 31;
	}

	flag = m_registry->I2CWrite(m_cameraID, 0x7c, 0xab, m_registry->userParam);

	flag = m_registry->I2CRead(m_cameraID, 0x12, &gHssd_rd_reg_data, m_registry->userParam);
	gHssd_rd_reg_data = (gHssd_rd_reg_data & 0xe0) | (gHssd_Value & 0x1f);

	flag = m_registry->I2CWrite(m_cameraID, 0x12, gHssd_rd_reg_data, m_registry->userParam);
	flag = m_registry->I2CRead(m_cameraID, 0x12, &gHssd_rd_reg_data, m_registry->userParam);

	flag = m_registry->I2CWrite(m_cameraID, 0x7c, 0xa8, m_registry->userParam);
}

//set gain 0-7
void DetectorWrapper::Sensor_Gain_Set(BYTE_T value)
{
	BYTE_T Gain_Value = 0;
	ITA_RESULT flag = ITA_OK;

	Gain_Value = value;
	if (Gain_Value > 7)
	{
		Gain_Value = 7;
	}
	flag = m_registry->I2CWrite(m_cameraID, 0x7c, 0xa8, m_registry->userParam);
	flag = m_registry->I2CWrite(m_cameraID, 0x28, Gain_Value, m_registry->userParam);
}

//set rasel
void DetectorWrapper::Sensor_gRaSel(BYTE_T value)
{
	BYTE_T gRaSel_Value = 0;
	ITA_RESULT flag = ITA_OK;
	unsigned char  wr_data;
	BYTE_T i;

	gRaSel_Value = value;

	if (gRaSel_Value < 8)
	{
		wr_data = 0;
		for (i = 0; i < gRaSel_Value; i++)
		{
			wr_data = ((1 << i) | wr_data);
		}
		flag = m_registry->I2CWrite(m_cameraID, 0x7C, 0xAB, m_registry->userParam);//page3
		flag = m_registry->I2CWrite(m_cameraID, 0x1b, wr_data, m_registry->userParam);
		flag = m_registry->I2CWrite(m_cameraID, 0x7C, 0xA8, m_registry->userParam);//page0
	}
	else if (gRaSel_Value < 16)
	{
		wr_data = 0;
		for (i = 0; i < gRaSel_Value - 8; i++)
		{
			wr_data = ((1 << i) | wr_data);
		}
		flag = m_registry->I2CWrite(m_cameraID, 0x7C, 0xAB, m_registry->userParam);//page3
		flag = m_registry->I2CWrite(m_cameraID, 0x1a, wr_data, m_registry->userParam);
		flag = m_registry->I2CWrite(m_cameraID, 0x1b, 0xff, m_registry->userParam);
		flag = m_registry->I2CWrite(m_cameraID, 0x7C, 0xA8, m_registry->userParam);//page3
	}
	else if (gRaSel_Value < 24)
	{
		wr_data = 0;
		for (i = 0; i < gRaSel_Value - 16; i++)
		{
			wr_data = ((1 << i) | wr_data);
		}
		flag = m_registry->I2CWrite(m_cameraID, 0x7C, 0xAB, m_registry->userParam);//page3
		flag = m_registry->I2CWrite(m_cameraID, 0x19, wr_data, m_registry->userParam);
		flag = m_registry->I2CWrite(m_cameraID, 0x1a, 0xff, m_registry->userParam);
		flag = m_registry->I2CWrite(m_cameraID, 0x1b, 0xff, m_registry->userParam);
		flag = m_registry->I2CWrite(m_cameraID, 0x7C, 0xA8, m_registry->userParam);//page3
	}
	else if (gRaSel_Value < 32)
	{
		wr_data = 0;
		for (i = 0; i < gRaSel_Value - 24; i++)
		{
			wr_data = ((1 << i) | wr_data);
		}
		flag = m_registry->I2CWrite(m_cameraID, 0x7C, 0xAB, m_registry->userParam);//page3
		flag = m_registry->I2CWrite(m_cameraID, 0x18, wr_data, m_registry->userParam);
		flag = m_registry->I2CWrite(m_cameraID, 0x19, 0xff, m_registry->userParam);
		flag = m_registry->I2CWrite(m_cameraID, 0x1a, 0xff, m_registry->userParam);
		flag = m_registry->I2CWrite(m_cameraID, 0x1b, 0xff, m_registry->userParam);
		flag = m_registry->I2CWrite(m_cameraID, 0x7C, 0xA8, m_registry->userParam);//page3
	}

	else if (gRaSel_Value < 40)
	{
		wr_data = 0;
		for (i = 0; i < gRaSel_Value - 32; i++)
		{
			wr_data = ((1 << i) | wr_data);

		}
		unsigned char gHssd_rd_reg_data = wr_data;
		flag = m_registry->I2CWrite(m_cameraID, 0x7C, 0xAB, m_registry->userParam);//page3
		flag = m_registry->I2CWrite(m_cameraID, 0x17, wr_data, m_registry->userParam);

		flag = m_registry->I2CRead(m_cameraID, 0x17, &gHssd_rd_reg_data, m_registry->userParam);

		flag = m_registry->I2CWrite(m_cameraID, 0x18, 0xff, m_registry->userParam);
		flag = m_registry->I2CWrite(m_cameraID, 0x19, 0xff, m_registry->userParam);
		flag = m_registry->I2CWrite(m_cameraID, 0x1a, 0xff, m_registry->userParam);
		flag = m_registry->I2CWrite(m_cameraID, 0x1b, 0xff, m_registry->userParam);
		flag = m_registry->I2CWrite(m_cameraID, 0x7C, 0xA8, m_registry->userParam);//page3
	}
}

//nuc_step nuc_high nuc_low 
void DetectorWrapper::Sensor_NUCParamSet(BYTE_T Ad_step, UINT16_T ad_high, UINT16_T ad_low)
{
	ITA_RESULT flag = ITA_OK;
	unsigned char  wr_data;

	wr_data = Ad_step;
	flag = m_registry->I2CWrite(m_cameraID, 0x7C, 0xAB, m_registry->userParam);//page3
	flag = m_registry->I2CWrite(m_cameraID, 0x07, wr_data, m_registry->userParam);
	flag = m_registry->I2CWrite(m_cameraID, 0x7C, 0xa8, m_registry->userParam);//page0

	flag = m_registry->I2CWrite(m_cameraID, 0x7C, 0xA8, m_registry->userParam);//page0
	wr_data = (ad_high >> 8) & 0xFF;
	flag = m_registry->I2CWrite(m_cameraID, 0x42, wr_data, m_registry->userParam);
	wr_data = ad_high & 0xFF;
	flag = m_registry->I2CWrite(m_cameraID, 0x41, wr_data, m_registry->userParam);

	flag = m_registry->I2CWrite(m_cameraID, 0x7C, 0xA8, m_registry->userParam);//page0
	wr_data = (ad_low >> 8) & 0xFF;
	flag = m_registry->I2CWrite(m_cameraID, 0x44, wr_data, m_registry->userParam);
	wr_data = ad_low & 0xFF;
	flag = m_registry->I2CWrite(m_cameraID, 0x43, wr_data, m_registry->userParam);

	flag = m_registry->I2CWrite(m_cameraID, 0x7C, 0xa8, m_registry->userParam);//page0
}

void DetectorWrapper::Sensor_PNMOS_Set(BYTE_T gPorNmos, BYTE_T gStb_en, BYTE_T gStbNum)
{
	ITA_RESULT flag = ITA_OK;
	unsigned char  wr_data;

	wr_data = ((gStb_en & 0x01) << 0x07) | (gStbNum & 0x0f);
	flag = m_registry->I2CWrite(m_cameraID, 0x7C, 0xAB, m_registry->userParam);//page3
	flag = m_registry->I2CWrite(m_cameraID, 0x0c, wr_data, m_registry->userParam);
	flag = m_registry->I2CWrite(m_cameraID, 0x7C, 0xa8, m_registry->userParam);//page0

	if (gPorNmos == 0)//2:PMOS    8 : NMOS
	{
		wr_data = 0x02;
	}
	else
	{
		wr_data = 0x08;
	}
	flag = m_registry->I2CWrite(m_cameraID, 0x7C, 0xAB, m_registry->userParam);//page3
	flag = m_registry->I2CWrite(m_cameraID, 0x1c, wr_data, m_registry->userParam);
	flag = m_registry->I2CWrite(m_cameraID, 0x7C, 0xa8, m_registry->userParam);//page0
}

void DetectorWrapper::Sensor_Poll_Set(BYTE_T value)
{
	unsigned char  rd_reg_data = 0, wr_data = 0;
	ITA_RESULT flag = ITA_OK;

	flag = m_registry->I2CWrite(m_cameraID, 0x7C, 0xAB, m_registry->userParam);//page3
	flag = m_registry->I2CRead(m_cameraID, 0x14, &rd_reg_data, m_registry->userParam);
	wr_data = (rd_reg_data & 0xf0) | (value & 0x0f);
	flag = m_registry->I2CWrite(m_cameraID, 0x14, wr_data, m_registry->userParam);
	flag = m_registry->I2CWrite(m_cameraID, 0x7C, 0xA8, m_registry->userParam);//page0
}

void DetectorWrapper::Sensor_REF_POLL(BYTE_T value)
{
	unsigned char  rd_reg_data = 0, wr_data = 0;
	ITA_RESULT flag = ITA_OK;

	flag = m_registry->I2CWrite(m_cameraID, 0x7C, 0xAB, m_registry->userParam);//page3
	flag = m_registry->I2CRead(m_cameraID, 0x13, &rd_reg_data, m_registry->userParam);
	wr_data = (rd_reg_data & 0xe0) | (value & 0x1f);
	flag = m_registry->I2CWrite(m_cameraID, 0x13, wr_data, m_registry->userParam);
	flag = m_registry->I2CWrite(m_cameraID, 0x7C, 0xA8, m_registry->userParam);//page0
}

ITA_RESULT DetectorWrapper::Sensor_VCM_Set(BYTE_T value)
{
	ITA_RESULT flag = ITA_OK;
	flag = m_registry->I2CWrite(m_cameraID, 0x7c, 0xA8, m_registry->userParam); //page0; 
	flag = m_registry->I2CWrite(m_cameraID, 0x35, value, m_registry->userParam); // VCM //init 0x11
	flag = m_registry->I2CWrite(m_cameraID, 0x7C, 0xa8, m_registry->userParam);//page 
	return flag;
}

ITA_RESULT DetectorWrapper::Sensor_VRD_Set(BYTE_T value)  // init 0x68
{
	ITA_RESULT flag = ITA_OK;
	flag = m_registry->I2CWrite(m_cameraID, 0x7C, 0xAB, m_registry->userParam);//page 
	flag = m_registry->I2CWrite(m_cameraID, 0x1f, value, m_registry->userParam);// VRD 
	flag = m_registry->I2CWrite(m_cameraID, 0x7C, 0xa8, m_registry->userParam);//page 
	return flag;
}
ITA_RESULT DetectorWrapper::Sensor_RC_Set(BYTE_T value)  //0xbd
{
	ITA_RESULT flag = ITA_OK;

	flag = m_registry->I2CWrite(m_cameraID, 0x7C, 0xa9, m_registry->userParam);//page 
	flag = m_registry->I2CWrite(m_cameraID, 0x40, value, m_registry->userParam);// VC
	flag = m_registry->I2CWrite(m_cameraID, 0x7C, 0xa8, m_registry->userParam);//page 
	return flag;
}

ITA_RESULT DetectorWrapper::Sensor_Gsk_Set(BYTE_T value)   //0X42
{
	//TODO
	ITA_RESULT flag = ITA_OK;

	flag = m_registry->I2CWrite(m_cameraID, 0x7C, 0xAB, m_registry->userParam);//page0 
	flag = m_registry->I2CWrite(m_cameraID, 0x0F, value, m_registry->userParam);//bit[2]: 0 : GSK????? 1:GSK???? 
	flag = m_registry->I2CWrite(m_cameraID, 0x7C, 0xa8, m_registry->userParam);//page 
	return flag;
}

ITA_RESULT DetectorWrapper::Sensor_Ad_Step_Set(BYTE_T value)   //0X42
{
	ITA_RESULT flag = ITA_OK;
	flag = m_registry->I2CWrite(m_cameraID, 0x7C, 0xAB, m_registry->userParam);//page3
	flag = m_registry->I2CWrite(m_cameraID, 0x05, value, m_registry->userParam);
	flag = m_registry->I2CWrite(m_cameraID, 0x7C, 0xa8, m_registry->userParam);//page0
	return flag;
}

void DetectorWrapper::SensorPerfectSet(void)
{
	Sensor_INT_Set(40);
	Sensor_gHssd_Set(16);
	Sensor_Gain_Set(3);
	Sensor_gRaSel(35);
	Sensor_NUCParamSet(32, 8500, 7000);
	Sensor_PNMOS_Set(0, 1, 8);
	Sensor_Poll_Set(0);
	Sensor_REF_POLL(0);
}
void DetectorWrapper::NUC_Fine_Set(UINT16_T ad_high, UINT16_T ad_low)
{
	ITA_RESULT flag = ITA_OK;
	unsigned char  wr_data;

	flag = m_registry->I2CWrite(m_cameraID, 0x7C, 0xA8, m_registry->userParam);//page0

	wr_data = (ad_high >> 8) & 0xFF;
	flag = m_registry->I2CWrite(m_cameraID, 0x46, wr_data, m_registry->userParam);
	wr_data = ad_high & 0xFF;
	flag = m_registry->I2CWrite(m_cameraID, 0x45, wr_data, m_registry->userParam);

	flag = m_registry->I2CWrite(m_cameraID, 0x7C, 0xA8, m_registry->userParam);//page0
	wr_data = (ad_low >> 8) & 0xFF;
	flag = m_registry->I2CWrite(m_cameraID, 0x48, wr_data, m_registry->userParam);
	wr_data = ad_low & 0xFF;
	flag = m_registry->I2CWrite(m_cameraID, 0x47, wr_data, m_registry->userParam);

	flag = m_registry->I2CWrite(m_cameraID, 0x7C, 0xa8, m_registry->userParam);//page0
}
