/*!
 * \file
 * \author Pavel Lakiza
 * \date May 2025
 * \brief Implementation of ui utilities
 */

#include <QApplication>
#include <QFileInfo>
#include <QMessageBox>
#include <QScreen>
#include <QWidget>

#include "uiutility.h"

namespace Frontend::Utility
{

//! Show save dialog when closing a widget and process its output
int showSaveDialog(QWidget* pWidget, QString const& title, QString const& message)
{
    auto isSave = QMessageBox::warning(pWidget, title, message, QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    switch (isSave)
    {
    case QMessageBox::Save:
        return 1;
    case QMessageBox::Discard:
        return 0;
    default:
        return -1;
    }
}

//! Resize widget to maximize its width and height
void fullScreenResize(QWidget* pWidget)
{
    QRect screenGeometry = QApplication::QGuiApplication::primaryScreen()->availableGeometry();
    pWidget->resize(screenGeometry.width(), screenGeometry.height());
}

//! Substitute a file suffix to the expected one, if necessary
void modifyFileSuffix(QString& pathFile, QString const& expectedSuffix)
{
    QFileInfo info(pathFile);
    QString currentSuffix = info.suffix();
    if (currentSuffix.isEmpty())
        pathFile.append(QString(".%1").arg(expectedSuffix));
    else if (currentSuffix != expectedSuffix)
        pathFile.replace(currentSuffix, expectedSuffix);
}

} // namespace Frontend::Utility
