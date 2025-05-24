/*!
 * \file
 * \author Pavel Lakiza
 * \date May 2025
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

const QString skGeometry = "geometry";
const QString skState = "state";
const QString skDockingState = "dockingState";
const QString skRecent = "recent";
const QString skFileName = "Settings.ini";
const QString skMainWindow = "mainWindow";

} // namespace Settings

namespace Size
{

const QSize skToolBarIcon = QSize(25, 25);
const uint skMaxRecentProjects = 5;

} // namespace Size

} // namespace Frontend::Constants

#endif // UICONSTANTS_H
