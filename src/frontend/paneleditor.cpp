/*!
 * \file
 * \author Pavel Lakiza
 * \date June 2025
 * \brief Implementation of the PanelEditor class
 */

#include <QDialog>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QToolBar>
#include <QVBoxLayout>

#include "paneleditor.h"
#include "qtpropertybrowser/customdoublespinbox.h"
#include "uiconstants.h"
#include "uiutility.h"

using namespace Frontend;
using namespace Backend;

QString PanelEditor::mLastPath = QString();

PanelEditor::PanelEditor(Backend::Panel& panel, QWidget* pParent)
    : QWidget(pParent)
    , mPanel(panel)
{
    createContent();
    update();
}

PanelEditor::~PanelEditor()
{
}

QSize PanelEditor::sizeHint() const
{
    return QSize(325, 75);
}

//! Create all the widgets and corresponding actions
void PanelEditor::createContent()
{
    // Constants
    int kNumDecimals = 5;
    QPair<double, double> kLimitsDensity = {0, 1e5};
    QPair<double, double> kLimitsCoords = {-100, 100};
    QPair<double, double> kLimitsDepths = {0, 1};

    // Create the widgets to edit properties
    mpDataLayout = new QGridLayout;
    // Density
    mpDataLayout->addWidget(new QLabel(tr("Density:")), kDensity, 0);
    mpDataLayout->addWidget(createDoubleSpinBox(kLimitsDensity, kNumDecimals), kDensity, 1);
    // Coordinates
    mpDataLayout->addWidget(new QLabel(tr("X coordinates:")), kXCoords, 0);
    mpDataLayout->addWidget(new QLabel(tr("Z coordinates:")), kZCoords, 0);
    int numCoordinates = mPanel.xCoords().size();
    for (int i = 0; i != numCoordinates; ++i)
    {
        mpDataLayout->addWidget(createDoubleSpinBox(kLimitsCoords, kNumDecimals), kXCoords, 1 + i);
        mpDataLayout->addWidget(createDoubleSpinBox(kLimitsCoords, kNumDecimals), kZCoords, 1 + i);
    }
    // Depths
    mpDataLayout->addWidget(new QLabel(tr("Depths:")), kDepths, 0);
    int numDepths = mPanel.depths().size();
    for (int i = 0; i != numDepths; ++i)
        mpDataLayout->addWidget(createDoubleSpinBox(kLimitsDepths, kNumDecimals), kDepths, 1 + i);
    // Prevent stretching
    mpDataLayout->setRowStretch(mpDataLayout->rowCount() + 1, 1);
    mpDataLayout->setColumnStretch(mpDataLayout->columnCount() + 1, 1);

    // Create the toolbar
    QToolBar* pToolBar = new QToolBar;
    pToolBar->setIconSize(Constants::Size::skToolBarIcon);
    pToolBar->setOrientation(Qt::Vertical);

    // Create the actions
    QAction* pExportAction = new QAction(QIcon(":/icons/export.svg"), tr("E&xport"), this);
    QAction* pRoundAction = new QAction(QIcon(":/icons/cut.svg"), tr("&Round"), this);

    // Connect the actions
    connect(pExportAction, &QAction::triggered, this, &PanelEditor::processExport);
    connect(pRoundAction, &QAction::triggered, this, &PanelEditor::processRound);

    // Add the actions
    pToolBar->addAction(pExportAction);
    pToolBar->addAction(pRoundAction);
    Utility::setShortcutHints(pToolBar);

    // Set the resulting layout
    QHBoxLayout* pMainLayout = new QHBoxLayout;
    pMainLayout->addLayout(mpDataLayout);
    pMainLayout->addWidget(pToolBar);
    setLayout(pMainLayout);
}

//! Update values of widgets
void PanelEditor::update()
{
    // Create the setter
    auto setValue = [this](double value, int iRow, int iColumn)
    {
        CustomDoubleSpinBox* pSpinBox = (CustomDoubleSpinBox*) mpDataLayout->itemAtPosition(iRow, iColumn)->widget();
        pSpinBox->blockSignals(true);
        pSpinBox->setValue(value);
        pSpinBox->blockSignals(false);
    };

    // Refresh the density
    setValue(mPanel.density(), kDensity, 1);

    // Refresh the coordinates
    int numCoordinates = mPanel.xCoords().size();
    for (int i = 0; i != numCoordinates; ++i)
    {
        setValue(mPanel.xCoords()[i], kXCoords, 1 + i);
        setValue(mPanel.zCoords()[i], kZCoords, 1 + i);
    }

    // Refresh the depths
    int numDepths = mPanel.depths().size();
    for (int i = 0; i != numDepths; ++i)
        setValue(mPanel.depths()[i], kDepths, 1 + i);
}

CustomDoubleSpinBox* PanelEditor::createDoubleSpinBox(QPair<double, double> const& limits, int numDecimals)
{
    CustomDoubleSpinBox* pSpinBox = new CustomDoubleSpinBox;
    pSpinBox->setMinimum(limits.first);
    pSpinBox->setMaximum(limits.second);
    pSpinBox->setDecimals(numDecimals);
    pSpinBox->setStepType(CustomDoubleSpinBox::AdaptiveDecimalStepType);
    pSpinBox->setWheelEnabled(false);
    connect(pSpinBox, &QDoubleSpinBox::valueChanged, this, [this](double) { processDataChange(); });
    return pSpinBox;
}

//! Process changing data of a panel
void PanelEditor::processDataChange()
{
    // Create the getter
    auto getValue = [this](int iRow, int iColumn)
    {
        CustomDoubleSpinBox* pSpinBox = (CustomDoubleSpinBox*) mpDataLayout->itemAtPosition(iRow, iColumn)->widget();
        return pSpinBox->value();
    };

    // Modify the panel density
    mPanel.setDensity(getValue(kDensity, 1));

    // Modify the panel coordinates
    KCL::Vec4 xCoords = mPanel.xCoords();
    KCL::Vec4 zCoords = mPanel.zCoords();
    int numCoordinates = xCoords.size();
    for (int i = 0; i != numCoordinates; ++i)
    {
        xCoords[i] = getValue(kXCoords, 1 + i);
        zCoords[i] = getValue(kZCoords, 1 + i);
    }
    mPanel.setXCoords(xCoords);
    mPanel.setZCoords(zCoords);

    // Modify the panel depths
    KCL::Vec3 depths = mPanel.depths();
    int numDepths = xCoords.size();
    for (int i = 0; i != depths.size(); ++i)
        depths[i] = getValue(kDepths, 1 + i);
    mPanel.setDepths(depths);

    emit dataChanged();
}

//! Process exporting of panel data to a file
void PanelEditor::processExport()
{
    QString pathFile = QFileDialog::getSaveFileName(this, tr("Save panel"), PanelEditor::mLastPath, tr("KCL files (*.txt *.dat)"));
    if (pathFile.isEmpty())
        return;
    mPanel.write(pathFile);
}

//! Process rounding off panel data
void PanelEditor::processRound()
{
    // Create the dialog
    QDialog* pDialog = new QDialog(this);
    pDialog->setWindowTitle(tr("Round panel data"));

    // Construct the precision layout
    QHBoxLayout* pDecimalsLayout = new QHBoxLayout;
    QSpinBox* pDecimalsEdit = new QSpinBox;
    pDecimalsEdit->setMinimum(0);
    pDecimalsEdit->setMaximum(17);
    pDecimalsEdit->setValue(5);
    pDecimalsLayout->addWidget(new QLabel(tr("Precision: ")));
    pDecimalsLayout->addWidget(pDecimalsEdit, 1, Qt::AlignLeft);

    // Construct the control layout
    QHBoxLayout* pControlLayout = new QHBoxLayout;
    QPushButton* pRoundButton = new QPushButton(tr("Round"));
    pControlLayout->addWidget(pRoundButton, 0, Qt::AlignRight);

    // Create the connectinos
    connect(pRoundButton, &QPushButton::clicked, this,
            [this, pDecimalsEdit]()
            {
                int numDecimals = pDecimalsEdit->value();
                double precision = qPow(10, -numDecimals);
                mPanel = mPanel.round(precision, precision);
                update();
                qInfo() << tr("Panel data rounded to %1 decimals").arg(QString::number(numDecimals));
                emit dataChanged();
            });

    // Combine all the widgets
    QVBoxLayout* pMainLayout = new QVBoxLayout;
    pMainLayout->addLayout(pDecimalsLayout);
    pMainLayout->addLayout(pControlLayout);
    pDialog->setLayout(pMainLayout);

    // Execute the dialog
    pDialog->exec();
}
