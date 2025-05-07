/*!
 * \file
 * \author Pavel Lakiza
 * \date May 2025
 * \brief Declaration of the Panel class
 */

#ifndef PANEL_H
#define PANEL_H

#include <kcl/model.h>

namespace Backend
{

class Panel
{
public:
    Panel();
    Panel(double thickness, KCL::Vec4 const& xCoords, KCL::Vec4 const& zCoords,
          KCL::Vec3 const& depths, double youngsModulus, double density);
    KCL::MassProperties massProperties() const;

    double thickness() const;
    KCL::Vec4 const& xCoords() const;
    KCL::Vec4 const& zCoords() const;
    KCL::Vec3 const& depths() const;
    double youngsModulus() const;
    double density() const;

    void setThickness(double thickness);
    void setXCoords(KCL::Vec4 const& xCoords);
    void setZCoords(KCL::Vec4 const& zCoords);
    void setDepths(KCL::Vec3 const& depths);
    void setYoungsModulus(double youngsModulus);
    void setDensity(double density);

private:
    KCL::Model build() const;

    double mThickness;
    KCL::Vec4 mXCoords;
    KCL::Vec4 mZCoords;
    KCL::Vec3 mDepths;
    double mYoungsModulus;
    double mDensity;
};

} // namespace Backend

#endif // PANEL_H
