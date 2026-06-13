#pragma once
#include <NodeGraph/NodeGraphView.h>
#include <DragDrop.h>

class CAnimStateMachineEditor;
class CAnimStateMachineGraphViewModel;
class CStateNodeItem;

class CGraphViewWidget : public CryGraphEditor::CNodeGraphView
{
	Q_OBJECT
public:
	CGraphViewWidget(CAnimStateMachineEditor& editor);
	virtual QWidget* CreatePropertiesWidget(CryGraphEditor::GraphItemSet& selectedItems) override;

protected:
	void dragEnterEvent(QDragEnterEvent* pEvent) override;
	void dragMoveEvent(QDragMoveEvent* pEvent) override;
	void dropEvent(QDropEvent* pEvent) override;

private:
	CStateNodeItem* FindStateNodeAt(const QPointF& scenePos) const;
	void HandleAssetDrop(const std::vector<CAsset*>& assets, CStateNodeItem* pNode);

	CAnimStateMachineEditor& m_editor;
	CAnimStateMachineGraphViewModel* m_pModel;
};
