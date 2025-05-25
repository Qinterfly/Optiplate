/*!
 * \file
 * \author Pavel Lakiza
 * \date May 2025
 * \brief Implementation of the Panel class
 */

#include <Eigen/Dense>
#include <future>
#include <QDebug>
#include <QXmlStreamWriter>

#include "fileutility.h"
#include "mathutility.h"
#include "panel.h"
#include "properties.h"

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

Panel::Panel(double thickness, Vec4 const& xCoords, Vec4 const& zCoords, Vec3 const& depths, double youngsModulus, double density)
    : mThickness(thickness)
    , mXCoords(xCoords)
    , mZCoords(zCoords)
    , mDepths(depths)
    , mYoungsModulus(youngsModulus)
    , mDensity(density)
{
}

//! Evaluate inertia properties of a panel
Properties Panel::massProperties(double timeout) const
{
    double const kTimeFactor = 1e6;
    Model model = build();
    auto fun = [&model]() { return Properties(model.massProperties()); };
    if (timeout > 0)
    {
        std::future<Properties> future = std::async(fun);
        auto duration = std::chrono::microseconds((int) std::round(timeout * kTimeFactor));
        std::future_status status = future.wait_for(duration);
        if (status != std::future_status::ready)
            return Properties();
        return future.get();
    }
    else
    {
        return fun();
    }
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
        if (d < 0.0)
            return false;
    }
    return true;
}

//! Read a panel from a XML stream
void Panel::read(QXmlStreamReader& stream)
{
    while (stream.readNextStartElement())
    {
        if (stream.name() == "mass")
            mThickness = stream.readElementText().toDouble();
        else if (stream.name() == "xCoords")
            Utility::readData(mXCoords.begin(), mXCoords.end(), stream);
        else if (stream.name() == "zCoords")
            Utility::readData(mZCoords.begin(), mZCoords.end(), stream);
        else if (stream.name() == "depths")
            Utility::readData(mDepths.begin(), mDepths.end(), stream);
        else if (stream.name() == "youngsModulus")
            mYoungsModulus = stream.readElementText().toDouble();
        else if (stream.name() == "density")
            mDensity = stream.readElementText().toDouble();
        else
            stream.skipCurrentElement();
    }
}

//! Write the panel to a XML stream
void Panel::write(QXmlStreamWriter& stream)
{
    stream.writeStartElement("panel");
    stream.writeTextElement("thickness", QString::number(mThickness));
    Utility::writeData("xCoords", mXCoords.begin(), mXCoords.end(), stream);
    Utility::writeData("zCoords", mZCoords.begin(), mZCoords.end(), stream);
    Utility::writeData("depths", mDepths.begin(), mDepths.end(), stream);
    stream.writeTextElement("youngsModulus", QString::number(mYoungsModulus));
    stream.writeTextElement("density", QString::number(mDensity));
    stream.writeEndElement();
}

//! Round panel data up to the specified precision
Panel Panel::round(double precisionGeometry, double precisionMechanical)
{
    Panel panel;
    panel.mThickness = Utility::roundTo(mThickness, precisionGeometry);
    for (int i = 0; i != mXCoords.size(); ++i)
    {
        panel.mXCoords[i] = Utility::roundTo(mXCoords[i], precisionGeometry);
        panel.mZCoords[i] = Utility::roundTo(mZCoords[i], precisionGeometry);
    }
    for (int i = 0; i != mDepths.size(); ++i)
        panel.mDepths[i] = Utility::roundTo(mDepths[i], precisionGeometry);
    panel.mYoungsModulus = Utility::roundTo(mYoungsModulus, precisionMechanical);
    panel.mDensity = Utility::roundTo(mDensity, precisionMechanical);
    return panel;
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
