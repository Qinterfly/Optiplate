/*!
 * \file
 * \author Pavel Lakiza
 * \date July 2025
 * \brief Implementation of the SolutionBrowser class
 */

#include <QApplication>
#include <QListView>
#include <QMenu>
#include <QVBoxLayout>

#include "solutionbrowser.h"
#include "solutionitem.h"
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

//! Remove the viewer content
void SolutionBrowser::clear()
{
    mpView->setModel(nullptr);
}

//! Update the viewer content
void SolutionBrowser::update(QList<Backend::Optimizer::Solution> const& solutions, Backend::Optimizer::Options const& options)
{
    // Clear the items
    clear();

    // Set the model
    SolutionModel* pModel = new SolutionModel(solutions, options);
    mpView->setModel(pModel);

    // Specify the connections between the model and view widget
    connect(mpView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &SolutionBrowser::processSelection);

    // Select the last item
    int numItems = pModel->rowCount();
    if (numItems > 0)
        mpView->setCurrentIndex(pModel->index(numItems - 1, 0));
    else
        emit solutionSelected();
}

//! Create all the widgets and corresponding actions
void SolutionBrowser::createContent()
{
    // Create the view widget
    mpView = new QListView;
    mpView->setFont(qApp->font());
    mpView->setSelectionMode(QAbstractItemView::SingleSelection);
    mpView->setSelectionBehavior(QAbstractItemView::SelectItems);
    mpView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mpView->setContextMenuPolicy(Qt::CustomContextMenu);
    mpView->setDragEnabled(true);
    mpView->setAcceptDrops(false);

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
    // Check if the model is available
    if (!mpView->selectionModel())
        return;

    // Retrieve the selected items
    QModelIndexList indices = mpView->selectionModel()->selectedIndexes();
    if (indices.isEmpty())
        return;
    SolutionModel* pModel = (SolutionModel*) mpView->model();
    SolutionItem* pItem = (SolutionItem*) pModel->itemFromIndex(indices.first());
    Backend::Optimizer::Solution const& solution = pItem->solution();

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
    connect(pViewAction, &QAction::triggered, this, [this, solution]() { emit viewPanelRequested(solution); });
    connect(pSetAction, &QAction::triggered, this, [this, solution]() { emit setPanelRequested(solution); });

    // Show the menu
    QPoint position = mpView->mapToGlobal(point);
    pMenu->exec(position);
}

//! Process selection of model items
void SolutionBrowser::processSelection(QItemSelection const& selected, QItemSelection const& deselected)
{
    QModelIndexList indices = mpView->selectionModel()->selectedIndexes();
    if (!indices.empty())
    {
        SolutionModel* pModel = (SolutionModel*) mpView->model();
        SolutionItem* pItem = (SolutionItem*) pModel->itemFromIndex(indices.first());
        emit solutionSelected(pItem->solution());
    }
    else
    {
        emit solutionSelected();
    }
}
