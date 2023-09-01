#ifndef CINFRARED_AF_H
#define CINFRARED_AF_H

#include "../ITALaserAF.h"

//��չ��ͷ����
struct LenBasePara
{
	int s32LimitNear = 13500;// Զ����λ
	int s32LimitFar = 23500;//������λ
	int n_midpoint_hall = 0;//�м�λ��
	int curveLength = 0;// �뽹���߳���
	short* defocusCurve;//�뽹����
	int return_diff = 0; //�س̲�У׼����ֵ-----------
};

struct LenLaserPara
{

	int s32Length = 500;//�����г�һ��
	int s32Diff = 1000;//�س̲� 
	int s32LastDiff = 1000;//���һ���س̲� 
	int s32LowSpeed = 200;//�Աȶ�����
	int s32FastSpeed = 900;//�Աȶȿ���
	int laser_diff = 0; //����У׼����ֵ-----------
};

struct Len_contrast_para
{
	int s32Length = 500;//�����г�һ��
	int s32LowDiff = 1000;//�س̲�
	int s32LowSpeed = 300;//��������
	int s32FastSpeed = 800;//�������
	int s32LastDiff = 1000;//���һ���س̲�
	int s32ThresholdN1 = 3;//�ԱȶȶԽ�������δ�������ֵ����ֵ1
	int s32ThresholdN2 = 5;//�ԱȶȶԽ�������δ�������ֵ����ֵ2
	int s32ThresholdRatio = 1100;//�ԱȶȶԽ��������ȱ�ֵ��ֵ
};


class CInfraredAF
{
public:
	CInfraredAF(int cameraID, ITALaserAFRegistry *registry, ITA_FIELD_ANGLE lensType);
	~CInfraredAF();
	int updateLens(ITA_FIELD_ANGLE lensType);//���¾�ͷ���¶�����

	int CAF();

	int LDAF();

	int DiffCalibrateAF(int *diff);//------------

	int LaserCalibrateAF(int *diff);//------------

	int FocusBaseInfoSet(ITA_FOCUS_BASE_CFG_S *pstBaseCfg);

	int FocusLaserInfoset(ITA_FOCUS_LASER_CFG_S *pstLaserCfg);

	int FocusContrastInfoset(ITA_FOCUS_CONTRAST_CFG_S *pstContrastCfg);

private:
	//����
	char* g_curvePath;//�뽹����·��
	struct LenBasePara LenBasePara;
	struct Len_contrast_para LenContrastPara;
	struct LenLaserPara LenLaserPara;//��׼��ͷ
	//stLenPara st_wideAngleLen_para;//��Ǿ�ͷ
	//stLenPara st_mediumTeleLen_para;//�г�����ͷ
	//stLenPara st_teleLen_para;//������ͷ
	ITALaserAFRegistry *m_registry;
	int m_cameraID;
	ITA_FIELD_ANGLE m_lensType;
};


#endif // !CINFRARED_AF_H
