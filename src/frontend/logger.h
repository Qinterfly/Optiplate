/*!
 * \file
 * \author Pavel Lakiza
 * \date May 2025
 * \brief Declaration of the Logger class
 */

#ifndef LOGGER_H
#define LOGGER_H

#include <QWidget>

QT_FORWARD_DECLARE_CLASS(QTextEdit)

namespace Frontend
{

//! Log all the messages sent
class Logger : public QWidget
{
public:
    Logger(QWidget* pParent = nullptr);
    Logger(Logger const& another) = delete;
    Logger& operator=(Logger const& another) = delete;
    ~Logger();

    void log(QtMsgType messageType, const QString& message);

private:
    QTextEdit* mpEdit;
};

}

#endif // LOGGER_H
