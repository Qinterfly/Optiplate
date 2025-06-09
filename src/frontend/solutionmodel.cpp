/*!
 * \file
 * \author Pavel Lakiza
 * \date June 2025
 * \brief Implementation of the SolutionModel class
 */

#include <QMimeData>
#include <QXmlStreamWriter>

#include "project.h"
#include "solutionitem.h"
#include "solutionmodel.h"
#include "uiconstants.h"

using namespace Frontend;

SolutionModel::SolutionModel(QList<Backend::Optimizer::Solution> const& solutions, Backend::Optimizer::Options const& options, QObject* pParent)
    : QStandardItemModel(pParent)
    , mSolutions(solutions)
    , mOptions(options)
{
    QStandardItem* pRootItem = invisibleRootItem();
    for (auto const& solution : solutions)
        pRootItem->appendRow(new SolutionItem(solution, mOptions));
}

SolutionModel::~SolutionModel()
{
}

QStringList SolutionModel::mimeTypes() const
{
    return QStringList() << Constants::MimeType::skSolutionModel;
}

QMimeData* SolutionModel::mimeData(QModelIndexList const& indices) const
{
    QByteArray encodedData;
    QMimeData* pMimeData = new QMimeData;
    QXmlStreamWriter stream(&encodedData);
    stream.setAutoFormatting(true);
    stream.writeStartDocument();
    stream.writeStartElement("mime");
    for (QModelIndex const& index : indices)
    {
        if (index.isValid())
        {
            SolutionItem* pItem = (SolutionItem*) itemFromIndex(indices.first());
            pItem->solution().write(stream);
        }
    }
    stream.writeEndElement();
    stream.writeEndDocument();
    pMimeData->setData(Constants::MimeType::skSolutionModel, encodedData);
    return pMimeData;
}
