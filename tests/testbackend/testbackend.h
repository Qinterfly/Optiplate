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
    // Creating panels
    void testCreateRandomPanel();
    void testCreateBasePanel();
    void testCreateRealPanel();

    // Updating panels
    void testUpdateBasePanel();
    void testUpdateRealPanel();

    // Data handling
    void testWriteProject();

private:
    bool isEqual(double firstValue, double secondValue, double precision);
    void log(QString message);
    double generateDouble(QPair<double, double> const& limits);

    Backend::Project mBaseProject;
    Backend::Project mRealProject;
};

}

#endif // TESTBACKEND_H
