#ifndef CINFRARED_AF_H
#define CINFRARED_AF_H

#include "../ITALaserAF.h"

//扩展镜头参数
struct LenBasePara
{
	int s32LimitNear = 13500;// 远焦限位
	int s32LimitFar = 23500;//近焦限位
	int n_midpoint_hall = 0;//中间位置
	int curveLength = 0;// 离焦曲线长度
	short* defocusCurve;//离焦曲线
	int return_diff = 0; //回程差校准补偿值-----------
};

struct LenLaserPara
{

	int s32Length = 500;//慢速行程一半
	int s32Diff = 1000;//回程差 
	int s32LastDiff = 1000;//最后一步回程差 
	int s32LowSpeed = 200;//对比度慢速
	int s32FastSpeed = 900;//对比度快速
	int laser_diff = 0; //激光校准补偿值-----------
};

struct Len_contrast_para
{
	int s32Length = 500;//慢速行程一半
	int s32LowDiff = 1000;//回程差
	int s32LowSpeed = 300;//激光慢速
	int s32FastSpeed = 800;//激光快速
	int s32LastDiff = 1000;//最后一步回程差
	int s32ThresholdN1 = 3;//对比度对焦，连续未更新最大值，阈值1
	int s32ThresholdN2 = 5;//对比度对焦，连续未更新最大值，阈值2
	int s32ThresholdRatio = 1100;//对比度对焦，清晰度比值阈值
};


class CInfraredAF
{
public:
	CInfraredAF(int cameraID, ITALaserAFRegistry *registry, ITA_FIELD_ANGLE lensType);
	~CInfraredAF();
	int updateLens(ITA_FIELD_ANGLE lensType);//更新镜头重新读曲线

	int CAF();

	int LDAF();

	int DiffCalibrateAF(int *diff);//------------

	int LaserCalibrateAF(int *diff);//------------

	int FocusBaseInfoSet(ITA_FOCUS_BASE_CFG_S *pstBaseCfg);

	int FocusLaserInfoset(ITA_FOCUS_LASER_CFG_S *pstLaserCfg);

	int FocusContrastInfoset(ITA_FOCUS_CONTRAST_CFG_S *pstContrastCfg);

private:
	//参数
	char* g_curvePath;//离焦曲线路径
	struct LenBasePara LenBasePara;
	struct Len_contrast_para LenContrastPara;
	struct LenLaserPara LenLaserPara;//标准镜头
	//stLenPara st_wideAngleLen_para;//广角镜头
	//stLenPara st_mediumTeleLen_para;//中长焦镜头
	//stLenPara st_teleLen_para;//长焦镜头
	ITALaserAFRegistry *m_registry;
	int m_cameraID;
	ITA_FIELD_ANGLE m_lensType;
};


#endif // !CINFRARED_AF_H
