/*************************************************************
Copyright (C), 2021--2022, Wuhan Guide Sensmart Tech Co., Ltd
File name   : ERROR.h
Author      : Guide Sensmart RD. wangyan
Version     : 1.0
Date        : 2021/11/9
Description : ERROR code definition.
*************************************************************/
#ifndef ITA_ERROR_H
#define ITA_ERROR_H

typedef enum Result
{
	ITA_OK = 0,                    //success
	ITA_ERROR = -1,                //general error
	ITA_ARG_OUT_OF_RANGE = -2,     //输入参数不合法，超出范围。
	ITA_FILE_NOT_EXIST = -3,       //文件不存在。
	ITA_CREATE_FILE_FAILURE = -4,  //创建文件失败。
	ITA_ASSERT_FAILURE = -5,       //断言错误。
	ITA_WRONG_PACKAGE_HEADER = -6, //错误的数据包头，数据包不合法。
	ITA_INACTIVE_CALL = -7,        //无效的调用。此次调用与ITA当前状态冲突，例如正在打快门的过程中，用户再次调用快门会返回此错误。
	ITA_DATA_SIZE_ERROR = -8,      //数据长度错误。例如读文件时偏移量超过文件大小。
	ITA_CHECKSUM_ERROR = -9,       //数据校验错误。
	ITA_DIV_ZERO_ERROR = -10,      //除以0错误。
	ITA_MT_NOT_READY = -11,        //未成功读取数据包或曲线包。
	ITA_LOG_BUF_OVERFLOW = -12,    //一行日志内容过长，超出日志缓存大小。
	ITA_LOG_FILE_OPEN_FAILED = -13,//创建日志文件失败。
	ITA_FIRST_NUC_NOT_FINISH = -14,//第一次NUC未完成，不要显示图像，否则花屏。
	ITA_SHUTTER_CLOSING = -15,     //快门处于闭合状态，建议用户定格处理，不要显示图像。
	ITA_HAL_UNREGISTER = -16,      //用户未注册函数。
	ITA_CHANGE_GEAR_TIMEOUT = -17, //手动切档超时。
	ITA_INVALIDE_PARAMLINE = -18,  //无效的参数行数据。
	ITA_CONF_FILE_TOO_LARGE = -19, //配置文件过大，超出缓存大小。
	ITA_BUF_OVERFLOW = -20,        //长度超过缓存大小。
	ITA_NO_MT_CONF = -21,          //未配置测温参数。
	ITA_NO_ISP_CONF = -22,         //未配置成像参数。
	ITA_SKIP_FRAME = -23,          //忽略此帧不做处理（在设置帧率的情况下会出现）。
	ITA_WAIT_TIMEOUT = -24,        //等待超时。
	ITA_ILLEGAL_PATH = -25,        //不合法的路径，包含不合法的字符。
	ITA_OUT_OF_MEMORY = -26,       //系统内存不足，申请分配的内存过大。
	ITA_CENTRAL_NOT_READY = -27,   //用户首次调用计算中心温时ITA未就绪。
	ITA_ALREADY_SAME_VALUE = -28,  //用户设置的值与ITA当前状态下使用的值相同。
	ITA_INVALID_BACKGROUND = -29,
	ITA_CHANGE_RANGE = -30,			//正在切测温范围或者焦温档,建议用户测温做定格处理或稍后测温。
	/*IMAGE ERROR*/
	ITA_NULL_PTR_ERR = -1001,         //空指针错误。
	ITA_ILLEGAL_PAPAM_ERR = -1002,    //非法参数错误。
	ITA_ALLOCATE_MEM_ERR = -1003,     //分配内存错误。
	ITA_UNSUPPORT_OPERATION = -1004,  //在当前的配置状态下，不支持该操作。
	ITA_BADPOINT_OVERMUCH = -1005,    //坏点个数超出产品规格要求。
	/*MT ERROR*/
	ITA_NO_PACKAGE = -2001,           //无数据包。
	ITA_INVALID_KF_ERROR = -2002,     //无效的校温参数KF。
	ITA_INVALID_Y16_ERROR = -2003,    //无效的Y16数据。
	ITA_UNINIT_PARAM_ERROR = -2004,   //参数未初始化。
	ITA_INVALID_SENSOR_VALUE = -2005, //无效的传感器值。
	ITA_MT_OUT_OF_RANGE = -2006,      //测温参数值超出有效范围。
	ITA_FPA_TEMP_ARRAY_ERROR = -2007, //焦温表值不合法。
	ITA_INVALID_DISTANCE = -2008,     //无效的距离参数。
	ITA_INVALID_COORD = -2009,        //无效的坐标点。
	ITA_INVALIDE_ENVIRTEMP = -2010,   //无效的环温。
	/*HAL ERROR*/
	ITA_READ_PACKAGE_ERROR = -3001,   //读数据包错误。
	ITA_WRITE_PACKAGE_ERROR = -3002,  //写数据包错误。
	ITA_READ_CALIBRATE_ERROR = -3003, //读校温参数错误。
	ITA_WRITE_CALIBRATE_ERROR = -3004,//写校温参数错误。
	ITA_I2C_ERROR = -3005,            //I2C操作异常。
	ITA_SHUTTER_CONTROL_ERROR = -3006,//快门控制异常。
	ITA_READ_SENSOR_ERROR = -3007,    //读取温传出错。
	ITA_PWM_DRIVE_ERROR = -3008,      //PWM驱动错误。
	ITA_READ_AF_DATA_ERROR = -3009,   //EEPROM操作错误。
	ITA_SPI_ERROR = -3010,            //SPI操作错误。
	ITA_AUTOFOCUS_ERROR = -3011,	  //对焦错误
	ITA_CALIBRATION_ERROR = -3012,	  //校准错误
	/*ToolBox ERROR*/
	ITA_RESPONSE_RATE_INEFFECTIVE = -5001,	//自动响应率设置未生效
	ITA_END_ERROR_CODES,
	ITA_POLYGON_NOT_SUPPORTED = -6001,
	ITA_POLYGON_AREA_EMPTY
}ITA_RESULT;

#define CHECK_NULL_POINTER(p) {if (!p) \
return ITA_NULL_PTR_ERR; }

#endif // !ITA_ERROR_H
