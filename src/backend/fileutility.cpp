/*!
 * \file
 * \author Pavel Lakiza
 * \date June 2025
 * \brief Implementation of file utilities
 */

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "fileutility.h"

namespace Backend::Utility
{

//! Open a file and check its extension
QSharedPointer<QFile> openFile(QString const& pathFile, QString const& expectedSuffix, QIODevice::OpenModeFlag const& mode)
{
    // Check if the output file has the correct extension
    QFileInfo info(pathFile);
    if (info.suffix() != expectedSuffix)
    {
        qWarning() << QObject::tr("Unknown extension was specified for the file: %1").arg(pathFile);
        return nullptr;
    }

    // Open the file for the specified mode
    QSharedPointer<QFile> pFile(new QFile(pathFile));
    if (!pFile->open(mode))
    {
        qWarning() << QObject::tr("Could not open the file: %1").arg(pathFile);
        return nullptr;
    }
    return pFile;
}

//! Read a vector from a XML stream
void readData(double* pBegin, int numData, QXmlStreamReader& stream)
{
    double* pData = pBegin;
    double* pEnd = pBegin + numData;
    while (stream.readNextStartElement())
    {
        if (stream.name() == "value")
        {
            if (pData < pEnd)
            {
                *pData = stream.readElementText().toDouble();
                ++pData;
            }
        }
        else
        {
            stream.skipCurrentElement();
        }
    }
}

//! Write a vector to a XML stream
void writeData(QString const& name, double const* pBegin, int numData, QXmlStreamWriter& stream)
{
    stream.writeStartElement(name);
    double const* pEnd = pBegin + numData;
    for (double const* pData = pBegin; pData != pEnd; ++pData)
        stream.writeTextElement("value", QString::number(*pData));
    stream.writeEndElement();
}

}
