/*!
 * \file
 * \author Pavel Lakiza
 * \date May 2025
 * \brief Declaration of the TestBackend class
 */

#ifndef TESTBACKEND_H
#define TESTBACKEND_H

#include <QTest>

#include "panel.h"

namespace Tests
{

class TestBackend : public QObject
{
    Q_OBJECT

public:
    TestBackend();
    virtual ~TestBackend() = default;

private slots:
    void testCreateBasePanel();
    void testCreateRealPanel();

private:
    bool isEqual(double firstValue, double secondValue, double precision);

    Backend::Panel mBasePanel;
    Backend::Panel mRealPanel;
};

}

#endif // TESTBACKEND_H
