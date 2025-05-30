/*!
 * \file
 * \author Pavel Lakiza
 * \date May 2025
 * \brief Implementation of the Logger class
 */

#include <QPlainTextEdit>
#include <QVBoxLayout>

#include "logger.h"

using namespace Frontend;

Logger::Logger(QWidget* pParent)
    : QWidget(pParent)
{
    QVBoxLayout* pLayout = new QVBoxLayout;
    mpEdit = new QTextEdit;
    pLayout->addWidget(mpEdit);
    setLayout(pLayout);
}

Logger::~Logger()
{

}

//! Represent a received message
void Logger::log(QtMsgType messageType, QString const& message)
{
    // qDebug() << message;
    // QTextCursor cursor(mpTextEdit->document());
    // cursor.beginEditBlock();
    // cursor.insertText("test");

    // cursor.endEditBlock();
    // mpTextEdit->append("test");
    // mpTextEdit->insertPlainText(message);
    QString filterMessage = message;
    filterMessage.remove('\"');
    mpEdit->append(filterMessage);
    // append(filterMessage);
    // append(message);
    // insertPlainText(message);
    // mMutex.lock();
    // QTextCursor cursor(document());
    // cursor.insertText("test");
    // mMutex.unlock();
}
