/*!
 * \file
 * \author Pavel Lakiza
 * \date May 2025
 * \brief Implementation of the PropertiesViewer class
 */

#include <QApplication>
#include <QClipboard>
#include <QHeaderView>
#include <QKeyEvent>
#include <QMimeData>
#include <QTableWidget>
#include <QVBoxLayout>

#include "propertiesviewer.h"
#include "uiutility.h"

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
    mpTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mpTable->setSelectionMode(QAbstractItemView::ContiguousSelection);
    mpTable->verticalHeader()->hide();

    // Combine all the widgets
    pLayout->addWidget(mpTable);

    // Set the layout
    setLayout(pLayout);
}

//! Clear the viewer content
void PropertiesViewer::clear()
{
    mpTable->clear();
    mpTable->setRowCount(0);
    mpTable->horizontalHeader()->hide();
}

//! Update the content
void PropertiesViewer::update(Backend::Panel const& panel, Backend::Properties const& target)
{
    double const kTimeout = 1.0;
    Backend::Properties current = panel.massProperties(kTimeout);
    update(current, target);
}

void PropertiesViewer::update(Backend::Properties const& current, Backend::Properties const& target)
{
    int const kNumColumns = 4;
    QStringList const kDirections = {"X", "Y", "Z"};
    int const kNumDirections = kDirections.size();

    // Clean up all the content
    clear();

    // Retrieve current mass properties and compare them with the target ones
    if (!current.isValid())
        return;
    Backend::Properties errors = current.compare(target);

    // Set the data
    mpTable->setColumnCount(kNumColumns);
    appendRow(tr("Mass"), current.mass, target.mass, errors.mass);
    for (int i = 0; i != kNumDirections; ++i)
        appendRow(tr("Center of gravity %1").arg(kDirections[i]), current.centerGravity[i], target.centerGravity[i], errors.centerGravity[i]);
    for (int i = 0; i != kNumDirections; ++i)
        appendRow(tr("Inertia moment %1").arg(kDirections[i]), current.inertiaMoments[i], target.inertiaMoments[i], errors.inertiaMoments[i]);
    for (int i = 0; i != kNumDirections; ++i)
        appendRow(tr("Inertia product %1").arg(kDirections[i]), current.inertiaProducts[i], target.inertiaProducts[i], errors.inertiaProducts[i]);

    // Set the data alignment
    for (int iRow = 0; iRow != mpTable->rowCount(); ++iRow)
        for (int iColumn = 0; iColumn != mpTable->columnCount(); ++iColumn)
            mpTable->item(iRow, iColumn)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    // Set the header
    mpTable->horizontalHeader()->show();
    mpTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    mpTable->setHorizontalHeaderLabels({tr("Property"), tr("Current value"), tr("Target value"), tr("Error, %")});
}

//! Copy the table content when the
void PropertiesViewer::keyPressEvent(QKeyEvent* pEvent)
{
    if (pEvent->key() == Qt::Key_C && pEvent->modifiers() & Qt::ControlModifier)
    {
        QList<QTableWidgetItem*> const& items = mpTable->selectedItems();
        int iLastRow = 0;
        int numItems = items.size();
        QByteArray text;
        for (int i = 0; i != numItems; ++i)
        {
            QTableWidgetItem* pItem = items[i];
            int iRow = pItem->row();
            if (i > 0)
            {
                if (iRow != iLastRow)
                    text.append('\n');
                else
                    text.append('\t');
            }
            text.append(pItem->text().toStdString());
            iLastRow = iRow;
        }
        QMimeData* pMimeData = new QMimeData;
        pMimeData->setData("text/plain", text);
        QApplication::clipboard()->setMimeData(pMimeData);
    }
}

//! Append a property
void PropertiesViewer::appendRow(QString const& name, double current, double target, double error)
{
    int iRow = mpTable->rowCount();
    mpTable->insertRow(iRow);
    // Name
    mpTable->setItem(iRow, 0, new QTableWidgetItem(name));
    // Current value
    mpTable->setItem(iRow, 1, new QTableWidgetItem(QString::number(current, 'g', 5)));
    // Target value
    mpTable->setItem(iRow, 2, new QTableWidgetItem(QString::number(target, 'g', 5)));
    // Error
    error *= 100;
    QColor color(Utility::errorColorName(error));
    QTableWidgetItem* pItem = new QTableWidgetItem(Utility::toString(error, 3));
    pItem->setData(Qt::DecorationRole, color);
    mpTable->setItem(iRow, 3, pItem);
};
