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
#define COLLECT_Y16_NUMBER	20	//每个黑体采集15个Y16

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

	/* 自动测温 */
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
	// 测温分析对象 
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
	int m_fpaGear;			//保存当前档位，与测温库计算的档位比较，不同则切档。
	bool m_isGearManual;	//手动切档打开后，自动切档将失效。
	int m_gearManual;		//手动档位，与当前档位m_fpaGear比较，不等就切换。
	bool m_isChangeRange;	//是否切换测温范围，切换过程中更新数据包不能测温。
	bool m_isChangeGear;	//是否切换档位，切换过程中更新曲线和K不能测温。
	bool m_subAvgB;			//个别产品例如芯晟ASIC算法差异导致Y16减去本底均值之后才能测温。默认不用减。
	short m_avgB;
	float m_scale;
	ITA_ROTATE_TYPE m_rotate;
	bool m_isRotate;	//如果旋转，那么重新计算宽高。
	ITA_FLIP_TYPE m_flip;
	ITA_PIXEL_FORMAT m_pixelFormat;
	ITA_PALETTE_TYPE m_paletteIndex;
	int m_frameSize;
	short m_x16CenterValue;
	bool m_reCalcSize;	//当缩放倍数和像素格式变化时检查是否需要重新申请内存。
	unsigned char *m_imgDst;
	short *m_y16Data;
	short *m_y16TempMatrix; //m_subAvgB需要减本底的情况下申请内存，不影响用户使用的内存。
	int m_tempMatrixLen;//温度矩阵缓存长度。矩阵分辨率可能变化，那要重新分配内存。例如产品中超分辨率图片需要放大。
#ifdef SOURCE_COPY
	short *m_x16Data;	//源数据的拷贝，原则上不需要。
#endif // SOURCE_COPY
	/*对Y16数据做后端处理用不上K和B数据。*/
	short *m_bArray;			//X16数据转Y16需要B。记录一帧本底。4帧本底运算统一用short。
	bool m_loop;	//记录是否处于一帧的处理过程中，ISP接口返回后才能析构，否则释放shutterTool后ISP中使用时死机。避免用锁，MCU上锁不好实现。
	bool m_reverse;	//是否反向，ITA_X16和ITA_MCU_X16模式下有效。
	unsigned int m_lastSensorTime;	//上次获取温传的时间，每秒取一次。
	/*快门闭合过程中采本底*/
	bool m_isCollectBackground;	//采本底标记
	int m_backgroundCount;		//本底计数，共采集4帧。
	int m_skipCount;			//由于缓存若干帧原始数据，快门闭合后要跳过5帧再开始采本底，避免采到闭合前的图像。
	int m_isFPGAB;	//FPGA是否开始采集本底。0，就绪；1，FPGA开始采集本底；2，FPGA结束采集本底。SDK快门策略有效，需要通知FPGA采本底，待采集完成再弹开快门。
	bool m_fpgaBing; //true，正在采集本底。串口发送指令后，下一帧不一定马上被标记1正在采集本底。这个是异步的。需要有个标记表明采集过本底。
	unsigned int m_FPGAStartTime;	//记录通知FPGA开始采集本底的时间，用于超时判断。
	bool m_firstNUCing;	//初始化过程中第一次NUC完成后再回调图像，避免偶发花屏问题。
	int m_nucStatus;	//记录NUC状态：0，初始空状态；1，开始做NUC；2，正在做NUC；3，NUC已完成。
	/*nuc完成后可能存在2个问题：本底异常造成Y16异常；测温低0.5°左右。再打一次快门可以解决。*/
	int m_nucAnotherShutter; //重复快门纠错机制。默认0关闭。1，用户打开nuc重复快门机制；2，即将重复快门；3，正在快门。快门动作完成后切到1状态。
	int m_nucRepeatCorrection; //重复NUC纠错机制。默认0关闭。1，用户打开nuc重复纠错机制。
	int m_nucRepeatCount; //最多连续重复3次NUC。
	Debugger m_debugger;
	GuideLog *m_logger;
	bool m_debug;
	unsigned int g_currentTime;	//统一用一个变量，执行一个周期只用获取一次时间。
	unsigned int m_ispTime;
	//bool m_closeShutterPolicy;	//关闭策略后不再检测自动快门和NUC。算法内计算档位仍然要用，切档更新曲线。
	bool m_closeImage;	//Y16转Y8开关。某些产品只需要测温功能，图像可以利用芯片完成。
	/*自动校温功能。ZX03C校温：分别对着30°、33°、36°黑体采集一次Y16数据，每次采集时需关闭所快门策略，
	然后判断当前开始的采集时间和上一次快门时间是否相差10s，如果小于10s需要等待，直到相差10s，
	如果大于10s直接打快门采集，采集时每隔1s采集一个Y16，需采集15次，然后求15次的平均值。
	ZX03B校温：分别对着30°、33°、36°黑体采集一次Y16数据，每次采集需循环五次：先打一次普通快门，
	然后取第二、第三秒的Y16数据，获取的10个Y16数据求平均。*/
	int m_isAutoCollect;	//记录采集状态：0，初始空状态；1，准备采集；2，正在采集；3，一次标定黑体采集完成。
	unsigned int m_shutterCollectInterval;//上次快门之后间隔多少秒开始打快门采集Y16。避免频繁快门导致部分模组掉温。
	unsigned int m_collectInterval;//相邻两次采集间隔时间
	unsigned int m_collectNumberOfTimes;	//一次快门之后需要采集的次数
	//int m_shutterNumberOfTimes;	//每个黑体采集完成需要的快门次数，另外一种采集方案暂时不实现。
	//int m_shutterTimes;		//一次黑体采集周期内快门计数，<= m_shutterNumberOfTimes。
	unsigned int m_collectTimes;//一次黑体采集周期内采集Y16计数，<= m_collectNumberOfTimes。
	int m_collectCount;		//第几个黑体采集，一般3次，30 33 36.
	float m_blackTempArray[BLACK_BODY_NUMBER];	//标定黑体温度
	ITACollectFinish m_collectCBArray[BLACK_BODY_NUMBER];
	void *m_cbParamArray[BLACK_BODY_NUMBER];
	short m_blackY16Array[COLLECT_Y16_NUMBER];//每个黑体采集15个Y16值，数组要>=m_collectNumberOfTimes。
	short m_y16AvgArray[BLACK_BODY_NUMBER];	//每个标定黑体的Y16平均值
	ITA_COLLECT_TYPE m_collectType;
	float m_collectParam;
	/*校坏点*/
	int badPointTotalNumber;
	ITA_POINT* m_badPointsArray;
	int m_badPointsCount;
	ITA_POINT m_badPoint;
	int m_shutterFrameNumber; //单步控制快门从闭合到打开整个过程的时间
	int m_bDelayFrame;	//采集本底延时
	int m_shutterCloseDelay; //闭合快门的延时，MCU时间较长
	int m_shutterOpenDelay; //打开快门的延时，MCU时间较长

	unsigned int m_fps;	//计算的实际帧率
	unsigned int m_lastIspTime;
	unsigned int m_userIspTime; //用户设置的平均帧处理时间。因为实时计算的帧时间很不稳定，影响实际帧率。
	unsigned int m_maxFrameRate;
	/*另外一种方案，连续处理用户指定帧率后连续丢帧以满足帧率*/
	unsigned int m_userFps; //用户设置的帧率
	//unsigned int m_frameCount; //每秒连续处理的帧数

	unsigned int m_nucStartTime; //CB360产品记录nuc开始时间，2秒后快门状态置0。
	AutoFocus *m_af;
	bool m_isAF; //用户是否开始使用AF，如果是就逐帧传Y16。
	bool m_isCalcCentral; //计算中心温

	/*探测器参数*/
	unsigned char m_validMode;//0：探测器VSYNC和HSYNC高有效；1：探测器VSYNC和HSYNC低有效。默认为0。
	unsigned char m_dataMode;//0：先发16位数据的高8bits；1：先发16位数据的低8bits。默认为1。
	unsigned char m_frameFrequency;//探测器帧频，单位fps。默认25。

	int m_builtInFlash;  //1：120 90°模组P25Q内置Flash。
	//激光对焦 ZC12A
	CInfraredAF *m_laserAF;

	stDGGTPara guoGaiPara;//去锅盖参数

	CollectK* m_collectK;//采集k模块
	//制作K数据
	unsigned short* m_gain_mat;//K矩阵数据;
	StartReview* m_startReview;//复核温度
	int m_coldHotDelay;//冷机热机时间判定。
	unsigned char m_coldHotStatus;//冷热机状态: 1:冷机 0：热机

	unsigned char m_asicStatus;/*1:给asic发指令前，快门状态为0 2:给asic发指令后，快门状态是0 3：给asic发指令后，快门状态变为1 4：给asic发指令后，快门状态恢复为0，可以采集*/
	
	ToolBox* m_toolBox;
};

#endif // ULTIMATE_EDITION

#endif // !ITA_WRAPPER_H
