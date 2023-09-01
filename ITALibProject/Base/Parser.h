/*************************************************************
Copyright (C), 2021--2022, Wuhan Guide Sensmart Tech Co., Ltd
File name   : Parser.h
Author      : Guide Sensmart RD. wangyan
Version     : 1.0
Date        : 2021/11/9
Description : Parser module.
*************************************************************/
#ifndef ITA_PARSER_H
#define ITA_PARSER_H

#include <string>
#include "../ITA.h"
#include "../MT/MeasureTempCore.h"
#include "../MT/MeasureTempCoreTEC.h"
#include "../Timo/Flash120.h"
#include "../Timo/Timo120.h"

using std::string;
using std::stof;

typedef struct DATA_HEAD {
	int usHeadLength;//文件头长度
	char cModuleMtMark[60];//模组测温标识符
	unsigned char    ucGearMark;//测温档位标志
	unsigned char    ucFocusNumber;//焦温个数
	short            sTMin;//最小测量温度
	short            sTMax;//最大测量温度
	unsigned char    ucGain;//增益
	unsigned char    ucInt;//积分时间
	unsigned char    ucRes;//res
	unsigned char    ucCurveNumber;//曲线条数
	unsigned char    ucDistanceCompensateMode;//距离补偿模式
	unsigned char    ucDistanceNumber;//距离点个数
	unsigned short   usWidth;//图像宽
	unsigned short   usHeight;//图像高
	unsigned short   usCurveTemperatureNumber;//曲线温度点个数

	unsigned short   usFocusArrayLength;//焦温数组长度

	int usCurveDataLength;//曲线数据长度
	int   usKMatLength;//k矩阵长度						

	//距离补偿参数
	float            fCompensateA1;
	float            fCompensateA2;
	float            fCompensateB1;
	float            fCompensateB2;
	float            fCompensateC1;
	float            fCompensateC2;
	//标定距离点
	unsigned short usDistanceArray[5];
	//日期与时间
	char  cDate[10];
	char  cTime[10];
	char   cModuleCode[24];//模组编号
	char   cCollectionNumber[12];//采集料号
								 //使用最新的header规范
								 //工装号
	char   moduleToolID;
	//治具号
	char   moduleBoardID;
	//模组在治具上的位置
	char   modulePosition;
	//用于生成曲线的黑体数目
	char   blackBodyNum;
	unsigned char focusType;
	unsigned char lensType;	//视场角类型 0:56°；1:25°；2:120°；3:50°；4:90°；5:33°。
	unsigned char mtType;	//测温模式 0：人体；1：工业低温；2：工业高温。
	char fCorrectError[23];//校验误差
    //unsigned short usReserve;//从这里开始与最新的规范不同。wangyan
    //float fCorrectError[7];//校验误差
	unsigned short usMTPointX;//标定测温点坐标
	unsigned short usMTPointY;
}PackageHeader;

typedef enum ITAConfigType
{
	ITA_FUNCTION_CONFIG,
	ITA_MT_CONFIG,
	ITA_ISP_CONFIG
}ITA_CONFIG_TYPE;


typedef struct ITA_PRIVATE_DATA {
	long long second;         //关机时间，单位秒。1970年1月1日到现在的时间。         
	float orinalShutterTemp;  //开机快门温。
}ITAPrivateData;

typedef enum ContentType
{
	CONTENT_DETECTOR_CONFIG = 1,	//探测器信息
	CONTENT_MT_GENERAL = 2,			//通用测温信息
	CONTENT_MT_EXPERT = 3,			//专家测温参数
	CONTENT_STATUS = 4,				//状态信息
	CONTENT_TIMO = 5,				//模组信息
	CONTENT_IMAGE = 6,				//成像参数
	CONTENT_AF = 7,					//调焦信息
	CONTENT_REGIONAL_ANALYSIS = 8,	//区域分析信息
}CONTENT_TYPE;

typedef struct ITABurnInfo {
	unsigned char burnSwitch;
	unsigned char burnFlag;
	short burnDelayTime;
	int burnThreshold;
}ITA_BURN_INFO;

#define MAX_FOCUS_NUMBER			100
#define MAX_PACKAGE_HEADER			512
#define MAX_CONFIG_FILE_SIZE		128*1024	//8192
#define MAX_CONFIG_LINE_SIZE		512
#define MAX_TEC_CURVE_SIZE			16384 //32768
#define MAX_PRIVATE_DATA_SIZE       64*1024
#define MAX_CURVE_LENGTH			25000 //单条曲线最大的长度，单位short

class Parser
{
public:
	Parser(ITA_MODE mode, int width, int height, ITA_PRODUCT_TYPE type);
	~Parser();
	/*手持和机芯产品只有曲线包*/
	ITA_RESULT parseCurveData(int cameraID, ITARegistry *registry, ITA_FIELD_ANGLE lensType, ITA_RANGE range, int gear);
	ITA_RESULT parsePackageData(int cameraID, ITARegistry *registry, ITA_RANGE range, ITA_FIELD_ANGLE lensType);
	/*参数bool shutterStatus：是否解析快门状态。MCU普通快门时不要解析，MCU NUC时需要解析。*/
	ITA_RESULT parseParamLine(unsigned char *paramLine, int len, MtParams *mtParams, bool shutterStatus);
	ITA_RESULT parseTECLine(unsigned char *paramLine, int len, IMAGE_MEASURE_STRUCT *tecParams);
	ITA_RESULT getSensorTemp(int cameraID, ITARegistry *registry, MtParams *mtParams);
	bool isLensChanged();
	void isCorrChanged(float HighSCoff,float HighLCork,float LowSCoff,float LowLCork);
	unsigned short *getCurrentK();
	short *getCurves();
	//返回曲线长度，单位short。
	int getCurvesLen();
	ITA_RESULT setCurves(short * curveData, int curveLen);
	//模组产品离线测温需要保存数据包头和焦温表
	unsigned char *getReservedData(int *length,bool isSubB,short bAverage);
	ITA_RESULT setReservedData(unsigned char *data, int length, bool* isSubB, short* bAverage);
	short *getFPArray();
	PackageHeader *getPH();
	DetectorParameter *getDP();
	ITA_RESULT changeGear(int cameraID, ITARegistry *registry, int currentGear, ITA_FIELD_ANGLE lensType);
	unsigned char *getConfBuf();
	void releaseConfBuf();
	ITA_RESULT parseConfig(unsigned char *buf, int bufSize, ITAConfig *pConfig);
	/*设置从配置文件中读取的产品型号*/
	void setProductType(ITA_PRODUCT_TYPE type);
	void setLogger(GuideLog *logger);
	ITA_RESULT parseCalibrateParam(int cameraID, ITARegistry *registry, ITA_RANGE range, MtParams *mtParams); 
	ITA_RESULT correctBadPoint(ITA_POINT point);
	ITA_RESULT saveBadPoints(ITA_POINT *pointsArray, int size, int cameraID, ITARegistry *registry, ITA_RANGE range);
	short getAvgB();
	void setConfEncrypt(bool isEncrypt);
	ITA_RESULT isColdOrHot(int cameraID, ITARegistry *registry, MtParams *mtParams, int period, unsigned char* status);
	ITA_RESULT saveColdOrHot(int cameraID, ITARegistry *registry, MtParams * mtParams);
	void setBuiltInFlash(int type, ITARegistry * registry, int cameraID);
	/*120 90°模组P25Q内置Flash读数据包，单独写个接口不影响之前的产品。*/
	ITA_RESULT parseFlashPackage(int cameraID, ITARegistry *registry, ITA_RANGE range);
	ITA_RESULT getCorrectionFactors(float *pCorrectionFactors);
	ITA_RESULT saveK(int camaraId, unsigned short* m_gain_mat, ITARegistry* m_registry, int m_curGear, short temp);
	int getlltt();
	bool getCoffStatus();
	ITA_BURN_INFO getBurnInfo();
	void setChStatus(unsigned char status);
private:
	float getTemp(short ad);
	float getFPATemp(short ad);
	/*读取一行数据，返回该行的长度。*/
	int readLine(unsigned char *buf, int dataSize, unsigned char *lineBuf, int lineBufSize);
	int stringToInt(char * line, int *a, char * s);
	int stringToFloat(char * line, float *a, char * s);
	int stringToDistanceRange(char * line, float *a, float *b, char * s);
	int stringToCharArray(char * line, char *p, int pLen, char * s);
	ITA_RESULT readBuiltInFlash();
	ITA_RESULT readFlashPackage(unsigned char *buf, int offset, int size, ITA_RANGE range);
	ITA_RESULT writeFlashPackage(unsigned char *buf, int offset, int size, ITA_RANGE range);
private:
	ITA_MODE m_mode;
	ITA_PRODUCT_TYPE m_type;
	ITA_RANGE m_range;
	int m_frameSize;
	int m_width, m_height;
	unsigned short *m_kArray;	//X16数据转Y16需要K。
	//从模组数据包读取的数据
	int jwTableNumber;
	short* jwTableArray;
	short * m_4CurvesData;	//载入当前档位的4条曲线，切换档位时重新载入。节省内存。
	unsigned char *headerArray;
	PackageHeader m_ph;
	unsigned char *m_confBuf;
	GuideLog *m_logger;
	short m_avgB;
	bool m_isConfEncrypt;
	bool m_isCold;
	int m_builtInFlash;  //1：120 90°模组P25Q内置Flash。
	Flash120 *m_flash120;
	int m_lowPackageLen;
	unsigned char *m_lowPackage;
	int m_highPackageLen;
	unsigned char *m_highPackage;
	int m_humanPackageLen;
	unsigned char *m_humanPackage;
	unsigned char m_versionV;
	unsigned short m_lowStartSector, m_highStartSector, m_humanStartSector;
	float m_correctionFactors[9];     //内参矩阵参数和校正系数
	DetectorParameter m_dp;       //数据包中120探测器参数
	unsigned char *m_reservedData; //模组产品离线测温需要保存数据包头和焦温表
	short m_last_lltt; //上一次免校温镜头状态。如果状态变化那么更新曲线。
	short m_lltt; //免校温镜头。1，是；0，否。
	float m_lastHighSCoff;
	float m_lastHighLCork;
	float m_lastLowSCoff;
	float m_lastLowLCork;
	bool m_isCoffChange;

	ITA_BURN_INFO m_burnInfo;

	int curCurveLen;
	int curJwNumber;
	int curGJwNumber;

};

#endif // !ITA_PARSER_H