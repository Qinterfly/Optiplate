/*!
 * \file
 * \author Pavel Lakiza
 * \date May 2025
 * \brief Implementation of math utilities
 */

#include <QDebug>
#include <QFileInfo>
#include <QObject>

#include "mathutility.h"

namespace Backend::Utility
{

//! Check whether three points are located clockwise or counterclockwise
int orientation(Point const& p, Point const& q, Point const& r)
{
    int product = (q.x - p.x) * (r.y - p.y) - (r.x - p.x) * (q.y - p.y);
    if (product > 0)
        return 1; // Counterclockwise
    else if (product < 0)
        return -1; // Clockwise
    return 0;      // Collinear
}

//! Create a convex hull
std::vector<int> jarvisMarch(std::vector<Point> const& points)
{
    // Check if the number of points is enough
    int n = points.size();
    if (n < 3)
    {
        qWarning() << QObject::tr("There must be at least three points to build a convex hull");
        return {};
    }

    // Find the leftmost point
    int l = 0;
    for (int i = 1; i != n; ++i)
        if (points[i].y < points[l].y)
            l = i;

    // Search for points in clockwise orientation
    std::vector<int> order;
    int p = l;
    int q;
    do
    {
        order.push_back(p);
        q = (p + 1) % n;
        for (int i = 0; i < n; i++)
            if (orientation(points[p], points[i], points[q]) == -1)
                q = i;
        p = q;
    } while (p != l);

    return order;
}

//! Compute relative error between two numbers
double relativeError(double current, double base)
{
    double result = current - base;
    if (std::abs(base) > std::numeric_limits<double>::epsilon())
        result /= base;
    return result;
}

//! Round the number up to the specified precision
double roundTo(double value, double precision)
{
    return std::round(value / precision) * precision;
}

}
