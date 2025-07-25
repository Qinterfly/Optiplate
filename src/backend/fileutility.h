/*!
 * \file
 * \author Pavel Lakiza
 * \date June 2025
 * \brief Declaration of file utilities
 */

#ifndef FILEUTILITY_H
#define FILEUTILITY_H

#include <QDir>
#include <QFile>
#include <QString>

QT_FORWARD_DECLARE_CLASS(QXmlStreamReader)
QT_FORWARD_DECLARE_CLASS(QXmlStreamWriter)

namespace Backend::Utility
{

//! Base case for combining a filepath
template<typename T>
QString combineFilePath(T const& value)
{
    return value;
}

//! Combine several components of a filepath, adding slashes if necessary
template<typename T, typename... Args>
QString combineFilePath(T const& first, Args... args)
{
    return QDir(first).filePath(combineFilePath(args...));
}

QSharedPointer<QFile> openFile(QString const& pathFile, QString const& expectedSuffix, QIODevice::OpenModeFlag const& mode);
void readData(double* pBegin, int numData, QXmlStreamReader& stream);
void writeData(QString const& name, double const* pBegin, int numData, QXmlStreamWriter& stream);
}

#endif // FILEUTILITY_H
