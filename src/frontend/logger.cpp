/*!
 * \file
 * \author Pavel Lakiza
 * \date May 2025
 * \brief Implementation of the Logger class
 */

#include <QApplication>
#include <QPlainTextEdit>
#include <QVBoxLayout>

#include "logger.h"

using namespace Frontend;

Logger::Logger(QWidget* pParent)
    : QTextEdit(pParent)
{
    setReadOnly(true);
    setFont(QFont("RobotoMono"));
}

Logger::~Logger()
{
}

//! Represent a message sent
void Logger::log(QtMsgType messageType, QString const& message)
{
    QString filterMessage = message;
    filterMessage = filterMessage.remove('\"');
    append(filterMessage);
}
