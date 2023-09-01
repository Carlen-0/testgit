#ifndef MEASURE_TEMP_CORE_H
#define MEASURE_TEMP_CORE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include "../Module/GuideLog.h"
#include "../Base/Config.h"

#include "../ITA.h"

#define OFFSET 1024
#define Y16_RANGE 16383
#define NSEPERATENUM 50
#define G_NLEN 15    //焦温平滑缓存长度

#ifdef EMISS_CURVES_2100
#define nEmissCurveLen 40960	//扩展至2100度
#else
#define nEmissCurveLen 16384	//800°
#endif // EMISS_CURVES_2100

//发射率修正表
extern short nEmissCurve[nEmissCurveLen];

//测温相关参数结构体
typedef struct Mt_Params
{
	/* 1. 初始化的时候赋值 */
	float         fCoefA1;//距离补偿系数，从参数行里读取
	float         fCoefA2;
	float         fCoefA3;
	float         fCoefB1;
	float         fCoefB2;
	float         fCoefB3;
	float         fCoefC1;
	float         fCoefC2;  
	float         fCoefC3;


	unsigned char          bEnvironmentCorrection;//环温修正开关
	unsigned char          bLensCorrection;//镜筒温漂校正开关
	unsigned char          bShutterCorrection;//快门温漂校正开关
	unsigned char          bUserTempCorrection;//二次校温开关
	unsigned char          bDistanceCompensate;//距离补偿开关
	unsigned char          bEmissCorrection;//发射率校正开关
	unsigned char          bTransCorrection;//透过率校正开关
	unsigned char          bAtmosphereCorrection;//大气透过率校正开关
	unsigned char          bHumidityCorrection;//湿度校正开关
	unsigned char		   ucGearMark;	//0：常温档  1：高温档
	unsigned char          bLogPrint;//bLogPrint开关

	unsigned char	ucFpaTempNum;//焦温点数
	unsigned char	ucDistanceNum;//距离个数
	float			gFpaTempArray[10];//标定焦温数组
	float			gDistanceArray[4];//标定距离数组

	float			fMinTemp;//最小可测量温度，人体测温为0
	float			fMaxTemp;//最大可测量温度，人体测温为60

	int	usCurveTempNumber;	//曲线长度，人体测温为600  工业低温2100   工业高温6200

//	float			fInitHighLensCorrK;//初始化高温档镜筒温漂修正系数，默认为-200
	//float			fInitLowLensCorrK;//初始化常温档镜筒温漂修正系数，默认为-850                  //删除

	/* 2. 刷新帧的时候，需要更新的值 */
	float         fOrinalShutterTemp;//开机快门温度
	float         fLastShutterTemp;//上一次打快门时的快门温度                      //删除
	float         fCurrentShutterTemp;//当前打快门时的快门温度
	float         fRealTimeShutterTemp;//实时快门温度
	float         fRealTimeLensTemp;//实时镜筒温度
	float         fCurrentLensTemp;//当前打快门时的镜筒温度
	//float       fLastLensTemp;//上一次打快门时的镜筒温度                            //删除
	float         fRealTimeFpaTemp;//实时焦平面温度
	float         fEnvironmentTemp;//环境温度
//	short         sLastShutterValue;//上一次打快门时x16                             //删除
	short         sCurrentShutterValue;//当前打快门时x16
	int           iStartingTime;//开机时间
	int           n_fpaGear;	//档位
	int           iShutterValue;//上一次打快门时x16                             //删除

	unsigned char bOrdinaryShutter;//普通快门标志位
	unsigned char bNucShutterFlag;//NUC快门标志位

	/* 3. 用户菜单可设置的值(刷新帧的时候，也需要更新的值) */
	float         fNearKf;	//近距离kf系数
	float         fNearB;	//近距离偏置系数
	float         fFarKf;	//远距离kf系数
	float         fFarB;	//远距离偏置系数

	float         fNearKf2;	//近距离kf系数
	float         fNearB2;	//近距离偏置系数
	float         fFarKf2;	//远距离kf系数
	float         fFarB2;	//远距离偏置系数

	float         fHighShutterCorrCoff;		//高温档快门校正系数(工业专用)
	float         fLowShutterCorrCoff;		//常温档快门校正系数(工业专用)
	float         fShutterCorrCoff;		    //其他测温档位快门校正系数(工业专用)
//	float         fHighShutterCorrCoff1;	//23℃环温高温挡快门校正系数(工业专用)
	//float       fHighShutterCorrCoff2;	//45℃环温高温挡快门校正系数(工业专用)
//	float         fLowShutterCorrCoff1;		//23℃环温常温档快门校正系数(工业专用)            //删除
//	float         fLowShutterCorrCoff2;		//45℃环温常温档快门校正系数(工业专用)            //删除

	float         fHighLensCorrK;	//高温档镜筒温漂修正系数
	float         fLowLensCorrK;	//常温档镜筒温漂修正系数
	float         fLensCorrK;	//其他档镜筒温漂修正系数

	float         fDistance;	//目标距离
	float		  fEmiss;		//发射率(工业专用)
	float		  fTransmit;	//透过率(工业专用)
	float         fHumidity;	//湿度(0.01-1.0,默认0.6)
	float		  fReflectTemp;	//反射温度(工业专用)
	float         fWindowTemperature;								//窗口温度（默认23）
	float         fAtmosphereTemperature;							//大气温度（默认23）
	float         fAtmosphereTransmittance;							//大气透过率(范围0~100，默认100)
	float         fCorrectDistance;                      //校温距离
	int           mtType;//测温类型，0:人体测温   1：工业测温常温段    2：工业测温高温段
	int           m_fieldType;//非免校准镜头类型
	int			  m_cflType;//免校准镜头类型
	int           m_focusType;
	int           mtDisType; //距离修正类型   0:6参数； 1:9参数
	
}MtParams;

//自动校温相关参数结构体
typedef struct Auto_CorrectTempPara
{
	int nBlackBodyNumber;			//输入黑体数量	
	float TBlackBody[10];		//输入各个黑体的温度点 整数不扩大
	short Y16[10];				//输入各个黑体的Y16
	float ErrorNoCorrect[10];	//返回校温前误差
	float ErrorCorrect[10];		//返回校温后误差
}AutoCorrectTempPara;

struct TEMP_Y16_CURVE_FORMULA;

// 测温类
class CMeasureTempCore
{
public:
	CMeasureTempCore(MtParams *mtParams, int w, int h, short* allCurvesArray, int curvesLen);
	void FinalRelease();
	~CMeasureTempCore();

	void reload(MtParams *mtParams, short* allCurvesArray, int curvesLen);
	/*测温参数m_stMtParams在ITAWrapper中实时更新，切换范围时通知CMeasureTempCore。*/
	int ChangeRange();
	/*修改镜筒温漂修正系数或者快门校正系数这2个参数时要刷新，因为算法不是读m_stMtParams中的参数*/
	int RefreshCorrK();
	// Y16单点测温
	int GetTempByY16(float &fTemp, short nObjY16, float *kf = NULL, float *b = NULL);
    // 分析对象测温
    int GetAnalysisTempByY16(ITAAnalysisY16Info *y16Array, ITAAnalysisTemp *tempArray, int len);
	// 坐标点测温
	int GetTempByCoord(float &fTemp, short *pSrcY16, unsigned short x, unsigned short y);

	// Y16输出温度矩阵（快速算法）
	int GetTempMatrixByY16(float *pObjTempMatrix, short* pSrcY16, int y16W, int y16H, int x, int y, int w, int h, float fDistance);

	// 温度映射
	short SurfaceTempToBody(float &fBodyTemp, float fSurfaceTemp, float fEnvirTemp);

	// 自动校温接口，内部校温使用
	bool AutoCorrectTempNear(float &fNearKf, float &fNearB, AutoCorrectTempPara *autocorTpara);

	bool AutoCorrectTempNear_ByUser(float &fNearKf2, float &fNearB2, AutoCorrectTempPara *autocorTpara);

	bool AutoCorrectTempFar(float &fFarKf, float &fFarB, AutoCorrectTempPara *autocorTpara);

	bool AutoCorrectTempFar_ByUser(float &fFarKf2, float &fFarB2, AutoCorrectTempPara *autocorTpara);

	short CalY16ByTemp(short &y16, float fTemperature);

	//温度矩阵反算Y16矩阵
	int CalY16MatrixByTempMatrix(float *pTempMatrix, short *pDst, int nImageHeight, int nImageWidth);

	//获取环温
	float GetEnvirTemp(float &fEnvirTemp);
	
	void calcMatrixFromMap(float *psTempMatrix, float *psTempMap, short *psValue, int nLen);

	int calcMatrixFromCurveOld(float *psTempMatrix, short* pSrcY16, int y16W, int y16H, int x, int y, int w, int h, float fDistance);

	void calcMatrixFromCurve(float *psTempMatrix, float *psTempMap, short *psValue, int nLen);

	// 获取焦温档位。每秒更新温传后调用本接口计算档位。
	int GetFpaGear();

	void setLogger(GuideLog *logger);

	void ResetFocusTempState();

	float smoothFocusTemp(float jpmTemp);

	int autoChangeRange(short *pSrc, int width, int height, int range, float areaTh1, float areaTh2, int low2high, int high2low);

	int autoMultiChangeRange(short* pSrc, int width, int height, int range,
		float areaTh1, float areaTh2, int low2mid, int mid2low, int mid2high, int high2mid);

	void refreshGear(int gear);

protected:
	// 快门温漂修正
	short ShutterTempDriftCorr(float &fShutterCorrIncre);

	// 镜筒温漂修正
	short LensTempDriftCorr(float &fLensCorrIncre);

	//透过率修正
	short TransmitCorr(short nObjY16, int fTransmit);

	//发射率修正
	float EmissCorrection(short nTNoEmiss, int nEmiss, int nY16Reflect);
	int GetY16FromT(int ReflectT);

	//距离修正
	float DistCorr(float &fTemp, float objTemp, float distance, float EnvTemp , int Param);

	// 湿度修正
	// short HumidityCorr(short fObjY16, float fHumidity);

	// 环境温度修正
	// float EnvirTempCorr(float fObjTemp, float fEnvirTemp);

	// 单条曲线测温
	float GetSingleCurveTemperature(short nObjY16, short *pCurve, int nSignleCurveLen, int distanceMark, float *kf, float *b);

	//由单点温度反算Y16
	short GetY16ByTemp(float fTemperature, short *pCurve, int distanceMark);

	//距离修正反算
	float DistanceReverseCorrection(float iTemperature , int param);

	//发射率修正反算
	float EmissionReverseCorrection(float temp , float fEmiss);

	short DeEmissCor(short nTEmiss, short nY16Reflect, int nEmiss);

	void calcTempMap(float *psTempMap, short sMax, short sMin);

	void calcTempCurveOld(float *psTempMap, short sMax, short sMin);

	void calcTempCurve(float *psTempMap, short sMax, short sMin);

    void curveKfWithB(int distanceMark, float *fKf, float *fB);

    float curveY16Temp(const TEMP_Y16_CURVE_FORMULA *formula);
private:
	int nImgWidth ;
	int nImgHeight ;
	int nImgLen ;
	float m_fLensCorrK ;
	float m_fShutterCorrK ;
	int m_ucFpaGear;//当前所处焦温档位
	//short *pSrcY16;
	//float *pfTemp;
	//short *pusAllCurve;
	short *pusCurve;
	/*mtParams在MTWrapper构造函数中完成初始化。在加载数据包loadData时载入数据。这些都在CMeasureTempCore实例化之前完成。*/
	MtParams *m_stMtParams;	//引用外部实例，共享。在ITAWrapper中实时更新后，不用再拷贝一份。
	//日志添加
	float fLensCorrIncrement;
	float fShutterCorrIncrement;
	GuideLog *m_logger;
	float *m_pTempMap;
	float g_fFocusTemp[G_NLEN];
	int g_nFocusCnt;
	float smoothFocusT;
	short *m_sortY16;
	int m_sortLen;
};
#endif // !MEASURE_TEMP_CORE_H
