#include "../ITAFusion.h"
#include "../AppAlg/ImageFusion.h"

ITA_API HANDLE_T ITA_FusionInit(ITA_PIXEL_FORMAT format, ITA_FIELD_ANGLE lensType, int irWidth, int irHeight, int vlWidth, int vlHeight, unsigned char detail, const char* fileDir)
{
#ifdef IMAGE_FUSION_EDITION
	if (format == ITA_YUV420_YUYV || format == ITA_YUV420_UYVY || format == ITA_YUV420_Plane || format == ITA_YVU420_Plane)
	{
		CImgFusion* imgFusion = new CImgFusion(format,lensType, irWidth, irHeight, vlWidth, vlHeight, detail);
		ITA_RESULT ret = imgFusion->ImgFusionInit(fileDir);
		if (ret == ITA_OK)
		{
			return (HANDLE_T)imgFusion;
		}
		else {
			return 0;
		}
		
	}
	else {
		return 0;
	}
#else
	return 0;
#endif
}

ITA_API void ITA_FusionUnit(HANDLE_T instance)
{
#ifdef IMAGE_FUSION_EDITION
	if (!instance)
	{
		return;
	}
	CImgFusion* p = (CImgFusion*)instance;
	delete p;
#endif
}

ITA_API ITA_RESULT ITA_ImageFusion(HANDLE_T instance, unsigned char* irData, unsigned char* visData, int irWidth, int irHeight,
	int visWidth, int visHeight, float distance, unsigned char* fusionImg)
{
#ifdef IMAGE_FUSION_EDITION
	CHECK_NULL_POINTER(instance);
	CHECK_NULL_POINTER(irData);
	CHECK_NULL_POINTER(visData);
	CHECK_NULL_POINTER(fusionImg);
	if (irWidth <= 0 || irHeight <= 0 || visWidth <= 0 || visHeight <= 0) {
		return ITA_ARG_OUT_OF_RANGE;
	}
	ITA_RESULT ret = ITA_OK;
	CImgFusion* p = (CImgFusion*)instance;
	p->RefreshAffineMat(distance);
	ret = p->GetImgData(irData, irWidth, irHeight, visData, visWidth, visHeight);
	if (ret == ITA_OK)
	{
		p->VLAndIRFusYUV();
		p->PutFusImgData(fusionImg);
	}
	return ITA_OK;
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_DetectPoint(HANDLE_T instance, unsigned char* data, int width, int height, ITA_PIXEL_FORMAT format, ITARectangle rect, int index, ITA_IMAGE_TYPE imgType)
{
#ifdef IMAGE_FUSION_EDITION
	CHECK_NULL_POINTER(instance);
	CHECK_NULL_POINTER(data);
	CImgFusion* p = (CImgFusion*)instance;
	if (format == ITA_YUV420_YUYV || format == ITA_YUV420_UYVY || format == ITA_YUV420_Plane || format == ITA_YVU420_Plane)
	{
		return p->DetectPoint(data, width, height, rect, index, imgType);
	}
	else {
		return ITA_ERROR;
	}
	
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_AffineTransTable(HANDLE_T instance)
{
#ifdef IMAGE_FUSION_EDITION
	CHECK_NULL_POINTER(instance);
	CImgFusion* p = (CImgFusion*)instance;
	p->AffineTransTable();
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_AverageAllMatrix(HANDLE_T instance)
{
#ifdef IMAGE_FUSION_EDITION
	CHECK_NULL_POINTER(instance);
	CImgFusion* p = (CImgFusion*)instance;
	p->AverageAllMatrix();
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_ReverseCalAffineTransTable(HANDLE_T instance,int index)
{
#ifdef IMAGE_FUSION_EDITION
	CHECK_NULL_POINTER(instance);
	CImgFusion* p = (CImgFusion*)instance;
	p->ReverseCalAffineTransTable(index);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_Getinterp1HermitePchip(HANDLE_T instance)
{
#ifdef IMAGE_FUSION_EDITION
	CHECK_NULL_POINTER(instance);
	CImgFusion* p = (CImgFusion*)instance;
	return p->Getinterp1HermitePchip();
#else
	return ITA_UNSUPPORT_OPERATION;
#endif

}

ITA_API ITA_RESULT ITA_CalculateRect(HANDLE_T instance, float distance, ITARectangle* rect)
{
#ifdef IMAGE_FUSION_EDITION
	CHECK_NULL_POINTER(instance);
	CHECK_NULL_POINTER(rect);
	CImgFusion* p = (CImgFusion*)instance;
	p->CalculateRect((double)distance, rect);
	return ITA_OK;
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_SetLoggerPath(HANDLE_T instance,const char* path)
{
#ifdef IMAGE_FUSION_EDITION
	CHECK_NULL_POINTER(instance);
	CImgFusion* p = (CImgFusion*)instance;
	p->SetLogger(path);
#else
	return ITA_UNSUPPORT_OPERATION;
#endif
}