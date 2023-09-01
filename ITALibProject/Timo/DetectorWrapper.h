/*************************************************************
Copyright (C), 2021--2022, Wuhan Guide Sensmart Tech Co., Ltd
File name   : DetectorWrapper.h
Author      : Guide Sensmart RD. wangyan
Version     : 1.0
Date        : 2021/11/9
Description : Configure Timo 256 detector.
*************************************************************/
#ifndef DETECTOR_WRAPPER_H
#define DETECTOR_WRAPPER_H

#include "../ITA.h"
#include "../Module/PortingAPI.h"
#include "../Module/GuideLog.h"

//探测器配置信息。参考示例代码或者探测器手册。
typedef struct DETECTOR_CONF {
	unsigned char addr;
	unsigned char regValue;
}DetectorConf;

typedef enum
{
	SENSOR_V1B = 1,
	SENSOR_V1C,
	SENSOR_NONE,
}SENSOR_TYPE;

typedef struct
{
	UINT16_T Detect_Tmp;
	UINT16_T INT;
	UINT16_T Gain;
	UINT16_T Rasel;
	UINT16_T Hssd;
	UINT16_T NUC_Step;
	UINT16_T NUC_low;
	UINT16_T NUC_high;
	UINT16_T Vsk_ad_low;
	UINT16_T Vsk_ad_high;
	UINT16_T Vsk_Thres;
	UINT16_T gPorNmos;
	UINT16_T gStb_en;
	UINT16_T gStbNum;
	UINT16_T Ref_poll;
	UINT16_T Poll;
	UINT16_T NUCValue;
	UINT16_T GSK;
	UINT16_T VCM;
	UINT16_T VRD;
	UINT16_T RD_RC;
	UINT16_T PMOS_Loop_High;
	UINT16_T PMOS_Loop_Low;
	UINT16_T Rasel_Loop_High;
	UINT16_T Rasel_Loop_Low;
	/*UINT16_T RegRead;
	UINT16_T MosDiff;
	UINT16_T NUCDiff;
	UINT16_T detect_table[20];
	UINT16_T reserved[48];*/
	//V1C新增参数
	UINT16_T NUC_HIGH_fine;
	UINT16_T NUC_LOW_fine;
	UINT16_T ADC_CALIB_NMIDDLE;
	UINT16_T SensorVerion;
	UINT16_T AD_STEP;	//5-250
	UINT16_T AD_STEP_HIGH;
	UINT16_T AD_STEP_LOW;
}SENSORCONFIG_STATUS;

class DetectorWrapper
{
public:

	DetectorWrapper(ITARegistry * registry, int cameraID, ITA_RANGE range);

	~DetectorWrapper();

	/**
	* @brief:	探测器初始化。
	* @param:	int clkHz			时钟，单位HZ。支持12M和6M两种时钟。
	* @return:	成功返回GUIDEIR_OK。失败返回GUIDEIR_ERR。
	* @note:
	**/
	ITA_RESULT DetectorInit(int clkHz, unsigned char validMode, unsigned char dataMode, unsigned char frameFrequency);

	/**
	* @brief:	做NUC。
	* @param:	无。
	* @return:	成功返回GUIDEIR_OK。失败返回GUIDEIR_ERR。
	* @note:
	**/
	int DoNUC();

	/**
	* @brief:	切换档位。
	* @param:	ITA_RANGE range		档位序号
	* @return:	成功返回GUIDEIR_OK。失败返回GUIDEIR_ERR。
	* @note:	同一测温范围下切换档位时探测器配置目前一样，不用更新。切换测温范围时需要重新配置探测器。
	**/
	int ChangeGears(ITA_RANGE range);

	void setLogger(GuideLog *logger);
	void setDebug(bool debug);

private:
	int SensorParamisValid(SENSORCONFIG_STATUS *pcConfig);
	int CheckOtherSensorParamValid(SENSORCONFIG_STATUS *pSensorConfig);
	int SetNucAll(unsigned char NucData);
	void Sensor_INT_Set(UINT16_T value);
	void Sensor_gHssd_Set(BYTE_T value);
	void Sensor_Gain_Set(BYTE_T value);
	void Sensor_gRaSel(BYTE_T value);
	void Sensor_NUCParamSet(BYTE_T Ad_step, UINT16_T ad_high, UINT16_T ad_low);
	void Sensor_PNMOS_Set(BYTE_T gPorNmos, BYTE_T gStb_en, BYTE_T gStbNum);
	void Sensor_Poll_Set(BYTE_T value);
	void Sensor_REF_POLL(BYTE_T value);
	ITA_RESULT Sensor_VCM_Set(BYTE_T value);
	ITA_RESULT Sensor_VRD_Set(BYTE_T value);
	ITA_RESULT Sensor_RC_Set(BYTE_T value);
	ITA_RESULT Sensor_Gsk_Set(BYTE_T value);
	ITA_RESULT Sensor_Ad_Step_Set(BYTE_T value);
	void SensorPerfectSet(void);
	void NUC_Fine_Set(UINT16_T ad_high, UINT16_T ad_low);
	
private:
	//在hisi平台上，init时值正常，ChangeGears时异常，sensorType=-1258291184
	//int m_sensorType;	//SENSOR_V1B = 1, SENSOR_V1C = 2
	SENSORCONFIG_STATUS m_params;
	int m_cameraID;
	ITARegistry *m_registry;
	ITA_RANGE m_range;
	GuideLog *m_logger;
	bool m_debug;
	int m_clkHz;
};
#endif // !DETECTOR_WRAPPER_H
