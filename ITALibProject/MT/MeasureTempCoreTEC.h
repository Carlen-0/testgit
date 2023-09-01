/*
* TempMeasureCore.h
*
* 2020年9月10日
*
*修改版本1：增加窗口透过率修正以及大气透过率修正功能，增加相对应的结构体参数以及开关
*修改时间：2020年10月16日
*
*修改版本2:
*将B1修正调整到距离修正之前（在具体的校温流程中，会先调整KF,B1来验证温漂，然后带着这两个参数采集距离参数，因此将B1放在正算的最后一步会带来一些误差），同理将B1反算调整到对应位置。
*
*修改版本3：
*a.导入新发射率修正代码，具体包括：
*1.添加发射率曲线表
*2.添加反射温度对应的Y16---nY16Reflect
*3.增加反射温度是否变化标志---IsReflectTChange
*修改时间：2020年10月21日
*
*修改版本4：
*a.导入快速计算温度矩阵接口
*b.调整GetTempByCoord接口入参
*c.添加宏定义gdmin，gdmax
*d.添加自动校温接口
*
*
*修改版本5：
a.统一所有参数，算法库内统一进行缩放（避免浮点运算），对外接口中所有参数均为实际值
b.增加Y16偏移修正 
*
*修改版本6：
*a.加入新环温修正算法（未验证）、新环温修正反算算法、新环温修正对应的参数，以及调用环温修正的位置
*b.统一所有变量、函数的命名方式
*c.增加了Y16Offset功能，即直接调整Y16左右平移（一般不用，默认值为0，现有的功能如果用了Y16Offset会导致修正结果错误）；
*d.修改了两点校正部分逻辑；之前为了加速计算，设置了两点校正后温度差异和小于1°直接跳出循环不再遍历，但从客户使用的情况看可能出现找不到小于1°的参数，导致后续参数被刷新，出现计算错误。
*修改时间：2021年5月6日
*
*修改版本7：
*加入更新曲线接口，解决切换档位时无法切换曲线的问题（现在只在初始化构造函数的时候更新）;
*
*
*修改版本8：
*加入测温类型（人体or工业），加入距离参数类型（6or9），并对相应代码作出修改;
*
*
*修改版本9
*加入自动校温功能
*
*
*修改版本10
1. 去掉不用设置的测温参数，包括sY16Reflect、sY16Windows、disType距离修正类型。
2. 去掉RefreshMeasureParam
3. 去掉RefreshCurve，构造函数中传入曲线内存地址，切换范围或者档位时会更新曲线数据，地址不变，算法直接使用，不在拷贝一份。
4. 去掉GetTempByCoord。
5. 温度矩阵扩展为4个，包括2种快速计算方式和2种无损计算方式。
6. 人体测温增加体表转体内的功能。
7. 去除SDK编译警告信息

*/
#ifndef MEASURETEMP_CORE_TEC_H
#define MEASURETEMP_CORE_TEC_H

#include <string>
#include <math.h>
#include "MeasureTempCore.h"

//测温相关参数结构体
struct IMAGE_MEASURE_STRUCT
{
	short sY16Offset;										//Y16偏移量(默认0)
	//short sY16Reflect;									//反射温度对应的Y16
	//short sY16Windows;									//窗口温度对应的Y16

	int nKF;												//查曲线时Y16的缩放量(定点化100倍，默认值为100)
	int nB1;												//查曲线时Y16的偏移量(定点化100倍)

	int nDistance_a0;										//距离校正系数(定点化10000000000倍，默认0)
	int nDistance_a1;										//距离校正系数(定点化1000000000倍，默认0)
	int nDistance_a2;										//距离校正系数(定点化10000000倍，默认0)
	int nDistance_a3;										//距离校正系数(定点化100000倍，默认0)
	int nDistance_a4;										//距离校正系数(定点化100000倍，默认0)
	int nDistance_a5;										//距离校正系数(定点化10000倍，默认0)
	int nDistance_a6;										//距离校正系数(定点化1000倍，默认0)
	int nDistance_a7;										//距离校正系数(定点化100倍，默认0)
	int nDistance_a8;										//距离校正系数(定点化100倍，默认0)

	int nK1;												//快门温漂系数(定点化100倍，默认0)
	int nK2;												//镜筒温漂系数(定点化100倍，默认0)
	int nK3;												//环温修正系数(定点化10000倍，默认0)
	int nB2;												//环温修正偏移量(定点化10000倍，默认0)

	int nKFOffset;											//自动校温KF偏移量，置零后恢复出厂校温设置，默认0
	int nB1Offset;											//自动校温B1偏移量，置零后恢复出厂校温设置，默认0

	int nGear;												//测温档位, 0:低温档, 1:常温挡

	//int nShutterCurveIndex;									//快门温度反查曲线对应的Y16值
	int fHumidity;											//湿度(定点化100倍，默认60)
	int nAtmosphereTransmittance;							//大气透过率(定点化100倍，范围0~100，默认100)

	int mtType;												//测温类型，0:人体测温   1：工业测温
	//int disType;											//距离修正类型：0:六距离参数    1:九距离参数

	float fEmiss;											//发射率(0.01-1.0,默认1.0)
	float fDistance;										//测温距离 
	float fReflectT;										//反射温度（低温档默认3，常温档默认23）
	float fAmbient;											//环境温度（取开机快门温）
	float fAtmosphereTemperature;							//大气温度
	float fWindowTransmittance;								//窗口透过率(范围0~1，默认1)
	float fWindowTemperature;								//窗口温度（低温档默认3，常温档默认23）
	//float fCorrection1;									//快门温飘修正量
	//float fCorrection2;									//镜筒温飘修正量

	float fRealTimeShutterTemp;								//实时快门温度
	float fOrinalShutterTemp;								//开机快门温
	float fRealTimeLensTemp;								//实时镜筒温
	float fCurrentLensTemp;									//最近一次打快门时的镜筒温

	bool bHumidityCorrection;								//湿度修正开关，默认打开。
	bool bShutterCorrection;								//快门修正开关，默认打开。
	bool bLensCorrection;									//镜筒修正开关，默认打开。
	bool bEmissCorrection;									//发射率修正开关，默认打开。
	bool bDistanceCorrection;								//距离修正开关，默认打开。
	bool bAmbientCorrection;								//环温修正开关，默认打开。
	bool bB1Correction;										//B1修正开关，默认打开。
	bool bAtmosphereCorrection;								//大气透过率修正开关，默认打开。
	bool bWindowTransmittanceCorrection;					//窗口透过率开关，默认打开。
	//bool bIsReflectTChange;									//反射温度是否变化标志
	//bool bIsWindowsTChange;									//窗口温度是否变化标志
	short avgB;
	unsigned char bOrdinaryShutter;//普通快门标志位
	unsigned char bNucShutterFlag;//NUC快门标志位
	unsigned char bLogPrint;//bLogPrint开关
	//镜头类型
	int lensType;
};

// 测温类
class MeasureTempCoreTEC
{
public:
	MeasureTempCoreTEC(int w, int h, int curveLength, short *pusCurve,IMAGE_MEASURE_STRUCT *params);
	virtual ~MeasureTempCoreTEC();

	// 刷新Y16、参数行、更新测温参数
	//void RefreshMeasureParam(IMAGE_MEASURE_STRUCT &Params, short *SrcY16);

	//刷新测温曲线（切挡时调用）
	//void RefreshCurve(short *curve);
	void reload(int curveLength, short *pusCurve, IMAGE_MEASURE_STRUCT *params);

	// Y16单点测温
	void GetTempByY16(float &Temp, short y16value);

	// 坐标点测温
	//void GetTempByCoord(float &fTemp, unsigned short x, unsigned short y);

	// Y16输出温度矩阵（全图快速算法）
	int GetTempMatrixFast(short *pSrc, float *pTempMatrix, int nImageWidth, int nImageHeight);
	// Y16输出温度矩阵（ROI快速算法，支持修改距离和发射率）
	int GetTempMatrixFast(short *pSrc, float *pTempMatrix, int nImageWidth, int nImageHeight, int x, int y, int ROIWidth, int ROIHeight, float ROIDistance, float ROIEmiss);
	// Y16输出温度矩阵（全图无损算法）
	int GetTempMatrix(short *pSrc, float *pTempMatrix, int nImageWidth, int nImageHeight);
	// Y16输出温度矩阵（ROI无损算法，支持修改距离和发射率）
	int GetTempMatrix(short *pSrc, float *pTempMatrix, int nImageWidth, int nImageHeight, int x, int y, int ROIWidth, int ROIHeight, float ROIDistance, float ROIEmiss);

	//温度反算Y16
	void CalY16ByTemp(short &y16, float fTemp);
	//温度矩阵反算Y16矩阵
	int CalY16MatrixByTempMatrix(float *pTempMatrix, short *pDst, int nImageHeight, int nImageWidth);

	//二次校温
	void SecondCorrectByOnePoint(float bb_temperature, short bb_y16);
	void SecondCorrectByTwoPoint(float bb_temperature_1, float bb_temperature_2, short bb_y16_1, short bb_y16_2, int* coff);

	//自动校温
	void AutoCorrectTemp(float* fBlackBodyTemps, short* sY16s, int nLength);

	// 温度映射
	short SurfaceTempToBody(float &fBodyTemp, float fSurfaceTemp, float fEnvirTemp);
	void setLogger(GuideLog *logger);
	//自动切测温范围
	int autoChangeRange(short *pSrc, int width, int height, int range, float areaTh1, float areaTh2, int low2high, int high2low);
	int autoMultiChangeRange(short* pSrc, int width, int height, int range,
		float areaTh1, float areaTh2, int low2mid, int mid2low, int mid2high, int high2mid);
protected:

	//湿度修正
	short HumidityCorrection(short fObjY16, int fHumidity);

	//Y16 to temp（包含快门温漂修正和镜筒温漂修正）
	float StandardY16ToTempWithY16Correction(short y16value);
	float StandardY16ToTempWithY16CorrectionInSecondCorrect(short y16value, int KF, int nOffset);

	//发射率修正
	float EmissionCorrectionNew(float iTemperature);

	//距离修正
	float DistanceCorrectionWithSixPara(float iTemperature);
	float DistanceCorrectionWithNinePara(float iTemperature);

	// 快门温漂修正
	void ShutterTempDriftCorr();

	// 镜筒温漂修正
	void LensTempDriftCorr();

	//环温修正
	float EnvironmentCorrection(float temp);

	//湿度修正反算
	short HumidityReverseCorrection(short y16, int fHumidity);
	

	//发射率修正反算
	float EmissionReverseCorrectionNew(float iTemperature);

	//距离修正反算
	float DistanceReverseCorrectionWithNinePara(float iTemperature);
	float DistanceReverseCorrectionWithSixPara(float iTemperature);
	
	//环温修正反算
	float EnvironmentReverseCorrection(float temp);

	//temp to Y16（包含快门温漂修正和镜筒温漂修正）
	short StandardTempToY16(float iTemperature, bool fromSmall2Big);
	
	//大气透过率修正
	float AtmosphereCorrection(float iTemperature);

	//大气透过率反算
	float AtmosphereReverseCorrection(float iTemperature);

	//窗口透过率修正
	float WindowTransmittanceCorrection(float iTemperature);

	//窗口透过率修正反算
	float WindowTransmittanceReverseCorrection(float iTemperature);

	//获取反射温度对应的Y16
	short GetY16FromT(int ReflectT);

	short DeEmissCor(short nTEmiss, short nY16Reflect, int nEmiss);

	short EmissCor(short nTNoEmiss, short nY16Reflect, int nEmiss);

private:
	//int	ImageWidth;											//图像宽度
	//int	ImageHidth;											//图像高度
	int curveDataLength;									//测温曲线长度
	short *CurveData;										//温度曲线
	IMAGE_MEASURE_STRUCT *TMparams;
	int nShutterCurveIndex;								//快门温度反查曲线对应的Y16值
	float fCorrection1;									//快门温飘修正量
	float fCorrection2;									//镜筒温飘修正量
	float *m_fTempMatrix;	//不能频繁申请内存，接口压力测试死机。申请内存时Android上返回不为空的指针，但实际不可用。
	int m_tempLen;
	GuideLog *m_logger;
	short *m_sortY16;
	int m_sortLen;
};
#endif // !MEASURETEMP_CORE_TEC_H


//快排
void Quick_Sort(float *arr, int begin, int end);
void Quick_Sort(short *arr, int begin, int end);
