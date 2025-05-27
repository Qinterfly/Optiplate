/*!
 * \file
 * \author Pavel Lakiza
 * \date May 2025
 * \brief Declaration of math utilities
 */

#ifndef MATHUTILITY_H
#define MATHUTILITY_H

#include <vector>

namespace Backend::Utility
{

struct Point
{
    double x;
    double y;
};

std::vector<int> jarvisMarch(std::vector<Point> const& points);
double relativeError(double current, double base);
double roundTo(double value, double precision = 1.0);

}

#endif // MATHUTILITY_H
