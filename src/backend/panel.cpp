/*!
 * \file
 * \author Pavel Lakiza
 * \date May 2025
 * \brief Implementation of the Panel class
 */

#include <Eigen/Dense>
#include <QDebug>

#include "panel.h"
#include "utility.h"

using namespace Backend;
using namespace KCL;

Panel::Panel()
    : mThickness(0.0)
{
    mXCoords.fill(0.0);
    mZCoords.fill(0.0);
    mDepths.fill(0.0);
    mYoungsModulus = 0.0;
    mDensity = 0.0;
}

Panel::Panel(double thickness, Vec4 const& xCoords, Vec4 const& zCoords, Vec3 const& depths,
             double youngsModulus, double density)
    : mThickness(thickness)
    , mXCoords(xCoords)
    , mZCoords(zCoords)
    , mDepths(depths)
    , mYoungsModulus(youngsModulus)
    , mDensity(density)
{
}

//! Evaluate inertia properties of a panel
KCL::MassProperties Panel::massProperties() const
{
    Model model = build();
    return model.massProperties();
}

//! Renumerate vertices so that they are located clockwise
void Panel::renumerate()
{
    int numPoints = mXCoords.size();
    int numDepths = mDepths.size();

    // Find the plane equation
    auto coeffs = depthEquation();

    // Build the convex hull
    std::vector<Utility::Point> points(numPoints);
    for (int i = 0; i != numPoints; ++i)
        points[i] = {mXCoords[i], mZCoords[i]};
    std::vector<int> indices = Utility::jarvisMarch(points);

    // Update the vertices
    KCL::Vec4 xCoords, zCoords;
    for (int i = 0; i != numPoints; ++i)
    {
        int iSlice = indices[i];
        xCoords[i] = mXCoords[iSlice];
        zCoords[i] = mZCoords[iSlice];
    }
    mXCoords = xCoords;
    mZCoords = zCoords;

    // Update the depths
    auto depths = evaluateDepths(coeffs);
    std::copy_n(depths.begin(), numDepths, mDepths.begin());
}

//! Compute coefficients of a depth equation
KCL::Vec3 Panel::depthEquation() const
{
    Eigen::Matrix3f A;
    Eigen::Vector3f b;
    int numDepths = mDepths.size();
    for (int i = 0; i != numDepths; ++i)
    {
        A(i, 0) = mXCoords[i];
        A(i, 1) = mZCoords[i];
        A(i, 2) = 1.0;
        b(i) = mDepths[i];
    }
    ColPivHouseholderQR<Matrix3f> dec(A);
    Vector3f c = dec.solve(b);
    return {c(0), c(1), c(2)};
}

//! Compute depths at all points
KCL::Vec4 Panel::evaluateDepths(KCL::Vec3 const& coeffs) const
{
    int numPoints = mXCoords.size();
    KCL::Vec4 depths;
    for (int i = 0; i != numPoints; ++i)
        depths[i] = mXCoords[i] * coeffs[0] + mZCoords[i] * coeffs[1] + coeffs[2];
    return depths;
}

//! Check if panel data are correct
bool Panel::isValid() const
{
    auto depths = evaluateDepths(depthEquation());
    for (double d : depths)
    {
        if (d < 0)
            return false;
    }
    return true;
}

//! Create a model based on the specified panel properties
Model Panel::build() const
{
    double const kCharacterLength = 1.0;

    // Assigning constants
    Constants* pConstants = new Constants();
    pConstants->referenceLength = kCharacterLength;

    // Assigning general data
    GeneralData* pGenData = new GeneralData();
    pGenData->referenceLength = kCharacterLength;
    pGenData->iSymmetry = 1;
    pGenData->iLiftSurfaces = 1;
    pGenData->torsionalFactor = 1;
    pGenData->bendingFactor = 1;

    // Assigning panel data
    auto pPanel = new IsotropicPanel();
    pPanel->thickness = mThickness;
    pPanel->coords0 = {mXCoords[0], mZCoords[0]};
    pPanel->coords1 = {mXCoords[1], mZCoords[1]};
    pPanel->coords2 = {mXCoords[2], mZCoords[2]};
    pPanel->coords3 = {mXCoords[3], mZCoords[3]};
    pPanel->depths = mDepths;
    pPanel->density = mDensity;
    pPanel->youngsModulus = mYoungsModulus;

    // Assigning analysis parameters
    AnalysisParameters* pParameters = new AnalysisParameters();
    pParameters->iSymmetry = -1;

    // Creating an elastic surface
    auto surface = ElasticSurface();
    surface.insertElement(ElementType::OD, pGenData);
    surface.insertElement(ElementType::PN, pPanel);
    surface.insertElement(ElementType::PK);
    surface.insertElement(ElementType::QK);
    surface.setPolyData(PolyType::Plate);

    // Creating a special surface
    auto specialSurface = ElasticSurface();
    specialSurface.insertElement(ElementType::CO, pConstants);
    specialSurface.insertElement(ElementType::WP, pParameters);

    // Adding surfaces to a model
    Model model = Model();
    model.surfaces.push_back(surface);
    model.specialSurface = specialSurface;

    return model;
}

double Panel::thickness() const
{
    return mThickness;
}

KCL::Vec4 const& Panel::xCoords() const
{
    return mXCoords;
}

KCL::Vec4 const& Panel::zCoords() const
{
    return mZCoords;
}

KCL::Vec3 const& Panel::depths() const
{
    return mDepths;
}

double Panel::youngsModulus() const
{
    return mYoungsModulus;
}

double Panel::density() const
{
    return mDensity;
}

void Panel::setThickness(double thickness)
{
    mThickness = thickness;
}

void Panel::setXCoords(KCL::Vec4 const& xCoords)
{
    mXCoords = xCoords;
}

void Panel::setZCoords(KCL::Vec4 const& zCoords)
{
    mZCoords = zCoords;
}

void Panel::setDepths(KCL::Vec3 const& depths)
{
    mDepths = depths;
}

void Panel::setYoungsModulus(double youngsModulus)
{
    mYoungsModulus = youngsModulus;
}

void Panel::setDensity(double density)
{
    mDensity = density;
}
