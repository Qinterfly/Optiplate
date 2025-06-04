/*!
 * \file
 * \author Pavel Lakiza
 * \date June 2025
 * \brief Declaration of the Properties class
 */

#ifndef PROPERTIES_H
#define PROPERTIES_H

#include <QString>

#include <kcl/model.h>

QT_FORWARD_DECLARE_CLASS(QXmlStreamReader)
QT_FORWARD_DECLARE_CLASS(QXmlStreamWriter)

namespace Backend
{

//! Inertia properties of a KCL model
struct Properties : public KCL::MassProperties
{
    Properties();
    Properties(double value);
    Properties(KCL::MassProperties const& properties);
    ~Properties() = default;

    bool isValid() const;
    int numValidValues() const;
    std::vector<double> validValues() const;
    double maxAbsValidValue() const;
    std::vector<double> allValues() const;
    Properties compare(Properties const& another, Properties const& weight = Properties(1.0)) const;
    void read(QXmlStreamReader& stream);
    void write(QString const& name, QXmlStreamWriter& stream) const;
};

}

#endif // PROPERTIES_H
