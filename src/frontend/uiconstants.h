/*!
 * \file
 * \author Pavel Lakiza
 * \date June 2025
 * \brief Common graphical constants shared between several windows
 */

#ifndef UICONSTANTS_H
#define UICONSTANTS_H

#include <QColor>
#include <QList>
#include <QSize>
#include <QString>

namespace Frontend::Constants
{

namespace Settings
{

const QString skLanguage = "language";
const QString skGeometry = "geometry";
const QString skState = "state";
const QString skDockingState = "dockingState";
const QString skRecent = "recent";
const QString skFileName = "Settings.ini";
const QString skMainWindow = "mainWindow";

}

namespace Size
{

const QSize skToolBarIcon = QSize(25, 25);
const uint skMaxRecentProjects = 5;

}

namespace MimeType
{

const QString skSolutionModel = "project/solution";

}

namespace Render
{

const QList<QColor> skStandardColors = {"red", "green", "blue", "orange", "cyan", "magenta", "gray", "purple", "darkyellow"};

const float skMinRelativeDistance = 0.01f;

}

}

#endif // UICONSTANTS_H
