/*************************************************************
Copyright (C), 2021--2022, Wuhan Guide Sensmart Tech Co., Ltd
File name   : ImageProcessor.cpp
Author      : Guide Sensmart RD. wangyan
Version     : 1.0
Date        : 2021/11/9
Description : Imaging processing module.
*************************************************************/
#include "ImageProcessor.h"

ImageProcessor::ImageProcessor(ITA_MODE mode, int w, int h, float scale, ITA_PIXEL_FORMAT pixelFormat)
{
	m_IrImg = new CInfraredCore(w, h, scale);
	if (ITA_X16 == mode || ITA_MCU_X16 == mode)
	{
		m_IrImg->put_nuc_switch(true);	//非均匀校正
		m_IrImg->put_rpbp_switch(true);	//坏点替换
		m_IrImg->put_tff_switch(true);	//时域滤波
		m_IrImg->put_rn_switch(true);	//去噪
		m_IrImg->put_rvs_switch(true);	//去竖纹，占用CPU最高。
		m_IrImg->put_rhs_switch(false);	//关闭去横纹
	}
	else
	{   /*关闭前端图像算法*/
		m_IrImg->put_nuc_switch(false);	//非均匀校正
		m_IrImg->put_rpbp_switch(false);//坏点替换
		m_IrImg->put_tff_switch(false);	//时域滤波
		m_IrImg->put_rn_switch(false);	//去噪
		m_IrImg->put_rvs_switch(false);	//去竖纹，占用CPU最高。
		m_IrImg->put_rhs_switch(false);	//关闭去横纹
	}
	//图像算法开关默认全部关闭，由用户根据需要打开。
	m_IrImg->put_sp_switch(false);	//关闭锐化
	m_IrImg->put_iie_switch(false);
	//m_IrImg->put_clahe_switch(false);
	m_IrImg->put_gmc_switch(false);
	m_IrImg->put_y8adjustbc_switch(false);
	//调光，默认打开。如果关闭，那么不处理Y16转Y8，不会生成imgDst图像数据。
	m_IrImg->put_drt_switch(true);
	m_IrImg->put_psdclr_switch(true);
	//BGR
	//m_IrImg->put_colorimg_tpye(0);
	//RGB
	//m_IrImg->put_colorimg_tpye(2);
	setPixelFormat(pixelFormat);
	m_mode = mode;
	m_width = w;
	m_height = h;
	m_scale = scale;
	m_logger = NULL;
	memset(&m_defaultConf, 0, sizeof(ITAConfig));
	m_avgB = 0;
	m_snapShot = 0;  //0，初始状态；1，抓当前帧；2，正在录流。
	m_isDrtY8 = true;
	m_equalLineType = ITA_EQUAL_NONE;
	m_drtType = ITA_DRT_MIX;
	m_nucRepeatCorrection = 0; //重复NUC纠错机制。默认0关闭。1，用户打开nuc重复纠错机制。
	m_threshold = 400; //阈值，120模组不大于400，256模组待定。
	m_biCubic = NULL;
	isGuoGai = false;//去锅盖算法默认关闭。
	//m_IrImgInstance = NULL;
}

ImageProcessor::~ImageProcessor()
{
	delete m_IrImg;
	m_IrImg = NULL;
	if (m_biCubic) {
		delete m_biCubic;
		m_biCubic = nullptr;
	}
	/*if (m_IrImgInstance) {
		delete m_IrImgInstance;
		m_IrImgInstance = nullptr;
	}*/
}

ITA_RESULT ImageProcessor::setCurrentK(unsigned short * currentK, int len)
{
	memcpy(m_IrImg->IRContent.pus_gain_mat, currentK, len);
	m_IrImg->put_rpbp_updatalist_switch(true);
	//m_logger->output(LOG_INFO, "setCurrentK. len=%d", len);
	return ITA_OK;
}

//NUC状态：0，初始空状态；1，开始做NUC；2，正在做NUC；3，NUC已完成。
ITA_RESULT ImageProcessor::updateB(short * b, int len, int nucStatus)
{
	ITA_RESULT ret = ITA_OK;
	memcpy(m_IrImg->IRContent.pus_shutter_base, b, len);
	//计算本底均值
	long long sum = 0;
	int size = len / 2;
	for (int i = 0; i < size; i++)
	{
		sum += b[i];
	}
	m_avgB = (short)(sum / size);
	//判断本底均值是否合法
	//打快门后出现麻点，NUC概率性失效解决方案
	if (m_nucRepeatCorrection && 2 == nucStatus)
	{
		long long sumDelta = 0;
		for (int i = 0; i < size; i++) {
			sumDelta += abs(b[i] - m_avgB);
		}
		int avgDelta = (int)(sumDelta / size);
		//<=400,判断正常，适用于120模组；如果用于256模组，那么须修改m_threshold。
		if (avgDelta > m_threshold)
		{
			ret = ITA_INVALID_BACKGROUND;
			m_logger->output(LOG_ERROR, "ITA_INVALID_BACKGROUND. %d %d", avgDelta, m_threshold);
		}
	}
	return ret;
}

short ImageProcessor::getAvgB()
{
	return m_avgB;
}

ITA_RESULT ImageProcessor::getImageInfo(ITA_ROTATE_TYPE rotate, ITA_PIXEL_FORMAT pixelFormat, ITAImgInfo * pInfo)
{
	int realW = m_width, realH = m_height;
	switch (rotate)
	{
	case ITA_ROTATE_NONE:
	case ITA_ROTATE_180:
		realW = m_width;
		realH = m_height;
		break;
	case ITA_ROTATE_90:
	case ITA_ROTATE_270:
		realW = m_height;
		realH = m_width;
		break;
	default:
		break;
	}
	pInfo->w = realW;
	pInfo->h = realH;
	pInfo->y16Len = realW*realH;
	realW = (int)(realW * m_scale);
	realH = (int)(realH * m_scale);
	pInfo->imgW = realW;
	pInfo->imgH = realH;
	switch (pixelFormat)
	{
	case ITA_RGB888:
	case ITA_BGR888:
		pInfo->imgDataLen = realW*realH * 3;
		break;
	case ITA_RGBA8888:
		pInfo->imgDataLen = realW*realH * 4;
		break;
	case ITA_RGB565:
		pInfo->imgDataLen = realW*realH * 2;
		break;
	case ITA_YUV422_YUYV:
	case ITA_YUV422_UYVY:
	case ITA_YUV422_Plane:
	case ITA_YVU422_Plane:
		pInfo->imgDataLen = realW*realH * 2;
		break;
	case ITA_YUV420_YUYV:
	case ITA_YUV420_UYVY:
	case ITA_YUV420_Plane:
	case ITA_YVU420_Plane:
		pInfo->imgDataLen = realW*realH * 3 / 2;
		break;
	default:
		return ITA_UNSUPPORT_OPERATION;
	}
	return ITA_OK;
}

ITA_RESULT ImageProcessor::process(short * srcData, int srcSize, ITAISPResult * pResult)
{
	if (ITA_X16 == m_mode || ITA_MCU_X16 == m_mode)
		memcpy(m_IrImg->IRContent.pus_x16_mat, srcData, srcSize);
	else
		//不是对X16数据做处理，传入的已是Y16.
		memcpy(m_IrImg->IRContent.pst_src_img.pus_data, srcData, srcSize);
	//pResult->y16Data在缩放的情况下可能会变化，在ITA_IMAGE_PROCESS打开或者关闭时会变化，所以每次更新。
	m_IrImg->put_output_y16_switch(true, pResult->y16Data);

	m_IrImg->InfraredImageProcess();
	if(m_isDrtY8)
		memcpy(pResult->imgDst, m_IrImg->IRContent.pst_dst_img.puc_data, pResult->info.imgDataLen);
	//if (ITA_X16 == m_mode || ITA_MCU_X16 == m_mode)
	//{
	//	//算法内部已经拷贝。
	//	memcpy(pResult->y16Data, m_IrImg->pus_output_y16, pResult->info.y16Len * 2);
	//}
	//else
	//{
	//	memcpy(pResult->y16Data, m_IrImg->IRContent.pst_src_img.pus_data, pResult->info.y16Len * 2);
	//}
	//m_snapShot：0，初始状态；1，抓当前帧；2，正在录流；3，停止录流。
	if (1 == m_snapShot)
	{
		if (ITA_X16 == m_mode || ITA_MCU_X16 == m_mode)
		{
			m_debugger->snapShot(SNAP_SHOT_X, (unsigned char*)srcData, srcSize);
			m_debugger->snapShot(SNAP_SHOT_B, (unsigned char*)m_IrImg->IRContent.pus_shutter_base, pResult->info.y16Len * 2);
			m_debugger->snapShot(SNAP_SHOT_K, (unsigned char*)m_IrImg->IRContent.pus_gain_mat, pResult->info.y16Len * 2);
		}
		m_debugger->snapShot(SNAP_SHOT_Y, (unsigned char*)pResult->y16Data, pResult->info.y16Len * 2);
		m_debugger->snapShot(SNAP_SHOT_RGB, pResult->imgDst, pResult->info.imgDataLen);
		m_snapShot = 0;
	}
	else if (2 == m_snapShot)
	{
		m_debugger->snapShot(SNAP_RECORD_Y, (unsigned char*)pResult->y16Data, pResult->info.y16Len * 2);
		if (ITA_X16 == m_mode || ITA_MCU_X16 == m_mode)
		{
			m_debugger->snapShot(SNAP_RECORD_X, (unsigned char*)srcData, srcSize);
		}
	}
	else if (3 == m_snapShot)
	{
		if (ITA_X16 == m_mode || ITA_MCU_X16 == m_mode)
		{
			m_debugger->snapShot(SNAP_STOP_X, NULL, 0);
			m_debugger->snapShot(SNAP_STOP_Y, NULL, 0);
		}
		else
		{
			m_debugger->snapShot(SNAP_STOP_Y, NULL, 0);
		}
		m_snapShot = 0;
	}
	return ITA_OK;
}

ITA_RESULT ImageProcessor::copyY8(short * srcData, int srcSize, ITAISPResult * pResult, unsigned char * y8Data, int y8Size)
{
	//y8Data内存由用户分配后传入，可能会变化，所以每次更新。
	m_IrImg->put_output_y8_switch(true, y8Data);

	return process(srcData, srcSize, pResult);
}

void ImageProcessor::setLogger(GuideLog * logger)
{
	m_logger = logger;
	m_IrImg->setLogger(logger);
}

int ImageProcessor::getPaletteNum()
{
	return m_IrImg->getPaletteTotalNum();
}

ITA_RESULT ImageProcessor::setPaletteIndex(ITA_PALETTE_TYPE paletteIndex)
{
	if (paletteIndex >= getPaletteNum() || paletteIndex < 0)
	{
		m_logger->output(LOG_ERROR, "setPaletteIndex %d max=%d", paletteIndex, getPaletteNum());
		return ITA_ARG_OUT_OF_RANGE;
	}
	m_IrImg->put_psdclr_type(paletteIndex);
	return ITA_OK;
}

ITA_RESULT ImageProcessor::setContrast(int contrast)
{
	if (contrast >= 0 && contrast <= 511)
	{
		m_IrImg->put_linear_contrast(contrast);
		return ITA_OK;
	}
	return ITA_ARG_OUT_OF_RANGE;
}

ITA_RESULT ImageProcessor::setBrightness(int brightness)
{
	if (brightness >= 0 && brightness <= 255)
	{
		m_IrImg->put_linear_brightness(brightness);
		return ITA_OK;
	}
	return ITA_ARG_OUT_OF_RANGE;
}

ITA_RESULT ImageProcessor::setDimmingType(ITA_DRT_TYPE type, ITADRTParam *param)
{
	ITA_RESULT ret;
	if (type < ITA_DRT_LINEAR || type > ITA_DRT_MANUAL)
		return ITA_ARG_OUT_OF_RANGE;
	//手动调光可以重复设置不同参数值。
	if (type != ITA_DRT_MANUAL && type == m_drtType)
		return ITA_INACTIVE_CALL;
	if (ITA_DRT_MANUAL == type && param)
	{
		if (param->manltone_maxY16 <= param->manltone_minY16)
			return ITA_ILLEGAL_PAPAM_ERR;
		m_IrImg->put_manultone_maxy16(param->manltone_maxY16);
		m_IrImg->put_manultone_miny16(param->manltone_minY16);
	}
	else if (ITA_DRT_MANUAL == type && !param)
	{
		m_logger->output(LOG_ERROR, "setDimmingType ITA_DRT_MANUAL NULL.");
		return ITA_ILLEGAL_PAPAM_ERR;
	}
	ret = m_IrImg->put_drt_type(type);
	if (ITA_OK == ret)
		m_drtType = type;
	return ret;
}

ITA_DRT_TYPE ImageProcessor::getDimmingType()
{
	return m_drtType;
}

ITA_RESULT ImageProcessor::setEqualLineType(ITA_EQUAL_LINE_TYPE type, ITAEqualLineParam param)
{
	if (type < ITA_EQUAL_NONE || type > ITA_EQUAL_MIDDLE)
		return ITA_ARG_OUT_OF_RANGE;
	/*if (type == m_equalLineType)
		return ITA_INACTIVE_CALL;*/
	EqualLineMode modeVal;
	modeVal.highY16 = param.highY16;
	modeVal.lowY16 = param.lowY16;
	modeVal.color = param.color;
	modeVal.otherColor = param.otherColor;
	switch (type)
	{
	case ITA_EQUAL_NONE:		//关闭等温线功能
		m_IrImg->put_psdclr_equallinemode(false);
		break;
	case ITA_EQUAL_HIGH:
		modeVal.type = EQUAL_LINE_High;
		m_IrImg->put_psdclr_equallinemode(true);
		m_IrImg->put_psdclr_equallinepara(modeVal);
		break;
	case ITA_EQUAL_LOW:
		modeVal.type = EQUAL_LINE_Low;
		m_IrImg->put_psdclr_equallinemode(true);
		m_IrImg->put_psdclr_equallinepara(modeVal);
		break;
	case ITA_EQUAL_HIGHLOW:
		modeVal.type = EQUAL_LINE_HighLow;
		m_IrImg->put_psdclr_equallinemode(true);
		m_IrImg->put_psdclr_equallinepara(modeVal);
		break;
	case ITA_EQUAL_MIDDLE:
		modeVal.type = EQUAL_LINE_Middle;
		m_IrImg->put_psdclr_equallinemode(true);
		m_IrImg->put_psdclr_equallinepara(modeVal);
		break;
	default:
		break;
	}
	m_equalLineType = type;
	return ITA_OK;
}

ITA_EQUAL_LINE_TYPE ImageProcessor::getEqualLineType()
{
	return m_equalLineType;
}

ITA_RESULT ImageProcessor::setFlip(ITA_FLIP_TYPE type)
{
	ITA_RESULT ret;
	if (type < ITA_FLIP_NONE || type >ITA_FLIP_HOR_VER)
	{
		return ITA_ARG_OUT_OF_RANGE;
	}
	//镜像设置
	if (type != 0)
	{
		m_IrImg->put_flip_switch(true);
		ret = m_IrImg->put_flip_type(type - 1);
	}
	else
	{
		ret = m_IrImg->put_flip_switch(false);
	}
	return ret;
}

ITA_RESULT ImageProcessor::setRotate(ITA_ROTATE_TYPE type)
{
	ITA_RESULT ret;
	if (type<ITA_ROTATE_NONE || type >ITA_ROTATE_270)
		return ITA_ARG_OUT_OF_RANGE;
	//旋转设置
	if (type != 0)
	{
		m_IrImg->put_rotate_switch(true);
		ret = m_IrImg->put_rotate_type(type - 1);//0顺时针旋转90 1顺时针旋转180 2顺时针旋转270
	}
	else
	{
		ret = m_IrImg->put_rotate_switch(false);
	}
	return ret;
}

ITA_RESULT ImageProcessor::setScale(float scale)
{
	ITA_RESULT ret = m_IrImg->put_ImgZoom(scale);
	if (ITA_OK == ret)
		m_scale = scale;
	return ret;
}

ITA_RESULT ImageProcessor::setHsmWeight(int hsmWeight)
{
	return m_IrImg->put_hsm_weight(hsmWeight);
}

ISPParam ImageProcessor::getISPParam()
{
	ISPParam param;
	memset(&param, 0, sizeof(ISPParam));
	m_IrImg->get_tff_switch(&param.isTimeFilter);		//默认0
	m_IrImg->get_rn_switch(&param.isSpatialFilter);		//默认0
	m_IrImg->get_rvs_switch(&param.isRemoveVertical);	//默认0
	m_IrImg->get_rhs_switch(&param.isRemoveHorizontal);	//默认0
	m_IrImg->get_hsm_switch(&param.isHSM);				//默认0
	m_IrImg->get_sp_switch(&param.isSharpening);		//默认0
	m_IrImg->get_iie_switch(&param.isDetailEnhance);	//默认0
	m_IrImg->get_gmc_switch(&param.isGammaCorrection);	//默认0
	m_IrImg->get_y8adjustbc_switch(&param.isY8AdjustBC);//默认0
	m_IrImg->get_drt_switch(&param.isDrtY8);
	m_IrImg->get_linear_brightness(&param.brightness);	//默认20
	m_IrImg->get_linear_contrast(&param.contrast);//默认200
	m_IrImg->get_tff_std(&param.tffStd);
	m_IrImg->get_linear_restrain_rangethre(&param.restrainRange);
	m_IrImg->get_discard_upratio(&param.discardUpratio);
	m_IrImg->get_discard_downratio(&param.discardDownratio);
	m_IrImg->get_heq_plat_thresh(&param.heqPlatThresh);
	m_IrImg->get_heq_range_max(&param.heqRangeMax);
	m_IrImg->get_heq_midvalue(&param.heqMidvalue);
	m_IrImg->get_iie_enhance_coef(&param.iieEnhanceCoef);
	m_IrImg->get_iie_pos_std(&param.iieGaussStd);
	m_IrImg->get_gmc_gamma(&param.gmcGamma);
	m_IrImg->get_gmc_gammatype(&param.gmcType);
	m_IrImg->get_hsm_weight(&param.hsmWeight);
	m_IrImg->get_hsm_stayThr(&param.hsmStayThr);
	m_IrImg->get_hsm_stayWeight(&param.hsmStayWeight);
	m_IrImg->get_hsm_interval(&param.hsmInterval);
	m_IrImg->get_hsm_midfilterSwitch(&param.hsmMidfilter);
	m_IrImg->get_hsm_midfilterWinSize(&param.hsmMidfilterWinsize);
	m_IrImg->get_hsm_delta_upthr(&param.hsmDeltaUpthr);
	m_IrImg->get_hsm_delta_downthr(&param.hsmDeltaDownthr);
	m_IrImg->get_hsm_point_thr(&param.hsmPointThr);
	m_IrImg->get_ImgZoomType(&param.zoomType);
	m_IrImg->get_sp_laplace_weight_y16(&param.laplaceWeight);
	m_IrImg->get_mix_ThrLow(&param.mixThrLow);
	m_IrImg->get_mix_ThrHigh(&param.mixThrHigh);
	m_IrImg->get_linear_restrain_rangethre(&param.linearResrainRange);
	m_IrImg->get_rpbp_auto_switch(&param.isAutoRpBdPs);
	return param;
}

void ImageProcessor::getParamInfo(ITAParamInfo * info)
{
	m_IrImg->get_tff_switch(&info->isTimeFilter);		//默认0
	m_IrImg->get_rn_switch(&info->isSpatialFilter);		//默认0
	m_IrImg->get_rvs_switch(&info->isRemoveVertical);	//默认0
	m_IrImg->get_rhs_switch(&info->isRemoveHorizontal);	//默认0
	m_IrImg->get_hsm_switch(&info->isHsm);				//默认0
	m_IrImg->get_sp_switch(&info->isSharpening);		//默认0
	m_IrImg->get_iie_switch(&info->isDetailEnhance);	//默认0
	m_IrImg->get_gmc_switch(&info->isGammaCorrection);	//默认0
	m_IrImg->get_y8adjustbc_switch(&info->isY8AdjustBC);//默认0
	//m_IrImg->get_drt_switch(&info->isDrtY8);
	m_IrImg->get_linear_brightness(&info->brightness);	//默认20
	m_IrImg->get_linear_contrast(&info->contrast);//默认200
	m_IrImg->get_tff_std(&info->tffStd);
	m_IrImg->get_linear_restrain_rangethre(&info->restrainRange);
	m_IrImg->get_discard_upratio(&info->discardUpratio);
	m_IrImg->get_discard_downratio(&info->discardDownratio);
	m_IrImg->get_heq_plat_thresh(&info->heqPlatThresh);
	m_IrImg->get_heq_range_max(&info->heqRangeMax);
	m_IrImg->get_heq_midvalue(&info->heqMidvalue);
	m_IrImg->get_iie_enhance_coef(&info->iieEnhanceCoef);
	m_IrImg->get_iie_pos_std(&info->iieGaussStd);
	m_IrImg->get_gmc_gamma(&info->gmcGamma);
	m_IrImg->get_gmc_gammatype(&info->gmcType);
	m_IrImg->get_hsm_weight(&info->hsmWeight);
	m_IrImg->get_hsm_stayThr(&info->hsmStayThr);
	m_IrImg->get_hsm_stayWeight(&info->hsmStayWeight);
	m_IrImg->get_hsm_interval(&info->hsmInterval);
	m_IrImg->get_hsm_midfilterSwitch(&info->isHsmMidfilter);
	m_IrImg->get_hsm_midfilterWinSize(&info->hsmMidfilterWinSize);
	m_IrImg->get_hsm_delta_upthr(&info->hsmDeltaUpthr);
	m_IrImg->get_hsm_delta_downthr(&info->hsmDeltaDownthr);
	m_IrImg->get_hsm_point_thr(&info->hsmPointThr);
	m_IrImg->get_ImgZoomType(&info->zoomType);
	m_IrImg->get_sp_laplace_weight_y16(&info->laplaceWeight);
	m_IrImg->get_mix_ThrLow(&info->mixThrLow);
	m_IrImg->get_mix_ThrHigh(&info->mixThrHigh);
	m_IrImg->get_linear_restrain_rangethre(&info->linearRestrainRangethr);
	m_IrImg->get_rpbp_auto_switch(&info->isAutoRpBdPs);
}

ITA_RESULT ImageProcessor::setISPParam(ISPParam param)
{
	m_IrImg->put_tff_switch(param.isTimeFilter);
	m_IrImg->put_rn_switch(param.isSpatialFilter);
	m_IrImg->put_rvs_switch(param.isRemoveVertical);
	m_IrImg->put_rhs_switch(param.isRemoveHorizontal);
	m_IrImg->put_hsm_switch(param.isHSM);
	m_IrImg->put_sp_switch(param.isSharpening);
	m_IrImg->put_iie_switch(param.isDetailEnhance);
	m_IrImg->put_gmc_switch(param.isGammaCorrection);
	m_IrImg->put_y8adjustbc_switch(param.isY8AdjustBC);
	m_IrImg->put_drt_switch(param.isDrtY8);
	if(param.isDrtY8 != m_isDrtY8)
		m_isDrtY8 = param.isDrtY8;
	m_IrImg->put_tff_std(param.tffStd);
	m_IrImg->put_linear_restrain_rangethre(param.restrainRange);
	m_IrImg->put_discard_upratio(param.discardUpratio);
	m_IrImg->put_discard_downratio(param.discardDownratio);
	m_IrImg->put_heq_plat_thresh(param.heqPlatThresh);
	m_IrImg->put_heq_range_max(param.heqRangeMax);
	m_IrImg->put_heq_midvalue(param.heqMidvalue);
	m_IrImg->put_iie_enhance_coef(param.iieEnhanceCoef);
	m_IrImg->put_iie_pos_std(param.iieGaussStd);
	m_IrImg->put_gmc_gamma(param.gmcGamma);
	m_IrImg->put_gmc_gammatype(param.gmcType);
	m_IrImg->put_ImgZoomType(param.zoomType);
	m_IrImg->put_hsm_weight(param.hsmWeight);
	m_IrImg->put_hsm_stayThr(param.hsmStayThr);
	m_IrImg->put_hsm_stayWeight(param.hsmStayWeight);
	m_IrImg->put_sp_laplace_weight_y16(param.laplaceWeight);
	m_IrImg->put_hsm_interval(param.hsmInterval);
	m_IrImg->put_hsm_midfilterSwitch(param.hsmMidfilter);
	m_IrImg->put_hsm_midfilterWinSize(param.hsmMidfilterWinsize);
	m_IrImg->put_hsm_delta_upthr(param.hsmDeltaUpthr);
	m_IrImg->put_hsm_delta_downthr(param.hsmDeltaDownthr);
	m_IrImg->put_hsm_point_thr(param.hsmPointThr);
	m_IrImg->put_mix_ThrLow(param.mixThrLow);
	m_IrImg->put_mix_ThrHigh(param.mixThrHigh);
	m_IrImg->put_linear_restrain_rangethre(param.linearResrainRange);
	m_IrImg->put_rpbp_auto_switch(param.isAutoRpBdPs);
	return ITA_OK;
}

int ImageProcessor::loadConfig(ITAConfig * config, ITA_RANGE range, ITA_FIELD_ANGLE lensType)
{
	//从配置文件中找到对应的一组参数，设置新的值。
	ITAISPConfig *pISPConf;
	int i;
	if (!config->ispConfig || config->ispConfigCount <= 0)
	{
		m_logger->output(LOG_WARN, "ImageProcessor::loadConfig  ITA_NO_ISP_CONF ret = %d ", ITA_NO_ISP_CONF);
		return ITA_NO_ISP_CONF;
	}
	//从配置文件中找到对应的一组参数，设置新的值。
	for (i = 0; i < config->ispConfigCount; i++)
	{
		pISPConf = config->ispConfig + i;
		if (pISPConf->lensType == lensType && pISPConf->mtType == range)
		{
			//pISPConf->lensType;				//ITA_FIELD_ANGLE 视场角类型 0:56°；1:25°；2:120°；3:50°；4:90°；5:33°。
			//pISPConf->mtType;					//测温模式 0：人体；1：工业低温；2：工业高温。
			if(pISPConf->tffStdS == 1)
				m_IrImg->put_tff_std(pISPConf->tffStd);					//时域滤波标准差，控制时域滤波程度，受响应率影响很大，具体产品具体设置。
			if (pISPConf->vStripeWinWidthS == 1)
				m_IrImg->put_rvs_win_width(pISPConf->vStripeWinWidth);		//越小，高频图像含噪越少，去条纹更浅越大，高频图像含噪越多，去条纹更深
			if (pISPConf->vStripeStdS == 1)
				m_IrImg->put_rvs_std(pISPConf->vStripeStd);				//灰度标准差越大，去条纹纹作用越明显
			if (pISPConf->vStripeWeightThreshS == 1)
				m_IrImg->put_rvs_weight_thresh(pISPConf->vStripeWeightThresh);	//值过大，参与计算像素多，条纹去不掉值过小，参与计算像素少，反条纹加重
			if (pISPConf->vStripeDetailThreshS == 1)
				m_IrImg->put_rvs_detail_thresh(pISPConf->vStripeDetailThresh);	//值过小，计算的条纹幅值越小，条纹去不掉值过大，计算的条纹幅值越大，反条纹加重
			if (pISPConf->hStripeWinWidthS == 1)
				m_IrImg->put_rhs_win_width(pISPConf->hStripeWinWidth);		//越小，高频图像含噪越少，去条纹更浅越大，高频图像含噪越多，去条纹更深
			if (pISPConf->hStripeStdS == 1)
				m_IrImg->put_rhs_std(pISPConf->hStripeStd);				//灰度标准差越大，去条纹纹作用越明显
			if (pISPConf->hStripeWeightThreshS == 1)
				m_IrImg->put_rhs_weight_thresh(pISPConf->hStripeWeightThresh);	//值过大，参与计算像素多，条纹去不掉值过小，参与计算像素少，反条纹加重
			if (pISPConf->hStripeDetailThreshS == 1)
				m_IrImg->put_rhs_detail_thresh(pISPConf->hStripeDetailThresh);	//值过小，计算的条纹幅值越小，条纹去不掉值过大，计算的条纹幅值越大，反条纹加重
			if (pISPConf->rnArithTypeS == 1)
				m_IrImg->put_rn_arith_type(pISPConf->rnArithType);			//0为高斯空域滤波， 1为高斯灰度滤波
			if (pISPConf->distStdS == 1)
				m_IrImg->put_rn_dist_std(pISPConf->distStd);				//std越大，滤波效果越强，图像越平滑
			if (pISPConf->grayStdS == 1)
				m_IrImg->put_rn_gray_std(pISPConf->grayStd);				//std越大，滤波效果越强，图像越平滑
			if (pISPConf->discardUpratioS == 1)
				m_IrImg->put_discard_upratio(pISPConf->discardUpratio);			//?控制上抛点比例
			if (pISPConf->discardDownratioS == 1)
				m_IrImg->put_discard_downratio(pISPConf->discardDownratio);		//?控制下抛点比例
			if (pISPConf->linearBrightnessS == 1)
				m_IrImg->put_linear_brightness(pISPConf->linearBrightness);		//值越大亮度越大
			if (pISPConf->linearContrastS == 1)
				m_IrImg->put_linear_contrast(pISPConf->linearContrast);			//值越大对比度越大
			if (pISPConf->linearRestrainRangethreS == 1)
				m_IrImg->put_linear_restrain_rangethre(pISPConf->linearRestrainRangethre); //直接影响小动态范围的对比度参数(均匀面抑制)
			if (pISPConf->heqPlatThreshS == 1)
				m_IrImg->put_heq_plat_thresh(pISPConf->heqPlatThresh);			//影响图像整体对比度
			if (pISPConf->heqRangeMaxS == 1)
				m_IrImg->put_heq_range_max(pISPConf->heqRangeMax);			//值越大，映射范围越大，整体对比度越大
			if (pISPConf->heqMidvalueS == 1)
				m_IrImg->put_heq_midvalue(pISPConf->heqMidvalue);			//影响图像整体亮度
			if (pISPConf->iieEnhanceCoefS == 1)
				m_IrImg->put_iie_enhance_coef(pISPConf->iieEnhanceCoef);			//细节增强系数，值越大，高频叠加效果越明显，但噪声越显著
			//if (pISPConf->iieGrayStdS == 1)
			//	m_IrImg->put_iie_gray_std(pISPConf->iieGrayStd);				//值越大，平滑效果越明显
			if (pISPConf->iieGaussStdS == 1)
				m_IrImg->put_iie_pos_std(pISPConf->iieGaussStd);			//值越大，高频分量越多
			if (pISPConf->iieDetailThrS == 1)
				m_IrImg->put_iie_detail_thr(pISPConf->iieDetailThr);			//细节阈值，值越大增强效果越明显，但噪声越显著
			//if (pISPConf->claheCliplimitS == 1)
			//	m_IrImg->put_clahe_cliplimit(pISPConf->claheCliplimit);			//阈值越大，图像对比度越强
			//if (pISPConf->claheHistMaxS == 1)
			//	m_IrImg->put_tff_switch(pISPConf->claheHistMax);			//值越大，直方图能映射的最大值越大
			//if (pISPConf->claheBlockWidthS == 1)
			//	m_IrImg->put_clahe_block_width(pISPConf->claheBlockWidth);		//列方向分块数量
			//if (pISPConf->claheBlockHeightS == 1)
			//	m_IrImg->put_clahe_block_height(pISPConf->claheBlockHeight);		//行方向分块数量
			if (pISPConf->spLaplaceWeightS == 1)
				m_IrImg->put_sp_laplace_weight_y16(pISPConf->spLaplaceWeight);		//值越大，锐化效果越明显，但会增加噪声。
			//if (pISPConf->gmcTypeS == 1)
			//	m_IrImg->put_tff_switch(pISPConf->gmcType);				//0单Gamma校正；1双Gamma校正。
			if (pISPConf->gmcGammaS == 1)
				m_IrImg->put_gmc_gamma(pISPConf->gmcGamma);				//Gamma校正值
			if (pISPConf->adjustbcBrightS == 1)
				m_IrImg->put_y8adjustbc_bright(pISPConf->adjustbcBright);			//决定Y8图像亮度期望，值越大亮度越大
			if (pISPConf->adjustbcContrastS == 1)
				m_IrImg->put_y8adjustbc_contrast(pISPConf->adjustbcContrast);		//决定Y8图像对比度期望，值越大对比度越大
			if (pISPConf->zoomTypeS == 1)
				m_IrImg->put_ImgZoomType(pISPConf->zoomType);				//0为最邻近插值；1为双线性插值。
			if (pISPConf->mixThrLowS == 1) {
				m_IrImg->put_mix_ThrLow(pISPConf->mixThrLow);
			}
			if (pISPConf->mixThrHighS == 1) {
				m_IrImg->put_mix_ThrHigh(pISPConf->mixThrHigh);
			}
			break;
		}
	}
	if (i >= config->ispConfigCount)
	{
		m_logger->output(LOG_WARN, "ImageProcessor::loadConfig  No matching configuration found. ispConfigCount = %d, ret = %d ", config->ispConfigCount, ITA_NO_ISP_CONF);
		return ITA_NO_ISP_CONF;
	}
	else
		return ITA_OK;
}

void ImageProcessor::setDebugger(Debugger * debugger)
{
	m_debugger = debugger;
}

ITA_RESULT ImageProcessor::control(ITA_DEBUG_TYPE type, void * param)
{
	ITA_RESULT ret = ITA_OK;
	switch (type)
	{
	case ITA_SNAPSHOT:				//拍照保存当前帧图像。
		if (m_snapShot == 1)
		{
			m_logger->output(LOG_WARN, "ITA_SNAPSHOT failed. status=%d", m_snapShot);
			return ITA_INACTIVE_CALL;
		}
		m_snapShot = 1;				//0，初始状态；1，抓当前帧；2，正在录流；3，停止录流。
		break;
	case ITA_START_RECORD:			//开始录X16/Y16数据。
		if (m_snapShot != 0)
		{
			m_logger->output(LOG_WARN, "ITA_START_RECORD failed. status=%d", m_snapShot);
			return ITA_INACTIVE_CALL;
		}
		if (ITA_X16 == m_mode || ITA_MCU_X16 == m_mode)
		{
			m_debugger->snapShot(SNAP_START_X, NULL, 0);
			m_debugger->snapShot(SNAP_START_Y, NULL, 0);
		}
		else
		{
			m_debugger->snapShot(SNAP_START_Y, NULL, 0);
		}
		m_snapShot = 2;
		break;
	case ITA_STOP_RECORD:			//停止录X16/Y16数据。
		if (m_snapShot != 2)
		{
			m_logger->output(LOG_WARN, "ITA_STOP_RECORD failed. status=%d", m_snapShot);
			return ITA_INACTIVE_CALL;
		}
		m_snapShot = 3;
		break;
	default:
		break;
	}
	return ret;
}

ITA_RESULT ImageProcessor::setPixelFormat(ITA_PIXEL_FORMAT pixelFormat)
{
	eCOLORIMG_TYPE type = COLORIMG_TYPE_RGB;

	switch (pixelFormat)
	{
	case ITA_RGB888:			//默认像素格式
		type = COLORIMG_TYPE_RGB;
		break;
	case ITA_BGR888:
		type = COLORIMG_TYPE_BGR;
		break;
	case ITA_RGBA8888:
		type = COLORIMG_TYPE_RGBA;
		break;
	case ITA_RGB565:
		type = COLORIMG_TYPE_RGB565;
		break;
	case ITA_YUV422_YUYV:
		type = COLORIMG_TYPE_YUV422YUYV;
		break;
	case ITA_YUV422_UYVY:
		type = COLORIMG_TYPE_YUV422UYVY;
		break;
	case ITA_YUV422_Plane:
		type = COLORIMG_TYPE_YUV422Plane;
		break;
	case ITA_YVU422_Plane:
		type = COLORIMG_TYPE_YVU422Plane;
		break;
	case ITA_YUV420_YUYV:
		type = COLORIMG_TYPE_YUV420NV12;
		break;
	case ITA_YUV420_UYVY:
		type = COLORIMG_TYPE_YUV420NV21;
		break;
	case ITA_YUV420_Plane:
		type = COLORIMG_TYPE_YUV420PLANE;
		break;
	case ITA_YVU420_Plane:
		type = COLORIMG_TYPE_YVU420PLANE;
		break;
	default:
		break;
	}
	m_IrImg->put_colorimg_tpye(type);
	/*int imgLen;
	m_IrImg->get_colorimg_bytenum(&imgLen);*/
	return ITA_OK;
}

void ImageProcessor::setThreshold(void * threshold)
{
	if (threshold)
	{
		m_threshold = *(int *)threshold; //阈值，120模组不大于400，256模组待定。
		m_logger->output(LOG_INFO, "setThreshold %d %d", m_nucRepeatCorrection, m_threshold);
	}
}

void ImageProcessor::setRepeatCorrection(int repeatCorrection)
{
	m_nucRepeatCorrection = repeatCorrection; //重复NUC纠错机制。默认0关闭。1，用户打开nuc重复纠错机制。
	m_logger->output(LOG_INFO, "setRepeatCorrection %d %d", m_nucRepeatCorrection, m_threshold);
}

ITA_RESULT ImageProcessor::zoomY16(short * srcY16, int srcW, int srcH, short * dstY16, int dstW, int dstH, float times)
{
	if (!srcY16 || !dstY16)
	{
		return ITA_NULL_PTR_ERR;
	}
	if (srcW <= 0 || srcH <= 0 || dstW <= 0 || dstH <= 0 || times <= 0 || times > 20)
	{
		return ITA_ARG_OUT_OF_RANGE;
	}
	if (dstW < srcW*times || dstH < srcH*times)
	{
		return ITA_ILLEGAL_PAPAM_ERR;
	}
	if (times >= 1) {
		m_IrImg->imgBigger(srcY16, srcW, srcH, dstY16, dstW, dstH);
	}
	else {
		m_IrImg->imgSmaller(srcY16, srcW, srcH, dstY16, dstW, dstH);
	}
	return ITA_OK;
}

ITA_RESULT ImageProcessor::processBiCubic(void* dst, void* src, int w, int h, int w_zoom, int h_zoom, ITA_DATA_TYPE dataType)
{
	ITA_RESULT ret = ITA_OK;
	if (!dst || !src)
	{
		return ITA_NULL_PTR_ERR;
	}
	if (w <= 0 || h <= 0 || w_zoom <= 0 || h_zoom <= 0 || w_zoom > w * 8 || h_zoom > h * 8) {
		return ITA_ARG_OUT_OF_RANGE;
	}
	if (dataType < ITA_DATA_Y8 || dataType > ITA_DATA_Y16) {
		return ITA_ARG_OUT_OF_RANGE;
	}
	if (dataType == ITA_DATA_Y8)
	{
		if (!m_biCubic) {
			m_biCubic = new SR_BICUBIC;
		}
		unsigned char* imgdst = (unsigned char*)dst;
		unsigned char* imgsrc = (unsigned char*)src;
		m_biCubic->bicubic(imgdst, imgsrc, w, h, w_zoom, h_zoom);
	}
	if (dataType == ITA_DATA_Y16)
	{
		short* imgdst = (short*)dst;
		short* imgsrc = (short*)src;
		if (!m_biCubic) {
			m_biCubic = new SR_BICUBIC;
		}
		m_biCubic->bicubic(imgdst, imgsrc, w, h, w_zoom, h_zoom);
	}
	return ret;
}

ITA_RESULT ImageProcessor::convertY8(const unsigned char* src, int srcW, int srcH, unsigned char* dst,int dstW, int dstH, int dateLen, ITA_Y8_DATA_MODE datamode)
{
	if (!m_IrImg && !m_IrImg->IRContent.pst_dst_img.puc_data)
	{
		return ITA_ERROR;
	}
	unsigned char* tmp = m_IrImg->IRContent.pst_dst_img.puc_data;
	memset(tmp, 0, dateLen);
	if (datamode == ITA_Y8_XINSHENG)
	{
		m_IrImg->tvRang2FullRange(tmp, src, srcW, srcH);
	}
	else {
		memcpy(tmp, src, srcW * srcH);
	}
	bool rotateSwitch = false;
	m_IrImg->get_rotate_switch(&rotateSwitch);
	int rotateW = srcW;
	int rotateH = srcH;
	if (rotateSwitch)
	{
		int rotateType;
		m_IrImg->get_rotate_type(&rotateType);
		m_IrImg->Rotation(dst, tmp, srcW, srcH, rotateType);
		if (rotateType == 0 || rotateType == 2)
		{
			rotateW = srcH;
			rotateH = srcW;
		}
		memcpy(tmp, dst, dateLen);
	}
	bool flipSwitch = false;
	m_IrImg->get_flip_switch(&flipSwitch);
	if (flipSwitch) {
		int flipType = 0;
		m_IrImg->get_flip_type(&flipType);
		m_IrImg->Flip(dst, tmp, rotateW, rotateH, flipType);
		memcpy(tmp, dst, dateLen);
	}
	int zoomType = Near;
	float zoomTimes = 0.0f;	
	m_IrImg->get_ImgZoom(&zoomTimes);
	if (zoomTimes != 1.0)
	{
		m_IrImg->get_ImgZoomType(&zoomType);
		m_IrImg->Resize(dst, tmp, rotateW, rotateH, zoomTimes, zoomType);
		memcpy(tmp, dst, dstW * dstH);
	}
	int paletteIndex = 0;
	int colorType = 0;
	m_IrImg->get_psdclr_type(&paletteIndex);
	m_IrImg->get_colorimg_tpye(&colorType);
	m_IrImg->PseudoColor(dst, tmp, dstH,dstW, paletteIndex, colorType);
	return ITA_OK;
}

ITA_RESULT ImageProcessor::getColorImage(unsigned char *pY8, int w, int h, unsigned char* pPalette, int paletteLen, unsigned char* pImage, int imgLen)
{
	////关闭伪彩开关
	//if (!m_IrImgInstance) {
	//	m_IrImgInstance = new CInfraredCore(w, h,1);
	//}
	//m_IrImgInstance->put_psdclr_switch(false);
	int imageSize = w * h;
	//memcpy(m_IrImgInstance->IRContent.pst_src_img.pus_data, y16, imageSize*2);
	////处理Y16-Y8,只走调光接口:ModelDRT
	//m_IrImgInstance->InfraredImageProcess();
	//unsigned char* srcData = m_IrImgInstance->IRContent.pst_dst_img.puc_data;
	//根据自定义伪彩信息生成RGB图像
	int temp;
	for (int i = 0; i < imageSize; i++)
	{
		temp = pY8[i];
		pImage[i * 3 + 0] = pPalette[(temp * 4) + 0];
		pImage[i * 3 + 1] = pPalette[(temp * 4) + 1];
		pImage[i * 3 + 2] = pPalette[(temp * 4) + 2];
	}
	return ITA_OK;
}

ITA_RESULT ImageProcessor::customPalette(ITA_PALETTE_OP_TYPE type, void* param)
{
	if (type < ITA_PALETTE_OP_ADD || type > ITA_PALETTE_OP_DELETE) {
		return ITA_ARG_OUT_OF_RANGE;
	}
	ITAPaletteInfo* paletteInfo = (ITAPaletteInfo*)param;
	if (!paletteInfo) {
		return ITA_NULL_PTR_ERR;
	}
	if (!m_IrImg->customPalette)
	{
		m_IrImg->customPalette = new CustomPalette();
	}
	ITA_RESULT result;
	CustomPalette* cusPal = m_IrImg->customPalette;
	unsigned char* data = paletteInfo->data;
	int len = paletteInfo->dataLen;
	int id = paletteInfo->paletteID;
	int totalNum = m_IrImg->getPaletteTotalNum();
	int siglePaletteLen = 256 * 4;
	if (len <= 0 || len % siglePaletteLen != 0) {
		return ITA_ARG_OUT_OF_RANGE;
	}
	if (type == ITA_PALETTE_OP_ADD) {
		if (id < PSEUDONUM || id > totalNum) {
			return ITA_ARG_OUT_OF_RANGE;
		}
		if (!data) {
			return ITA_NULL_PTR_ERR;
		}
		int customPaletteId = id - PSEUDONUM;
		result = cusPal->addCustomPalette(paletteInfo, customPaletteId);
		//cusPal->printCustomPalette();
	}
	else if (type == ITA_PALETTE_OP_UPDATE) {
		//更新伪彩索引
		if (id < PSEUDONUM || id >= totalNum) {
			return ITA_ARG_OUT_OF_RANGE;
		}
		if (!data) {
			return ITA_NULL_PTR_ERR;
		}
		int customPaletteId = id - PSEUDONUM;
		result = cusPal->updateCustomPalette(paletteInfo, customPaletteId);
		//cusPal->printCustomPalette();
	}
	else {
		if (id < PSEUDONUM || id >= totalNum) {
			return ITA_ARG_OUT_OF_RANGE;
		}
		int customPaletteId = id - PSEUDONUM;
		result = cusPal->deleteCustomPalette(paletteInfo, customPaletteId);
		//cusPal->printCustomPalette();
	}
	return result;
}

bool ImageProcessor::getPotCoverSwitch()
{
	return isGuoGai;
}

ITA_RESULT ImageProcessor::putGuoGaiPara(stDGGTPara* guoGaiPara)
{
	return m_IrImg->putGuoGaiTPara(guoGaiPara);
}

ITA_RESULT ImageProcessor::getGuoGaiPara(stDGGTPara* guoGaiPara)
{
	return m_IrImg->getGuoGaiTPara(guoGaiPara);
}

ITA_RESULT ImageProcessor::potCoverRestrain(ITA_POTCOVER_OP_TYPE type, void* para)
{
	if (type > ITA_POTCOVER_GET || type < ITA_POTCOVER_GET_MODEL)
	{
		return ITA_ARG_OUT_OF_RANGE;
	}
	ITA_SWITCH* flag = (ITA_SWITCH*)para;
	ITA_RESULT ret = ITA_ERROR;
	switch (type)
	{
	case ITA_POTCOVER_GET_MODEL:
	{
		ITAPotcoverModelData* modelData = (ITAPotcoverModelData*)para;
		ret = m_IrImg->getGuoGaiModel(modelData->modelData, modelData->width, modelData->height);
		break;
	}
	case ITA_POTCOVER_SET_MODEL:
	{
		ITAPotcoverModelData* modelData = (ITAPotcoverModelData*)para;
		ret = m_IrImg->putGuoGaiModel(modelData->modelData,modelData->width,modelData->height);
		break;
	}
	case ITA_POTCOVER_SET_CALMODEL:
	{
		bool isCalModel;
		if (*flag < ITA_DISABLE || *flag > ITA_ENABLE)
		{
			ret = ITA_ARG_OUT_OF_RANGE;
			break;
		}
		if (*flag == ITA_ENABLE)
		{
			isCalModel = true;
		}
		else {
			isCalModel = false;
		}
		ret = m_IrImg->putCalGuoGaiSwitch(isCalModel);
		break;
	}
	case ITA_POTCOVER_GET_CALMODEL:
	{
		bool isCalModel;
		ret = m_IrImg->getCalGuoGaiSwitch(&isCalModel);
		if (isCalModel) {
			*flag = ITA_ENABLE;
		}
		else {
			*flag = ITA_DISABLE;
		}
		break;
	}
	case ITA_POTCOVER_SET_ALGOTYPE:
	{
		ITA_POTCOVER_ALGO_TYPE* algoType = (ITA_POTCOVER_ALGO_TYPE*)para;
		if (*algoType < ITA_POTCOVER_FIX || *algoType > ITA_POTCOVER_VAR2)
		{
			return ITA_ARG_OUT_OF_RANGE;
		}
		ret = m_IrImg->putGuoGaiArithType(*algoType);
		break;
	}
	case ITA_POTCOVER_GET_ALGOTYPE:
	{
		ITA_POTCOVER_ALGO_TYPE* algoType = (ITA_POTCOVER_ALGO_TYPE*)para;
		int type = 0;
		ret = m_IrImg->getGuoGaiArithType(&type);
		*algoType = (ITA_POTCOVER_ALGO_TYPE)type;
		break;
	}
	case ITA_POTCOVER_SET:
	{
		bool isPotCover;
		if (*flag < ITA_DISABLE || *flag > ITA_ENABLE)
		{
			ret = ITA_ARG_OUT_OF_RANGE;
			break;
		}
		if (*flag == ITA_ENABLE)
		{
			isPotCover = true;
		}
		else {
			isPotCover = false;
		}
		isGuoGai = isPotCover;
		ret = m_IrImg->putGuoGaiSwitch(isPotCover);
		break;
	}
	case ITA_POTCOVER_GET:
	{
		bool isCalModel;
		ret = m_IrImg->getGuoGaiSwitch(&isCalModel);
		if (isCalModel) {
			*flag = ITA_ENABLE;
		}
		else {
			*flag = ITA_DISABLE;
		}
		break;
	}
	default:
		ret = ITA_UNSUPPORT_OPERATION;
		break;
	}
	return ret;
}

bool ImageProcessor::getGainMat(unsigned short* pus_high_base, unsigned short* pus_low_base, unsigned short* pn_gain_mat,
	int n_width, int n_height)
{
	return m_IrImg->getGainMat(pus_high_base, pus_low_base, pn_gain_mat, n_width, n_height);
}

