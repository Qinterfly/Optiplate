/*!
 * \file
 * \author Pavel Lakiza
 * \date June 2025
 * \brief Implementation of the Properties class
 */

#include <QXmlStreamWriter>

#include "fileutility.h"
#include "mathutility.h"
#include "properties.h"

using namespace Backend;

int const skNumDirections = 3;

Properties::Properties()
    : Properties(0.0)
{
}

Properties::Properties(double value)
{
    mass = value;
    centerGravity.fill(value);
    inertiaMoments.fill(value);
    inertiaProducts.fill(value);
}

//! Copy fields from a base class
Properties::Properties(MassProperties const& properties)
{
    mass = properties.mass;
    centerGravity = properties.centerGravity;
    inertiaMoments = properties.inertiaMoments;
    inertiaProducts = properties.inertiaProducts;
}

//! Check if the set of properties is correct
bool Properties::isValid() const
{
    return mass > 0.0;
}

//! Number of not NaN values
int Properties::numValidValues() const
{
    return validValues().size();
}

//! Not NaN values of inertia properties
std::vector<double> Properties::validValues() const
{
    std::vector<double> result;
    std::vector<double> values = allValues();
    std::copy_if(values.begin(), values.end(), std::back_inserter(result), [](double value) { return !std::isnan(value); });
    return result;
}

//! Values of inertia properties
std::vector<double> Properties::allValues() const
{
    std::vector<double> result;
    result.push_back(mass);
    for (int i = 0; i != skNumDirections; ++i)
    {
        result.push_back(centerGravity[i]);
        result.push_back(inertiaMoments[i]);
        result.push_back(inertiaProducts[i]);
    }
    return result;
}

//! Found the maximum absolute valid value
double Properties::maxAbsValidValue() const
{
    std::vector<double> const& values = validValues();
    double result = -std::numeric_limits<double>::infinity();
    for (double v : values)
        result = std::max(result, std::abs(v));
    return result;
}

//! Compare two sets of properties with weights
Properties Properties::compare(Properties const& another, Properties const& weight) const
{
    double const kWeightThreshold = 0.0;
    double const kDefaultValue = std::nan("");

    Properties result(kDefaultValue);
    if (weight.mass > kWeightThreshold)
        result.mass = Utility::relativeError(mass, another.mass);
    for (int i = 0; i != skNumDirections; ++i)
    {
        if (weight.centerGravity[i] > kWeightThreshold)
            result.centerGravity[i] = Utility::relativeError(centerGravity[i], another.centerGravity[i]);
        if (weight.inertiaMoments[i] > kWeightThreshold)
            result.inertiaMoments[i] = Utility::relativeError(inertiaMoments[i], another.inertiaMoments[i]);
        if (weight.inertiaProducts[i] > kWeightThreshold)
            result.inertiaProducts[i] = Utility::relativeError(inertiaProducts[i], another.inertiaProducts[i]);
    }
    return result;
}

//! Read properties from a XML stream
void Properties::read(QXmlStreamReader& stream)
{
    while (stream.readNextStartElement())
    {
        if (stream.name() == "mass")
            mass = stream.readElementText().toDouble();
        else if (stream.name() == "centerGravity")
            Utility::readData(centerGravity.data(), centerGravity.size(), stream);
        else if (stream.name() == "inertiaMoments")
            Utility::readData(inertiaMoments.data(), inertiaMoments.size(), stream);
        else if (stream.name() == "inertiaProducts")
            Utility::readData(inertiaProducts.data(), inertiaProducts.size(), stream);
        else
            stream.skipCurrentElement();
    }
}

//! Write properties to a XML stream
void Properties::write(QString const& name, QXmlStreamWriter& stream) const
{
    stream.writeStartElement(name);
    stream.writeTextElement("mass", QString::number(mass));
    Utility::writeData("centerGravity", centerGravity.data(), centerGravity.size(), stream);
    Utility::writeData("inertiaMoments", inertiaMoments.data(), inertiaMoments.size(), stream);
    Utility::writeData("inertiaProducts", inertiaProducts.data(), inertiaProducts.size(), stream);
    stream.writeEndElement();
}
