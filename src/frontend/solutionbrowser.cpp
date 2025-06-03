/*!
 * \file
 * \author Pavel Lakiza
 * \date June 2025
 * \brief Implementation of the SolutionBrowser class
 */

#include <QListView>
#include <QMenu>
#include <QVBoxLayout>

#include "solutionbrowser.h"
#include "solutionmodel.h"

using namespace Frontend;

SolutionBrowser::SolutionBrowser(QWidget* pParent)
    : QWidget(pParent)
{
    createContent();
}

SolutionBrowser::~SolutionBrowser()
{
}

QSize SolutionBrowser::sizeHint() const
{
    return QSize(50, 600);
}

//! Update the viewer content
void SolutionBrowser::update(QList<Backend::Optimizer::Solution> const& solutions)
{
    SolutionModel* pModel = new SolutionModel(solutions);

    // Set the model
    mpView->setModel(nullptr);
    mpView->setModel(pModel);

    // Specify the connections between the model and view widget
    connect(mpView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &SolutionBrowser::processSelection);

    // Select the last item
    mpView->setCurrentIndex(pModel->index(pModel->rowCount() - 1, 0));
}

//! Create all the widgets and corresponding actions
void SolutionBrowser::createContent()
{
    // Create the view widget
    mpView = new QListView;
    mpView->setSelectionMode(QAbstractItemView::SingleSelection);
    mpView->setSelectionBehavior(QAbstractItemView::SelectItems);
    mpView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mpView->setContextMenuPolicy(Qt::CustomContextMenu);

    // Connect the view widget
    connect(mpView, &QListView::customContextMenuRequested, this, &SolutionBrowser::processContextMenu);

    // Insert the widgets into the main layout
    QVBoxLayout* pLayout = new QVBoxLayout;
    pLayout->addWidget(mpView);
    setLayout(pLayout);
}

//! Create the context menu
void SolutionBrowser::processContextMenu(QPoint const& point)
{
    // Retrieve the selected items
    QModelIndexList indices = mpView->selectionModel()->selectedIndexes();
    if (indices.isEmpty())
        return;
    int iSolution = indices.first().row();

    // Create the context menu
    QMenu* pMenu = new QMenu(this);
    pMenu->setAttribute(Qt::WA_DeleteOnClose);

    // Create the actions
    QAction* pViewAction = new QAction(tr("&View panel"));
    QAction* pSetAction = new QAction(tr("&Set panel"));

    // Fill up the menu
    pMenu->addAction(pViewAction);
    pMenu->addAction(pSetAction);

    // Connect the actions
    connect(pViewAction, &QAction::triggered, this, [this, iSolution]() { emit viewPanelRequested(iSolution); });
    connect(pSetAction, &QAction::triggered, this, [this, iSolution]() { emit setPanelRequested(iSolution); });

    // Show the menu
    QPoint position = mpView->mapToGlobal(point);
    pMenu->exec(position);
}

//! Process selection of model items
void SolutionBrowser::processSelection(QItemSelection const& selected, QItemSelection const& deselected)
{
    QModelIndexList indices = mpView->selectionModel()->selectedIndexes();
    if (!indices.empty())
        emit solutionSelected(indices.first().row());
    else
        emit solutionSelected(-1);
}
