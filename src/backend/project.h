/*!
 * \file
 * \author Pavel Lakiza
 * \date May 2025
 * \brief Declaration of the Project class
 */

#ifndef PROJECT_H
#define PROJECT_H

#include <QDateTime>
#include <QString>

#include "optimizer.h"
#include "panel.h"

QT_FORWARD_DECLARE_CLASS(QXmlStreamReader)
QT_FORWARD_DECLARE_CLASS(QXmlStreamWriter)

namespace Backend
{

//! Project configuration
class Configuration
{
public:
    Configuration();

    QDateTime const& creationDate() const;
    QString name;
    Optimizer::State state;
    Properties target;
    Properties weight;
    Optimizer::Options options;

    void read(QXmlStreamReader& stream);
    void write(QXmlStreamWriter& stream);

private:
    QDateTime mCreationDate;
};

//! Project which contains panel and solution results
class Project
{
public:
    Project(QString const& name = QString());
    ~Project() = default;

    Configuration& configuration();
    Panel& panel();
    QList<Optimizer::Solution> const& solutions() const;
    QString const& pathFile() const;

    void setSolutions(QList<Optimizer::Solution> const& solutions);

    void clear();
    bool read(QString const& pathFile);
    bool write(QString const& pathFile);

    static QString fileSuffix();

private:
    Configuration mConfiguration;
    Panel mPanel;
    QList<Optimizer::Solution> mSolutions;
    QString mPathFile;
};

}

#endif // PROJECT_H
