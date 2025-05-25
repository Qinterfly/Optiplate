/*!
 * \file
 * \author Pavel Lakiza
 * \date May 2025
 * \brief Implementation of the PropertiesViewer class
 */

#include <QApplication>
#include <QHeaderView>
#include <QTableWidget>
#include <QVBoxLayout>

#include "propertiesviewer.h"

using namespace Frontend;

PropertiesViewer::PropertiesViewer(QWidget* pParent)
    : QWidget(pParent)
{
    createContent();
}

PropertiesViewer::~PropertiesViewer()
{
}

//! Create all the widgets associated with the viewer
void PropertiesViewer::createContent()
{
    QVBoxLayout* pLayout = new QVBoxLayout;

    // Create and configure the editor
    mpTable = new QTableWidget;

    // Combine all the widgets
    pLayout->addWidget(mpTable);

    // Set the layout
    setLayout(pLayout);
}

//! Clear the viewer content
void PropertiesViewer::clear()
{
    mpTable->clear();
}

//! Update the text
void PropertiesViewer::update(Backend::Panel const& panel, Backend::Properties const& target)
{
    QStringList const kDirections = {"X", "Y", "Z"};
    int const kNumDirections = kDirections.size();

    // Clean up all the content
    clear();

    // Create the working functions
    auto appendRow = [this](QString const& name, double value, double error)
    {
        int iRow = mpTable->rowCount();
        mpTable->insertRow(iRow);
        mpTable->setItem(iRow, 0, new QTableWidgetItem(name));
        mpTable->setItem(iRow, 1, new QTableWidgetItem(QString::number(value, 'g', 5)));
        mpTable->setItem(iRow, 2, new QTableWidgetItem(QString::number(value * 100, 'e', 2)));
    };

    // Retrieve current mass properties and compare them with the target ones
    Backend::Properties current = panel.massProperties();
    if (!current.isValid())
        return;
    Backend::Properties errors = current.compare(target);

    // Set the data
    mpTable->setColumnCount(3);
    appendRow(tr("Mass"), current.mass, errors.mass);
    for (int i = 0; i != kNumDirections; ++i)
        appendRow(tr("Center of gravity %1").arg(kDirections[i]), current.centerGravity[i], errors.centerGravity[i]);
    for (int i = 0; i != kNumDirections; ++i)
        appendRow(tr("Inertia moment %1").arg(kDirections[i]), current.inertiaMoments[i], errors.inertiaMoments[i]);
    for (int i = 0; i != kNumDirections; ++i)
        appendRow(tr("Inertia product %1").arg(kDirections[i]), current.inertiaProducts[i], errors.inertiaProducts[i]);

    // Set the header
    mpTable->verticalHeader()->hide();
    mpTable->setHorizontalHeaderLabels({tr("Property"), tr("Value"), tr("Error, %")});
}
