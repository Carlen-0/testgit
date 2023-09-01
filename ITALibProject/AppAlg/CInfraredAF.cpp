#include <string.h>
#include "CInfraredAF.h"
#include "../Module/PortingAPI.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
/***************/


#define errorTimes 	40
#define errorTimes_low 	10
#define maxSpeed 900


CInfraredAF::CInfraredAF(int cameraID, ITALaserAFRegistry *registry, ITA_FIELD_ANGLE lensType)
{
	m_registry = (ITALaserAFRegistry *)porting_calloc_mem(1, sizeof(ITALaserAFRegistry), ITA_OTHER_MODULE);
	*m_registry = *registry;
	m_cameraID = cameraID;
	m_lensType = ITA_ANGLE_56;
	//standard lens 
	LenBasePara.s32LimitNear = 13500;
	LenBasePara.s32LimitFar = 23500;
	LenBasePara.n_midpoint_hall = (LenBasePara.s32LimitNear + LenBasePara.s32LimitFar) / 2;
	LenBasePara.curveLength = 2000; //曲线长度最大2000*sizeof(int)
	LenBasePara.defocusCurve = (short*)porting_calloc_mem(LenBasePara.curveLength, sizeof(short), ITA_OTHER_MODULE);
	updateLens(lensType);
	printf("-----------CInfraredAF init \n");
	printf("----LimitNear:%d LimitFar:%d return_diff:%d \n", \
		LenBasePara.s32LimitNear, LenBasePara.s32LimitFar, LenBasePara.return_diff);
}

CInfraredAF::~CInfraredAF()
{
	porting_free_mem(LenBasePara.defocusCurve);
}

int CInfraredAF::updateLens(ITA_FIELD_ANGLE lensType)
{
	int length = 0;
	ITA_RESULT result;
	//镜头一样就不更新
	if (m_lensType != ITA_ANGLE_56 && m_lensType == lensType)
	{
		return 0;
	}
	m_lensType = lensType;
	memset(LenBasePara.defocusCurve, 0, LenBasePara.curveLength * sizeof(short));
	result = m_registry->ReadCurveData(m_cameraID, (unsigned char *)LenBasePara.defocusCurve,
		LenBasePara.curveLength * sizeof(short), &length, m_lensType, m_registry->userParam);
	if (ITA_OK == result && length > 0)
	{
		LenBasePara.curveLength = length / sizeof(short);
	}
	return 0;
}
/***************/

/*
函数名称：readCurve
函数说明：读取离焦曲线
输入参数：curvePath 离焦曲线路径
输出参数：
返回值：0 表示对焦结束
*/


//调焦初始化函数，在第一次调焦时调用


/*
函数名称：CAF
函数说明：对比度对焦
输入参数：无
输出参数：
返回值：0 表示对焦结束
*/
int CInfraredAF::CAF() {
	int endHall = 0;
	int clearity = 0;
	int hall = 0;
	int clearityK = 0;
	int maxK = 0;
	int maxHallFast = 0;
	int n = 0;
	int maxClearity = 0;
	int maxHallLow = 0;
	int startHallLow = 0;//慢速起点
	int endHallLow = 0;//慢速终点
	int clearityBase = 0;
	int runTimes = 0;//每次循环运行次数
	int last_hall[errorTimes] = { 0, };
	int cur_clearity[errorTimes] = { 0, };
	int i = 0;
	int runTimes_low = 0;
	//int time_dif;
	//struct timeval start_time, end_time;
	m_registry->GetHall(m_cameraID, &hall, m_registry->userParam);
	maxHallFast = hall;


	if (hall < LenBasePara.n_midpoint_hall) {
		printf("CAF 1,goto LimitNear %d\n",LenBasePara.s32LimitNear);
		m_registry->focus_move_to_hall(m_cameraID, LenBasePara.s32LimitNear, maxSpeed, 1, m_registry->userParam);
		//_sleep(120);
		porting_thread_sleep(120);
		m_registry->GetHall(m_cameraID, &hall, m_registry->userParam);
		printf("LimitNear=%d,LenContrastPara.s32ThresholdRatio=%d\n", hall,LenContrastPara.s32ThresholdRatio);

		endHall = LenBasePara.s32LimitFar;
		m_registry->GetClearity(m_cameraID, &clearityBase, m_registry->userParam);//get the base of clearity
																				  //printf("clearityBase=%d\n",clearityBase);
		maxClearity = clearityBase;
		maxK = 1000;
		//to far
		printf("CAF 2,goto LimitFar_fx\n");
		m_registry->focus_move_to_hall(m_cameraID, LenBasePara.s32LimitFar, LenContrastPara.s32FastSpeed, 0, m_registry->userParam);//move fast to far,find the maxClearity and the hall
		runTimes = 0;
		while (!((runTimes>errorTimes)|| (hall >= (endHall - 50)) \
			|| ((maxK >= LenContrastPara.s32ThresholdRatio) && (n >= LenContrastPara.s32ThresholdN1)) \
			|| ((maxK <= 900) && (n >= LenContrastPara.s32ThresholdN2)) )) 
		//while (((runTimes<=errorTimes)&& (hall <= (endHall - 50)) \
	&& ((maxK <= LenContrastPara.s32ThresholdRatio) || (n >= LenContrastPara.s32ThresholdN1)) \
			&& ((maxK >= 900) || (n <= LenContrastPara.s32ThresholdN2)) )) 
		{
			porting_thread_sleep(20);

			m_registry->GetHall(m_cameraID, &hall, m_registry->userParam);
			last_hall[runTimes] = hall;

			m_registry->GetClearity(m_cameraID, &clearity, m_registry->userParam);
			cur_clearity[runTimes] = clearity;
			clearityK = (clearity * 1000 / clearityBase);
			//printf("clearity=%d,hall=%d,clearityK=%d,runTimes=%d\n",clearity,hall,clearityK,runTimes);


			//if (clearityK >  maxK) {
			if (clearity > maxClearity) {
				maxClearity = clearity;
				maxK = clearityK;
				maxHallFast = hall;
				if (runTimes >= 3) {
					maxHallFast = last_hall[runTimes - 3];

				}

				n = 0;
			}
			else {
				n++;
			}
			printf("maxClearity=%d, clearity=%d,clearityK=%d,maxK=%d,maxK_n=%d,maxHallFast%d,hall=%d,runTimes=%d\n",maxClearity ,clearity,clearityK,maxK,n,maxHallFast,hall,runTimes);

			runTimes++;
			if(runTimes==40)
			{
				break;
				//printf("runTimes=40");
			}
		}
		m_registry->SetStop(m_cameraID, m_registry->userParam);

		//printf("a1=%d,a2=%d,a3=%d\n",a1,a2,a3);

		m_registry->GetHall(m_cameraID, &hall, m_registry->userParam);
		printf("maxK=%d,maxK_n=%d,maxHallFast%d,hall=%d,runTimes=%d\n",maxK,n,maxHallFast,hall,runTimes);
		/*for (i = 0; i < runTimes; i++)
		{
		printf("cur_clearity=%d\n",cur_clearity[i]);
		printf("last_hall=%d\n",last_hall[i]);
		}*/


		//low
		maxHallFast = maxHallFast - LenContrastPara.s32LowDiff;
		printf("maxHallFast - LenContrastPara.s32LowDiff=%d\n", maxHallFast);
		startHallLow = maxHallFast + LenContrastPara.s32Length;
		endHallLow = maxHallFast - LenContrastPara.s32Length;
		if (startHallLow >LenBasePara.s32LimitFar) {
			startHallLow = LenBasePara.s32LimitFar;
		}
		if (endHallLow < LenBasePara.s32LimitNear) {
			endHallLow = LenBasePara.s32LimitNear;
		}
		printf("CAF 3,goto startHallLow %d\n",startHallLow);
		m_registry->focus_move_to_hall(m_cameraID, startHallLow, LenContrastPara.s32FastSpeed, 1, m_registry->userParam);
		m_registry->GetHall(m_cameraID, &hall, m_registry->userParam);
		printf("starthall=%d\n", hall);
		m_registry->GetClearity(m_cameraID, &clearity, m_registry->userParam);
		m_registry->GetHall(m_cameraID, &hall, m_registry->userParam);
		maxClearity = clearity;
		maxHallLow = hall;

		m_registry->focus_move_to_hall(m_cameraID, endHallLow, LenContrastPara.s32LowSpeed, 0, m_registry->userParam);
		runTimes_low = 0;
		while ((hall>(endHallLow + 50)) && (runTimes_low <= errorTimes_low)) {
			porting_thread_sleep(20);

			m_registry->GetHall(m_cameraID, &hall, m_registry->userParam);
			last_hall[runTimes + runTimes_low] = hall;


			m_registry->GetClearity(m_cameraID, &clearity, m_registry->userParam);
			cur_clearity[runTimes + runTimes_low] = clearity;

			if (clearity > maxClearity) {
				maxClearity = clearity;
				maxHallLow = hall;
				if (runTimes_low >= 3) {
					maxHallLow = last_hall[runTimes + runTimes_low - 3];
				}
				else {
					maxHallLow = last_hall[runTimes];
				}
			}
			printf("runTimes_low=%d,maxClearity=%d,maxHallLow=%d,hall=%d\n",runTimes_low,maxClearity,maxHallLow,hall);
			runTimes_low++;

		}
		m_registry->SetStop(m_cameraID, m_registry->userParam);
		m_registry->GetHall(m_cameraID, &hall, m_registry->userParam);
		printf("maxClearity=%d,maxHallLow=%d,hall=%d,runTimes_low=%d\n",maxClearity,maxHallLow,hall,runTimes_low);
		/*for (i = runTimes; i < runTimes+runTimes_low; i++)
		{
		printf("cur_clearity=%d\n",cur_clearity[i]);
		printf("last_hall=%d\n",last_hall[i]);
		}
		*/

		printf("CAF 5,goto maxHallLow+hcc %d\n",maxHallLow + LenContrastPara.s32LowDiff);
		m_registry->focus_move_to_hall(m_cameraID, maxHallLow + LenContrastPara.s32LowDiff, LenContrastPara.s32FastSpeed, 1, m_registry->userParam);
		//focus_move_to_hall(maxHallLow + LenContrastPara.s32LowDiff, LenContrastPara.s32FastSpeed,1);
		m_registry->GetHall(m_cameraID, &hall, m_registry->userParam);
		printf("over,hall=%d\n",hall);
		m_registry->SetStop(m_cameraID, m_registry->userParam);
	}
	else {
		printf("CAF 6,goto LimitFar\n");
		m_registry->focus_move_to_hall(m_cameraID, LenBasePara.s32LimitFar, maxSpeed ,1, m_registry->userParam);
		//_sleep(120);
		porting_thread_sleep(120);
		m_registry->GetHall(m_cameraID, &hall, m_registry->userParam);
		printf("LimitFar=%d, LenContrastPara.s32ThresholdRatio=%d\n", hall,LenContrastPara.s32ThresholdRatio);

		endHall = LenBasePara.s32LimitNear;
		m_registry->GetClearity(m_cameraID, &clearityBase, m_registry->userParam);//get the base of clearity
																				  //printf("clearityBase=%d\n",clearityBase);
		maxClearity = clearityBase;
		maxK = 1000;
		printf("CAF 7,goto LimitNear_fx\n");
		m_registry->focus_move_to_hall(m_cameraID, LenBasePara.s32LimitNear, LenContrastPara.s32FastSpeed, 0, m_registry->userParam);//move fast to near,find the maxClearity and the hall
		runTimes = 0;
		while (!((runTimes>errorTimes)||(hall <= (endHall + 50)) || \
			((maxK >= LenContrastPara.s32ThresholdRatio) && (n >= LenContrastPara.s32ThresholdN1)) || \
			((maxK <= 900 && n >= LenContrastPara.s32ThresholdN2)) )) 
		{
			porting_thread_sleep(20);

			m_registry->GetHall(m_cameraID, &hall, m_registry->userParam);
			last_hall[runTimes] = hall;

			m_registry->GetClearity(m_cameraID, &clearity, m_registry->userParam);
			cur_clearity[runTimes] = clearity;
			clearityK = clearity * 1000 / clearityBase;


			//if (clearityK > maxK) {
			if (clearity >  maxClearity) {
				maxClearity = clearity;
				maxK = clearityK;
				maxHallFast = hall;
				if (runTimes >= 3) {
					maxHallFast = last_hall[runTimes - 3];

				}
				n = 0;

			}
			else {
				n++;
			}

			printf("maxClearity=%d, clearity=%d,clearityK=%d,maxK=%d,maxK_n=%d,maxHallFast%d,hall=%d,runTimes=%d\n",maxClearity ,clearity,clearityK,maxK,n,maxHallFast,hall,runTimes);
			runTimes++;
			if(runTimes==40)
			{
				break;
				//printf("runTimes=40");
			}

		}
		m_registry->SetStop(m_cameraID, m_registry->userParam);

		//printf("a1=%d,a2=%d,a3=%d\n",a1,a2,a3);

		m_registry->GetHall(m_cameraID, &hall, m_registry->userParam);
		printf("maxK=%d,maxHallFast=%d,n=%d,hall=%d,runTimes=%d\n",maxK,maxHallFast,n,hall,runTimes);
		/*for (i = 0; i < runTimes; i++)
		{
		printf("cur_clearity=%d\n",cur_clearity[i]);
		printf("last_hall=%d\n",last_hall[i]);
		}*/

		//low	
		maxHallFast = maxHallFast + LenContrastPara.s32LowDiff;
		startHallLow = maxHallFast - LenContrastPara.s32Length;
		endHallLow = maxHallFast + LenContrastPara.s32Length;
		if (startHallLow <LenBasePara.s32LimitNear) {
			startHallLow = LenBasePara.s32LimitNear;
		}
		if (endHallLow > LenBasePara.s32LimitFar) {
			endHallLow = LenBasePara.s32LimitFar;
		}
		printf("CAF 8,goto startHallLow %d\n",startHallLow);
		m_registry->focus_move_to_hall(m_cameraID, startHallLow, LenContrastPara.s32FastSpeed, 1, m_registry->userParam);
		m_registry->GetHall(m_cameraID, &hall, m_registry->userParam);
		printf("starthall=%d\n", hall);
		m_registry->GetClearity(m_cameraID, &clearity, m_registry->userParam);

		maxClearity = clearity;
		maxHallLow = hall;
		printf("CAF 9,goto endHallLow %d\n",endHallLow);
		m_registry->focus_move_to_hall(m_cameraID, endHallLow, LenContrastPara.s32LowSpeed, 0, m_registry->userParam);
		runTimes_low = 0;
		while ((hall<(endHallLow - 50)) && (runTimes_low <= errorTimes_low)) {
			porting_thread_sleep(20);

			m_registry->GetHall(m_cameraID, &hall, m_registry->userParam);
			last_hall[runTimes + runTimes_low] = hall;

			m_registry->GetClearity(m_cameraID, &clearity, m_registry->userParam);
			cur_clearity[runTimes + runTimes_low] = clearity;

			if (clearity > maxClearity) {
				maxClearity = clearity;
				maxHallLow = hall;
				if (runTimes_low >= 3) {
					maxHallLow = last_hall[runTimes + runTimes_low - 3];
				}
				else {
					maxHallLow = last_hall[runTimes];
				}
			}
			printf("runTimes_low=%d,maxClearity=%d,maxHallLow=%d,hall=%d\n",runTimes_low,maxClearity,maxHallLow,hall);
			runTimes_low++;
		}
		m_registry->SetStop(m_cameraID, m_registry->userParam);

		//printf("maxClearity=%d,maxHallLow=%d,hall=%d,runTimes_low=%d\n",maxClearity,maxHallLow,hall,runTimes_low);
		/*for (i = runTimes; i < runTimes+runTimes_low; i++)
		{
		printf("cur_clearity=%d\n",cur_clearity[i]);
		printf("last_hall=%d\n",last_hall[i]);
		}*/
		m_registry->GetHall(m_cameraID, &hall, m_registry->userParam);
		//printf("endHallLow = %d\n", hall);
		//printf("maxHallLow = %d\n", maxHallLow);
		printf("CAF 10,goto maxHallLow - LenContrastPara.s32LowDiff=%d\n",maxHallLow - LenContrastPara.s32LowDiff);
		m_registry->focus_move_to_hall(m_cameraID, maxHallLow - LenContrastPara.s32LowDiff, LenContrastPara.s32FastSpeed, 1, m_registry->userParam);
		//focus_move_to_hall(maxHallLow - LenContrastPara.s32LowDiff, LenContrastPara.s32FastSpeed,1);
        m_registry->SetStop(m_cameraID, m_registry->userParam);
		printf("over,hall=%d\n",hall);

	}
    m_registry->SetStop(m_cameraID, m_registry->userParam);
	return 0;
}

int CInfraredAF::LDAF()
{
	int i = 0;
	int clearity = 0;
	int maxClearity = 0;
	int maxHallLow = 0;

	int distance = 0;
	m_registry->GetDistance(m_cameraID, &distance, m_registry->userParam);
	distance = distance / 10;
    printf("LDAF 2,distance= %d cm\n",distance);
	if (distance > 2000) {//超过20m就只取20m
		distance = 2000;
	}
	if (distance <= 100) {//低于1m就用对比度调焦
		CAF();//不往下走了
		return 0;
	}

	short hallTable = LenBasePara.defocusCurve[distance];
    printf("hallTable= %d\n", hallTable);
	
	int leftHall = hallTable + LenLaserPara.laser_diff - LenLaserPara.s32Length;
	int rightHall = hallTable + LenLaserPara.laser_diff + LenLaserPara.s32Length;
	int hall;
	int last_hall[errorTimes] = { 0, };
	int runTimes = 0;//每次循环运行次数
    printf("LenLaserPara.laser_diff= %d\n",LenLaserPara.laser_diff);
	printf("hallTable= %d\n", hallTable);
	m_registry->GetHall(m_cameraID, &hall, m_registry->userParam);
	printf("hall_start= %d\n", hall);
	char pos = (hall < leftHall) ? 0 : ((hall < hallTable) ? 1 : ((hall < rightHall) ? 2 : 3));
	if (pos == 0) {
		m_registry->focus_move_to_hall(m_cameraID, leftHall, LenLaserPara.s32FastSpeed, 1, m_registry->userParam);//go to the hallTable ,-lengthLow
		m_registry->GetHall(m_cameraID, &hall, m_registry->userParam);
		printf("LDAF 31,move_to_lefthall= %d\n", hall);
	}
	else if (pos == 1)
	{
		m_registry->focus_move_to_hall(m_cameraID, leftHall, LenLaserPara.s32FastSpeed, 1, m_registry->userParam);//go to the hallTable ,+lengthLow	
		m_registry->GetHall(m_cameraID, &hall, m_registry->userParam);
		printf("LDAF 32,move_to_lefthall= %d\n", hall);
	}
	else if (pos == 2)
	{
		m_registry->focus_move_to_hall(m_cameraID, rightHall, LenLaserPara.s32FastSpeed, 1, m_registry->userParam);//go to the hallTable ,+lengthLow	
		m_registry->GetHall(m_cameraID, &hall, m_registry->userParam);
		printf("LDAF 33,move_to_righthall= %d\n", hall);
	}
	else if (pos == 3)
	{
		m_registry->focus_move_to_hall(m_cameraID, rightHall, LenLaserPara.s32FastSpeed, 1, m_registry->userParam);//go to the hallTable ,+lengthLow	
		m_registry->GetHall(m_cameraID, &hall, m_registry->userParam);
		printf("LDAF 34,move_to_righthall= %d\n", hall);
	}
	//low find the max clearity of hall ,and go to the hall
	if (pos == 0 || pos == 1) {
		m_registry->GetClearity(m_cameraID, &clearity, m_registry->userParam);
		m_registry->GetHall(m_cameraID, &hall, m_registry->userParam);
		maxClearity = clearity;
		maxHallLow = hall;
		m_registry->focus_move_to_hall(m_cameraID, rightHall, LenLaserPara.s32LowSpeed, 0, m_registry->userParam);
		runTimes = 0;
		while ((hall < (rightHall - 50)) && (runTimes <= errorTimes_low)) {
			porting_thread_sleep(20);
			m_registry->GetHall(m_cameraID, &hall, m_registry->userParam);
			last_hall[runTimes] = hall;
			m_registry->GetClearity(m_cameraID, &clearity, m_registry->userParam);
			if (clearity > maxClearity) {
				maxClearity = clearity;
				maxHallLow = hall;
				if (runTimes >= 3) {
					maxHallLow = last_hall[runTimes - 3];
				}
				else {
					maxHallLow = last_hall[0];
				}
			}
			printf("runTimes=%d,maxClearity=%d,maxHallLow=%d,hall=%d\n",runTimes,maxClearity,maxHallLow,hall);
			runTimes++;
		}
		m_registry->SetStop(m_cameraID, m_registry->userParam);
		printf("maxClearity=%d,maxHallLow=%d,hall=%d\n",maxClearity,maxHallLow,hall);

		m_registry->focus_move_to_hall(m_cameraID, maxHallLow - LenLaserPara.s32Diff, LenLaserPara.s32FastSpeed, 1, m_registry->userParam);
		m_registry->GetHall(m_cameraID, &hall, m_registry->userParam);
		m_registry->SetStop(m_cameraID, m_registry->userParam);
		printf("over,maxHall-hcc= %d\n", hall);
	}
	else {
		m_registry->focus_move_to_hall(m_cameraID, leftHall, LenLaserPara.s32LowSpeed, 0, m_registry->userParam);
		m_registry->GetClearity(m_cameraID, &clearity, m_registry->userParam);
		m_registry->GetHall(m_cameraID, &hall, m_registry->userParam);
		maxClearity = clearity;
		maxHallLow = hall;
		runTimes = 0;
		while ((hall > (leftHall + 50)) && (runTimes <= errorTimes)) {
			porting_thread_sleep(20);
			m_registry->GetHall(m_cameraID, &hall, m_registry->userParam);
			last_hall[runTimes] = hall;

			m_registry->GetClearity(m_cameraID, &clearity, m_registry->userParam);
			if (clearity > maxClearity) {
				maxClearity = clearity;
				maxHallLow = hall;
				if (runTimes >= 3) {
					maxHallLow = last_hall[runTimes - 3];

				}
				else {
					maxHallLow = last_hall[0];
				}
			}
			printf("runTimes=%d,maxClearity=%d,maxHallLow=%d,hall=%d\n",runTimes,maxClearity,maxHallLow,hall);
			runTimes++;
		}
		m_registry->SetStop(m_cameraID, m_registry->userParam);
		printf("maxClearity=%d,maxHallLow=%d,hall=%d \n",maxClearity,maxHallLow,hall);

		m_registry->focus_move_to_hall(m_cameraID, maxHallLow + LenLaserPara.s32Diff, LenLaserPara.s32FastSpeed, 1, m_registry->userParam);
		m_registry->GetHall(m_cameraID, &hall, m_registry->userParam);
		printf("over,maxHall+hcc= %d \n", hall);
		m_registry->SetStop(m_cameraID, m_registry->userParam);
	}
    m_registry->SetStop(m_cameraID, m_registry->userParam);
	return 0;
}

int CInfraredAF::FocusBaseInfoSet(ITA_FOCUS_BASE_CFG_S *pstBaseCfg)
{
	LenBasePara.s32LimitNear = pstBaseCfg->s32LimitNear;
	LenBasePara.s32LimitFar = pstBaseCfg->s32LimitFar;
	LenBasePara.return_diff = pstBaseCfg->s32ReturnDiff;
    LenBasePara.n_midpoint_hall = (LenBasePara.s32LimitNear + LenBasePara.s32LimitFar) / 2;
	if (LenBasePara.return_diff != 0)
	{
		LenContrastPara.s32LowDiff = pstBaseCfg->s32ReturnDiff;
		LenContrastPara.s32LastDiff = pstBaseCfg->s32ReturnDiff;
		LenLaserPara.s32Diff = pstBaseCfg->s32ReturnDiff;
		
	}
	printf("-----------FocusBaseInfoSet \n");
	printf("----LimitNear:%d LimitFar:%d return_diff:%d \n", \
		LenBasePara.s32LimitNear, LenBasePara.s32LimitFar, LenBasePara.return_diff);
	return 0;
}

int CInfraredAF::FocusLaserInfoset(ITA_FOCUS_LASER_CFG_S *pstLaserCfg)
{
	LenLaserPara.s32Diff = pstLaserCfg->s32Diff;
	LenLaserPara.s32FastSpeed = pstLaserCfg->s32FastSpeed;
	LenLaserPara.s32LowSpeed = pstLaserCfg->s32LowSpeed;
	LenLaserPara.s32Length = pstLaserCfg->s32Length;
	LenLaserPara.laser_diff = pstLaserCfg->s32LaserDiff;

	printf("-----------FocusLaserInfoset \n");
	printf("LenLaserPara.s32Diff = %d, \n\
		LenLaserPara.s32FastSpeed = %d,  \n\
		LenLaserPara.s32LowSpeed = %d,  \n\
		LenLaserPara.s32Length = %d,  \n\
		LenLaserPara.laser_diff = %d   \n", 
		pstLaserCfg->s32Diff,pstLaserCfg->s32FastSpeed,pstLaserCfg->s32LowSpeed,pstLaserCfg->s32Length,pstLaserCfg->s32LaserDiff);
	return 0;
}

int CInfraredAF::FocusContrastInfoset(ITA_FOCUS_CONTRAST_CFG_S *pstContrastCfg)
{
	LenContrastPara.s32Length = pstContrastCfg->s32Length;
	LenContrastPara.s32LowDiff = pstContrastCfg->s32LowDiff;//回程差
	LenContrastPara.s32LowSpeed = pstContrastCfg->s32LowSpeed;//激光慢速
	LenContrastPara.s32FastSpeed = pstContrastCfg->s32FastSpeed;//激光快速
	LenContrastPara.s32LastDiff = pstContrastCfg->s32LastDiff;//最后一步回程差
	LenContrastPara.s32ThresholdN1 = pstContrastCfg->s32ThresholdN1;//对比度对焦，连续未更新最大值，阈值1
	LenContrastPara.s32ThresholdN2 = pstContrastCfg->s32ThresholdN2;//对比度对焦，连续未更新最大值，阈值2
	LenContrastPara.s32ThresholdRatio = pstContrastCfg->s32ThresholdRatio;//对比度对焦，清晰度比值阈值

	printf("-----------FocusContrastInfoset \n");
	printf("LenContrastPara.s32Length = %d,  \n \
	LenContrastPara.s32LowSpeed = %d,  \n \
	LenContrastPara.s32FastSpeed = %d,  \n \
	LenContrastPara.s32LastDiff = %d,  \n \
	LenContrastPara.s32ThresholdN1 = %d,  \n \
	LenContrastPara.s32ThresholdN2 = %d,  \n \
	LenContrastPara.s32ThresholdRatio = %d,  \n \
	", pstContrastCfg->s32Length,
		pstContrastCfg->s32LowSpeed,
		pstContrastCfg->s32FastSpeed,
		pstContrastCfg->s32LastDiff,
		pstContrastCfg->s32ThresholdN1,
		pstContrastCfg->s32ThresholdN2,
		pstContrastCfg->s32ThresholdRatio);

	return 0;
}



int CInfraredAF::DiffCalibrateAF(int *diff)
{
	int clearity = 0;
	int hall = 0;
	int returndiff[3] = { 0, };
	int returnDiff = 0;
	int last_hall1[100] = { 0, };
	int last_hall2[100] = { 0, };
	int runTimes1 = 0;
	int runTimes2 = 0;
	m_registry->focus_move_to_hall(m_cameraID, LenBasePara.s32LimitNear, LenLaserPara.s32FastSpeed, 1, m_registry->userParam);
	m_registry->GetHall(m_cameraID, &hall, m_registry->userParam);
		printf("caliAF 1,hall= %d\n", hall);

	for (int i= 0; i<3; i++)
	{
		int maxHallLow1 = 0;
		int maxHallLow2 = 0;
		int maxClearity = 0;
		runTimes1 = 0;
		runTimes2 = 0;
		m_registry->focus_move_to_hall(m_cameraID, LenBasePara.s32LimitFar, LenLaserPara.s32LowSpeed, 0, m_registry->userParam);
		while ((hall<(LenBasePara.s32LimitFar - 50))&&(runTimes1< 100)) {
			porting_thread_sleep(20);
			m_registry->GetHall(m_cameraID, &hall, m_registry->userParam);
			last_hall1[runTimes1] = hall;

			m_registry->GetClearity(m_cameraID, &clearity, m_registry->userParam);


			if (clearity > maxClearity) {
				maxClearity = clearity;
				maxHallLow1 = hall;
				if (runTimes1 >= 3) {
					maxHallLow1 = last_hall1[runTimes1 - 3];
				}

			}
			runTimes1++;
			printf("runTimes1=%d,clearity=%d,maxHallLow1=%d,hall=%d\n",runTimes1,clearity,maxHallLow1,hall);
		}
		m_registry->SetStop(m_cameraID, m_registry->userParam);	//
		m_registry->GetHall(m_cameraID, &hall, m_registry->userParam);
		printf("caliAF 2,hall= %d\n", hall);
		m_registry->focus_move_to_hall(m_cameraID, LenBasePara.s32LimitNear, LenLaserPara.s32LowSpeed, 0, m_registry->userParam);
		maxClearity = 0;
		while ((hall>(LenBasePara.s32LimitNear + 50))&&(runTimes2< 100))  {
			porting_thread_sleep(20);
			m_registry->GetHall(m_cameraID, &hall, m_registry->userParam);
			last_hall2[runTimes2] = hall;

			m_registry->GetClearity(m_cameraID, &clearity, m_registry->userParam);


			if (clearity > maxClearity) {
				maxClearity = clearity;
				maxHallLow2 = hall;
				if (runTimes2 >= 3) {
					maxHallLow2 = last_hall2[runTimes2 - 3];
				}

			}
			runTimes2++;
			printf("runTimes2=%d,clearity=%d,maxHallLow2=%d,hall=%d\n",runTimes2,clearity,maxHallLow2,hall);
		}
		m_registry->SetStop(m_cameraID, m_registry->userParam);
		m_registry->GetHall(m_cameraID, &hall, m_registry->userParam);
		printf("caliAF 3,hall= %d\n", hall);
		returndiff[i] = maxHallLow1 - maxHallLow2;
		printf("maxHallLow1 =%d, runTimes1=%d,runTimes2=%d,maxHallLow2=%d\n", maxHallLow1, runTimes1, runTimes2, maxHallLow2);
	}
	m_registry->SetStop(m_cameraID, m_registry->userParam);
	returnDiff = (returndiff[0] + returndiff[1] + returndiff[2]) / 3;
	printf("returnDiff=%d,returndiff[0]=%d,returndiff[1]=%d,returndiff[2]=%d\n",returnDiff,returndiff[0],returndiff[1],returndiff[2]);
	if((abs(returndiff[0]-returnDiff)<300)&&(abs(returndiff[1]-returnDiff)<300)&&(abs(returndiff[2]-returnDiff)<300))
	{
		LenBasePara.return_diff = returnDiff;
		
		*diff = returnDiff;
		return 1;
	}
    else
	{
		LenBasePara.return_diff =0;
		*diff =0;
		return 0;
	}
}


int CInfraredAF::LaserCalibrateAF(int *diff)
{

	int clearity = 0;
	int hall = 0;
	int laserdiff[3] = { 0, };
	int laser_diff = 0;
	int last_hall[100] = { 0, };
	int runTimes = 0;

	int distance = 0;
	m_registry->GetDistance(m_cameraID, &distance, m_registry->userParam);
	distance = distance / 10;

	short hallTable1 = LenBasePara.defocusCurve[distance];
	for (int i = 0; i<3; i++)
	{
		m_registry->focus_move_to_hall(m_cameraID, LenBasePara.s32LimitNear, LenLaserPara.s32FastSpeed, 1, m_registry->userParam);
        m_registry->GetHall(m_cameraID, &hall, m_registry->userParam);
	    printf("laserAF 1,hall= %d\n", hall);
		int maxHallLow = 0;

		int maxClearity = 0;
		runTimes = 0;
		
		m_registry->focus_move_to_hall(m_cameraID, LenBasePara.s32LimitFar, LenLaserPara.s32LowSpeed, 0, m_registry->userParam);

		while ((hall<(LenBasePara.s32LimitFar - 50))&&(runTimes< 100)) 
		{
			porting_thread_sleep(20);
			m_registry->GetHall(m_cameraID, &hall, m_registry->userParam);
			last_hall[runTimes] = hall;

			m_registry->GetClearity(m_cameraID, &clearity, m_registry->userParam);

			if (clearity > maxClearity) {
				maxClearity = clearity;
				maxHallLow = hall;
				if (runTimes >= 3) {
					maxHallLow = last_hall[runTimes - 3];
				}
			}
			printf("runTimes=%d,clearity=%d,maxHallLow=%d,hall=%d\n",runTimes,clearity,maxHallLow,hall);
			runTimes++;
		}
		m_registry->SetStop(m_cameraID, m_registry->userParam);
		laserdiff[i] = maxHallLow - hallTable1;
		printf("maxHallLow=%d, runTimes=%d\n", maxHallLow,runTimes);
		//m_registry->focus_move_to_hall(m_cameraID, LenBasePara.s32LimitNear, LenLaserPara.s32FastSpeed, 1, m_registry->userParam);
		m_registry->GetHall(m_cameraID, &hall, m_registry->userParam);
		printf("laserAF 2,hall= %d\n", hall);
	}
	m_registry->SetStop(m_cameraID, m_registry->userParam);
	m_registry->GetHall(m_cameraID, &hall, m_registry->userParam);
    printf("laserAF end,hall= %d\n", hall);
	laser_diff = (laserdiff[0] + laserdiff[1] + laserdiff[2]) / 3;
	
	printf("laser_diff=%d,laserdiff[0]=%d,laserdiff[1]=%d,laserdiff[2]=%d\n", laser_diff, laserdiff[0], laserdiff[1], laserdiff[2]);

	if ((abs(laserdiff[0] - laser_diff) < 300) && (abs(laserdiff[1] - laser_diff) < 300) && (abs(laserdiff[2] - laser_diff) < 300))
	{
		LenLaserPara.laser_diff = 0;
		LDAF();
		m_registry->GetHall(m_cameraID, &hall, m_registry->userParam);

		if (abs(laser_diff + hallTable1 - hall)<300)
		{
			LenLaserPara.laser_diff = laser_diff;
			*diff = laser_diff;
			return 1;
		}
		else {
			LenLaserPara.laser_diff = 0;
			*diff = 0;
			return 0;
		}
	}
	else
	{
			LenLaserPara.laser_diff = 0;
			*diff = 0;
			return 0;
	}
		
}
