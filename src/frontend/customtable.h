/*!
 * \file
 * \author Pavel Lakiza
 * \date November 2023
 * \brief Declaration of the CustomTable class
 */

#ifndef CUSTOMTABLE_H
#define CUSTOMTABLE_H

#include <QTableWidget>

namespace Frontend
{

class CustomTable : public QTableWidget
{
    Q_OBJECT

public:
    CustomTable(QWidget* pParent = nullptr);
    virtual ~CustomTable();

    void setDataAlignment(Qt::Alignment alignment);

private:
    void createActions();
    void copySelection();
    void pasteSelection();
    void clearSelection();
    void processContextMenuRequested(QPoint const& position);
};

}

#endif // CUSTOMTABLE_H
