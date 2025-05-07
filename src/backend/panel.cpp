/*!
 * \file
 * \author Pavel Lakiza
 * \date May 2025
 * \brief Implementation of the Panel class
 */

#include "panel.h"

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
