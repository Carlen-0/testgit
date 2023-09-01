#ifndef ITA_DEVICETEMPERTRUEANALYSIS_H
#define ITA_DEVICETEMPERTRUEANALYSIS_H
#include <vector>

#if 1

#include"../ITA.h"
#else

typedef struct ITA_POINT {
    int x;
    int y;
}ITAPoint;

typedef enum Result
{
    ITA_OK = 0,                    //success
    ITA_ARG_OUT_OF_RANGE = -2, // 点坐标超过范围
    // 宽高未设置
    // edgePoints个数低于2
    // 

}ITA_RESULT;

#endif

typedef struct Y16info {
    short MaxY16;
    short MinY16;
    short AvgY16;
    ITAPoint MaxY16Index;
    ITAPoint MinY16Index;
} Y16info;

class GuideLog;

namespace ita {

    typedef std::vector<ITAPoint> PolygonEdges;
    class Polygon {
    public:
        struct Line {
            int x1;
            int x2;
            int y;
            Line() :x1(0), x2(0), y(0) {}
            Line(int _x1, int _x2, int _y) :x1(_x1), x2(_x2), y(_y) {}
            ~Line() {};
            Line(const Line& v)
            {
                this->x1 = v.x1; this->x2 = v.x2; this->y = v.y;
            }
            Line& operator=(const Line& v)
            {
                this->x1 = v.x1; this->x2 = v.x2; this->y = v.y; return *this;
            }

            bool operator<(const Line& v)
            {
                return (this->y < v.y);
            }
#if __cplusplus >= 201103L
            Line(Line&& v)
            {
                this->x1 = v.x1; this->x2 = v.x2; this->y = v.y;
            }
            Line& operator=(Line&& v)
            {
                this->x1 = v.x1; this->x2 = v.x2; this->y = v.y; return *this;
            }
#endif
        };

    public:
        typedef std::vector<Line>::value_type value_type;
        typedef std::vector<Line>::reference reference;
        typedef std::vector<Line>::const_reference const_reference;
        typedef std::vector<Line>::size_type  size_type;

    public:
        Polygon() :m_items() {};
        ~Polygon() {};
        Polygon(const Polygon& v) { m_items = v.m_items; }
        Polygon& operator=(const Polygon& v) { m_items = v.m_items; return *this; }
#if __cplusplus >= 201103L
        Polygon(Polygon&& v) { m_items = std::move(v.m_items); }
        Polygon& operator=(Polygon&& v) { m_items = std::move(v.m_items); return *this; }
#endif
        size_type size() const { return m_items.size(); };
        bool empty() const { return m_items.empty(); }
        void reserve(size_type n) { m_items.reserve(n); }
        reference at(size_type n) { return m_items.at(n); };
        const_reference at(size_type n) const { return m_items.at(n); };
        void push_back(const value_type& v) { m_items.push_back(v); };
#if __cplusplus >= 201103L
        void push_back(value_type&& v) { m_items.push_back(std::forward<value_type>(v)); }
#endif
    private:
        std::vector<Line> m_items;
    };

} // namespace ita

// deprecated
ITA_RESULT ITA_TempertrueAnalysis(short* y16Array, int height, int width, std::vector<ITAPoint>& results, Y16info& y16Info);
// deprecated
ITA_RESULT ITA_PolygonAnalysis_MinRect_old(const std::vector<ITAPoint>& edgePoints, int height, int width, std::vector<ITAPoint>& results);
ITA_RESULT ITA_PolygonAnalysis_old(const std::vector<ITAPoint>& edgePoints, int height, int width, std::vector<ITAPoint>& results);
// deprecated
int ITA_PointInPolygon_Impl(const std::vector<ITAPoint>& edgePoints, ITAPoint point);




// // RETANGE ����
// ITA_RESULT ITA_PolygonAnalysis_RETANGE(const std::vector<ITAPoint>& edgePoints,int height, int width, std::vector<ITAPoint>& results);
// // HEXAGON ������
// ITA_RESULT ITA_PolygonAnalysis_HEXAGON(const std::vector<ITAPoint>& edgePoints, int height, int width, std::vector<ITAPoint>& results);
// // POLYGON �ֻ�����
// ITA_RESULT ITA_PolygonAnalysis_POLYGON(const std::vector<ITAPoint>& edgePoints, int height, int width, std::vector<ITAPoint>& results);



ITA_RESULT ITA_PolygonAnalysis_POINT(const ita::PolygonEdges& edgePoints,
    int height, int width, ita::Polygon& result, GuideLog* logger);

ITA_RESULT ITA_PolygonAnalysis_LINE(const ita::PolygonEdges& edgePoints,
    int height, int width, ita::Polygon& result, GuideLog* logger);

ITA_RESULT ITA_PolygonAnalysis_RETANGE(const ita::PolygonEdges& edgePoints,
    int height, int width, ita::Polygon& result, GuideLog* logger);

ITA_RESULT ITA_PolygonAnalysis_ELIPSE(const ita::PolygonEdges& edgePoints,
    int height, int width, ita::Polygon& result, GuideLog* logger);

ITA_RESULT ITA_PolygonAnalysis_CIRCLE(const ita::PolygonEdges& edgePoints,
    int height, int width, ita::Polygon& result, GuideLog* logger);

ITA_RESULT ITA_PolygonAnalysis(const ita::PolygonEdges& edgePoints, ITA_SHAPE_TYPE type,
    int height, int width, ita::Polygon& result, GuideLog* logger);



#endif // ITA_DEVICETEMPERTRUEANALYSIS_H
