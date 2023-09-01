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
	int usHeadLength;//�ļ�ͷ����
	char cModuleMtMark[60];//ģ����±�ʶ��
	unsigned char    ucGearMark;//���µ�λ��־
	unsigned char    ucFocusNumber;//���¸���
	short            sTMin;//��С�����¶�
	short            sTMax;//�������¶�
	unsigned char    ucGain;//����
	unsigned char    ucInt;//����ʱ��
	unsigned char    ucRes;//res
	unsigned char    ucCurveNumber;//��������
	unsigned char    ucDistanceCompensateMode;//���벹��ģʽ
	unsigned char    ucDistanceNumber;//��������
	unsigned short   usWidth;//ͼ���
	unsigned short   usHeight;//ͼ���
	unsigned short   usCurveTemperatureNumber;//�����¶ȵ����

	unsigned short   usFocusArrayLength;//�������鳤��

	int usCurveDataLength;//�������ݳ���
	int   usKMatLength;//k���󳤶�						

	//���벹������
	float            fCompensateA1;
	float            fCompensateA2;
	float            fCompensateB1;
	float            fCompensateB2;
	float            fCompensateC1;
	float            fCompensateC2;
	//�궨�����
	unsigned short usDistanceArray[5];
	//������ʱ��
	char  cDate[10];
	char  cTime[10];
	char   cModuleCode[24];//ģ����
	char   cCollectionNumber[12];//�ɼ��Ϻ�
								 //ʹ�����µ�header�淶
								 //��װ��
	char   moduleToolID;
	//�ξߺ�
	char   moduleBoardID;
	//ģ�����ξ��ϵ�λ��
	char   modulePosition;
	//�����������ߵĺ�����Ŀ
	char   blackBodyNum;
	unsigned char focusType;
	unsigned char lensType;	//�ӳ������� 0:56�㣻1:25�㣻2:120�㣻3:50�㣻4:90�㣻5:33�㡣
	unsigned char mtType;	//����ģʽ 0�����壻1����ҵ���£�2����ҵ���¡�
	char fCorrectError[23];//У�����
    //unsigned short usReserve;//�����￪ʼ�����µĹ淶��ͬ��wangyan
    //float fCorrectError[7];//У�����
	unsigned short usMTPointX;//�궨���µ�����
	unsigned short usMTPointY;
}PackageHeader;

typedef enum ITAConfigType
{
	ITA_FUNCTION_CONFIG,
	ITA_MT_CONFIG,
	ITA_ISP_CONFIG
}ITA_CONFIG_TYPE;


typedef struct ITA_PRIVATE_DATA {
	long long second;         //�ػ�ʱ�䣬��λ�롣1970��1��1�յ����ڵ�ʱ�䡣         
	float orinalShutterTemp;  //���������¡�
}ITAPrivateData;

typedef enum ContentType
{
	CONTENT_DETECTOR_CONFIG = 1,	//̽������Ϣ
	CONTENT_MT_GENERAL = 2,			//ͨ�ò�����Ϣ
	CONTENT_MT_EXPERT = 3,			//ר�Ҳ��²���
	CONTENT_STATUS = 4,				//״̬��Ϣ
	CONTENT_TIMO = 5,				//ģ����Ϣ
	CONTENT_IMAGE = 6,				//�������
	CONTENT_AF = 7,					//������Ϣ
	CONTENT_REGIONAL_ANALYSIS = 8,	//���������Ϣ
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
#define MAX_CURVE_LENGTH			25000 //�����������ĳ��ȣ���λshort

class Parser
{
public:
	Parser(ITA_MODE mode, int width, int height, ITA_PRODUCT_TYPE type);
	~Parser();
	/*�ֳֺͻ�о��Ʒֻ�����߰�*/
	ITA_RESULT parseCurveData(int cameraID, ITARegistry *registry, ITA_FIELD_ANGLE lensType, ITA_RANGE range, int gear);
	ITA_RESULT parsePackageData(int cameraID, ITARegistry *registry, ITA_RANGE range, ITA_FIELD_ANGLE lensType);
	/*����bool shutterStatus���Ƿ��������״̬��MCU��ͨ����ʱ��Ҫ������MCU NUCʱ��Ҫ������*/
	ITA_RESULT parseParamLine(unsigned char *paramLine, int len, MtParams *mtParams, bool shutterStatus);
	ITA_RESULT parseTECLine(unsigned char *paramLine, int len, IMAGE_MEASURE_STRUCT *tecParams);
	ITA_RESULT getSensorTemp(int cameraID, ITARegistry *registry, MtParams *mtParams);
	bool isLensChanged();
	void isCorrChanged(float HighSCoff,float HighLCork,float LowSCoff,float LowLCork);
	unsigned short *getCurrentK();
	short *getCurves();
	//�������߳��ȣ���λshort��
	int getCurvesLen();
	ITA_RESULT setCurves(short * curveData, int curveLen);
	//ģ���Ʒ���߲�����Ҫ�������ݰ�ͷ�ͽ��±�
	unsigned char *getReservedData(int *length,bool isSubB,short bAverage);
	ITA_RESULT setReservedData(unsigned char *data, int length, bool* isSubB, short* bAverage);
	short *getFPArray();
	PackageHeader *getPH();
	DetectorParameter *getDP();
	ITA_RESULT changeGear(int cameraID, ITARegistry *registry, int currentGear, ITA_FIELD_ANGLE lensType);
	unsigned char *getConfBuf();
	void releaseConfBuf();
	ITA_RESULT parseConfig(unsigned char *buf, int bufSize, ITAConfig *pConfig);
	/*���ô������ļ��ж�ȡ�Ĳ�Ʒ�ͺ�*/
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
	/*120 90��ģ��P25Q����Flash�����ݰ�������д���ӿڲ�Ӱ��֮ǰ�Ĳ�Ʒ��*/
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
	/*��ȡһ�����ݣ����ظ��еĳ��ȡ�*/
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
	unsigned short *m_kArray;	//X16����תY16��ҪK��
	//��ģ�����ݰ���ȡ������
	int jwTableNumber;
	short* jwTableArray;
	short * m_4CurvesData;	//���뵱ǰ��λ��4�����ߣ��л���λʱ�������롣��ʡ�ڴ档
	unsigned char *headerArray;
	PackageHeader m_ph;
	unsigned char *m_confBuf;
	GuideLog *m_logger;
	short m_avgB;
	bool m_isConfEncrypt;
	bool m_isCold;
	int m_builtInFlash;  //1��120 90��ģ��P25Q����Flash��
	Flash120 *m_flash120;
	int m_lowPackageLen;
	unsigned char *m_lowPackage;
	int m_highPackageLen;
	unsigned char *m_highPackage;
	int m_humanPackageLen;
	unsigned char *m_humanPackage;
	unsigned char m_versionV;
	unsigned short m_lowStartSector, m_highStartSector, m_humanStartSector;
	float m_correctionFactors[9];     //�ڲξ��������У��ϵ��
	DetectorParameter m_dp;       //���ݰ���120̽��������
	unsigned char *m_reservedData; //ģ���Ʒ���߲�����Ҫ�������ݰ�ͷ�ͽ��±�
	short m_last_lltt; //��һ����У�¾�ͷ״̬�����״̬�仯��ô�������ߡ�
	short m_lltt; //��У�¾�ͷ��1���ǣ�0����
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