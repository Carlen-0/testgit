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
	ITA_ARG_OUT_OF_RANGE = -2,     //����������Ϸ���������Χ��
	ITA_FILE_NOT_EXIST = -3,       //�ļ������ڡ�
	ITA_CREATE_FILE_FAILURE = -4,  //�����ļ�ʧ�ܡ�
	ITA_ASSERT_FAILURE = -5,       //���Դ���
	ITA_WRONG_PACKAGE_HEADER = -6, //��������ݰ�ͷ�����ݰ����Ϸ���
	ITA_INACTIVE_CALL = -7,        //��Ч�ĵ��á��˴ε�����ITA��ǰ״̬��ͻ���������ڴ���ŵĹ����У��û��ٴε��ÿ��Ż᷵�ش˴���
	ITA_DATA_SIZE_ERROR = -8,      //���ݳ��ȴ���������ļ�ʱƫ���������ļ���С��
	ITA_CHECKSUM_ERROR = -9,       //����У�����
	ITA_DIV_ZERO_ERROR = -10,      //����0����
	ITA_MT_NOT_READY = -11,        //δ�ɹ���ȡ���ݰ������߰���
	ITA_LOG_BUF_OVERFLOW = -12,    //һ����־���ݹ�����������־�����С��
	ITA_LOG_FILE_OPEN_FAILED = -13,//������־�ļ�ʧ�ܡ�
	ITA_FIRST_NUC_NOT_FINISH = -14,//��һ��NUCδ��ɣ���Ҫ��ʾͼ�񣬷�������
	ITA_SHUTTER_CLOSING = -15,     //���Ŵ��ڱպ�״̬�������û���������Ҫ��ʾͼ��
	ITA_HAL_UNREGISTER = -16,      //�û�δע�ắ����
	ITA_CHANGE_GEAR_TIMEOUT = -17, //�ֶ��е���ʱ��
	ITA_INVALIDE_PARAMLINE = -18,  //��Ч�Ĳ��������ݡ�
	ITA_CONF_FILE_TOO_LARGE = -19, //�����ļ����󣬳��������С��
	ITA_BUF_OVERFLOW = -20,        //���ȳ��������С��
	ITA_NO_MT_CONF = -21,          //δ���ò��²�����
	ITA_NO_ISP_CONF = -22,         //δ���ó��������
	ITA_SKIP_FRAME = -23,          //���Դ�֡��������������֡�ʵ�����»���֣���
	ITA_WAIT_TIMEOUT = -24,        //�ȴ���ʱ��
	ITA_ILLEGAL_PATH = -25,        //���Ϸ���·�����������Ϸ����ַ���
	ITA_OUT_OF_MEMORY = -26,       //ϵͳ�ڴ治�㣬���������ڴ����
	ITA_CENTRAL_NOT_READY = -27,   //�û��״ε��ü���������ʱITAδ������
	ITA_ALREADY_SAME_VALUE = -28,  //�û����õ�ֵ��ITA��ǰ״̬��ʹ�õ�ֵ��ͬ��
	ITA_INVALID_BACKGROUND = -29,
	ITA_CHANGE_RANGE = -30,			//�����в��·�Χ���߽��µ�,�����û���������������Ժ���¡�
	/*IMAGE ERROR*/
	ITA_NULL_PTR_ERR = -1001,         //��ָ�����
	ITA_ILLEGAL_PAPAM_ERR = -1002,    //�Ƿ���������
	ITA_ALLOCATE_MEM_ERR = -1003,     //�����ڴ����
	ITA_UNSUPPORT_OPERATION = -1004,  //�ڵ�ǰ������״̬�£���֧�ָò�����
	ITA_BADPOINT_OVERMUCH = -1005,    //�������������Ʒ���Ҫ��
	/*MT ERROR*/
	ITA_NO_PACKAGE = -2001,           //�����ݰ���
	ITA_INVALID_KF_ERROR = -2002,     //��Ч��У�²���KF��
	ITA_INVALID_Y16_ERROR = -2003,    //��Ч��Y16���ݡ�
	ITA_UNINIT_PARAM_ERROR = -2004,   //����δ��ʼ����
	ITA_INVALID_SENSOR_VALUE = -2005, //��Ч�Ĵ�����ֵ��
	ITA_MT_OUT_OF_RANGE = -2006,      //���²���ֵ������Ч��Χ��
	ITA_FPA_TEMP_ARRAY_ERROR = -2007, //���±�ֵ���Ϸ���
	ITA_INVALID_DISTANCE = -2008,     //��Ч�ľ��������
	ITA_INVALID_COORD = -2009,        //��Ч������㡣
	ITA_INVALIDE_ENVIRTEMP = -2010,   //��Ч�Ļ��¡�
	/*HAL ERROR*/
	ITA_READ_PACKAGE_ERROR = -3001,   //�����ݰ�����
	ITA_WRITE_PACKAGE_ERROR = -3002,  //д���ݰ�����
	ITA_READ_CALIBRATE_ERROR = -3003, //��У�²�������
	ITA_WRITE_CALIBRATE_ERROR = -3004,//дУ�²�������
	ITA_I2C_ERROR = -3005,            //I2C�����쳣��
	ITA_SHUTTER_CONTROL_ERROR = -3006,//���ſ����쳣��
	ITA_READ_SENSOR_ERROR = -3007,    //��ȡ�´�����
	ITA_PWM_DRIVE_ERROR = -3008,      //PWM��������
	ITA_READ_AF_DATA_ERROR = -3009,   //EEPROM��������
	ITA_SPI_ERROR = -3010,            //SPI��������
	ITA_AUTOFOCUS_ERROR = -3011,	  //�Խ�����
	ITA_CALIBRATION_ERROR = -3012,	  //У׼����
	/*ToolBox ERROR*/
	ITA_RESPONSE_RATE_INEFFECTIVE = -5001,	//�Զ���Ӧ������δ��Ч
	ITA_END_ERROR_CODES,
	ITA_POLYGON_NOT_SUPPORTED = -6001,
	ITA_POLYGON_AREA_EMPTY
}ITA_RESULT;

#define CHECK_NULL_POINTER(p) {if (!p) \
return ITA_NULL_PTR_ERR; }

#endif // !ITA_ERROR_H
