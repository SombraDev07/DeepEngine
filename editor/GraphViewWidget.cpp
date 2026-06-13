#include "StdAfx.h"
#include "GraphViewWidget.h"
#include "GraphViewModel.h"
#include "NodeGraphRuntimeContext.h"
#include "StateNodeItem.h"
#include <NodeGraph/NodeGraphViewStyle.h>
#include <NodeGraph/NodeWidget.h>
#include <AssetSystem/Asset.h>
#include <AssetSystem/AssetManager.h>
#include <AssetSystem/Browser/AssetModel.h>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QFileInfo>

CGraphViewWidget::CGraphViewWidget(CAnimStateMachineEditor& editor)
	: m_editor(editor)
{
	auto pContext = std::make_shared<CNodeGraphRuntimeContext>();
	m_pModel = new CAnimStateMachineGraphViewModel(*this, pContext);
	SetModel(m_pModel);
	setAcceptDrops(true);
}

QWidget* CGraphViewWidget::CreatePropertiesWidget(CryGraphEditor::GraphItemSet& selectedItems)
{
	return nullptr;
}

void CGraphViewWidget::dragEnterEvent(QDragEnterEvent* pEvent)
{
	auto pDragDropData = CDragDropData::FromMimeData(pEvent->mimeData());
	if (pDragDropData && pDragDropData->HasCustomData("Assets"))
	{
		CDragDropData::ShowDragText(this, "Assign to state node");
		pEvent->acceptProposedAction();
		return;
	}
	pEvent->ignore();
}

void CGraphViewWidget::dragMoveEvent(QDragMoveEvent* pEvent)
{
	pEvent->acceptProposedAction();
}

void CGraphViewWidget::dropEvent(QDropEvent* pEvent)
{
	auto pDragDropData = CDragDropData::FromMimeData(pEvent->mimeData());
	if (!pDragDropData || !pDragDropData->HasCustomData("Assets"))
	{
		pEvent->ignore();
		return;
	}

	auto assets = CAssetModel::GetAssets(*pDragDropData);
	if (assets.empty())
	{
		pEvent->ignore();
		return;
	}

	QPointF scenePos = mapToScene(pEvent->pos());
	CStateNodeItem* pNode = FindStateNodeAt(scenePos);

	if (pNode)
	{
		HandleAssetDrop(assets, pNode);
		pEvent->acceptProposedAction();
	}
	else
	{
		pEvent->ignore();
	}
}

CStateNodeItem* CGraphViewWidget::FindStateNodeAt(const QPointF& scenePos) const
{
	QList<QGraphicsItem*> itemsList = scene()->items(scenePos);
	for (QGraphicsItem* pItem : itemsList)
	{
		CryGraphEditor::CNodeWidget* pNodeWidget = qgraphicsitem_cast<CryGraphEditor::CNodeWidget*>(pItem);
		if (pNodeWidget)
		{
			return static_cast<CStateNodeItem*>(&pNodeWidget->GetItem());
		}
	}
	return nullptr;
}

void CGraphViewWidget::HandleAssetDrop(const std::vector<CAsset*>& assets, CStateNodeItem* pNode)
{
	if (assets.empty() || !pNode)
		return;

	const char* typeName = assets[0]->GetType()->GetTypeName();
	QString path = assets[0]->GetFile(0);

	if (strcmp(typeName, "Animation") == 0)
	{
		pNode->SetAnimationPath(path);
		pNode->SetName(QFileInfo(path).baseName());
	}
}
