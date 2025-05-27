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

    // int iRow = rowCount();
    // insertRow(iRow);
    // QString type = tr("Unknown");
    // QIcon icon = QIcon(":/icons/dialog-question.svg");
    // switch (messageType)
    // {
    // case QtDebugMsg:
    //     type = QObject::tr("Debug");
    //     icon = QIcon(":/icons/dialog-debug.svg");
    //     break;
    // case QtInfoMsg:
    //     type = QObject::tr("Info");
    //     icon = QIcon(":/icons/dialog-info.svg");
    //     break;
    // case QtWarningMsg:
    //     type = QObject::tr("Warning");
    //     icon = QIcon(":/icons/dialog-warning.svg");
    //     break;
    // case QtCriticalMsg:
    //     type = QObject::tr("Critical");
    //     icon = QIcon(":/icons/dialog-error.svg");
    //     break;
    // case QtFatalMsg:
    //     type = QObject::tr("Fatal");
    //     icon = QIcon(":/icons/dialog-fatal.svg");
    //     break;
    // }
    // QString time = QTime::currentTime().toString();
    // QString filterMessage = message;
    // filterMessage.remove('\"');
    // setItem(iRow, ColumnType::kTime, new QTableWidgetItem(time));
    // setItem(iRow, ColumnType::kType, new QTableWidgetItem(icon, type));
    // setItem(iRow, ColumnType::kMessage, new QTableWidgetItem(filterMessage));

    // // The last column is always maximized
    // horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
    // horizontalHeader()->setSectionResizeMode(ColumnType::kMessage, QHeaderView::ResizeMode::Stretch);
    // scrollToBottom();
}
