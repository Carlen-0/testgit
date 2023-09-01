#include "AutoFocus.h"
#include <stdlib.h>
#include <string.h>

AutoFocus::AutoFocus()
{
	autofocuspara.max_autofocus_time_cnt = MAX_AUTOFOCUS_TIME_CNT;//300
	autofocuspara.min_autofocus_time_cnt = MIN_AUTOFOCUS_TIME_CNT;//41
	autofocuspara.run_autofocus_time_cnt = RUN_AUTOFOCUS_TIME_CNT;//20
	autofocuspara.wait_autofocus_time_cnt = WAIT_AUTOFOCUS_TIME_CNT;//41-20=21
	autofocuspara.diff_min_autofocus_sharp_value = DIFF_MIN_AUTOFOCUS_SHARP_VALUE;		//10000
	autofocuspara.cnt_autofocus_get_near_will_stop = CNT_AUTOFOCUS_GET_NEAR_WILL_STOP;	//3
	autofocuspara.cnt_autofocus_get_far_will_stop = CNT_AUTOFOCUS_GET_FAR_WILL_STOP;	//4
	m_registry = 0;
	m_logger = 0;
#ifdef MULTI_THREAD_FUNCTION
	hThreadHandle = 0;
	m_mutex = 0;
	isRunning = false;
	actionType = (ITA_BC_TYPE)0;
	actionParam = 0; //0：正常调焦,起点为近焦；1：正常调焦,起点为远焦；2：校准参数,起点为近焦；3：校准参数,起点为远焦。
	m_y16Array = 0;
#endif // MULTI_THREAD_FUNCTION	
}


AutoFocus::~AutoFocus()
{
#ifdef MULTI_THREAD_FUNCTION
	m_logger->output(LOG_INFO, "AF thread_destroy %#x", hThreadHandle);
	isRunning = false;
	if (hThreadHandle)
	{
		porting_thread_destroy(hThreadHandle);
		hThreadHandle = 0;
	}
	m_logger->output(LOG_INFO, "AF mutex_destroy %#x %#x", hThreadHandle, m_mutex);
	if (m_mutex)
	{
		porting_mutex_destroy(m_mutex);
		m_mutex = 0;
	}
	m_logger->output(LOG_INFO, "AF free_mem %#x %#x", m_y16Array, m_mutex);
	if (m_y16Array)
	{
		porting_free_mem(m_y16Array);
		m_y16Array = 0;
	}
#endif // MULTI_THREAD_FUNCTION
}

void AutoFocus::AF_Auto(NI_U8 para_stop_cal_mode)
{
	NI_U8   loop_i;
	NI_U8   autofocus_status;
	NI_U8   autofocus_try_cnt_left;

	//自动调焦开始，调焦运行标记拉高，调焦失败标记拉低
	autofocuspara.autofocus_fail = 0;
	autofocuspara.autofocus_engage = 1;

	//校准模式，起点为远焦
	if (para_stop_cal_mode == 3)
	{
		m_logger->output(LOG_INFO, "[%s:%d] Auto Focus Engage, mode is %d", NAME_FILE_AUTOFOCUS_C, __LINE__, para_stop_cal_mode);
		//归200ms位置
		AF_GetStepFar(autofocuspara.max_autofocus_time_cnt * 2);
		//状态机初始值
		autofocus_status = STATE_AUTOFOCUS_GET_NEAR;
		//调焦将停时的步数获取成功标记清零
		autofocuspara.autofocus_getparastopsucess = 0;
	}
	//正常模式，起点为远焦
	else if (para_stop_cal_mode == 1)
	{
		//归0ms位置
		AF_GetStepFar(autofocuspara.max_autofocus_time_cnt);
		//状态机初始值
		autofocus_status = STATE_AUTOFOCUS_GET_NEAR;
	}
	//校准模式，起点为近焦
	else if (para_stop_cal_mode == 2)
	{
		m_logger->output(LOG_INFO, "[%s:%d] Auto Focus Engage, mode is %d", NAME_FILE_AUTOFOCUS_C, __LINE__, para_stop_cal_mode);
		//归0ms位置
		AF_GetStepNear(autofocuspara.max_autofocus_time_cnt * 2);
		//状态机初始值
		autofocus_status = STATE_AUTOFOCUS_GET_FAR;
		//调焦将停时的步数获取成功标记清零
		autofocuspara.autofocus_getparastopsucess = 0;
	}
	//正常模式，起点为近焦
	else
	{
		//归0ms位置
		AF_GetStepNear(autofocuspara.max_autofocus_time_cnt);
		//状态机初始值
		autofocus_status = STATE_AUTOFOCUS_GET_FAR;
	}

	autofocuspara.autofocus_sharp_value = 0;
	autofocuspara.autofocus_sharp_value_max = 0;
	autofocus_try_cnt_left = 40;

	//校准模式,起点为远焦 以及 正常调焦,起点为远焦
	if ((para_stop_cal_mode == 3) | (para_stop_cal_mode == 1))
	{
		//刚开始走3步，用于获得清晰函数值变化趋势
		for (loop_i = 0; loop_i < 3; loop_i++)
		{
			AF_GetStepNear(autofocuspara.run_autofocus_time_cnt);
			porting_thread_sleep(autofocuspara.wait_autofocus_time_cnt);
			AF_GetTotalValue();
			m_logger->output(LOG_INFO, "[%s:%d] Auto Focus will begin, status is %d, time_cnt is %d", NAME_FILE_AUTOFOCUS_C, __LINE__, autofocus_status, autofocuspara.autofocus_time_cnt);
		}
	}
	//校准模式,起点为近焦 以及 正常模式,起点为近焦
	else
	{
		//刚开始走3步，用于获得清晰函数值变化趋势
		for (loop_i = 0; loop_i < 3; loop_i++)
		{
			AF_GetStepFar(autofocuspara.run_autofocus_time_cnt);
			porting_thread_sleep(autofocuspara.wait_autofocus_time_cnt);
			AF_GetTotalValue();
			m_logger->output(LOG_INFO, "[%s:%d] Auto Focus will begin, status is %d, time_cnt is %d", NAME_FILE_AUTOFOCUS_C, __LINE__, autofocus_status, autofocuspara.autofocus_time_cnt);
		}
	}

	//进入开环自动调焦循环判断，当调焦到位或者尝试次数降为0，跳出while循环
	while ((autofocus_status != 0) & (autofocus_try_cnt_left > 0))
	{
		autofocus_try_cnt_left--;
		switch (autofocus_status)
		{
		case STATE_AUTOFOCUS_GET_NEAR:   //镜头外推，调近焦
			AF_GetStepNear(autofocuspara.run_autofocus_time_cnt);
			porting_thread_sleep(autofocuspara.wait_autofocus_time_cnt);
			AF_GetTotalValue();
			m_logger->output(LOG_INFO, "[%s:%d] Auto Focus get near, status is %d, time_cnt is %d", NAME_FILE_AUTOFOCUS_C, __LINE__, autofocus_status, autofocuspara.autofocus_time_cnt);
			//边界，换方向
			if ((autofocuspara.autofocus_time_cnt >= autofocuspara.max_autofocus_time_cnt - 5) & (abs((int)autofocuspara.autofocus_sharp_value - (int)autofocuspara.autofocus_sharp_value_max) < autofocuspara.diff_min_autofocus_sharp_value))
			{
				autofocus_status = STATE_AUTOFOCUS_GET_NEAR_SWITCH;
			}
			else
			{
				//找到峰值
				if ((autofocuspara.autofocus_sharp_value_pre0 > autofocuspara.autofocus_sharp_value) & (autofocuspara.autofocus_sharp_value_pre0 > autofocuspara.autofocus_sharp_value_pre1))
				{
					autofocus_status = STATE_AUTOFOCUS_GET_NEAR_JUDGE;
				}
				//调过，换方向
				else if ((autofocuspara.autofocus_sharp_value < autofocuspara.autofocus_sharp_value_pre0) & (autofocuspara.autofocus_sharp_value_pre0 < autofocuspara.autofocus_sharp_value_pre1))
				{
					autofocus_status = STATE_AUTOFOCUS_GET_NEAR_SWITCH;
				}
				else if (autofocuspara.autofocus_time_cnt >= autofocuspara.max_autofocus_time_cnt - 5)
				{
					autofocus_status = STATE_AUTOFOCUS_GET_NEAR_SWITCH;
				}
			}
			break;
		case STATE_AUTOFOCUS_GET_NEAR_SWITCH:
			for (loop_i = 0; loop_i < 2; loop_i++)
			{
				AF_GetStepFar(autofocuspara.run_autofocus_time_cnt);
				porting_thread_sleep(autofocuspara.wait_autofocus_time_cnt);
				AF_GetTotalValue();
				m_logger->output(LOG_INFO, "[%s:%d] Auto Focus near2far, status is %d, time_cnt is %d", NAME_FILE_AUTOFOCUS_C, __LINE__, autofocus_status, autofocuspara.autofocus_time_cnt);
			}
			autofocus_status = STATE_AUTOFOCUS_GET_FAR;
			break;
		case STATE_AUTOFOCUS_GET_NEAR_JUDGE:
			AF_GetStepNear(autofocuspara.run_autofocus_time_cnt);
			porting_thread_sleep(autofocuspara.wait_autofocus_time_cnt);
			AF_GetTotalValue();
			m_logger->output(LOG_INFO, "[%s:%d] Auto Focus get near, status is %d, time_cnt is %d", NAME_FILE_AUTOFOCUS_C, __LINE__, autofocus_status, autofocuspara.autofocus_time_cnt);
			//调到位，退出
			if ((autofocuspara.autofocus_time_cnt >= autofocuspara.max_autofocus_time_cnt - 5) & (abs((int)autofocuspara.autofocus_sharp_value - (int)autofocuspara.autofocus_sharp_value_max) < autofocuspara.diff_min_autofocus_sharp_value))
			{
				autofocus_status = STATE_AUTOFOCUS_GET_STOP;
			}
			else
			{
				//找到峰值
				if ((autofocuspara.autofocus_sharp_value_pre0 > autofocuspara.autofocus_sharp_value) & (autofocuspara.autofocus_sharp_value_pre0 > autofocuspara.autofocus_sharp_value_pre1))
				{
					autofocus_status = STATE_AUTOFOCUS_GET_NEAR_JUDGE;
				}
				//调过，换方向，增加抖动判断
				else if (((autofocuspara.autofocus_sharp_value < autofocuspara.autofocus_sharp_value_pre0) & (autofocuspara.autofocus_sharp_value_pre0 < autofocuspara.autofocus_sharp_value_pre1)) |
					((autofocuspara.autofocus_sharp_value > autofocuspara.autofocus_sharp_value_pre0) & (autofocuspara.autofocus_sharp_value_pre0 < autofocuspara.autofocus_sharp_value_pre1) & (autofocuspara.autofocus_sharp_value < autofocuspara.autofocus_sharp_value_pre1)))
				{
					autofocus_status = STATE_AUTOFOCUS_GET_NEAR_WILL_STOP;
				}
				//恢复
				else
				{
					autofocus_status = STATE_AUTOFOCUS_GET_NEAR;
				}
			}
			break;
		case STATE_AUTOFOCUS_GET_NEAR_WILL_STOP:
			for (loop_i = 0; loop_i < ((para_stop_cal_mode == 3) ? MAX_AUTOFOCUS_PARA_CAL_CNT : autofocuspara.cnt_autofocus_get_near_will_stop); loop_i++)
			{
				AF_GetStepFar(autofocuspara.run_autofocus_time_cnt);
				porting_thread_sleep((para_stop_cal_mode == 3) ? ((autofocuspara.autofocus_getparastopsucess == 0) ? 1000 : 1) : autofocuspara.wait_autofocus_time_cnt);
				AF_GetTotalValue();
				if (para_stop_cal_mode == 3)
				{
					AF_GetParaStop(loop_i, STATE_AUTOFOCUS_GET_NEAR_WILL_STOP);
				}
				m_logger->output(LOG_INFO, "[%s:%d] Auto Focus will stop, status is %d, time_cnt is %d", NAME_FILE_AUTOFOCUS_C, __LINE__, autofocus_status, autofocuspara.autofocus_time_cnt);
			}
			autofocus_status = STATE_AUTOFOCUS_GET_STOP;
			break;
		case STATE_AUTOFOCUS_GET_FAR:   //镜头回拉，调远焦
			AF_GetStepFar(autofocuspara.run_autofocus_time_cnt);
			porting_thread_sleep(autofocuspara.wait_autofocus_time_cnt);
			AF_GetTotalValue();
			m_logger->output(LOG_INFO, "[%s:%d] Auto Focus get far, status is %d, time_cnt is %d", NAME_FILE_AUTOFOCUS_C, __LINE__, autofocus_status, autofocuspara.autofocus_time_cnt);
			//边界，换方向
			if ((autofocuspara.autofocus_time_cnt <= 5) & (abs((int)autofocuspara.autofocus_sharp_value - (int)autofocuspara.autofocus_sharp_value_max) < autofocuspara.diff_min_autofocus_sharp_value))
			{
				autofocus_status = STATE_AUTOFOCUS_GET_FAR_SWITCH;
			}
			else
			{
				//找到峰值
				if ((autofocuspara.autofocus_sharp_value_pre0 > autofocuspara.autofocus_sharp_value) & (autofocuspara.autofocus_sharp_value_pre0 > autofocuspara.autofocus_sharp_value_pre1))
				{
					autofocus_status = STATE_AUTOFOCUS_GET_FAR_JUDGE;
				}
				//调过，换方向
				else if ((autofocuspara.autofocus_sharp_value < autofocuspara.autofocus_sharp_value_pre0) & (autofocuspara.autofocus_sharp_value_pre0 < autofocuspara.autofocus_sharp_value_pre1))
				{
					autofocus_status = STATE_AUTOFOCUS_GET_FAR_SWITCH;
				}
				else if (autofocuspara.autofocus_time_cnt <= 0)
				{
					autofocus_status = STATE_AUTOFOCUS_GET_FAR_SWITCH;
				}
			}
			break;
		case STATE_AUTOFOCUS_GET_FAR_SWITCH:
			for (loop_i = 0; loop_i < 2; loop_i++)
			{
				AF_GetStepNear(autofocuspara.run_autofocus_time_cnt);
				porting_thread_sleep(autofocuspara.wait_autofocus_time_cnt);
				AF_GetTotalValue();
				m_logger->output(LOG_INFO, "[%s:%d] Auto Focus far2near, status is %d, time_cnt is %d", NAME_FILE_AUTOFOCUS_C, __LINE__, autofocus_status, autofocuspara.autofocus_time_cnt);
			}
			autofocus_status = STATE_AUTOFOCUS_GET_NEAR;
			break;
		case STATE_AUTOFOCUS_GET_FAR_JUDGE:
			AF_GetStepFar(autofocuspara.run_autofocus_time_cnt);
			porting_thread_sleep(autofocuspara.wait_autofocus_time_cnt);
			AF_GetTotalValue();
			m_logger->output(LOG_INFO, "[%s:%d] Auto Focus get far, status is %d, time_cnt is %d", NAME_FILE_AUTOFOCUS_C, __LINE__, autofocus_status, autofocuspara.autofocus_time_cnt);
			//调到位，退出
			if ((autofocuspara.autofocus_time_cnt <= 5) & (abs((int)autofocuspara.autofocus_sharp_value - (int)autofocuspara.autofocus_sharp_value_max) < autofocuspara.diff_min_autofocus_sharp_value))
			{
				autofocus_status = STATE_AUTOFOCUS_GET_STOP;
			}
			else
			{
				//找到峰值
				if ((autofocuspara.autofocus_sharp_value_pre0 > autofocuspara.autofocus_sharp_value) & (autofocuspara.autofocus_sharp_value_pre0 > autofocuspara.autofocus_sharp_value_pre1))
				{
					autofocus_status = STATE_AUTOFOCUS_GET_FAR_JUDGE;
				}
				//调过，换方向，增加抖动判断
				else if (((autofocuspara.autofocus_sharp_value < autofocuspara.autofocus_sharp_value_pre0) & (autofocuspara.autofocus_sharp_value_pre0 < autofocuspara.autofocus_sharp_value_pre1)) |
					((autofocuspara.autofocus_sharp_value > autofocuspara.autofocus_sharp_value_pre0) & (autofocuspara.autofocus_sharp_value_pre0 < autofocuspara.autofocus_sharp_value_pre1) & (autofocuspara.autofocus_sharp_value < autofocuspara.autofocus_sharp_value_pre1)))
				{
					autofocus_status = STATE_AUTOFOCUS_GET_FAR_WILL_STOP;
				}
				//恢复
				else
				{
					autofocus_status = STATE_AUTOFOCUS_GET_FAR;
				}
			}
			break;
		case STATE_AUTOFOCUS_GET_FAR_WILL_STOP:
			for (loop_i = 0; loop_i < ((para_stop_cal_mode == 2) ? MAX_AUTOFOCUS_PARA_CAL_CNT : autofocuspara.cnt_autofocus_get_far_will_stop); loop_i++)
			{
				AF_GetStepNear(autofocuspara.run_autofocus_time_cnt);
				porting_thread_sleep((para_stop_cal_mode == 2) ? ((autofocuspara.autofocus_getparastopsucess == 0) ? 1000 : 1) : autofocuspara.wait_autofocus_time_cnt);
				AF_GetTotalValue();
				if (para_stop_cal_mode == 2)
				{
					AF_GetParaStop(loop_i, STATE_AUTOFOCUS_GET_FAR_WILL_STOP);
				}
				m_logger->output(LOG_INFO, "[%s:%d] Auto Focus will stop, status is %d, time_cnt is %d", NAME_FILE_AUTOFOCUS_C, __LINE__, autofocus_status, autofocuspara.autofocus_time_cnt);
			}
			autofocus_status = STATE_AUTOFOCUS_GET_STOP;
			break;
		default:
			break;
		}
	}

	m_logger->output(LOG_INFO, "[%s:%d] Auto Focus finish, status is %d, sharp_value is %d, sharp_value_max is %d, try_cnt_left is %d", NAME_FILE_AUTOFOCUS_C, __LINE__, autofocus_status, autofocuspara.autofocus_sharp_value, autofocuspara.autofocus_sharp_value_max, autofocus_try_cnt_left);

	//自动调焦结束，调焦运行标记拉低，调焦失败标记根据情况标记
	autofocuspara.autofocus_fail = (autofocus_try_cnt_left == 0) ? 1 : 0;
	autofocuspara.autofocus_engage = 0;
}

void AutoFocus::AF_Auto_Simple(NI_U8 para_stop_cal_mode)
{
	NI_U8   loop_i;
	NI_U8   autofocus_status;
	NI_U8   autofocus_try_cnt_left;

	//自动调焦开始，调焦运行标记拉高，调焦失败标记拉低
	autofocuspara.autofocus_fail = 0;
	autofocuspara.autofocus_engage = 1;

	//校准模式，起点为远焦
	if (para_stop_cal_mode == 3)
	{
		m_logger->output(LOG_INFO, "[%s:%d] Auto Focus Engage, mode is %d", NAME_FILE_AUTOFOCUS_C, __LINE__, para_stop_cal_mode);
		//归200ms位置
		AF_GetStepFar(autofocuspara.max_autofocus_time_cnt * 2);
		//状态机初始值
		autofocus_status = STATE_AUTOFOCUS_GET_NEAR;
		//调焦将停时的步数获取成功标记清零
		autofocuspara.autofocus_getparastopsucess = 0;
	}
	//正常模式，起点为远焦
	else if (para_stop_cal_mode == 1)
	{
		//归0ms位置
		AF_GetStepFar(autofocuspara.max_autofocus_time_cnt);
		//状态机初始值
		autofocus_status = STATE_AUTOFOCUS_GET_NEAR;
	}
	//校准模式，起点为近焦
	else if (para_stop_cal_mode == 2)
	{
		m_logger->output(LOG_INFO, "[%s:%d] Auto Focus Engage, mode is %d", NAME_FILE_AUTOFOCUS_C, __LINE__, para_stop_cal_mode);
		//归0ms位置
		AF_GetStepNear(autofocuspara.max_autofocus_time_cnt * 2);
		//状态机初始值
		autofocus_status = STATE_AUTOFOCUS_GET_FAR;
		//调焦将停时的步数获取成功标记清零
		autofocuspara.autofocus_getparastopsucess = 0;
	}
	//正常模式，起点为近焦
	else
	{
		//归0ms位置
		AF_GetStepNear(autofocuspara.max_autofocus_time_cnt);
		//状态机初始值
		autofocus_status = STATE_AUTOFOCUS_GET_FAR;
	}

	autofocuspara.autofocus_sharp_value = 0;
	autofocuspara.autofocus_sharp_value_max = 0;
	autofocus_try_cnt_left = 40;

	//校准模式,起点为远焦 以及 正常调焦,起点为远焦
	if ((para_stop_cal_mode == 3) | (para_stop_cal_mode == 1))
	{
		//刚开始走3步，用于获得清晰函数值变化趋势
		for (loop_i = 0; loop_i < 4; loop_i++)
		{
			AF_GetStepNear(autofocuspara.run_autofocus_time_cnt);
			porting_thread_sleep(autofocuspara.wait_autofocus_time_cnt);
			AF_GetTotalValue();
			m_logger->output(LOG_INFO, "[%s:%d] Auto Focus will begin, status is %d, time_cnt is %d", NAME_FILE_AUTOFOCUS_C, __LINE__, autofocus_status, autofocuspara.autofocus_time_cnt);
		}
	}
	//校准模式,起点为近焦 以及 正常模式,起点为近焦
	else
	{
		//刚开始走3步，用于获得清晰函数值变化趋势
		for (loop_i = 0; loop_i < 4; loop_i++)
		{
			AF_GetStepFar(autofocuspara.run_autofocus_time_cnt);
			porting_thread_sleep(autofocuspara.wait_autofocus_time_cnt);
			AF_GetTotalValue();
			m_logger->output(LOG_INFO, "[%s:%d] Auto Focus will begin, status is %d, time_cnt is %d", NAME_FILE_AUTOFOCUS_C, __LINE__, autofocus_status, autofocuspara.autofocus_time_cnt);
		}
	}

	//进入开环自动调焦循环判断，当调焦到位或者尝试次数降为0，跳出while循环
	while ((autofocus_status != 0) & (autofocus_try_cnt_left > 0))
	{
		autofocus_try_cnt_left--;
		switch (autofocus_status)
		{
		case STATE_AUTOFOCUS_GET_NEAR:   //镜头外推，调近焦
			AF_GetStepNear(autofocuspara.run_autofocus_time_cnt);
			porting_thread_sleep(autofocuspara.wait_autofocus_time_cnt);
			AF_GetTotalValue();
			m_logger->output(LOG_INFO, "[%s:%d] Auto Focus get near, status is %d, time_cnt is %d", NAME_FILE_AUTOFOCUS_C, __LINE__, autofocus_status, autofocuspara.autofocus_time_cnt);
			//找到峰值
			if ((autofocuspara.autofocus_sharp_value_pre0 > autofocuspara.autofocus_sharp_value) & (autofocuspara.autofocus_sharp_value_pre0 > autofocuspara.autofocus_sharp_value_pre1))
			{
				autofocus_status = STATE_AUTOFOCUS_GET_NEAR_WILL_STOP;
			}
			//调过，换方向
			else if ((autofocuspara.autofocus_sharp_value < autofocuspara.autofocus_sharp_value_pre0) & (autofocuspara.autofocus_sharp_value_pre0 < autofocuspara.autofocus_sharp_value_pre1))
			{
				autofocus_status = STATE_AUTOFOCUS_GET_NEAR_SWITCH;
			}
			//清晰值3帧稳定
			if (autofocuspara.autofocus_sharp_value_stable)
			{
				autofocus_status = STATE_AUTOFOCUS_GET_NEAR_WILL_STOP;
			}
			//边界
			else if (autofocuspara.autofocus_time_cnt >= autofocuspara.max_autofocus_time_cnt - 5)
			{
				if (autofocuspara.autofocus_sharp_value_stable)
				{
					autofocus_status = STATE_AUTOFOCUS_GET_NEAR_WILL_STOP;
				}
				else
				{
					autofocus_status = STATE_AUTOFOCUS_GET_NEAR_EDGE;
				}
			}
			break;
		case STATE_AUTOFOCUS_GET_NEAR_SWITCH:
			for (loop_i = 0; loop_i < 3; loop_i++)
			{
				AF_GetStepFar(autofocuspara.run_autofocus_time_cnt);
				porting_thread_sleep(autofocuspara.wait_autofocus_time_cnt);
				// osa_msleep(autofocuspara.min_autofocus_time_cnt);
				AF_GetTotalValue();
				m_logger->output(LOG_INFO, "[%s:%d] Auto Focus near2far, status is %d, time_cnt is %d", NAME_FILE_AUTOFOCUS_C, __LINE__, autofocus_status, autofocuspara.autofocus_time_cnt);
			}
			autofocus_status = STATE_AUTOFOCUS_GET_FAR;
			break;
		case STATE_AUTOFOCUS_GET_NEAR_WILL_STOP:
			for (loop_i = 0; loop_i < ((para_stop_cal_mode == 3) ? MAX_AUTOFOCUS_PARA_CAL_CNT : autofocuspara.cnt_autofocus_get_near_will_stop); loop_i++)
			{
				AF_GetStepFar(autofocuspara.run_autofocus_time_cnt);
				porting_thread_sleep((para_stop_cal_mode == 3) ? ((autofocuspara.autofocus_getparastopsucess == 0) ? 1000 : 1) : autofocuspara.wait_autofocus_time_cnt);
				AF_GetTotalValue();
				if (para_stop_cal_mode == 3)
				{
					AF_GetParaStop(loop_i, STATE_AUTOFOCUS_GET_NEAR_WILL_STOP);
				}
				m_logger->output(LOG_INFO, "[%s:%d] Auto Focus will stop, status is %d, time_cnt is %d", NAME_FILE_AUTOFOCUS_C, __LINE__, autofocus_status, autofocuspara.autofocus_time_cnt);
			}
			autofocus_status = STATE_AUTOFOCUS_GET_STOP;
			break;
		case STATE_AUTOFOCUS_GET_NEAR_EDGE:
			for (loop_i = 0; loop_i < 1; loop_i++)
			{
				AF_GetStepFar(autofocuspara.run_autofocus_time_cnt);
				porting_thread_sleep(autofocuspara.wait_autofocus_time_cnt);
				AF_GetTotalValue();
				m_logger->output(LOG_INFO, "[%s:%d] Auto Focus NEAR_EDGE, status is %d, time_cnt is %d", NAME_FILE_AUTOFOCUS_C, __LINE__, autofocus_status, autofocuspara.autofocus_time_cnt);
			}
			autofocus_status = STATE_AUTOFOCUS_GET_STOP;
			break;
		case STATE_AUTOFOCUS_GET_FAR:   //镜头回拉，调远焦
			AF_GetStepFar(autofocuspara.run_autofocus_time_cnt);
			porting_thread_sleep(autofocuspara.wait_autofocus_time_cnt);
			AF_GetTotalValue();
			m_logger->output(LOG_INFO, "[%s:%d] Auto Focus get far, status is %d, time_cnt is %d", NAME_FILE_AUTOFOCUS_C, __LINE__, autofocus_status, autofocuspara.autofocus_time_cnt);
			//找到峰值
			if ((autofocuspara.autofocus_sharp_value_pre0 > autofocuspara.autofocus_sharp_value) & (autofocuspara.autofocus_sharp_value_pre0 > autofocuspara.autofocus_sharp_value_pre1))
			{
				autofocus_status = STATE_AUTOFOCUS_GET_FAR_WILL_STOP;
			}
			//调过，换方向
			else if ((autofocuspara.autofocus_sharp_value < autofocuspara.autofocus_sharp_value_pre0) & (autofocuspara.autofocus_sharp_value_pre0 < autofocuspara.autofocus_sharp_value_pre1))
			{
				autofocus_status = STATE_AUTOFOCUS_GET_FAR_SWITCH;
			}
			//清晰值3帧稳定
			if (autofocuspara.autofocus_sharp_value_stable)
			{
				autofocus_status = STATE_AUTOFOCUS_GET_FAR_WILL_STOP;
			}
			//边界
			else if (autofocuspara.autofocus_time_cnt <= 5)
			{
				if (autofocuspara.autofocus_sharp_value_stable)
				{
					autofocus_status = STATE_AUTOFOCUS_GET_NEAR_WILL_STOP;
				}
				else
				{
					autofocus_status = STATE_AUTOFOCUS_GET_FAR_EDGE;
				}
			}
			break;
		case STATE_AUTOFOCUS_GET_FAR_SWITCH:
			for (loop_i = 0; loop_i < 3; loop_i++)
			{
				AF_GetStepNear(autofocuspara.run_autofocus_time_cnt);
				porting_thread_sleep(autofocuspara.wait_autofocus_time_cnt);
				AF_GetTotalValue();
				m_logger->output(LOG_INFO, "[%s:%d] Auto Focus far2near, status is %d, time_cnt is %d", NAME_FILE_AUTOFOCUS_C, __LINE__, autofocus_status, autofocuspara.autofocus_time_cnt);
			}
			autofocus_status = STATE_AUTOFOCUS_GET_NEAR;
			break;
		case STATE_AUTOFOCUS_GET_FAR_WILL_STOP:
			for (loop_i = 0; loop_i < ((para_stop_cal_mode == 2) ? MAX_AUTOFOCUS_PARA_CAL_CNT : autofocuspara.cnt_autofocus_get_far_will_stop); loop_i++)
			{
				AF_GetStepNear(autofocuspara.run_autofocus_time_cnt);
				porting_thread_sleep((para_stop_cal_mode == 2) ? ((autofocuspara.autofocus_getparastopsucess == 0) ? 1000 : 1) : autofocuspara.wait_autofocus_time_cnt);
				AF_GetTotalValue();
				if (para_stop_cal_mode == 2)
				{
					AF_GetParaStop(loop_i, STATE_AUTOFOCUS_GET_FAR_WILL_STOP);
				}
				m_logger->output(LOG_INFO, "[%s:%d] Auto Focus will stop, status is %d, time_cnt is %d", NAME_FILE_AUTOFOCUS_C, __LINE__, autofocus_status, autofocuspara.autofocus_time_cnt);
			}
			autofocus_status = STATE_AUTOFOCUS_GET_STOP;
			break;
		case STATE_AUTOFOCUS_GET_FAR_EDGE:
			for (loop_i = 0; loop_i < 1; loop_i++)
			{
				AF_GetStepNear(autofocuspara.run_autofocus_time_cnt);
				porting_thread_sleep(autofocuspara.wait_autofocus_time_cnt);
				AF_GetTotalValue();
				m_logger->output(LOG_INFO, "[%s:%d] Auto Focus FAR_EDGE, status is %d, time_cnt is %d", NAME_FILE_AUTOFOCUS_C, __LINE__, autofocus_status, autofocuspara.autofocus_time_cnt);
			}
			autofocus_status = STATE_AUTOFOCUS_GET_STOP;
			break;
		default:
			autofocus_status = STATE_AUTOFOCUS_GET_STOP;
			break;
		}
	}

	m_logger->output(LOG_INFO, "[%s:%d] Auto Focus finish, status is %d, sharp_value is %d, sharp_value_max is %d, try_cnt_left is %d", NAME_FILE_AUTOFOCUS_C, __LINE__, autofocus_status, autofocuspara.autofocus_sharp_value, autofocuspara.autofocus_sharp_value_max, autofocus_try_cnt_left);
}

void AutoFocus::AF_Init()
{
	ReadData_AFPARA();
	// pwmctrl.init();
	PWM_UpEn(); //pwmctrl.far();
	porting_thread_sleep(autofocuspara.max_autofocus_time_cnt + 300);
	PWM_Stop(); //pwmctrl.stop();

	autofocuspara.autofocus_time_cnt = 0;

	AF_GetStepNear(autofocuspara.max_autofocus_time_cnt);

	m_logger->output(LOG_INFO, "[%s:%d] Auto Focus initial , time_cnt is %d", NAME_FILE_AUTOFOCUS_C, __LINE__, autofocuspara.autofocus_time_cnt);
}

void AutoFocus::AF_Para_Init()
{
	autofocuspara.max_autofocus_time_cnt = MAX_AUTOFOCUS_TIME_CNT;
	autofocuspara.min_autofocus_time_cnt = MIN_AUTOFOCUS_TIME_CNT;
	autofocuspara.run_autofocus_time_cnt = RUN_AUTOFOCUS_TIME_CNT;
	autofocuspara.wait_autofocus_time_cnt = WAIT_AUTOFOCUS_TIME_CNT;
	autofocuspara.diff_min_autofocus_sharp_value = DIFF_MIN_AUTOFOCUS_SHARP_VALUE;
	autofocuspara.cnt_autofocus_get_near_will_stop = CNT_AUTOFOCUS_GET_NEAR_WILL_STOP;
	autofocuspara.cnt_autofocus_get_far_will_stop = CNT_AUTOFOCUS_GET_FAR_WILL_STOP;
}
/**
* @brief:  计算图像清晰度值。结果越大，图像清晰度越高。
* @param:  unsigned short* pSrc 图像缓存地址
* @param:  int width	图像宽
* @param:  int height	图像高
* @return: 图像清晰度值。
**/
int AutoFocus::laplacianY16() {
	int result = 0;
#ifdef MULTI_THREAD_FUNCTION

	int start_x = 64;
	int start_y = 48;
	int end_x = 192;	//    256/2=128
	int end_y = 144;		//    192/2=96
	int index = (start_y - 1) * m_width;

	if (!m_y16Array)
		return result;
	//m_logger->output(LOG_INFO, "laplacianY16 mutex_lock %#x %#x", m_y16Array, m_mutex);
	porting_mutex_lock(m_mutex);
	for (int i = start_y; i < end_y; i++) 
	{
		for (int j = start_x; j < end_x; j++) 
		{
			result += abs(m_y16Array[i*m_width + j + index] - m_y16Array[i*m_width + j - 1 + index]);
			result += abs(m_y16Array[i*m_width + j + index] - m_y16Array[(i - 1)*m_width + j + index]);
		}
	}
	//m_logger->output(LOG_INFO, "laplacianY16 mutex_unlock %#x %#x", m_y16Array, m_mutex);
	porting_mutex_unlock(m_mutex);
	result = result / 2;
#endif
	return result;
}
NI_S32 AutoFocus::AF_GetTotalValue()
{
	/*获取清晰度值*/
	if (autofocuspara.autofocus_sharp_value_max <= autofocuspara.autofocus_sharp_value)
	{
		autofocuspara.autofocus_sharp_value_max = autofocuspara.autofocus_sharp_value;
		autofocuspara.diff_min_autofocus_sharp_value = (NI_U16)autofocuspara.autofocus_sharp_value_max / 20;
		autofocuspara.autofocus_time_cnt_at_af_max = autofocuspara.autofocus_time_cnt;
	}
	autofocuspara.autofocus_sharp_value_pre1 = autofocuspara.autofocus_sharp_value_pre0;
	autofocuspara.autofocus_sharp_value_pre0 = autofocuspara.autofocus_sharp_value;
	autofocuspara.autofocus_sharp_value = laplacianY16();
	autofocuspara.autofocus_sharp_value = (autofocuspara.autofocus_sharp_value * 1000) / 7;
	m_logger->output(LOG_INFO, "sharp_value                =<%d>", autofocuspara.autofocus_sharp_value);
	m_logger->output(LOG_INFO, "sharp_value_pre0           =<%d>", autofocuspara.autofocus_sharp_value_pre0);
	m_logger->output(LOG_INFO, "sharp_value_pre1           =<%d>", autofocuspara.autofocus_sharp_value_pre1);
	m_logger->output(LOG_INFO, "sharp_value_max            =<%d>", autofocuspara.autofocus_sharp_value_max);
	m_logger->output(LOG_INFO, "diff_min_sharp_value_max   =<%d>", autofocuspara.diff_min_autofocus_sharp_value);
	return autofocuspara.autofocus_sharp_value;
}

void AutoFocus::AF_GetParaStop(NI_U8 current_loop, NI_U8 current_status)
{
	if ((autofocuspara.autofocus_sharp_value_pre0 > autofocuspara.autofocus_sharp_value) &
		(autofocuspara.autofocus_sharp_value_pre0 > autofocuspara.autofocus_sharp_value_pre1) &
		(abs((int)autofocuspara.autofocus_sharp_value_pre0 - (int)autofocuspara.autofocus_sharp_value_max) < autofocuspara.diff_min_autofocus_sharp_value) &
		(autofocuspara.autofocus_getparastopsucess == 0))
	{
		m_logger->output(LOG_INFO, "[%s:%d] Auto Focus ParaStopGet, Current Loop is %d, Current Status is %d", NAME_FILE_AUTOFOCUS_C, __LINE__, current_loop, current_status);
		if (current_status == STATE_AUTOFOCUS_GET_FAR_WILL_STOP)
		{
			autofocuspara.cnt_autofocus_get_far_will_stop = current_loop;
		}
		else if (current_status == STATE_AUTOFOCUS_GET_NEAR_WILL_STOP)
		{
			autofocuspara.cnt_autofocus_get_near_will_stop = current_loop;
		}
		autofocuspara.autofocus_getparastopsucess = 1;
		WriteData_AFPARA();
	}
}

void AutoFocus::AF_VcmInit()
{
	// NI_U32 i;

	PWM_TotalFar_1(); //pwmctrl.totalfar1();
}

int AutoFocus::AF_GetMax()
{
	/*获取清晰度最大值*/
	NI_U32 i;

	NI_U32 u32Sharpness_Value[60];
	NI_U32 sMax = 0;
	i = 0;

	for (i = 0; i < 60; i++)
	{
		PWM_TotalNear(); //pwmctrl.totalnear();
		// u32Sharpness_Value[i] = AF_GetTotalValue();
		u32Sharpness_Value[i] = AF_GetTotalValue();
		m_logger->output(LOG_INFO, "[%s:%d]:Sharpness_Value = <%d>,i = <%d>", NAME_FILE_AUTOFOCUS_C, __LINE__, u32Sharpness_Value[i], i);
		porting_thread_sleep(30);
		sMax = (u32Sharpness_Value[i] > sMax) ? u32Sharpness_Value[i] : sMax;
		//sMax = gdmax(u32Sharpness_Value[i], sMax);
	}
	m_logger->output(LOG_INFO, "[%s:%d]:Max_val = <%d>", NAME_FILE_AUTOFOCUS_C, __LINE__, sMax);

	return sMax;
}

void AutoFocus::AF_VCM_Down()
{
	/*返回找清晰点*/
	NI_U32 val;
	NI_U32 j;
	NI_U32 val_down;
	NI_U32 u32Sharpness_Value1[60];

	// val = AF_GetMax();
	val = AF_GetMax();

	for (j = 0; j < 60; j++)
	{
		PWM_TotalFar(); //pwmctrl.totalfar();
		// u32Sharpness_Value1[j] = AF_GetTotalValue();
		u32Sharpness_Value1[j] = AF_GetTotalValue();
		m_logger->output(LOG_INFO, "[%s:%d]:Sharpness_Value1 = <%d>,j = <%d>", NAME_FILE_AUTOFOCUS_C, __LINE__, u32Sharpness_Value1[j], j);

		val_down = u32Sharpness_Value1[j];

		if ((val_down < (val + 100)) && ((val - 100) < val_down))
		{
			m_logger->output(LOG_INFO, "[%s:%d]:val_down = <%d>", NAME_FILE_AUTOFOCUS_C, __LINE__, val_down);
			break;
		}
	}
}

void AutoFocus::AF_GetStepNear(int step_time)
{
	PWM_DownEn();// pwmctrl.near();
	porting_thread_sleep(step_time);
	PWM_Stop();// pwmctrl.stop();
	if (autofocuspara.autofocus_time_cnt <= autofocuspara.max_autofocus_time_cnt - step_time)
	{
		autofocuspara.autofocus_time_cnt += (NI_U16)step_time;
	}
	else
	{
		autofocuspara.autofocus_time_cnt = autofocuspara.max_autofocus_time_cnt;
	}
}

void AutoFocus::AF_GetStepFar(int step_time)
{
	PWM_UpEn();// pwmctrl.far();
	porting_thread_sleep(step_time);
	PWM_Stop();// pwmctrl.stop();
	if (autofocuspara.autofocus_time_cnt >= step_time)
	{
		autofocuspara.autofocus_time_cnt -= (NI_U16)step_time;
	}
	else
	{
		autofocuspara.autofocus_time_cnt = 0;
	}
}

void AutoFocus::AF_GetNear()
{
	PWM_DownEn();// pwmctrl.near();
	porting_thread_sleep(330);
	PWM_Stop();// pwmctrl.stop();
}

void AutoFocus::AF_GetFar()
{
	PWM_UpEn();// pwmctrl.far();
	porting_thread_sleep(330);
	PWM_Stop();// pwmctrl.stop();
}

void AutoFocus::AF_VcmSet()
{
	NI_U32 i;
	// NI_U32 j;
	// NI_U32 val;
	NI_U32 u32Pwm_Count[60];
	// NI_U32 u32Pwm_Count1[60];

	// NI_U32 sMax = 0;

	i = 0;
	for (i = 0; i < 60; i++)
	{
		porting_thread_sleep(15);
		PWM_TotalFar(); //pwmctrl.totalfar();

		porting_thread_sleep(20);

		// u32Pwm_Count[i] = AF_GetTotalValue();
		u32Pwm_Count[i] = AF_GetTotalValue();
		m_logger->output(LOG_INFO, "[%s:%d]:countxxxxxxxxxxxx = <%d>,i = <%d>", NAME_FILE_AUTOFOCUS_C, __LINE__, u32Pwm_Count[i], i);

	}
	m_logger->output(LOG_INFO, "[%s:%d]:af_count = <%d>", NAME_FILE_AUTOFOCUS_C, __LINE__, i);
}

void AutoFocus::AF_VcmSet1()
{
	NI_U32 i;
	// NI_U32 j;
	// NI_U32 val;
	NI_U32 u32Pwm_Count[60];
	// NI_U32 u32Pwm_Count1[60];

	// NI_U32 sMax = 0;

	i = 0;
	for (i = 0; i < 60; i++)
	{
		porting_thread_sleep(15);
		PWM_TotalNear(); //pwmctrl.totalnear();

		porting_thread_sleep(20);

		// u32Pwm_Count[i] = AF_GetTotalValue();
		u32Pwm_Count[i] = AF_GetTotalValue();
		m_logger->output(LOG_INFO, "[%s:%d]:countxxxxxxxxxxxx = <%d>,i = <%d>", NAME_FILE_AUTOFOCUS_C, __LINE__, u32Pwm_Count[i], i);

	}
	m_logger->output(LOG_INFO, "[%s:%d]:af_count = <%d>", NAME_FILE_AUTOFOCUS_C, __LINE__, i);
}

void AutoFocus::PWM_Stop()
{
	//pwmctrl.drive(PWM_DURATION_UP, 0);
	//GPIO_SetLevel(55, 0);   //SD拉低
	PWM_Drive(PWM_DURATION_UP, 0);
}

void AutoFocus::PWM_UpEn()
{
	//GPIO_SetLevel(55, 1);   //SD拉高
	//pwmctrl.drive(PWM_DURATION_UP, 1);
	PWM_Drive(PWM_DURATION_UP, 1);
}

void AutoFocus::PWM_DownEn()
{
	//GPIO_SetLevel(55, 1);   //SD拉高
	//pwmctrl.drive(PWM_DURATION_DOWN, 1);
	PWM_Drive(PWM_DURATION_DOWN, 1);
}

void AutoFocus::PWM_Drive(int dutyration, int engage)
{
	if (!m_registry || !m_registry->PWMDrive)
		return;
	m_registry->PWMDrive(m_cameraID, dutyration, engage, m_registry->userParam);
}

void AutoFocus::PWM_Init()
{
	PWM_UpEn(); //pwmctrl.far();
	porting_thread_sleep(autofocuspara.max_autofocus_time_cnt + 300);
	PWM_Stop(); //pwmctrl.stop();
}

void AutoFocus::PWM_TotalFar()
{
}

void AutoFocus::PWM_TotalNear()
{
}

void AutoFocus::PWM_TotalFar_1()
{
	PWM_UpEn(); //pwmctrl.far();
	porting_thread_sleep(1000);
	PWM_Stop(); //pwmctrl.stop();
}

void AutoFocus::PWM_TotalNear_1()
{
	PWM_DownEn(); //pwmctrl.near();
	porting_thread_sleep(1000);
	PWM_Stop(); //pwmctrl.stop();
}
//EEPROM单字节读，返回读取的数据
NI_U32 AutoFocus::eeprom_iic_rd(NI_U16 addr)
{
	NI_U32  eeprom_iic_rd_data = 0;
	unsigned char data = 0;
	ITA_RESULT ret;
	if (!m_registry || !m_registry->EEPROMRead)
		return 0;
	ret = m_registry->EEPROMRead(m_cameraID, addr, &data, m_registry->userParam);
	if(ITA_OK == ret)
		eeprom_iic_rd_data = data;
	return eeprom_iic_rd_data;
}

//EEPROM单字节写
void AutoFocus::eeprom_iic_wr(NI_U16 addr, NI_U8 data)
{
	if (!m_registry || !m_registry->EEPROMWrite)
		return;
	m_registry->EEPROMWrite(m_cameraID, addr, data, m_registry->userParam);
}
//读出EEPROM里的自动聚焦数据
void AutoFocus::ReadData_AFPARA()
{
	NI_U16  loop_i;
	NI_U16  buffer_len;
	NI_U8   read_buffer[IIC_EEPROM_AF_DATALEN_MAX];
	NI_U32  iic_read_date_temp;

	buffer_len = 0;
	iic_read_date_temp = 0;
	//! 读取数据长度信息
	iic_read_date_temp = eeprom_iic_rd(IIC_EEPROM_REGADDR_AFDATA_LEN);
	porting_thread_sleep(10);
	buffer_len = iic_read_date_temp & 0xff;
	iic_read_date_temp = eeprom_iic_rd(IIC_EEPROM_REGADDR_AFDATA_LEN + 1);
	porting_thread_sleep(10);
	buffer_len = ((iic_read_date_temp & 0xff) << 8) | buffer_len;
	//! 数据长度判断，如果超出长度则不读数据
	if (buffer_len > IIC_EEPROM_AF_DATALEN_MAX)
	{
		buffer_len = 0;
	}

	m_logger->output(LOG_INFO, "[%s:%d]:Read AF para from EEPROM, buffer_len = %d", NAME_FILE_EEPROM_C, __LINE__, buffer_len);

	if (buffer_len == 0)
	{
		AF_Para_Init();
	}
	else
	{
		for (loop_i = 0; loop_i < buffer_len; loop_i++)
		{
			iic_read_date_temp = eeprom_iic_rd((IIC_EEPROM_REGADDR_AFDATA + loop_i * 1));
			read_buffer[loop_i] = iic_read_date_temp & 0xff;
			m_logger->output(LOG_INFO, "[%s:%d]:addr = %04x, iic_read_date = 0x%02x", NAME_FILE_EEPROM_C, __LINE__, loop_i, iic_read_date_temp);
			porting_thread_sleep(10);
		}

		autofocuspara.max_autofocus_time_cnt = read_buffer[IIC_EEPROM_REGADDR_MAX_AUTOFOCUS_TIME_CNT] | (read_buffer[IIC_EEPROM_REGADDR_MAX_AUTOFOCUS_TIME_CNT + 1] << 8);
		autofocuspara.min_autofocus_time_cnt = read_buffer[IIC_EEPROM_REGADDR_MIN_AUTOFOCUS_TIME_CNT];
		autofocuspara.run_autofocus_time_cnt = read_buffer[IIC_EEPROM_REGADDR_RUN_AUTOFOCUS_TIME_CNT];
		autofocuspara.wait_autofocus_time_cnt = autofocuspara.min_autofocus_time_cnt - autofocuspara.run_autofocus_time_cnt;
		autofocuspara.diff_min_autofocus_sharp_value = read_buffer[IIC_EEPROM_REGADDR_DIFF_MIN_AUTOFOCUS_SHARP_VALUE] | (read_buffer[IIC_EEPROM_REGADDR_DIFF_MIN_AUTOFOCUS_SHARP_VALUE + 1] << 8);
		autofocuspara.cnt_autofocus_get_near_will_stop = read_buffer[IIC_EEPROM_REGADDR_CNT_AUTOFOCUS_GET_NEAR_WILL_STOP];
		autofocuspara.cnt_autofocus_get_far_will_stop = read_buffer[IIC_EEPROM_REGADDR_CNT_AUTOFOCUS_GET_FAR_WILL_STOP];
	}

}

void AutoFocus::WriteData_AFPARA()
{
	NI_U16  loop_i;
	NI_U8   write_buffer[IIC_EEPROM_AF_DATALEN_MAX];
	//! 打印数据长度信息
	//! 将数据写入数组
	write_buffer[IIC_EEPROM_REGADDR_MAX_AUTOFOCUS_TIME_CNT] = (autofocuspara.max_autofocus_time_cnt & 0xff);
	write_buffer[IIC_EEPROM_REGADDR_MAX_AUTOFOCUS_TIME_CNT + 1] = ((autofocuspara.max_autofocus_time_cnt >> 8) & 0xff);
	write_buffer[IIC_EEPROM_REGADDR_MIN_AUTOFOCUS_TIME_CNT] = autofocuspara.min_autofocus_time_cnt;
	write_buffer[IIC_EEPROM_REGADDR_RUN_AUTOFOCUS_TIME_CNT] = autofocuspara.run_autofocus_time_cnt;
	write_buffer[IIC_EEPROM_REGADDR_DIFF_MIN_AUTOFOCUS_SHARP_VALUE] = (autofocuspara.diff_min_autofocus_sharp_value & 0xff);
	write_buffer[IIC_EEPROM_REGADDR_DIFF_MIN_AUTOFOCUS_SHARP_VALUE + 1] = ((autofocuspara.diff_min_autofocus_sharp_value >> 8) & 0xff);
	write_buffer[IIC_EEPROM_REGADDR_CNT_AUTOFOCUS_GET_NEAR_WILL_STOP] = autofocuspara.cnt_autofocus_get_near_will_stop;
	write_buffer[IIC_EEPROM_REGADDR_CNT_AUTOFOCUS_GET_FAR_WILL_STOP] = autofocuspara.cnt_autofocus_get_far_will_stop;
	//! 打印数据长度信息
	m_logger->output(LOG_INFO, "[%s:%d]:Write AF para to EEPROM, buffer_Len = %d", NAME_FILE_EEPROM_C, __LINE__, IIC_EEPROM_AF_DATALEN_MAX);
	//! 写取数据长度信息及数据
	eeprom_iic_wr(IIC_EEPROM_REGADDR_AFDATA_LEN, (IIC_EEPROM_AF_DATALEN_MAX & 0xff));
	porting_thread_sleep(10);
	eeprom_iic_wr(IIC_EEPROM_REGADDR_AFDATA_LEN + 1, ((IIC_EEPROM_AF_DATALEN_MAX >> 8) & 0xff));
	porting_thread_sleep(10);

	for (loop_i = 0; loop_i < IIC_EEPROM_AF_DATALEN_MAX; loop_i++)
	{
		eeprom_iic_wr((IIC_EEPROM_REGADDR_AFDATA + loop_i), write_buffer[loop_i]);
		// m_logger->output(LOG_INFO, "[%s:%d]:addr = %04x, iic_write_date = 0x%02x",NAME_FILE_EEPROM_C,__LINE__,(IIC_EEPROM_REGADDR_AFDATA + loop_i),write_buffer[loop_i]);
		porting_thread_sleep(10);
	}
}

#ifdef MULTI_THREAD_FUNCTION
void AutoFocus::AFRunningFunc(void * param)
{
	AutoFocus* af = (AutoFocus*)param;
	while (af->isRunning)
	{
		if (af->actionType > 0)
		{
			//af->m_logger->output(LOG_INFO, "AFRunning start.%d", af->actionType);
			switch (af->actionType)
			{
			case ITA_AUTO_FOCUS:			//自动调焦
				af->AF_Auto_Simple((NI_U8)af->actionParam);
				//调焦结果应该通知
				break;
			case ITA_AF_NEAR:				//调最近焦
				break;
			case ITA_AF_FAR:				//调最远焦
				break;
			case ITA_STEP_NEAR:				//调整一个时间周期步长近焦
				break;
			case ITA_STEP_FAR:				//调整一个时间周期步长远焦
				break;
			default:
				break;
			}
			af->actionType = (ITA_BC_TYPE)0;
			//af->m_logger->output(LOG_INFO, "AFRunning end.%d", af->actionType);
		}
		else
			porting_thread_sleep(10);
	}
	if (af->postCb)
		af->postCb(af->m_cameraID);
}
#endif // MULTI_THREAD_FUNCTION

ITA_RESULT AutoFocus::AFControl(ITA_BC_TYPE type, void * param)
{
#ifdef MULTI_THREAD_FUNCTION
	//m_logger->output(LOG_INFO, "AFControl.%#x", hThreadHandle);
	if (!hThreadHandle)
	{
		AF_Init();
		m_mutex = porting_mutex_create();
		//m_logger->output(LOG_INFO, "AFControl.mutex_create %#x", m_mutex);
		isRunning = true;
		hThreadHandle = porting_thread_create(AFRunningFunc, this, 0, 0);
	}
	//m_logger->output(LOG_INFO, "AFControl.2 %#x %d", hThreadHandle, actionType);
	//AF过程中不响应其它AF操作。
	if (actionType)
	{
		m_logger->output(LOG_WARN, "AutoFocus is busying.%d %d", actionType, type);
		return ITA_INACTIVE_CALL;
	}
	//m_logger->output(LOG_INFO, "AFControl.3 %#x %#x", hThreadHandle, param);
	if (ITA_AUTO_FOCUS == type && param && *(int *)param >= 0 && *(int *)param <= 3)
		actionParam = *(int *)param;
	//m_logger->output(LOG_INFO, "AFControl.4 %#x", hThreadHandle);
	actionType = type;
	return ITA_OK;
#else
	return ITA_UNSUPPORT_OPERATION;
#endif // MULTI_THREAD_FUNCTION
}

ITA_RESULT AutoFocus::setRegister(ITARegistry * registry, int cameraID)
{
	if (!registry || !registry->EEPROMRead || !registry->EEPROMWrite || !registry->PWMDrive)
		return ITA_NULL_PTR_ERR;
	m_registry = registry;
	m_cameraID = cameraID;
	return ITA_OK;
}

void AutoFocus::setLogger(GuideLog * logger)
{
	m_logger = logger;
}

ITA_RESULT AutoFocus::refreshFrame(short * y16Array, int w, int h)
{
#ifdef MULTI_THREAD_FUNCTION
	if (!m_y16Array)
	{
		m_y16Array = (short *)porting_calloc_mem(w*h, sizeof(short), ITA_OTHER_MODULE);
		m_width = w;
		m_height = h;
	}
	//m_logger->output(LOG_INFO, "AF refreshFrame lock %#x", m_mutex);
	porting_mutex_lock(m_mutex);
	memcpy(m_y16Array, y16Array, m_width*m_height * 2);
	//m_logger->output(LOG_INFO, "AF refreshFrame unlock %#x", m_mutex);
	porting_mutex_unlock(m_mutex);
#endif
	return ITA_OK;
}

ITA_RESULT AutoFocus::addInterceptor(PostHandle postHandle)
{
	if (!postHandle)
		return ITA_NULL_PTR_ERR;
#ifdef MULTI_THREAD_FUNCTION
	postCb = postHandle;
	return ITA_OK;
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}
