/*
* TempMeasureCore.h
*
* 2020��9��10��
*
*�޸İ汾1�����Ӵ���͸���������Լ�����͸�����������ܣ��������Ӧ�Ľṹ������Լ�����
*�޸�ʱ�䣺2020��10��16��
*
*�޸İ汾2:
*��B1������������������֮ǰ���ھ����У�������У����ȵ���KF,B1����֤��Ư��Ȼ����������������ɼ������������˽�B1������������һ�������һЩ����ͬ��B1�����������Ӧλ�á�
*
*�޸İ汾3��
*a.�����·������������룬���������
*1.��ӷ��������߱�
*2.��ӷ����¶ȶ�Ӧ��Y16---nY16Reflect
*3.���ӷ����¶��Ƿ�仯��־---IsReflectTChange
*�޸�ʱ�䣺2020��10��21��
*
*�޸İ汾4��
*a.������ټ����¶Ⱦ���ӿ�
*b.����GetTempByCoord�ӿ����
*c.��Ӻ궨��gdmin��gdmax
*d.����Զ�У�½ӿ�
*
*
*�޸İ汾5��
a.ͳһ���в������㷨����ͳһ�������ţ����⸡�����㣩������ӿ������в�����Ϊʵ��ֵ
b.����Y16ƫ������ 
*
*�޸İ汾6��
*a.�����»��������㷨��δ��֤�����»������������㷨���»���������Ӧ�Ĳ������Լ����û���������λ��
*b.ͳһ���б�����������������ʽ
*c.������Y16Offset���ܣ���ֱ�ӵ���Y16����ƽ�ƣ�һ�㲻�ã�Ĭ��ֵΪ0�����еĹ����������Y16Offset�ᵼ������������󣩣�
*d.�޸�������У�������߼���֮ǰΪ�˼��ټ��㣬����������У�����¶Ȳ����С��1��ֱ������ѭ�����ٱ��������ӿͻ�ʹ�õ���������ܳ����Ҳ���С��1��Ĳ��������º���������ˢ�£����ּ������
*�޸�ʱ�䣺2021��5��6��
*
*�޸İ汾7��
*����������߽ӿڣ�����л���λʱ�޷��л����ߵ����⣨����ֻ�ڳ�ʼ�����캯����ʱ����£�;
*
*
*�޸İ汾8��
*����������ͣ�����or��ҵ�����������������ͣ�6or9����������Ӧ���������޸�;
*
*
*�޸İ汾9
*�����Զ�У�¹���
*
*
*�޸İ汾10
1. ȥ���������õĲ��²���������sY16Reflect��sY16Windows��disType�����������͡�
2. ȥ��RefreshMeasureParam
3. ȥ��RefreshCurve�����캯���д��������ڴ��ַ���л���Χ���ߵ�λʱ������������ݣ���ַ���䣬�㷨ֱ��ʹ�ã����ڿ���һ�ݡ�
4. ȥ��GetTempByCoord��
5. �¶Ⱦ�����չΪ4��������2�ֿ��ټ��㷽ʽ��2��������㷽ʽ��
6. ��������������ת���ڵĹ��ܡ�
7. ȥ��SDK���뾯����Ϣ

*/
#ifndef MEASURETEMP_CORE_TEC_H
#define MEASURETEMP_CORE_TEC_H

#include <string>
#include <math.h>
#include "MeasureTempCore.h"

//������ز����ṹ��
struct IMAGE_MEASURE_STRUCT
{
	short sY16Offset;										//Y16ƫ����(Ĭ��0)
	//short sY16Reflect;									//�����¶ȶ�Ӧ��Y16
	//short sY16Windows;									//�����¶ȶ�Ӧ��Y16

	int nKF;												//������ʱY16��������(���㻯100����Ĭ��ֵΪ100)
	int nB1;												//������ʱY16��ƫ����(���㻯100��)

	int nDistance_a0;										//����У��ϵ��(���㻯10000000000����Ĭ��0)
	int nDistance_a1;										//����У��ϵ��(���㻯1000000000����Ĭ��0)
	int nDistance_a2;										//����У��ϵ��(���㻯10000000����Ĭ��0)
	int nDistance_a3;										//����У��ϵ��(���㻯100000����Ĭ��0)
	int nDistance_a4;										//����У��ϵ��(���㻯100000����Ĭ��0)
	int nDistance_a5;										//����У��ϵ��(���㻯10000����Ĭ��0)
	int nDistance_a6;										//����У��ϵ��(���㻯1000����Ĭ��0)
	int nDistance_a7;										//����У��ϵ��(���㻯100����Ĭ��0)
	int nDistance_a8;										//����У��ϵ��(���㻯100����Ĭ��0)

	int nK1;												//������Ưϵ��(���㻯100����Ĭ��0)
	int nK2;												//��Ͳ��Ưϵ��(���㻯100����Ĭ��0)
	int nK3;												//��������ϵ��(���㻯10000����Ĭ��0)
	int nB2;												//��������ƫ����(���㻯10000����Ĭ��0)

	int nKFOffset;											//�Զ�У��KFƫ�����������ָ�����У�����ã�Ĭ��0
	int nB1Offset;											//�Զ�У��B1ƫ�����������ָ�����У�����ã�Ĭ��0

	int nGear;												//���µ�λ, 0:���µ�, 1:���µ�

	//int nShutterCurveIndex;									//�����¶ȷ������߶�Ӧ��Y16ֵ
	int fHumidity;											//ʪ��(���㻯100����Ĭ��60)
	int nAtmosphereTransmittance;							//����͸����(���㻯100������Χ0~100��Ĭ��100)

	int mtType;												//�������ͣ�0:�������   1����ҵ����
	//int disType;											//�����������ͣ�0:���������    1:�ž������

	float fEmiss;											//������(0.01-1.0,Ĭ��1.0)
	float fDistance;										//���¾��� 
	float fReflectT;										//�����¶ȣ����µ�Ĭ��3�����µ�Ĭ��23��
	float fAmbient;											//�����¶ȣ�ȡ���������£�
	float fAtmosphereTemperature;							//�����¶�
	float fWindowTransmittance;								//����͸����(��Χ0~1��Ĭ��1)
	float fWindowTemperature;								//�����¶ȣ����µ�Ĭ��3�����µ�Ĭ��23��
	//float fCorrection1;									//������Ʈ������
	//float fCorrection2;									//��Ͳ��Ʈ������

	float fRealTimeShutterTemp;								//ʵʱ�����¶�
	float fOrinalShutterTemp;								//����������
	float fRealTimeLensTemp;								//ʵʱ��Ͳ��
	float fCurrentLensTemp;									//���һ�δ����ʱ�ľ�Ͳ��

	bool bHumidityCorrection;								//ʪ���������أ�Ĭ�ϴ򿪡�
	bool bShutterCorrection;								//�����������أ�Ĭ�ϴ򿪡�
	bool bLensCorrection;									//��Ͳ�������أ�Ĭ�ϴ򿪡�
	bool bEmissCorrection;									//�������������أ�Ĭ�ϴ򿪡�
	bool bDistanceCorrection;								//�����������أ�Ĭ�ϴ򿪡�
	bool bAmbientCorrection;								//�����������أ�Ĭ�ϴ򿪡�
	bool bB1Correction;										//B1�������أ�Ĭ�ϴ򿪡�
	bool bAtmosphereCorrection;								//����͸�����������أ�Ĭ�ϴ򿪡�
	bool bWindowTransmittanceCorrection;					//����͸���ʿ��أ�Ĭ�ϴ򿪡�
	//bool bIsReflectTChange;									//�����¶��Ƿ�仯��־
	//bool bIsWindowsTChange;									//�����¶��Ƿ�仯��־
	short avgB;
	unsigned char bOrdinaryShutter;//��ͨ���ű�־λ
	unsigned char bNucShutterFlag;//NUC���ű�־λ
	unsigned char bLogPrint;//bLogPrint����
	//��ͷ����
	int lensType;
};

// ������
class MeasureTempCoreTEC
{
public:
	MeasureTempCoreTEC(int w, int h, int curveLength, short *pusCurve,IMAGE_MEASURE_STRUCT *params);
	virtual ~MeasureTempCoreTEC();

	// ˢ��Y16�������С����²��²���
	//void RefreshMeasureParam(IMAGE_MEASURE_STRUCT &Params, short *SrcY16);

	//ˢ�²������ߣ��е�ʱ���ã�
	//void RefreshCurve(short *curve);
	void reload(int curveLength, short *pusCurve, IMAGE_MEASURE_STRUCT *params);

	// Y16�������
	void GetTempByY16(float &Temp, short y16value);

	// ��������
	//void GetTempByCoord(float &fTemp, unsigned short x, unsigned short y);

	// Y16����¶Ⱦ���ȫͼ�����㷨��
	int GetTempMatrixFast(short *pSrc, float *pTempMatrix, int nImageWidth, int nImageHeight);
	// Y16����¶Ⱦ���ROI�����㷨��֧���޸ľ���ͷ����ʣ�
	int GetTempMatrixFast(short *pSrc, float *pTempMatrix, int nImageWidth, int nImageHeight, int x, int y, int ROIWidth, int ROIHeight, float ROIDistance, float ROIEmiss);
	// Y16����¶Ⱦ���ȫͼ�����㷨��
	int GetTempMatrix(short *pSrc, float *pTempMatrix, int nImageWidth, int nImageHeight);
	// Y16����¶Ⱦ���ROI�����㷨��֧���޸ľ���ͷ����ʣ�
	int GetTempMatrix(short *pSrc, float *pTempMatrix, int nImageWidth, int nImageHeight, int x, int y, int ROIWidth, int ROIHeight, float ROIDistance, float ROIEmiss);

	//�¶ȷ���Y16
	void CalY16ByTemp(short &y16, float fTemp);
	//�¶Ⱦ�����Y16����
	int CalY16MatrixByTempMatrix(float *pTempMatrix, short *pDst, int nImageHeight, int nImageWidth);

	//����У��
	void SecondCorrectByOnePoint(float bb_temperature, short bb_y16);
	void SecondCorrectByTwoPoint(float bb_temperature_1, float bb_temperature_2, short bb_y16_1, short bb_y16_2, int* coff);

	//�Զ�У��
	void AutoCorrectTemp(float* fBlackBodyTemps, short* sY16s, int nLength);

	// �¶�ӳ��
	short SurfaceTempToBody(float &fBodyTemp, float fSurfaceTemp, float fEnvirTemp);
	void setLogger(GuideLog *logger);
	//�Զ��в��·�Χ
	int autoChangeRange(short *pSrc, int width, int height, int range, float areaTh1, float areaTh2, int low2high, int high2low);
	int autoMultiChangeRange(short* pSrc, int width, int height, int range,
		float areaTh1, float areaTh2, int low2mid, int mid2low, int mid2high, int high2mid);
protected:

	//ʪ������
	short HumidityCorrection(short fObjY16, int fHumidity);

	//Y16 to temp������������Ư�����;�Ͳ��Ư������
	float StandardY16ToTempWithY16Correction(short y16value);
	float StandardY16ToTempWithY16CorrectionInSecondCorrect(short y16value, int KF, int nOffset);

	//����������
	float EmissionCorrectionNew(float iTemperature);

	//��������
	float DistanceCorrectionWithSixPara(float iTemperature);
	float DistanceCorrectionWithNinePara(float iTemperature);

	// ������Ư����
	void ShutterTempDriftCorr();

	// ��Ͳ��Ư����
	void LensTempDriftCorr();

	//��������
	float EnvironmentCorrection(float temp);

	//ʪ����������
	short HumidityReverseCorrection(short y16, int fHumidity);
	

	//��������������
	float EmissionReverseCorrectionNew(float iTemperature);

	//������������
	float DistanceReverseCorrectionWithNinePara(float iTemperature);
	float DistanceReverseCorrectionWithSixPara(float iTemperature);
	
	//������������
	float EnvironmentReverseCorrection(float temp);

	//temp to Y16������������Ư�����;�Ͳ��Ư������
	short StandardTempToY16(float iTemperature, bool fromSmall2Big);
	
	//����͸��������
	float AtmosphereCorrection(float iTemperature);

	//����͸���ʷ���
	float AtmosphereReverseCorrection(float iTemperature);

	//����͸��������
	float WindowTransmittanceCorrection(float iTemperature);

	//����͸������������
	float WindowTransmittanceReverseCorrection(float iTemperature);

	//��ȡ�����¶ȶ�Ӧ��Y16
	short GetY16FromT(int ReflectT);

	short DeEmissCor(short nTEmiss, short nY16Reflect, int nEmiss);

	short EmissCor(short nTNoEmiss, short nY16Reflect, int nEmiss);

private:
	//int	ImageWidth;											//ͼ����
	//int	ImageHidth;											//ͼ��߶�
	int curveDataLength;									//�������߳���
	short *CurveData;										//�¶�����
	IMAGE_MEASURE_STRUCT *TMparams;
	int nShutterCurveIndex;								//�����¶ȷ������߶�Ӧ��Y16ֵ
	float fCorrection1;									//������Ʈ������
	float fCorrection2;									//��Ͳ��Ʈ������
	float *m_fTempMatrix;	//����Ƶ�������ڴ棬�ӿ�ѹ�����������������ڴ�ʱAndroid�Ϸ��ز�Ϊ�յ�ָ�룬��ʵ�ʲ����á�
	int m_tempLen;
	GuideLog *m_logger;
	short *m_sortY16;
	int m_sortLen;
};
#endif // !MEASURETEMP_CORE_TEC_H


//����
void Quick_Sort(float *arr, int begin, int end);
void Quick_Sort(short *arr, int begin, int end);
