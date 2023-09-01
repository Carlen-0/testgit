/*************************************************************
Copyright (C), 2021--2022, Wuhan Guide Sensmart Tech Co., Ltd
File name   : ITAWrapper.h
Author      : Guide Sensmart RD. wangyan
Version     : 1.0
Date        : 2021/11/9
Description : ITA SDK management module.
*************************************************************/
#ifndef ITA_WRAPPER_H
#define ITA_WRAPPER_H

#include "../ITA.h"
#include "../Timo/DetectorWrapper.h"
#include "../InfraredImage/ImageProcessor.h"
#include "../Base/Monitor.h"
#include "../MT/MTWrapper.h"
#include "../MT/MTTEC.h"
#include "../Module/Debugger.h"
#include "../Timo/Timo120.h"
#include "../Timo/AutoFocus.h"
#include "../AppAlg/CInfraredAF.h"
#include "./CollectK.h"
#include "./StartReview.h"
#include "../ITAToolBox.h"
#include "../ToolBox/ToolBox.h"

#ifdef ULTIMATE_EDITION

#define BLACK_BODY_NUMBER	5
#define COLLECT_Y16_NUMBER	20	//ÿ������ɼ�15��Y16

class ITAWrapper
{
public:
	ITAWrapper(ITA_MODE mode, ITA_RANGE range, int width, int height, int cameraID);
	~ITAWrapper();

	ITA_RESULT setRegister(ITARegistry *registry);

	ITA_RESULT setMeasureRange(ITA_RANGE range);

	ITA_RESULT detectorControl(ITA_DC_TYPE type, void *param);

	ITA_RESULT configureDetector(int clock);

	ITA_RESULT prepare(ITAImgInfo *pInfo);

	ITA_RESULT doISP(unsigned char *srcData, int srcSize, ITAISPResult *pResult, unsigned char *y8Data, int y8Size);

	ITA_RESULT measureTempPoint(short y16, float *surfaceTemp);

	ITA_RESULT measureTempMatrix(short* y16Array, int y16W, int y16H,
		int x, int y, int w, int h, float distance, ITA_MATRIX_TYPE type, float *tempMatrix);

	ITA_RESULT baseControl(ITA_BC_TYPE type, void *param);

	ITA_RESULT measureControl(ITA_MC_TYPE type, void *param);

	ITA_RESULT imageControl(ITA_IC_TYPE type, void *param);

	ITA_RESULT getParamInfo(ITAParamInfo *info);

	ITA_RESULT debugger(ITA_DEBUG_TYPE type, void *param);

	ITA_RESULT addCustomPalette(int count, unsigned char *data, int dataLen);

	ITA_RESULT getColorImage(unsigned char *pY8, int w, int h, unsigned char *pPalette, int paletteLen, unsigned char *pImage, int imgLen);

	ITA_RESULT calcY16ByTemp(float surfaceTemp, short *y16);

	ITA_RESULT calcY16MatrixByTemp(float *tempMatrix, int w, int h, short *y16Matrix, int matrixSize);

	ITA_RESULT getVersion(ITAVersionInfo *pVersion);

	ITA_RESULT updateParamLine(unsigned char *paramLine, int len);

	ITA_RESULT measureBody(float surfaceTemp, float envirTemp, float *bodyTemp);

	ITA_RESULT setDimmingType(ITA_DRT_TYPE type, ITADRTParam *param);

	ITA_RESULT setEqualLine(ITA_EQUAL_LINE_TYPE type, ITAEqualLineParam param);

	ITA_RESULT correctBadPoints(ITA_CORRECT_TYPE type, void *param);

	ITA_RESULT startCollect(ITACollectParam collectParam, ITACollectFinish cb, void *param);

	ITA_RESULT startCalibrate(float *kf, float *b, ITACalibrateResult *result);

	ITA_RESULT calibrateByUser(ITA_RANGE range, ITA_CALIBRATE_TYPE type, float p);

	ITA_RESULT setTECInfo(ITATECInfo info);

	ITA_RESULT getTECInfo(ITATECInfo * pInfo);

	ITA_RESULT setMeasureInfo(ITAMeasureInfo info);

	ITA_RESULT getMeasureInfo(ITAMeasureInfo* info);

	ITA_RESULT addInterceptor(PostHandle postHandle);

	ITA_RESULT getScene(ITASceneParam *param);

	ITA_RESULT setScene(ITASceneParam param);

	ITA_RESULT zoomY16(short* srcY16, int srcW, int srcH, short* dstY16, int dstW, int dstH, float times);

	ITA_RESULT isChangeRange(short *pSrc, int width, int height, int range,
		float areaTh1, float areaTh2, int low2high, int high2low, int *isChange);

	ITA_RESULT isMultiChangeRange(short* pSrc, int width, int height, int range,
		float areaTh1, float areaTh2, int low2mid, int mid2low, int mid2high, int high2mid, int* isChange);

	ITA_RESULT processBiCubic(void* dst, void* src, int w, int h, int w_zoom, int h_zoom, ITA_DATA_TYPE dataType);

	ITA_RESULT convertY8(const unsigned char* src, int srcW, int srcH, unsigned char* dst, int dstW, int dstH, int dateLen, ITA_Y8_DATA_MODE datamode = ITA_Y8_NORMAL);

	ITA_RESULT customPalette(ITA_PALETTE_OP_TYPE type, void* param);
	
	ITA_RESULT laserAFRegister(ITALaserAFRegistry *registry);

	ITA_RESULT laserAF();

	ITA_RESULT contrastAF();

	ITA_RESULT calibrateAF(ITA_AF_CALIBRATE type, int *diff);

	ITA_RESULT setInfoAF(ITA_AF_INFO type, void *param);

	ITA_RESULT collectK(int gear, float blackTemp, ITACollectBaseFinish cb, void* userParam);

	ITA_RESULT saveK(int gear, ITACollectBaseFinish cb, void* param);

	ITA_RESULT potCoverRestrain(ITA_POTCOVER_OP_TYPE type, void* para);

	ITA_RESULT startReview(float temper, ITAReviewFinish cb, void* userParam);

	/* �Զ����� */
	ITA_RESULT AutoCorrectTemp(float* pBlackBodyTemp, short* pY16, int arrLength);

	/* ToolBox */
	ITA_RESULT AutoResponseRate(short lowY16, short highY16, int currentINT, int* pOutINT, short responseRate, short precision);
	ITA_RESULT AutoGetKMatirx(short* pLowBase, short* pHighBase, int width, int height, unsigned short* pKMatrix);
	ITA_RESULT AutoCorrectBadPoint(unsigned short* pKMatrix, int width, int height, unsigned short thresh, int* pBadPointInfo);
	ITA_RESULT AddBadPoint(unsigned short* pKMatrix, int x, int y, int width, int height);
	ITA_RESULT AddBadLine(unsigned short* pKMatrix, int lineIndex, int width, int height, ITAAddBadLineMode mode);
	ITA_RESULT IfConsistency(float centerPointTemp, float* pOtherPointTemp, int otherPointNum, ITAIfConsistencyType* pResult);
	ITA_RESULT GenerateCurve(double* pTempMatrix, short* pY16, int arrLength, int maxTemp, short* pCurve, int curveLength, int* pValidCurveLength);
	ITA_RESULT AutoGetDrift(ITADriftInfo* pDriftInfo, int driftInfoLength, int* pShutterDrift, int* pLenDrift);
	ITA_RESULT AutoMeasureDistanceCoff(float* pTempMatrix, double* pDistance, int numOfBlackBody, int numOfDistance,
		int standDistance, int* pDistanceCoff, ITADistanceCoffMode mode);
	ITA_RESULT AutoMeasureAmbientCoff(double* pStandardAmbientTemp, float standardAmbientTemp,
		double* pCurrentAmbientTemp, float currentAmbientTemp, int tempMatrixSize, int* pK3, int* pB2);
	ITA_RESULT AutoCheckTemperature(float* pStandTemp, float* pMeasureTemp, int arrLength,
		int* pNonConformTempIndex, int* pNonConformTempNum);
	// ���·������� 
	ITA_RESULT checkMeasureTempPreCondition();
	ITA_RESULT AnalysisAdd(ITAShape* shape, int size);
	ITA_RESULT AnalysisClear();
	ITA_RESULT AnalysisTemp(short* y16Data, int width, int height, ITAAnalysisTemp* analysisTemp, int size);
	ITA_RESULT AnalysisY16(short* y16Data, int width, int height, ITAAnalysisY16Info* analysisY16Info, int size);
	ITA_RESULT AnalysisGetRangeMask(ITARangeMask* rangeMask, int size);
private:
	int collectBackground(short * srcData, int srcSize);
	int shutterNow();
	int nucNow();
	ITA_RESULT doISPTEC(unsigned char *srcData, int srcSize, ITAISPResult *pResult, unsigned char *y8Data, int y8Size);

private:
	ITA_MODE m_mode;
	ITA_RANGE m_range;
	ITA_PRODUCT_TYPE m_productType;
	int m_width, m_height, m_cameraID, m_realW, m_realH;
	ITARegistry *m_registry;
	ITAConfig m_config;
	DetectorWrapper *m_detector;
	Timo120 *m_timo120;
	ImageProcessor *m_processor;
	Monitor *m_monitor;
	Parser *m_parser;
	MTBase *m_mtBase;
	int m_fpaGear;			//���浱ǰ��λ������¿����ĵ�λ�Ƚϣ���ͬ���е���
	bool m_isGearManual;	//�ֶ��е��򿪺��Զ��е���ʧЧ��
	int m_gearManual;		//�ֶ���λ���뵱ǰ��λm_fpaGear�Ƚϣ����Ⱦ��л���
	bool m_isChangeRange;	//�Ƿ��л����·�Χ���л������и������ݰ����ܲ��¡�
	bool m_isChangeGear;	//�Ƿ��л���λ���л������и������ߺ�K���ܲ��¡�
	bool m_subAvgB;			//�����Ʒ����о��ASIC�㷨���쵼��Y16��ȥ���׾�ֵ֮����ܲ��¡�Ĭ�ϲ��ü���
	short m_avgB;
	float m_scale;
	ITA_ROTATE_TYPE m_rotate;
	bool m_isRotate;	//�����ת����ô���¼����ߡ�
	ITA_FLIP_TYPE m_flip;
	ITA_PIXEL_FORMAT m_pixelFormat;
	ITA_PALETTE_TYPE m_paletteIndex;
	int m_frameSize;
	short m_x16CenterValue;
	bool m_reCalcSize;	//�����ű��������ظ�ʽ�仯ʱ����Ƿ���Ҫ���������ڴ档
	unsigned char *m_imgDst;
	short *m_y16Data;
	short *m_y16TempMatrix; //m_subAvgB��Ҫ�����׵�����������ڴ棬��Ӱ���û�ʹ�õ��ڴ档
	int m_tempMatrixLen;//�¶Ⱦ��󻺴泤�ȡ�����ֱ��ʿ��ܱ仯����Ҫ���·����ڴ档�����Ʒ�г��ֱ���ͼƬ��Ҫ�Ŵ�
#ifdef SOURCE_COPY
	short *m_x16Data;	//Դ���ݵĿ�����ԭ���ϲ���Ҫ��
#endif // SOURCE_COPY
	/*��Y16��������˴����ò���K��B���ݡ�*/
	short *m_bArray;			//X16����תY16��ҪB����¼һ֡���ס�4֡��������ͳһ��short��
	bool m_loop;	//��¼�Ƿ���һ֡�Ĵ�������У�ISP�ӿڷ��غ���������������ͷ�shutterTool��ISP��ʹ��ʱ����������������MCU��������ʵ�֡�
	bool m_reverse;	//�Ƿ���ITA_X16��ITA_MCU_X16ģʽ����Ч��
	unsigned int m_lastSensorTime;	//�ϴλ�ȡ�´���ʱ�䣬ÿ��ȡһ�Ρ�
	/*���űպϹ����вɱ���*/
	bool m_isCollectBackground;	//�ɱ��ױ��
	int m_backgroundCount;		//���׼��������ɼ�4֡��
	int m_skipCount;			//���ڻ�������֡ԭʼ���ݣ����űպϺ�Ҫ����5֡�ٿ�ʼ�ɱ��ף�����ɵ��պ�ǰ��ͼ��
	int m_isFPGAB;	//FPGA�Ƿ�ʼ�ɼ����ס�0��������1��FPGA��ʼ�ɼ����ף�2��FPGA�����ɼ����ס�SDK���Ų�����Ч����Ҫ֪ͨFPGA�ɱ��ף����ɼ�����ٵ������š�
	bool m_fpgaBing; //true�����ڲɼ����ס����ڷ���ָ�����һ֡��һ�����ϱ����1���ڲɼ����ס�������첽�ġ���Ҫ�и���Ǳ����ɼ������ס�
	unsigned int m_FPGAStartTime;	//��¼֪ͨFPGA��ʼ�ɼ����׵�ʱ�䣬���ڳ�ʱ�жϡ�
	bool m_firstNUCing;	//��ʼ�������е�һ��NUC��ɺ��ٻص�ͼ�񣬱���ż���������⡣
	int m_nucStatus;	//��¼NUC״̬��0����ʼ��״̬��1����ʼ��NUC��2��������NUC��3��NUC����ɡ�
	/*nuc��ɺ���ܴ���2�����⣺�����쳣���Y16�쳣�����µ�0.5�����ҡ��ٴ�һ�ο��ſ��Խ����*/
	int m_nucAnotherShutter; //�ظ����ž�����ơ�Ĭ��0�رա�1���û���nuc�ظ����Ż��ƣ�2�������ظ����ţ�3�����ڿ��š����Ŷ�����ɺ��е�1״̬��
	int m_nucRepeatCorrection; //�ظ�NUC������ơ�Ĭ��0�رա�1���û���nuc�ظ�������ơ�
	int m_nucRepeatCount; //��������ظ�3��NUC��
	Debugger m_debugger;
	GuideLog *m_logger;
	bool m_debug;
	unsigned int g_currentTime;	//ͳһ��һ��������ִ��һ������ֻ�û�ȡһ��ʱ�䡣
	unsigned int m_ispTime;
	//bool m_closeShutterPolicy;	//�رղ��Ժ��ټ���Զ����ź�NUC���㷨�ڼ��㵵λ��ȻҪ�ã��е��������ߡ�
	bool m_closeImage;	//Y16תY8���ء�ĳЩ��Ʒֻ��Ҫ���¹��ܣ�ͼ���������оƬ��ɡ�
	/*�Զ�У�¹��ܡ�ZX03CУ�£��ֱ����30�㡢33�㡢36�����ɼ�һ��Y16���ݣ�ÿ�βɼ�ʱ��ر������Ų��ԣ�
	Ȼ���жϵ�ǰ��ʼ�Ĳɼ�ʱ�����һ�ο���ʱ���Ƿ����10s�����С��10s��Ҫ�ȴ���ֱ�����10s��
	�������10sֱ�Ӵ���Ųɼ����ɼ�ʱÿ��1s�ɼ�һ��Y16����ɼ�15�Σ�Ȼ����15�ε�ƽ��ֵ��
	ZX03BУ�£��ֱ����30�㡢33�㡢36�����ɼ�һ��Y16���ݣ�ÿ�βɼ���ѭ����Σ��ȴ�һ����ͨ���ţ�
	Ȼ��ȡ�ڶ����������Y16���ݣ���ȡ��10��Y16������ƽ����*/
	int m_isAutoCollect;	//��¼�ɼ�״̬��0����ʼ��״̬��1��׼���ɼ���2�����ڲɼ���3��һ�α궨����ɼ���ɡ�
	unsigned int m_shutterCollectInterval;//�ϴο���֮���������뿪ʼ����Ųɼ�Y16������Ƶ�����ŵ��²���ģ����¡�
	unsigned int m_collectInterval;//�������βɼ����ʱ��
	unsigned int m_collectNumberOfTimes;	//һ�ο���֮����Ҫ�ɼ��Ĵ���
	//int m_shutterNumberOfTimes;	//ÿ������ɼ������Ҫ�Ŀ��Ŵ���������һ�ֲɼ�������ʱ��ʵ�֡�
	//int m_shutterTimes;		//һ�κ���ɼ������ڿ��ż�����<= m_shutterNumberOfTimes��
	unsigned int m_collectTimes;//һ�κ���ɼ������ڲɼ�Y16������<= m_collectNumberOfTimes��
	int m_collectCount;		//�ڼ�������ɼ���һ��3�Σ�30 33 36.
	float m_blackTempArray[BLACK_BODY_NUMBER];	//�궨�����¶�
	ITACollectFinish m_collectCBArray[BLACK_BODY_NUMBER];
	void *m_cbParamArray[BLACK_BODY_NUMBER];
	short m_blackY16Array[COLLECT_Y16_NUMBER];//ÿ������ɼ�15��Y16ֵ������Ҫ>=m_collectNumberOfTimes��
	short m_y16AvgArray[BLACK_BODY_NUMBER];	//ÿ���궨�����Y16ƽ��ֵ
	ITA_COLLECT_TYPE m_collectType;
	float m_collectParam;
	/*У����*/
	int badPointTotalNumber;
	ITA_POINT* m_badPointsArray;
	int m_badPointsCount;
	ITA_POINT m_badPoint;
	int m_shutterFrameNumber; //�������ƿ��Ŵӱպϵ����������̵�ʱ��
	int m_bDelayFrame;	//�ɼ�������ʱ
	int m_shutterCloseDelay; //�պϿ��ŵ���ʱ��MCUʱ��ϳ�
	int m_shutterOpenDelay; //�򿪿��ŵ���ʱ��MCUʱ��ϳ�

	unsigned int m_fps;	//�����ʵ��֡��
	unsigned int m_lastIspTime;
	unsigned int m_userIspTime; //�û����õ�ƽ��֡����ʱ�䡣��Ϊʵʱ�����֡ʱ��ܲ��ȶ���Ӱ��ʵ��֡�ʡ�
	unsigned int m_maxFrameRate;
	/*����һ�ַ��������������û�ָ��֡�ʺ�������֡������֡��*/
	unsigned int m_userFps; //�û����õ�֡��
	//unsigned int m_frameCount; //ÿ�����������֡��

	unsigned int m_nucStartTime; //CB360��Ʒ��¼nuc��ʼʱ�䣬2������״̬��0��
	AutoFocus *m_af;
	bool m_isAF; //�û��Ƿ�ʼʹ��AF������Ǿ���֡��Y16��
	bool m_isCalcCentral; //����������

	/*̽��������*/
	unsigned char m_validMode;//0��̽����VSYNC��HSYNC����Ч��1��̽����VSYNC��HSYNC����Ч��Ĭ��Ϊ0��
	unsigned char m_dataMode;//0���ȷ�16λ���ݵĸ�8bits��1���ȷ�16λ���ݵĵ�8bits��Ĭ��Ϊ1��
	unsigned char m_frameFrequency;//̽����֡Ƶ����λfps��Ĭ��25��

	int m_builtInFlash;  //1��120 90��ģ��P25Q����Flash��
	//����Խ� ZC12A
	CInfraredAF *m_laserAF;

	stDGGTPara guoGaiPara;//ȥ���ǲ���

	CollectK* m_collectK;//�ɼ�kģ��
	//����K����
	unsigned short* m_gain_mat;//K��������;
	StartReview* m_startReview;//�����¶�
	int m_coldHotDelay;//����Ȼ�ʱ���ж���
	unsigned char m_coldHotStatus;//���Ȼ�״̬: 1:��� 0���Ȼ�

	unsigned char m_asicStatus;/*1:��asic��ָ��ǰ������״̬Ϊ0 2:��asic��ָ��󣬿���״̬��0 3����asic��ָ��󣬿���״̬��Ϊ1 4����asic��ָ��󣬿���״̬�ָ�Ϊ0�����Բɼ�*/
	
	ToolBox* m_toolBox;
};

#endif // ULTIMATE_EDITION

#endif // !ITA_WRAPPER_H
