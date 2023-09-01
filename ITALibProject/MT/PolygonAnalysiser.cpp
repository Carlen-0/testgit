#include "PolygonAnalysiser.h"
#include "../AppAlg/DeviceTempertrueAnalysis.h"

#include "../Module/GuideLog.h"

using namespace ita;

PolygonAnalysiser::PolygonAnalysiser(int w, int h)
    : m_logger(nullptr),
    m_width(w),
    m_height(h),
    m_size(0),
    m_polygon()
{
}

PolygonAnalysiser::~PolygonAnalysiser()
{
    clear();

    if (m_logger)
    {
        m_logger = nullptr;
    }
}

void PolygonAnalysiser::setLogger(GuideLog* logger)
{
    m_logger = logger;
}

ITA_RESULT PolygonAnalysiser::analysiser(ITA_SHAPE_TYPE type,
    const PolygonEdges& edges, Polygon& ploygon)
{
    ITA_RESULT err = ITA_PolygonAnalysis(edges, type, m_height, m_width,
        ploygon, m_logger);

    return err;

    // 
    // return ITA_PolygonAnalysis_old(obj, m_height, m_width, ploygon);
    // return ITA_PolygonAnalysis_MinRect_old(obj, m_height, m_width, ploygon);
}

ITA_RESULT PolygonAnalysiser::add(const PolygonPool& items)
{
    for (int i = 0; i < items.size(); i++)
    {

#if __cplusplus >= 201103L
        m_polygon.push_back(std::move(items.at(i)));
#else
        m_polygon.push_back(items.at(i));
#endif

    }

    m_size = m_polygon.size();

    return ITA_OK;
}

ITA_RESULT PolygonAnalysiser::clear()
{
    m_polygon.clear();
    // m_temp.clear();
    m_size = 0;

    return ITA_OK;
}

PolygonAnalysiser::Polygon& PolygonAnalysiser::at(size_t i)
{
    return m_polygon.at(i);
}

size_t PolygonAnalysiser::size() const
{
    return m_size;
}

PolygonAnalysiser::PolygonEdges PolygonAnalysiser::convert(const ITAShape* shape)
{
    PolygonEdges edges;
    // edges.reserve(shape->size);

    for (int i = 0; i < shape->size; i++)
    {
        // ITAPoint tmp;
        // tmp.x = shape->point[i].x;
        // tmp.y = shape->point[i].y;
        // edges.push_back(tmp);

        edges.push_back(shape->point[i]);
    }

    return edges;
}
