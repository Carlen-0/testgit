#ifndef ITA_MT_POLYGONANALYSISER_H
#define ITA_MT_POLYGONANALYSISER_H

#include <stdio.h>
#include <vector>

#include "../ITA.h"
#include "../AppAlg/DeviceTempertrueAnalysis.h"


class GuideLog;
class PolygonAnalysiser
{
public:
    using PolygonEdges = ita::PolygonEdges;
    using Polygon = ita::Polygon;
    using PolygonPool = std::vector<Polygon>;
public:
    PolygonAnalysiser(int w, int h);
    ~PolygonAnalysiser();

    void setLogger(GuideLog* logger);

    static PolygonEdges convert(const ITAShape* shape);
    ITA_RESULT add(const PolygonPool& items);
    ITA_RESULT analysiser(ITA_SHAPE_TYPE type, const PolygonEdges& edges, Polygon& ploygon);
    ITA_RESULT clear();

    Polygon& at(size_t i);
    size_t size() const;

private:
    GuideLog* m_logger;
    int          m_width;
    int          m_height;
    size_t       m_size;
    PolygonPool  m_polygon;
};


#endif // ITA_MT_POLYGONANALYSISER_H
