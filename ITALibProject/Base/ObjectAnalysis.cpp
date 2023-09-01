#include "../ITA.h"
#include "Config.h"
#include "ITAWrapper.h"
#include "../MT/PolygonAnalysiser.h"

/**
* @brief:    ����һ����������
* @param:    HANDLE_T handle             ITAʵ����
* @param:    ITAShape* shape             һ�����飬����һ�����������ζ���
* @param:    int size                    ����ĳ��ȡ�
* @return:   ����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����
**/
ITA_API ITA_RESULT ITA_AnalysisAdd(HANDLE_T handle, ITAShape *shape, int size)
{
#ifdef ULTIMATE_EDITION
    CHECK_NULL_POINTER(handle);
    CHECK_NULL_POINTER(shape);

    ITAWrapper *p = (ITAWrapper *)handle;

    return p->AnalysisAdd(shape, size);
#else
    return ITA_UNSUPPORT_OPERATION;
#endif
}

ITA_API ITA_RESULT ITA_AnalysisGetRangeMask(HANDLE_T instance, ITARangeMask* rangeMask, int size)
{
#ifdef ULTIMATE_EDITION
    CHECK_NULL_POINTER(instance);
    CHECK_NULL_POINTER(rangeMask);
    ITAWrapper* p = (ITAWrapper*)instance;

    return p->AnalysisGetRangeMask(rangeMask,size);
#else
    return ITA_UNSUPPORT_OPERATION;
#endif
}


/**
* @brief:    ��շ�������
* @param:    OA_HANDLE_T handle            ITAʵ����
* @return:   ����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����
**/
ITA_API ITA_RESULT ITA_AnalysisClear(HANDLE_T handle)
{
#ifdef ULTIMATE_EDITION
    CHECK_NULL_POINTER(handle);
    ITAWrapper *p = (ITAWrapper *)handle;

    return p->AnalysisClear();
#else
    return ITA_UNSUPPORT_OPERATION;
#endif
}


/**
* @brief:    �Է��������ڲ�������¶������ദ��,��ȡ����������¶���Ϣ(���ֵ,��Сֵ,ƽ��ֵ)��
* @param:    HANDLE_T instance                 ITAʵ����
* @param:    short* y16Data                    Y16����
* @param:	 int width  int height			   Y16ͼ��ֱ��ʡ�
* @param:	 ITAAnalysisTemp* analysisTemp	   �������:����������¶����飬�ڴ����û�������ͷ�
* @param:	 int size						   ����������¶�����(analysisTemp)�Ĵ�С(size������������ӵķ������������ͬ)
* @return:   ����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����
**/
ITA_API ITA_RESULT ITA_AnalysisTemp(HANDLE_T instance, short* y16Data, int width, int height, ITAAnalysisTemp* analysisTemp, int size)
{
#ifdef ULTIMATE_EDITION
    CHECK_NULL_POINTER(instance);
    CHECK_NULL_POINTER(y16Data);
    CHECK_NULL_POINTER(analysisTemp);

    ITAWrapper* p = (ITAWrapper*)instance;

    return p->AnalysisTemp(y16Data, width, height, analysisTemp, size);
#else
    return ITA_UNSUPPORT_OPERATION;
#endif
}

/**
* @brief:    �Է��������ڲ������Y16�����ദ��,��ȡ���������Y16��Ϣ(���ֵ,��Сֵ,ƽ��ֵ)��
* @param:    HANDLE_T instance							ITAʵ����
* @param:    short* y16Data								Y16���ݡ�
* @param:	 int width  int height						Y16ͼ��ֱ��ʡ�
* @param:    ITAAnalysisY16Info* analysisY16Info        �������:�����ķ�������Y16����,����ÿ���������������Y16,��СY16�Լ�ƽ��Y16,�ڴ����û������ͷ�
* @param:	 int size									���������Y16��Ϣ����(analysisY16Info)�Ĵ�С(size������������ӵķ������������ͬ)
* @return:   ����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����
**/
ITA_API ITA_RESULT ITA_AnalysisY16Info(HANDLE_T instance, short* y16Data, int width, int height, ITAAnalysisY16Info* analysisY16Info, int size)
{
#ifdef ULTIMATE_EDITION
    CHECK_NULL_POINTER(instance);
    CHECK_NULL_POINTER(y16Data);
    CHECK_NULL_POINTER(analysisY16Info);

    ITAWrapper* p = (ITAWrapper*)instance;

    return p->AnalysisY16(y16Data, width, height, analysisY16Info, size);
#else
    return ITA_UNSUPPORT_OPERATION;
#endif
}

/**
* @brief:    �ж�һ�����Ƿ���һ�����������ڲ���
* @param:    ITAShape* shape             һ�����飬����һ������ζ���Ķ�����㡣
* @param:    ITAPoint point              һ�������ꡣ
* @param:    int *flag              	 0����ʾ�㲻�ڷ��������ڡ�  1����ʾ���ڷ��������ڡ�
* @return:   ����ֵ���ͣ�ITA_RESULT���ɹ�������ITA_OK��ʧ�ܣ�����
**/
ITA_API ITA_RESULT ITA_PointInPolygon(ITAShape* shape, ITAPoint point, int* flag)
{
#ifdef ULTIMATE_EDITION
    CHECK_NULL_POINTER(shape);
    CHECK_NULL_POINTER(flag);


    ITA_RESULT ret = ITA_OK;

    PolygonAnalysiser::PolygonEdges item = PolygonAnalysiser::convert(shape);
    *flag = 0;
    *flag = ITA_PointInPolygon_Impl(item, point);

    return ITA_OK;
#else
    return ITA_UNSUPPORT_OPERATION;
#endif
}

