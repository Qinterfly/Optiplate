/*!
 * \file
 * \author Pavel Lakiza
 * \date May 2025
 * \brief Implementation of the Project class
 */

#include <QXmlStreamWriter>

#include "fileutility.h"
#include "project.h"

using namespace Backend;

static const QString skProjectIOVersion = "1.0";

Configuration::Configuration()
    : target(0.0)
    , weight(1.0)
{
    mCreationDate = QDateTime::currentDateTime();
}

QDateTime const& Configuration::creationDate() const
{
    return mCreationDate;
}

//! Read configuration from a XML stream
void Configuration::read(QXmlStreamReader& stream)
{
    while (stream.readNextStartElement())
    {
        if (stream.name() == "creationDate")
            mCreationDate = QDateTime::fromString(stream.readElementText());
        else if (stream.name() == "name")
            name = stream.readElementText();
        else if (stream.name() == "state")
            state.read(stream);
        else if (stream.name() == "target")
            target.read(stream);
        else if (stream.name() == "weight")
            weight.read(stream);
        else if (stream.name() == "options")
            options.read(stream);
        else
            stream.skipCurrentElement();
    }
}

//! Write configuration to a XML stream
void Configuration::write(QXmlStreamWriter& stream)
{
    stream.writeStartElement("configuration");
    stream.writeTextElement("creationDate", mCreationDate.toString());
    stream.writeTextElement("name", name);
    state.write(stream);
    target.write("target", stream);
    weight.write("weight", stream);
    options.write(stream);
    stream.writeEndElement();
}

Project::Project(QString const& name)
{
    mConfiguration.name = name;
}

Configuration& Project::configuration()
{
    return mConfiguration;
}

Panel& Project::panel()
{
    return mPanel;
}

QList<Optimizer::Solution> const& Project::solutions() const
{
    return mSolutions;
}

QString const& Project::pathFile() const
{
    return mPathFile;
}

void Project::setSolutions(QList<Optimizer::Solution> const& solutions)
{
    mSolutions = solutions;
}

//! Read a project from a file
bool Project::read(QString const& pathFile)
{
    // Open the file for reading
    auto pFile = Utility::openFile(pathFile, Project::fileSuffix(), QIODevice::ReadOnly);
    if (!pFile)
        return false;
    QXmlStreamReader stream(pFile.data());

    // Check the document version
    if (stream.readNext())
    {
        if (stream.documentVersion() != skProjectIOVersion)
        {
            qWarning() << QObject::tr("The unsupported document version detected: %1").arg(stream.documentVersion());
            return false;
        }
    }

    // Check the root item
    stream.readNextStartElement();
    if (stream.name() != Project::fileSuffix())
    {
        qWarning() << QObject::tr("The unsupported project detected: %1").arg(stream.name());
        return false;
    }

    // Read the document content
    while (stream.readNextStartElement())
    {
        if (stream.name() == "configuration")
            mConfiguration.read(stream);
        else if (stream.name() == "panel")
            mPanel.read(stream);
        else if (stream.name() == "solution")
        {
            Optimizer::Solution solution;
            solution.read(stream);
            mSolutions.emplaceBack(std::move(solution));
        }
        else
            stream.skipCurrentElement();
    }

    // Remember the filepath
    mPathFile = pathFile;

    return true;
}

//! Output the project to a file
bool Project::write(QString const& pathFile)
{
    // Open the file for writing
    auto pFile = Utility::openFile(pathFile, Project::fileSuffix(), QIODevice::WriteOnly);
    if (!pFile)
        return false;
    QXmlStreamWriter stream(pFile.data());
    stream.setAutoFormatting(true);

    // Write the document type
    stream.writeStartDocument(skProjectIOVersion);
    stream.writeStartElement(Project::fileSuffix());

    // Write the configuration
    mConfiguration.write(stream);

    // Write the panel data
    mPanel.write(stream);

    // Write the solutions
    for (auto& s : mSolutions)
        s.write(stream);

    // Close the file
    stream.writeEndElement();
    stream.writeEndDocument();
    pFile->close();

    // Remember the filepath
    mPathFile = pathFile;

    return true;
}

//! Extension of a project file
QString Project::fileSuffix()
{
    return "oml";
}
