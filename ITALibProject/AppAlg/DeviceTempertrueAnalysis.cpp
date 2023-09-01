#include "DeviceTempertrueAnalysis.h"
#include <algorithm>
#include <math.h>

#include "../Module/GuideLog.h"


using namespace ita;

typedef struct ITA_POINTF
{
    float x;	// 一条平行线 和多边形的交叉点
    float y;	// 一条平行线 的y值
}ITAPointF;

typedef struct ITA_POINT_2P
{
    int x1;		// 一条直线的一个点
    int y1;		// 一条直线的一个点
    int x2;		// 一条直线的另一个点
    int y2;		// 一条直线的另一个点
}ITAPoint_2P;

void calcuMaxMin(const std::vector<ITAPoint>& EdgePoints, int& minX, int& maxX, int& minY, int& maxY)
{
    minX = 65535;
    minY = 65535;
    maxX = -1;
    maxY = -1;

    for (int i = 0; i < EdgePoints.size(); i++)
    {
        if (EdgePoints[i].x < minX)  minX = EdgePoints[i].x;
        if (EdgePoints[i].y < minY)  minY = EdgePoints[i].y;

        if (EdgePoints[i].x > maxX)  maxX = EdgePoints[i].x;
        if (EdgePoints[i].y > maxY)  maxY = EdgePoints[i].y;
    }
}

bool PointInPolygon(const std::vector<ITAPoint>& poly, int x, int y)
{
    bool flag = false;
    int size = int(poly.size());
    for (int i = 0, j = size - 1; i < size; j = i, i++)
    {
        int x1 = poly[i].x;
        int y1 = poly[i].y;
        int x2 = poly[j].x;
        int	y2 = poly[j].y;

        // 点与多边形顶点重合
        if ((x1 == x && y1 == y) || (x2 == x && y2 == y))
        {
            //return 'on'; // 点在轮廓上
            return true;
        }

        // 判断线段两端点是否在射线两侧
        if ((y1 < y && y2 >= y) || (y1 >= y && y2 < y)) // 只有一边取等号，当射线经过多边形顶点时，只计一次
        {
            // 线段上与射线 Y 坐标相同的点的 X 坐标
            // y=kx+b变换成x=(y-b)/k 其中k=(y2-y1)/(x2-x1)
            double crossX = (y - y1) * (x2 - x1) / (y2 - y1) + x1;

            // 点在多边形的边上
            if (crossX == x)
            {
                //return 'on'; // 点在轮廓上
                return true;
            }

            // 只考虑右射线 右射线穿过多边形的边界，每穿过一次flag的值变换一次
            if (crossX > x)
            {
                flag = !flag;  // 穿过奇数次为true，偶数次为false
            }
        }
    }

    // 射线穿过多边形边界的次数为奇数时点在多边形内
    //return flag ? 'in' : 'out'; // 点在轮廓内或外
    return flag ? true : false;
}

void searchLinePoint(const std::vector<ITAPoint>& edgePoints, std::vector<ITAPoint_2P>& lineHoriz, std::vector<ITAPointF>& linePoints)
{
    std::vector<ITAPoint> input = edgePoints;
    input.push_back(edgePoints[0]);

    for (int i = 0; i < input.size() - 1; i++)
    {
        if (input[i].y == input[i + 1].y)
        {
            lineHoriz.push_back({ input[i].x, input[i].y,  input[i + 1].x, input[i + 1].y });
        }
        else
        {
            ITAPoint maxY, minY;
            if (input[i].y > input[i + 1].y)
            {
                maxY = input[i];
                minY = input[i + 1];
            }
            else
            {
                minY = input[i];
                maxY = input[i + 1];
            }

            if (minY.x == maxY.x)
            {
                int miny = std::min(maxY.y, minY.y);
                int maxy = std::max(maxY.y, minY.y);

                for (int i = miny; i <= maxy; i++)
                {
                    linePoints.push_back({ float(minY.x), (float)i });
                }
            }
            else
            {
                for (int y = minY.y; y <= maxY.y; y++)
                {
                    float crossX = (y - minY.y) * (maxY.x - minY.x) / float(maxY.y - minY.y) + minY.x;
                    linePoints.push_back({ crossX, (float)y });
                }
            }
        }
    }
    struct {
        bool operator()(ITAPointF a, ITAPointF b) const
        {
            return a.y < b.y;
        }
    } customLess;
    std::sort(linePoints.begin(), linePoints.end(), customLess);
}

// 计算当前Y值的一条水平线，和多边形的交点
std::vector<float> countLinePionts(std::vector<ITAPointF>& linePoints, int y, int& index)
{
    std::vector<float> xValue;

    for (int v = index; v < linePoints.size(); v++)
    {
        if (linePoints[v].y == y)
        {
            bool isExit = false;
            for (int id = 0; id < xValue.size(); id++)
            {
                if (xValue[id] == linePoints[v].x) isExit = true;
            }

            if (!isExit)
                xValue.push_back(linePoints[v].x);
        }
        else if (linePoints[v].y > y)
        {
            index = v;
            break;
        }
    }

    return xValue;
}


// 1. 最开始的实现，速度奇慢，主要就是针对此函数优化
ITA_RESULT ITA_PolygonAnalysis_old(const std::vector<ITAPoint>& edgePoints, int height, int width, std::vector<ITAPoint>& results)
{
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            if (PointInPolygon(edgePoints, j, i))
            {
                results.push_back({ j,i });
            }
        }
    }

    return ITA_OK;
}

// 2. 对PolygonAnalysis_other优化，方法就是先找到多边形最小外围框
ITA_RESULT ITA_PolygonAnalysis_MinRect_old(const std::vector<ITAPoint>& edgePoints, int height, int width, std::vector<ITAPoint>& results)
{
    int minX, maxX, minY, maxY;
    calcuMaxMin(edgePoints, minX, maxX, minY, maxY);
    if (minX > width || maxX < 0 || minY > height || maxY < 0) {
        return ITA_ARG_OUT_OF_RANGE;
    }

    for (int i = minY; i <= maxY; i++)
    {
        for (int j = minX; j <= maxX; j++)
        {
            if (PointInPolygon(edgePoints, j, i))
            {
                results.push_back({ j, i });
            }
        }
    }
    return ITA_OK;
}


ITA_RESULT ITA_PolygonAnalysis_POLYGONv2(const std::vector<ITAPoint>& edgePoints,
    int height, int width, Polygon& results, GuideLog* logger)
{
    if (edgePoints.size() < 3) {

        LOGW(logger, "CONTOUR edge point size[%zu] < 3", edgePoints.size());

        return ITA_INVALID_COORD;
    }


    int minx = 65536;
    int maxx = 0;
    int cury = edgePoints[0].y;

    for (int p = 0; p < edgePoints.size(); p++)
    {
        if (cury != edgePoints[p].y)
        {
            results.push_back(Polygon::Line(minx, maxx, cury));
            minx = edgePoints[p].x;
            maxx = edgePoints[p].x;
            cury = edgePoints[p].y;
        }
        else
        {
            if (edgePoints[p].x > maxx)
            {
                maxx = edgePoints[p].x;
            }
            if (edgePoints[p].x < minx)
            {
                minx = edgePoints[p].x;
            }
        }
    }
    results.push_back(Polygon::Line(minx, maxx, cury));

    LOGI_IF(logger, results.empty(), "not found point. pointSize=(%d),  minx,maxx=(%d, %d), cury=(%d)",
        edgePoints.size(), minx, maxx, cury);

    return ITA_OK;
}

ITA_RESULT ITA_PolygonAnalysis_POLYGON(const PolygonEdges& edgePoints,
    int height, int width, Polygon& results, GuideLog* logger)
{
    if (edgePoints.size() < 3) {

        LOGW(logger, "POLYGON edge point size[%zu] < 3", edgePoints.size());

        return ITA_INVALID_COORD;
    }

    /************************************************************/
    // // 判断是 [手绘多边形] 还是 [有线宽的逐行数据]
    // bool isNormalPolygon = false;
    // for(int i = 0; i < edgePoints.size()-1; i++)
    // {
    //     if(edgePoints[i].y > edgePoints[i+1].y)
    //     {
    //         isNormalPolygon = true;
    //     }
    // }
    // if(!isNormalPolygon)
    // {
    //     return ITA_PolygonAnalysis_POLYGONv2(edgePoints, height, width, results, logger);
    // }
    /************************************************************/


    int gMinX, gMaxX, gMinY, gMaxY;

    // 1. 查找 [包围多边形] 的 最小矩形框
    calcuMaxMin(edgePoints, gMinX, gMaxX, gMinY, gMaxY);
    if (gMinX > width || gMaxX < 0 || gMinY > height || gMaxY < 0)
    {
        LOGW(logger, "point out for range(%d, %d). p1=(%d, %d), p2=(%d, %d)",
            width, height, gMinX, gMinY, gMaxX, gMaxY);

        return ITA_ARG_OUT_OF_RANGE;
    }

    int totalPoints = (gMaxY - gMinY + 1) * (gMaxX - gMinX + 1) + 1;
    results.reserve(totalPoints);


    // 2. 查找 [输入线段上 横着的线] 和 [输入线段上 的点]
    // lineHoriz 多边形的水平方向的线段
    // linePoints 非水平方向的线段上点, 点与点之间最小距离y=1
    std::vector<ITAPoint_2P> lineHoriz;
    std::vector<ITAPointF> linePoints;
    searchLinePoint(edgePoints, lineHoriz, linePoints);


    // 从上往下扫描
    std::vector<ITAPoint_2P> xResult;

    int index = 0;
    for (int y = gMinY; y < gMaxY + 1; y++)
    {
        // 3. 找到多边形上的一条线段，该线段的纵坐标等于y
        std::vector<ITAPoint_2P> xLine;
        for (int z = 0; z < lineHoriz.size(); z++)
        {
            if (lineHoriz[z].y1 == y)
            {
                bool isExit = false;
                for (int i = 0; i < xLine.size(); i++)
                {
                    if (xLine[i].y1 == lineHoriz[z].y1)
                    {
                        if (xLine[i].x1 + xLine[i].x2 == lineHoriz[z].x1 + lineHoriz[z].x2)
                        {
                            isExit = true;
                        }
                    }
                }
                if (!isExit)
                {
                    xLine.push_back(lineHoriz[z]);
                }
            }
        }

        // 4. 计算当前Y值的一条水平线，和多边形的交点
        // xValue 个数代表水平线和多边形的交点个数
        std::vector<float> xValue = countLinePionts(linePoints, y, index);
        if (xValue.size() == 0)
        {
            for (int i = 0; i < xLine.size(); i++)
            {
                int minx = std::min(xLine[i].x1, xLine[i].x2);
                int maxx = std::max(xLine[i].x1, xLine[i].x2);
                xResult.push_back({ minx, y,  maxx, y });
            }
            continue;
        }
        else if (xValue.size() == 1)
        {
            for (int i = 0; i < xLine.size(); i++)
            {
                int minx = std::min(xLine[i].x1, xLine[i].x2);
                int maxx = std::max(xLine[i].x1, xLine[i].x2);
                xResult.push_back({ minx, y,  maxx, y });
            }
            xResult.push_back({ (int)xValue[0], y, (int)xValue[0], y });
            continue;
        }
        std::sort(xValue.begin(), xValue.end());


        // 5. 水平线和多边形的交点，分析交点之间是否属于内部
        for (int m = 0; m < xValue.size() - 1; m++)
        {
            int centreVal = (xValue[m] + xValue[m + 1]) / 2;
            if (xLine.size() > 0)
            {
                for (int i = 0; i < xLine.size(); i++)
                {
                    int minx = std::min(xLine[i].x1, xLine[i].x2);
                    int maxx = std::max(xLine[i].x1, xLine[i].x2);

                    if (centreVal >= minx && centreVal <= maxx)
                    {
                        xResult.push_back({ minx, y,  maxx, y });
                    }
                    else
                    {
                        if (PointInPolygon(edgePoints, centreVal, y))
                        {
                            xResult.push_back({ (int)(xValue[m]), y,  (int)floor(xValue[m + 1]), y });
                        }
                    }
                }
            }
            else
            {
                if (PointInPolygon(edgePoints, centreVal, y))
                {
                    xResult.push_back({ (int)(xValue[m]), y,  (int)floor(xValue[m + 1]), y });
                }
            }
        }
    }

    for (int i = 0; i < xResult.size(); i++)
    {
        int minx = std::min(xResult[i].x1, xResult[i].x2);
        int maxx = std::max(xResult[i].x1, xResult[i].x2);

        results.push_back(Polygon::Line(minx, maxx, xResult.at(i).y1));
    }

    LOGI_IF(logger, results.empty(), "not found point. w,h=(%d, %d), min=(%d, %d), max=(%d, %d)",
        width, height, gMinX, gMinY, gMaxX, gMaxY);

    return ITA_OK;
}

ITA_RESULT ITA_TempertrueAnalysis(short* y16Array, int height, int width, std::vector<ITAPoint>& results, Y16info& y16Info)
{
    short MaxY16 = -32768;
    short MinY16 = 32767;
    int SumY16 = 0;
    int nRoiNum = 0;

    for (int i = 0; i < results.size(); i++)
    {
        int cur_x = results[i].x;
        int cur_y = results[i].y;
        short cur_y16 = y16Array[cur_y * width + cur_x];

        SumY16 += cur_y16;
        nRoiNum++;

        if (cur_y16 > MaxY16)
        {
            MaxY16 = cur_y16;
            y16Info.MaxY16Index.x = cur_x;
            y16Info.MaxY16Index.y = cur_y;
        }
        if (cur_y16 < MinY16)
        {
            MinY16 = cur_y16;
            y16Info.MaxY16Index.x = cur_x;
            y16Info.MaxY16Index.y = cur_y;
        }
    }

    y16Info.MaxY16 = MaxY16;
    y16Info.MinY16 = MinY16;
    y16Info.AvgY16 = SumY16 / nRoiNum;

    return ITA_OK;
}




int ITA_PointInPolygon_Impl(const std::vector<ITAPoint>& edgePoints, ITAPoint point)
{
    return PointInPolygon(edgePoints, point.x, point.y);
}

ITA_RESULT ITA_PolygonAnalysis(const PolygonEdges& edgePoints, ITA_SHAPE_TYPE type,
    int height, int width, Polygon& results, GuideLog* logger)
{
    ITA_RESULT ret = ITA_ERROR;
    switch (type)
    {
    case ITA_SHAPE_TYPE_NONE:
        return ITA_POLYGON_NOT_SUPPORTED;
    case ITA_SHAPE_TYPE_POINT:
        ret = ITA_PolygonAnalysis_POINT(edgePoints, height, width, results, logger);
        break;
    case ITA_SHAPE_TYPE_LINE:
        ret = ITA_PolygonAnalysis_LINE(edgePoints, height, width, results, logger);
        break;
    case ITA_SHAPE_TYPE_RETANGE:
        ret = ITA_PolygonAnalysis_RETANGE(edgePoints, height, width, results, logger);
        break;
    case ITA_SHAPE_TYPE_CIRCLE:
        ret = ITA_PolygonAnalysis_CIRCLE(edgePoints, height, width, results, logger);
        break;
    case ITA_SHAPE_TYPE_ELIPSE:
        ret = ITA_PolygonAnalysis_ELIPSE(edgePoints, height, width, results, logger);
        break;
    case ITA_SHAPE_TYPE_CONTOUR:
        ret = ITA_PolygonAnalysis_POLYGONv2(edgePoints, height, width, results, logger);
        break;
    default:
        ret = ITA_PolygonAnalysis_POLYGON(edgePoints, height, width, results, logger);
        break;
    }

    if (ret != ITA_OK)
    {
        return ret;
    }

    if (results.size() == 0)
    {
        return ITA_POLYGON_AREA_EMPTY;
    }

    return ITA_OK;
}

#if 0
// RETANGE
ITA_RESULT ITA_PolygonAnalysis_RETANGE(const std::vector<ITAPoint>& edgePoints,
    int height, int width, std::vector<ITAPoint>& results, GuideLog* logger)
{
    if (edgePoints.size() != 2) {

        LOGW(logger, "RETANGE edge point size[%zu] != 2", edgePoints.size());

        return ITA_INVALID_COORD;
    }

    int minX, maxX, minY, maxY;

    minX = edgePoints.at(0).x;
    minY = edgePoints.at(0).y;
    maxX = edgePoints.at(1).x;
    maxY = edgePoints.at(1).y;

    if (minX > width || maxX > width || maxX > height || maxY > height)
    {
        LOGW(logger, "point out for range(). p1=(%d, %d), p2=(%d, %d)",
            width, height, minX, minY, maxX, maxY);

        return ITA_INVALID_COORD;
    }

    results.reserve((maxY - minY + 2) * (maxX - minX + 2));

    // #pragma omp parallel
    for (int i = minY; i <= maxY; i++)
    {
        for (int j = minX; j <= maxX; j++)
        {
            results.push_back({ j, i });
        }
    }

    return ITA_OK;
}
#else
// HEXAGON
ITA_RESULT ITA_PolygonAnalysis_RETANGE(const PolygonEdges& edgePoints,
    int height, int width, Polygon& results, GuideLog* logger)
{
    if (edgePoints.size() != 2)
    {
        LOGW(logger, "RETANGE edge point size[%zu] != 2", edgePoints.size());

        return ITA_INVALID_COORD;
    }

    int p_x1 = edgePoints.at(0).x;
    int p_x2 = edgePoints.at(1).x;
    int p_y1 = edgePoints.at(0).y;
    int p_y2 = edgePoints.at(1).y;

    if (p_x1 > width || p_x2 > width || p_y1 > height || p_y2 > height)
    {
        LOGW(logger, "point out for range(%d, %d). p1=(%d, %d), p2=(%d, %d)",
            width, height, p_x1, p_y1, p_x2, p_y2);

        return ITA_INVALID_COORD;
    }

    if (p_x1 > p_x2 || p_y1 > p_y2)
    {
        LOGW(logger, "rect point invalid. (w,h)=(%d, %d) p1=(%d, %d), p2=(%d, %d)",
            width, height, p_x1, p_y1, p_x2, p_y2);
    }

    results.reserve(p_y2 - p_y1 + 1);
    for (int y = p_y1; y < p_y2; y++)
    {
        results.push_back(Polygon::Line(p_x1, p_x2, y));
    }

    return ITA_OK;
}
#endif

ITA_RESULT ITA_PolygonAnalysis_POINT(const PolygonEdges& edgePoints,
    int height, int width, Polygon& result, GuideLog* logger)
{
    if (edgePoints.size() != 1)
    {
        LOGW(logger, "POINT edge point size[%zu] != 1", edgePoints.size());

        return ITA_INVALID_COORD;
    }

    if (edgePoints.at(0).x > width || edgePoints.at(0).y > height)
    {
        LOGW(logger, "point out for range(%d, %d). p1=(%d, %d), p2=(%d, %d)",
            width, height, edgePoints.at(0).x, edgePoints.at(0).y);

        return ITA_INVALID_COORD;
    }

    const ITAPoint& p = edgePoints.at(0);
    result.push_back(Polygon::Line(p.x, p.x, p.y));

    return ITA_OK;
}

ITA_RESULT ITA_PolygonAnalysis_LINE(const PolygonEdges& edgePoints,
    int height, int width, Polygon& result, GuideLog* logger)
{
    if (edgePoints.size() != 2)
    {
        LOGW(logger, "LINE edge point size[%zu] != 2", edgePoints.size());

        return ITA_INVALID_COORD;
    }

    // (x1, y1)
    // (x2, y2)
    int p_x1 = edgePoints.at(0).x;
    int p_x2 = edgePoints.at(1).x;
    int p_y1 = edgePoints.at(0).y;
    int p_y2 = edgePoints.at(1).y;

    if (p_x1 > width || p_x2 > width || p_y1 > height || p_y2 > height)
    {
        LOGW(logger, "point out for range(%d, %d). p1=(%d, %d), p2=(%d, %d)",
            width, height, p_x1, p_y1, p_x2, p_y2);

        return ITA_INVALID_COORD;
    }

    if (p_x1 == p_x2)
    {
        int y1 = p_y1 < p_y2 ? p_y1 : p_y2;
        int y2 = p_y1 < p_y2 ? p_y2 : p_y1;
        int y = 0;

        for (int i = y1; i <= y2; i++)
        {
            y = i;

            result.push_back(Polygon::Line(p_x1, p_x1, y));
        }

        return ITA_OK;
    }

    if (p_y1 == p_y2)
    {
        int x1 = p_x1 < p_x2 ? p_x1 : p_x2;
        int x2 = p_x1 < p_x2 ? p_x2 : p_x1;
        int x = 0;

        result.push_back(Polygon::Line(x1, x2, p_y1));
        return ITA_OK;
    }

    int x1 = 0;
    int x2 = 0;
    int y1 = 0;
    int y2 = 0;
    if (p_x1 < p_x2)
    {
        x1 = p_x1;
        y1 = p_y1;

        x2 = p_x2;
        y2 = p_y2;
    }
    else
    {
        x1 = p_x2;
        y1 = p_y2;

        x2 = p_x1;
        y2 = p_y1;
    }

    float k = (y1 - y2) * 1.0f / (x1 - x2);
    float m = y2 - k * x2;

    int x = 0;
    int y = 0;
    for (int i = x1; i <= x2; i++)
    {
        x = i;
        y = k * x + m;

        result.push_back(Polygon::Line(x, x, y));
    }

    return ITA_OK;
}

ITA_RESULT ITA_PolygonAnalysis_ELIPSE(const PolygonEdges& edgePoints,
    int height, int width, Polygon& result, GuideLog* logger)
{
    if (edgePoints.size() != 2)
    {
        LOGW(logger, "ELIPSE edge point size[%zu] != 2", edgePoints.size());

        return ITA_INVALID_COORD;
    }

    // left top : (x1, y1)
    // right top: (x2, y1)
    // right bottom: (x2, y2)
    // left top:     (x1, y2)
    int p_x1 = edgePoints.at(0).x;
    int p_x2 = edgePoints.at(1).x;
    int p_y1 = edgePoints.at(0).y;
    int p_y2 = edgePoints.at(1).y;

    if (p_x1 > width || p_x2 > width || p_y1 > height || p_y2 > height)
    {
        LOGW(logger, "point out for range(%d, %d). p1=(%d, %d), p2=(%d, %d)",
            width, height, p_x1, p_y1, p_x2, p_y2);

        return ITA_INVALID_COORD;
    }

    if (p_x1 > p_x2 || p_y1 > p_y2)
    {
        LOGW(logger, "point invalid. (w,h)=(%d, %d) p1=(%d, %d), p2=(%d, %d)",
            width, height, p_x1, p_y1, p_x2, p_y2);
    }

    // circle
    if (p_y2 - p_y1 == p_x2 - p_x1)
    {
        return ITA_PolygonAnalysis_CIRCLE(edgePoints, height,
            width, result, logger);
    }

    result.reserve(p_y2 - p_y1 + 1);

    // y = kx + m;
    // 因为只考虑椭圆弦与X轴平行的情况,所以k始终为0
    // int k = 0;
    int m = 0;
    // x^2 / a^2 + y^2 / b^2 = 1
    float a = 0.0f;
    float b = 0.0f;

    float offset_x = 0.0f;
    float offset_y = 0.0f;


    // 椭圆长轴平行与X轴的情况
    a = (p_x2 - p_x1) * 1.0f / 2;
    b = (p_y2 - p_y1) * 1.0f / 2;

    offset_x = p_x1 + a;
    offset_y = p_y1 + b;

    // 椭圆长轴平行与Y轴的情况
    if (a < b)
    {
        float tmp = a;
        a = b;
        b = a;
        a = tmp;
    }


#define X1_EXPR(a, b, m) \
    (-sqrt((1 - ((m) * (m)) / ((b) * (b))) * ((a) * (a))))

#define X2_EXPR(a, b, m) \
    (sqrt((1 - ((m) * (m)) / ((b) * (b))) * ((a) * (a))))

    int x1 = 0;
    int x2 = 0;
    // int y  = 0;

    // 只求上半部分点,可以利用对称性求出下半部分点
    m = b > floor(b) ? (int)(b) : (int)(b - 1);
    for (int i = m; i > 0; i--)
    {
        // m = i;
        // y = i;

        // // 向下取整
        // x1 = (int)floor(X1_EXPR(a, b, i));
        // // 向上取整
        // x2 = (int)ceil(X2_EXPR(a, b, i));

        // 向上取整
        x1 = (int)ceil(X1_EXPR(a, b, i));
        // 向下取整
        x2 = (int)floor(X2_EXPR(a, b, i));

        result.push_back(Polygon::Line(x1 + offset_x, x2 + offset_x, i + offset_y));
        result.push_back(Polygon::Line(x1 + offset_x, x2 + offset_x, -i + offset_y));
    }
    // 过过两焦点的线
    if ((p_y2 - p_y1 % 2) != 0)
    {
        x1 = (int)floor(-a);
        x2 = (int)ceil(a);
        result.push_back(Polygon::Line(x1 + offset_x, x2 + offset_x, offset_y));
    }

    // 上下相切的点
    {
        result.push_back(Polygon::Line(offset_x, offset_x, b + offset_y));
        result.push_back(Polygon::Line(offset_x, offset_x, -b + offset_y));
    }


    return ITA_OK;

#undef X1_EXPR
#undef X2_EXPR
}

ITA_RESULT ITA_PolygonAnalysis_CIRCLE(const PolygonEdges& edgePoints,
    int height, int width, Polygon& result, GuideLog* logger)
{
    if (edgePoints.size() != 2)
    {
        LOGW(logger, "CIRCLE edge point size[%zu] != 2", edgePoints.size());


        return ITA_INVALID_COORD;
    }

    // left top : (x1, y1)
    // right top: (x2, y1)
    // right bottom: (x2, y2)
    // left top:     (x1, y2)
    int p_x1 = edgePoints.at(0).x;
    int p_x2 = edgePoints.at(1).x;
    int p_y1 = edgePoints.at(0).y;
    int p_y2 = edgePoints.at(1).y;

    if (p_x1 > width || p_x2 > width || p_y1 > height || p_y2 > height)
    {
        LOGW(logger, "point out for range(%d, %d). p1=(%d, %d), p2=(%d, %d)",
            width, height, p_x1, p_y1, p_x2, p_y2);

        return ITA_INVALID_COORD;
    }

    if (p_x1 > p_x2 || p_y1 > p_y2)
    {
        LOGW(logger, "point invalid. (w,h)=(%d, %d) p1=(%d, %d), p2=(%d, %d)",
            width, height, p_x1, p_y1, p_x2, p_y2);
    }

    if (p_x2 - p_x1 != p_y2 - p_y1)
    {
        LOGW(logger, "point invalid. (w,h)=(%d, %d) p1=(%d, %d), p2=(%d, %d)",
            width, height, p_x1, p_y1, p_x2, p_y2);

        return ITA_INVALID_COORD;
    }

    result.reserve(p_y2 - p_y1 + 1);

    int   m = 0;
    float r = 0.0f;

    float offset_x = 0.0f;
    float offset_y = 0.0f;

    r = (p_x2 - p_x1) * 1.0f / 2;
    offset_x = p_x1 + r;
    offset_y = p_y1 + r;

#define X1_EXPR(r, m) \
    (-sqrt((r) * (r) - (m) * (m)))


#define X2_EXPR(r, m) \
    (sqrt((r) * (r) - (m) * (m)))

    int x1 = 0;
    int x2 = 0;
    // int y  = 0;

    m = r > floor(r) ? (int)r : (int)(r - 1);
    for (int i = m; i > 0; i--)
    {
        // m = i;
        // y = i;

        x1 = (int)floor(X1_EXPR(r, i));
        x2 = (int)ceil(X2_EXPR(r, i));

        result.push_back(Polygon::Line(x1 + offset_x, x2 + offset_x, i + offset_y));
        result.push_back(Polygon::Line(x1 + offset_x, x2 + offset_x, -i + offset_y));
    }

    // 平行X轴过圆心的线
    if ((p_y2 - p_y2 % 2) != 0)
    {
        x1 = (int)floor(-r);
        x2 = (int)ceil(r);

        result.push_back(Polygon::Line(x1 + offset_x, x2 + offset_x, offset_y));
    }

    // 上下相切的点
    {
        result.push_back(Polygon::Line(offset_x, offset_x, r + offset_y));
        result.push_back(Polygon::Line(offset_x, offset_x, -r + offset_y));
    }

    return ITA_OK;

#undef X1_EXPR
#undef X2_EXPR
}
