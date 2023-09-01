/*************************************************************
Copyright (C), 2022--2023, Wuhan Guide Sensmart Tech Co., Ltd
File name   : ITARefriger.h
Author      : Guide Sensmart RD. wangyan
Version     : 1.0
Date        : 2022/10/29
Description : Temperature measurement model of refrigeration detector.
*************************************************************/
#ifndef ITA_FUSION_H
#define ITA_FUSION_H

#ifdef __cplusplus
#    define ITA_FUSION_START  extern "C" {
#    define ITA_FUSION_END    };
#else
#    define ITA_REFRIGER_START
#    define ITA_REFRIGER_END
#endif

ITA_FUSION_START

#include "ERROR.h"
#include "ITADTD.h"

//输入图像类型
typedef enum ITAImageType {
	ITA_VL = 0,
	ITA_IR
}ITA_IMAGE_TYPE;

typedef struct ITA_RECTANGLE {
	ITA_POINT leftTop;
	ITA_POINT rightDown;
}ITARectangle;


/**
* @brief:	融合模块初始化。
* @param:	ITA_PIXEL_FORMAT format			可见光、红外图像的数据格式。
* * @param:	ITA_FIELD_ANGLE lensType		图像的镜头类型
* * @param:	int irWidth,int irHeight		红外图像的宽，高。
* * @param:	int vlWidth,int vlHeight		可见光图像的宽，高。
* @param:	unsigned char detail			边缘轮廓强度。
* @param:	const char* fileDir				csv文件的存储路径
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
* @note:
**/
ITA_API HANDLE_T ITA_FusionInit(ITA_PIXEL_FORMAT format, ITA_FIELD_ANGLE lensType,int irWidth,int irHeight,int vlWidth,int vlHeight, unsigned char detail, const char* fileDir);

/**
* @brief:	释放一个融合（ITA）库实例资源。
* @param:	HANDLE_T instance	ITA实例。
* @return:	无。
**/
ITA_API void ITA_FusionUninit(HANDLE_T instance);

/**
* @brief:	图像融合,输入红外与可见光的YUV图像数据，实现图像数据的融合效果
* @param:	HANDLE_T instance	 			实例句柄。
* @param:	unsigned char* irData			YUV格式的红外图像数据。内存由用户申请和释放。
* @param:	unsigned char* visData			YUV格式的可见光图像数据。内存由用户申请和释放。
* @param:	int irWidth					    红外图像数据的宽度。
* @param:	int irHeight					红外图像数据的高度。
* @param:	int visWidth					可见光图像数据的宽度。
* @param:	int visHeight					可见光图像数据的高度。
* @param:	float distance					距离参数,可见光与红外图像选根据距离更新仿射矩阵。范围为1-25米。
* @param:	unsigned char* fusionImg		输出的YUV融合的图像数据。内存由用户申请和释放。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
* @note:
**/
ITA_API ITA_RESULT ITA_ImageFusion(HANDLE_T instance, unsigned char* irData, unsigned char* visData, int irWidth, int irHeight,
	int visWidth, int visHeight, float distance, unsigned char* fusionImg);

/**
* @brief:	圆心提取。
* @param:	HANDLE_T instance	 			实例句柄。
* @param:	unsigned char* data			    探测数据。
* @param:	int width,int height			图像的宽，高。
* @param:	ITA_PIXEL_FORMAT format			图像的格式。
* @param:	const ITARectangle rect        探测的矩形区域。
* @param:	int index						图像的索引。
* @param:	ITA_Image_TYPE imgType		    图像的类型。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
* @note:
**/
ITA_API ITA_RESULT ITA_DetectPoint(HANDLE_T instance,unsigned char* data,int width,int height, ITA_PIXEL_FORMAT format, ITARectangle rect, int index, ITA_IMAGE_TYPE imgType);

/**
* @brief:	仿射矩阵计算。
* @param:	HANDLE_T instance	 			实例句柄。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
* @note:
**/
ITA_API ITA_RESULT ITA_AffineTransTable(HANDLE_T instance);

/**
* @brief:	仿射矩阵旋转与缩放分量平均值。
* @param:	HANDLE_T instance	 			实例句柄。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
* @note:
**/
ITA_API ITA_RESULT ITA_AverageAllMatrix(HANDLE_T instance);

/**
* @brief:	反算仿射矩阵X方向与Y方向偏移量
* @param:	HANDLE_T instance	 		实例句柄。
* @param:	int index					输入图像的索引值。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
* @note:
**/
ITA_API ITA_RESULT ITA_ReverseCalAffineTransTable(HANDLE_T instance,int index);

/**
* @brief:	计算并获取所有标定距离下的X Y 偏移量，100mm为间距。
* @param:	HANDLE_T instance	 		实例句柄。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
* @note:
**/
ITA_API ITA_RESULT ITA_Getinterp1HermitePchip(HANDLE_T instance);

/**
* @brief:	计算画中画的左上角和右下角坐标。
* @param:	HANDLE_T instance	 		实例句柄。
* @param:	float distance				距离参数,可见光与红外图像选根据距离更新仿射矩阵。范围为1-25米。
* @param:	ITARectangle* rect			输出计算的画中画的左上角和右下角坐标。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
* @note:
**/
ITA_API ITA_RESULT ITA_CalculateRect(HANDLE_T instance, float distance, ITARectangle* rect);

/**
* @brief:	设置日志保存的完整路径。注意要在ITA_FusionInit之后调用。
* @param:	HANDLE_T instance	 	  实例句柄。
* @param:	const char* path		  输入保存日志的完整路径。
* @return:	返回值类型：ITA_RESULT。成功，返回ITA_OK；失败，返回值<0，参考ERROR.h。
* @note:
**/
ITA_API ITA_RESULT ITA_SetLoggerPath(HANDLE_T instance,const char* path);

ITA_FUSION_END

#endif // !ITA_FUSION_H