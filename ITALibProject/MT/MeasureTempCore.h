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
#define G_NLEN 15    //����ƽ�����泤��

#ifdef EMISS_CURVES_2100
#define nEmissCurveLen 40960	//��չ��2100��
#else
#define nEmissCurveLen 16384	//800��
#endif // EMISS_CURVES_2100

//������������
extern short nEmissCurve[nEmissCurveLen];

//������ز����ṹ��
typedef struct Mt_Params
{
	/* 1. ��ʼ����ʱ��ֵ */
	float         fCoefA1;//���벹��ϵ�����Ӳ��������ȡ
	float         fCoefA2;
	float         fCoefA3;
	float         fCoefB1;
	float         fCoefB2;
	float         fCoefB3;
	float         fCoefC1;
	float         fCoefC2;  
	float         fCoefC3;


	unsigned char          bEnvironmentCorrection;//������������
	unsigned char          bLensCorrection;//��Ͳ��ƯУ������
	unsigned char          bShutterCorrection;//������ƯУ������
	unsigned char          bUserTempCorrection;//����У�¿���
	unsigned char          bDistanceCompensate;//���벹������
	unsigned char          bEmissCorrection;//������У������
	unsigned char          bTransCorrection;//͸����У������
	unsigned char          bAtmosphereCorrection;//����͸����У������
	unsigned char          bHumidityCorrection;//ʪ��У������
	unsigned char		   ucGearMark;	//0�����µ�  1�����µ�
	unsigned char          bLogPrint;//bLogPrint����

	unsigned char	ucFpaTempNum;//���µ���
	unsigned char	ucDistanceNum;//�������
	float			gFpaTempArray[10];//�궨��������
	float			gDistanceArray[4];//�궨��������

	float			fMinTemp;//��С�ɲ����¶ȣ��������Ϊ0
	float			fMaxTemp;//���ɲ����¶ȣ��������Ϊ60

	int	usCurveTempNumber;	//���߳��ȣ��������Ϊ600  ��ҵ����2100   ��ҵ����6200

//	float			fInitHighLensCorrK;//��ʼ�����µ���Ͳ��Ư����ϵ����Ĭ��Ϊ-200
	//float			fInitLowLensCorrK;//��ʼ�����µ���Ͳ��Ư����ϵ����Ĭ��Ϊ-850                  //ɾ��

	/* 2. ˢ��֡��ʱ����Ҫ���µ�ֵ */
	float         fOrinalShutterTemp;//���������¶�
	float         fLastShutterTemp;//��һ�δ����ʱ�Ŀ����¶�                      //ɾ��
	float         fCurrentShutterTemp;//��ǰ�����ʱ�Ŀ����¶�
	float         fRealTimeShutterTemp;//ʵʱ�����¶�
	float         fRealTimeLensTemp;//ʵʱ��Ͳ�¶�
	float         fCurrentLensTemp;//��ǰ�����ʱ�ľ�Ͳ�¶�
	//float       fLastLensTemp;//��һ�δ����ʱ�ľ�Ͳ�¶�                            //ɾ��
	float         fRealTimeFpaTemp;//ʵʱ��ƽ���¶�
	float         fEnvironmentTemp;//�����¶�
//	short         sLastShutterValue;//��һ�δ����ʱx16                             //ɾ��
	short         sCurrentShutterValue;//��ǰ�����ʱx16
	int           iStartingTime;//����ʱ��
	int           n_fpaGear;	//��λ
	int           iShutterValue;//��һ�δ����ʱx16                             //ɾ��

	unsigned char bOrdinaryShutter;//��ͨ���ű�־λ
	unsigned char bNucShutterFlag;//NUC���ű�־λ

	/* 3. �û��˵������õ�ֵ(ˢ��֡��ʱ��Ҳ��Ҫ���µ�ֵ) */
	float         fNearKf;	//������kfϵ��
	float         fNearB;	//������ƫ��ϵ��
	float         fFarKf;	//Զ����kfϵ��
	float         fFarB;	//Զ����ƫ��ϵ��

	float         fNearKf2;	//������kfϵ��
	float         fNearB2;	//������ƫ��ϵ��
	float         fFarKf2;	//Զ����kfϵ��
	float         fFarB2;	//Զ����ƫ��ϵ��

	float         fHighShutterCorrCoff;		//���µ�����У��ϵ��(��ҵר��)
	float         fLowShutterCorrCoff;		//���µ�����У��ϵ��(��ҵר��)
	float         fShutterCorrCoff;		    //�������µ�λ����У��ϵ��(��ҵר��)
//	float         fHighShutterCorrCoff1;	//23�滷�¸��µ�����У��ϵ��(��ҵר��)
	//float       fHighShutterCorrCoff2;	//45�滷�¸��µ�����У��ϵ��(��ҵר��)
//	float         fLowShutterCorrCoff1;		//23�滷�³��µ�����У��ϵ��(��ҵר��)            //ɾ��
//	float         fLowShutterCorrCoff2;		//45�滷�³��µ�����У��ϵ��(��ҵר��)            //ɾ��

	float         fHighLensCorrK;	//���µ���Ͳ��Ư����ϵ��
	float         fLowLensCorrK;	//���µ���Ͳ��Ư����ϵ��
	float         fLensCorrK;	//��������Ͳ��Ư����ϵ��

	float         fDistance;	//Ŀ�����
	float		  fEmiss;		//������(��ҵר��)
	float		  fTransmit;	//͸����(��ҵר��)
	float         fHumidity;	//ʪ��(0.01-1.0,Ĭ��0.6)
	float		  fReflectTemp;	//�����¶�(��ҵר��)
	float         fWindowTemperature;								//�����¶ȣ�Ĭ��23��
	float         fAtmosphereTemperature;							//�����¶ȣ�Ĭ��23��
	float         fAtmosphereTransmittance;							//����͸����(��Χ0~100��Ĭ��100)
	float         fCorrectDistance;                      //У�¾���
	int           mtType;//�������ͣ�0:�������   1����ҵ���³��¶�    2����ҵ���¸��¶�
	int           m_fieldType;//����У׼��ͷ����
	int			  m_cflType;//��У׼��ͷ����
	int           m_focusType;
	int           mtDisType; //������������   0:6������ 1:9����
	
}MtParams;

//�Զ�У����ز����ṹ��
typedef struct Auto_CorrectTempPara
{
	int nBlackBodyNumber;			//�����������	
	float TBlackBody[10];		//�������������¶ȵ� ����������
	short Y16[10];				//������������Y16
	float ErrorNoCorrect[10];	//����У��ǰ���
	float ErrorCorrect[10];		//����У�º����
}AutoCorrectTempPara;

struct TEMP_Y16_CURVE_FORMULA;

// ������
class CMeasureTempCore
{
public:
	CMeasureTempCore(MtParams *mtParams, int w, int h, short* allCurvesArray, int curvesLen);
	void FinalRelease();
	~CMeasureTempCore();

	void reload(MtParams *mtParams, short* allCurvesArray, int curvesLen);
	/*���²���m_stMtParams��ITAWrapper��ʵʱ���£��л���Χʱ֪ͨCMeasureTempCore��*/
	int ChangeRange();
	/*�޸ľ�Ͳ��Ư����ϵ�����߿���У��ϵ����2������ʱҪˢ�£���Ϊ�㷨���Ƕ�m_stMtParams�еĲ���*/
	int RefreshCorrK();
	// Y16�������
	int GetTempByY16(float &fTemp, short nObjY16, float *kf = NULL, float *b = NULL);
    // �����������
    int GetAnalysisTempByY16(ITAAnalysisY16Info *y16Array, ITAAnalysisTemp *tempArray, int len);
	// ��������
	int GetTempByCoord(float &fTemp, short *pSrcY16, unsigned short x, unsigned short y);

	// Y16����¶Ⱦ��󣨿����㷨��
	int GetTempMatrixByY16(float *pObjTempMatrix, short* pSrcY16, int y16W, int y16H, int x, int y, int w, int h, float fDistance);

	// �¶�ӳ��
	short SurfaceTempToBody(float &fBodyTemp, float fSurfaceTemp, float fEnvirTemp);

	// �Զ�У�½ӿڣ��ڲ�У��ʹ��
	bool AutoCorrectTempNear(float &fNearKf, float &fNearB, AutoCorrectTempPara *autocorTpara);

	bool AutoCorrectTempNear_ByUser(float &fNearKf2, float &fNearB2, AutoCorrectTempPara *autocorTpara);

	bool AutoCorrectTempFar(float &fFarKf, float &fFarB, AutoCorrectTempPara *autocorTpara);

	bool AutoCorrectTempFar_ByUser(float &fFarKf2, float &fFarB2, AutoCorrectTempPara *autocorTpara);

	short CalY16ByTemp(short &y16, float fTemperature);

	//�¶Ⱦ�����Y16����
	int CalY16MatrixByTempMatrix(float *pTempMatrix, short *pDst, int nImageHeight, int nImageWidth);

	//��ȡ����
	float GetEnvirTemp(float &fEnvirTemp);
	
	void calcMatrixFromMap(float *psTempMatrix, float *psTempMap, short *psValue, int nLen);

	int calcMatrixFromCurveOld(float *psTempMatrix, short* pSrcY16, int y16W, int y16H, int x, int y, int w, int h, float fDistance);

	void calcMatrixFromCurve(float *psTempMatrix, float *psTempMap, short *psValue, int nLen);

	// ��ȡ���µ�λ��ÿ������´�����ñ��ӿڼ��㵵λ��
	int GetFpaGear();

	void setLogger(GuideLog *logger);

	void ResetFocusTempState();

	float smoothFocusTemp(float jpmTemp);

	int autoChangeRange(short *pSrc, int width, int height, int range, float areaTh1, float areaTh2, int low2high, int high2low);

	int autoMultiChangeRange(short* pSrc, int width, int height, int range,
		float areaTh1, float areaTh2, int low2mid, int mid2low, int mid2high, int high2mid);

	void refreshGear(int gear);

protected:
	// ������Ư����
	short ShutterTempDriftCorr(float &fShutterCorrIncre);

	// ��Ͳ��Ư����
	short LensTempDriftCorr(float &fLensCorrIncre);

	//͸��������
	short TransmitCorr(short nObjY16, int fTransmit);

	//����������
	float EmissCorrection(short nTNoEmiss, int nEmiss, int nY16Reflect);
	int GetY16FromT(int ReflectT);

	//��������
	float DistCorr(float &fTemp, float objTemp, float distance, float EnvTemp , int Param);

	// ʪ������
	// short HumidityCorr(short fObjY16, float fHumidity);

	// �����¶�����
	// float EnvirTempCorr(float fObjTemp, float fEnvirTemp);

	// �������߲���
	float GetSingleCurveTemperature(short nObjY16, short *pCurve, int nSignleCurveLen, int distanceMark, float *kf, float *b);

	//�ɵ����¶ȷ���Y16
	short GetY16ByTemp(float fTemperature, short *pCurve, int distanceMark);

	//������������
	float DistanceReverseCorrection(float iTemperature , int param);

	//��������������
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
	int m_ucFpaGear;//��ǰ�������µ�λ
	//short *pSrcY16;
	//float *pfTemp;
	//short *pusAllCurve;
	short *pusCurve;
	/*mtParams��MTWrapper���캯������ɳ�ʼ�����ڼ������ݰ�loadDataʱ�������ݡ���Щ����CMeasureTempCoreʵ����֮ǰ��ɡ�*/
	MtParams *m_stMtParams;	//�����ⲿʵ����������ITAWrapper��ʵʱ���º󣬲����ٿ���һ�ݡ�
	//��־���
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
