/*!
 * \file
 * \author Pavel Lakiza
 * \date May 2025
 * \brief Implementation of the Logger class
 */

#include <QTime>

#include "logger.h"

using namespace Frontend;

Logger::Logger(QWidget* pParent)
    : QTextEdit(pParent)
{
}

Logger::~Logger()
{

}

//! Represent a received message
void Logger::log(QtMsgType messageType, const QString& message)
{
    QString filterMessage = message;
    filterMessage.remove('\"');
    append(filterMessage);
}
