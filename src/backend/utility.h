/*!
 * \file
 * \author Pavel Lakiza
 * \date May 2025
 * \brief Declaration of the utility functions
 */

#ifndef UTILITY_H
#define UTILITY_H

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

} // namespace Backend::Utility

#endif // UTILITY_H
