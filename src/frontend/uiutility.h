/*!
 * \file
 * \author Pavel Lakiza
 * \date June 2025
 * \brief Declaration of ui utilities
 */

#ifndef UIUTILITY_H
#define UIUTILITY_H

#include <QtGlobal>

#include "optimizer.h"

QT_FORWARD_DECLARE_CLASS(QWidget);
QT_FORWARD_DECLARE_CLASS(QToolBar);

namespace Frontend::Utility
{

// Ui
int showSaveDialog(QWidget* pWidget, QString const& title, QString const& message);
void fullScreenResize(QWidget* pWidget);
void setShortcutHints(QToolBar* pToolBar);
QString errorColorName(double value, double acceptThreshold, double criticalThreshold);
QString toString(double value, int numDecimals);
int findByIteration(QList<Backend::Optimizer::Solution> const& solutions, int iteration);

// File
void modifyFileSuffix(QString& pathFile, QString const& expectedSuffix);
}

#endif // UIUTILITY_H
