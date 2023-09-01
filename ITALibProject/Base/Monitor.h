/*************************************************************
Copyright (C), 2021--2022, Wuhan Guide Sensmart Tech Co., Ltd
File name   : Monitor.h
Author      : Guide Sensmart RD. wangyan
Version     : 1.0
Date        : 2021/11/9
Description : ITA SDK Monitor.
*************************************************************/
#ifndef ITA_MONITOR_H
#define ITA_MONITOR_H

#include "../Module/PortingAPI.h"
#include "../Base/Config.h"
#include "../ITA.h"
#include "../Base/Parser.h"
#include "../InfraredImage/ImageProcessor.h"


#define AVGB_LOWER_LIMIT	3000

typedef enum MonitorStatus
{
	MONITOR_NONE,
	MONITOR_SHUTTER_NOW,
	MONITOR_NUC_NOW,
	MONITOR_NUC_CLOSED_LOOP, //120模组60℃环温RES+1闭环
	MONITOR_NUC_LOOP_REPEAT, //120模组60℃环温RES不变做NUC
	MONITOR_NUC_RECOVERY, //120模组环温下降到正常测温范围，RES恢复数据包中的值。
}MONITOR_STATUS;

//0：初始状态；1：正在Res闭环；2：闭环成功；3：闭环失败。
typedef enum ClosedLoopStatus
{
	CLOSED_LOOP_NONE,
	CLOSED_LOOP_ING,
	CLOSED_LOOP_SUCCESS,
	CLOSED_LOOP_FAILED, //120模组60℃环温RES闭环
}CLOSED_LOOP_STATUS;

class Monitor
{
public:
	Monitor(ITA_PRODUCT_TYPE type);
	~Monitor();

	ITA_RESULT setInterval(unsigned int second);

	bool isSaveColdHot(unsigned int second);

	ITA_RESULT autoShutter(ITA_SWITCH flag);

	/*自动快门检查*/
	MONITOR_STATUS timeLoopOnce(unsigned int currentTime);

	/*焦温表信息，120模组RES闭环用到。*/
	ITA_RESULT setFPAArray(short* fpaTempArray, int size, unsigned char startRes);

	/*更新本底均值，120模组RES闭环用到。*/
	ITA_RESULT updateAVGB(short avgB);

	/*将切档放在NUC闭环流程内；如果同时满足焦温高于最高采集焦温+3℃和本底均值小于设置的NUC闭环值-2000则执行RES闭环。
	如果满足焦温小于等于最高采集焦温，则RES重新设回曲线包的值。RES取值范围为8~11，可以采取遍历的方法进行闭环，
	RES越大AD值越大，焦温越高AD值越低；本底均值与减掉2000后的NUC闭环值，作为判断条件，如果大于则认为闭环完成，
	小于等于则RES读出值加1后重新做NUC后获取本底均值后再做判断。*/
	unsigned char calcRes(unsigned char currentRes);

	/*根据自动快门策略参数更新快门周期*/
	ITA_RESULT updatePeriod(unsigned int currentTime);

	//更新焦温，放大100倍。
	int updateSensorTemp(int fpaTemp);

	//更新锅盖参数
	void updatePotCoverAlgoPara(MtParams* mtParam, stDGGTPara* potCoverPara);

	ITA_RESULT shutterManual(int flag);

	ITA_RESULT nucManual(int flag);

	int closeShutterPolicy(bool isClose);

	/*设置从配置文件中读取的产品型号*/
	void setProductType(ITA_PRODUCT_TYPE type);
	void setLogger(GuideLog *logger);

	unsigned int getLastShutterTime();

	void setShutterTempUpper(float temp);
	void setNucTempUpper(float temp);
	void setShutterIntervalBottom(unsigned int interval);
	void setNucIntervalBottom(unsigned int interval);
	void setShutterParam(unsigned int p1, unsigned int p2, unsigned int p3, unsigned int p4, unsigned int p5, unsigned int p6);
private:
	ITA_PRODUCT_TYPE m_type;
	unsigned int m_shutterPeriod;
	unsigned int m_lastShutterTime;
	unsigned int m_lastNUCTime;
	unsigned int m_lastSaveCHTime;
	bool m_shutterManual;
	bool m_nucManual;
	int m_realTimeFPATemp;
	int m_fpaTempWhenShutter;	//上次快门时焦温
	int m_fpaTempWhenNUC;		//上次NUC时焦温
	int m_shutterTempUpper;		//温升上限
	int m_nucTempUpper;
	unsigned int m_shutterIntervalBottom;	//最小间隔时间
	unsigned int m_nucIntervalBottom;
	bool m_autoShutter;
	bool m_closeShutterPolicy;	//关闭策略后不再检测自动快门和NUC。
	GuideLog *m_logger;
	unsigned int m_originalTime;	//开机时间
	unsigned int m_bootTimeToNow1;	//开机60分钟内
	unsigned int m_shutterPeriod1;	//自动快门周期30秒
	unsigned int m_bootTimeToNow2;	//开机60分钟与120分钟之间
	unsigned int m_shutterPeriod2;	//自动快门周期60秒
	unsigned int m_bootTimeToNow3;	//开机120分钟之后
	unsigned int m_shutterPeriod3;	//自动快门周期90秒
	unsigned int m_saveStatusPeriod;//10分钟保存一次能热机状态
	bool m_userPeriod;	//用户是否设置快门周期。如果用户设置自动快门时间，那么默认的自动快门策略不再有效。
	/*将切档放在NUC闭环流程内；如果同时满足焦温高于最高采集焦温+3℃和本底均值小于设置的NUC闭环值-2000则执行RES闭环。
	如果满足焦温小于等于最高采集焦温，则RES重新设回曲线包的值。RES取值范围为8~11，可以采取遍历的方法进行闭环，
	RES越大AD值越大，焦温越高AD值越低；本底均值与减掉2000后的NUC闭环值，作为判断条件，如果大于则认为闭环完成，
	小于等于则RES读出值加1后重新做NUC后获取本底均值后再做判断。*/
	short m_maxFpaTemp;
	short m_avgB;
	//unsigned char m_startRes;
	CLOSED_LOOP_STATUS m_closedLoopStatus; //0：初始状态；1：正在Res闭环；2：闭环成功；3：闭环失败。
};

#endif // !ITA_MONITOR_H



