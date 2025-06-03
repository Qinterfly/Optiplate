/*!
 * \file
 * \author Pavel Lakiza
 * \date June 2025
 * \brief Implementation of the SolutionBrowser class
 */

#include <QListView>
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


    // Insert the widgets into the main layout
    QVBoxLayout* pLayout = new QVBoxLayout;
    pLayout->addWidget(mpView);
    setLayout(pLayout);
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
