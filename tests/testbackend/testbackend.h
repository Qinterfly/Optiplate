/*!
 * \file
 * \author Pavel Lakiza
 * \date May 2025
 * \brief Declaration of the TestBackend class
 */

#ifndef TESTBACKEND_H
#define TESTBACKEND_H

#include <QTest>

#include "project.h"

namespace Tests
{

class TestBackend : public QObject
{
    Q_OBJECT

public:
    TestBackend();
    virtual ~TestBackend() = default;

private slots:
    // Base
    void testCreateBasePanel();
    void testUpdateBasePanel();
    // Real
    void testCreateRealPanel();
    void testUpdateRealPanel();

private:
    bool isEqual(double firstValue, double secondValue, double precision);

    Backend::Project mBaseProject;
    Backend::Project mRealProject;
};

}

#endif // TESTBACKEND_H
