/*!
 * \file
 * \author Pavel Lakiza
 * \date June 2025
 * \brief Declaration of the SolutionBrowser class
 */

#ifndef SOLUTIONBROWSER_H
#define SOLUTIONBROWSER_H

#include <QWidget>

#include "optimizer.h"

QT_FORWARD_DECLARE_CLASS(QListView)
QT_FORWARD_DECLARE_CLASS(QItemSelection)

namespace Frontend
{

class SolutionBrowser : public QWidget
{
    Q_OBJECT

public:
    SolutionBrowser(QWidget* pParent = nullptr);
    virtual ~SolutionBrowser();

    QSize sizeHint() const override;
    void update(QList<Backend::Optimizer::Solution> const& solutions);

signals:
    void solutionSelected(Backend::Optimizer::Solution solution = Backend::Optimizer::Solution());
    void viewPanelRequested(Backend::Optimizer::Solution solution);
    void setPanelRequested(Backend::Optimizer::Solution solution);

private:
    // Content
    void createContent();

    // Signals and slots
    void processSelection(QItemSelection const& selected, QItemSelection const& deselected);
    void processContextMenu(QPoint const& point);

private:
    QListView* mpView;
};

}

#endif // SOLUTIONBROWSER_H
