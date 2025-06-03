/*!
 * \file
 * \author Pavel Lakiza
 * \date June 2025
 * \brief Declaration of the Panel class
 */

#ifndef PANEL_H
#define PANEL_H

#include <QString>

#include <kcl/model.h>

QT_FORWARD_DECLARE_CLASS(QXmlStreamReader)
QT_FORWARD_DECLARE_CLASS(QXmlStreamWriter)

namespace Backend
{

class Properties;

//! Class to represent panel data and compute inertia properties via KS-L
class Panel
{
public:
    Panel();
    Panel(double thickness, KCL::Vec4 const& xCoords, KCL::Vec4 const& zCoords,
          KCL::Vec3 const& depths, double youngsModulus, double density);
    ~Panel() = default;

    Properties massProperties(double timeout = 0) const;
    void renumerate();
    bool isValid() const;
    void read(QXmlStreamReader& stream);
    void write(QXmlStreamWriter& stream);
    void write(QString const& pathFile);
    Panel round(double precisionGeometry, double precisionMechanical);

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
    KCL::Vec3 depthEquation() const;
    KCL::Vec4 evaluateDepths(KCL::Vec3 const& coeffs) const;

    double mThickness;
    KCL::Vec4 mXCoords;
    KCL::Vec4 mZCoords;
    KCL::Vec3 mDepths;
    double mYoungsModulus;
    double mDensity;
};

}

#endif // PANEL_H
