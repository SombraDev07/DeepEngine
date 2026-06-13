#pragma once
#include <NodeGraph/AbstractNodeGraphViewModel.h>
#include <CryAnimation/IAnimationStateMachine.h>
#include <vector>

class CGraphViewWidget;

class CAnimStateMachineGraphViewModel : public CryGraphEditor::CNodeGraphViewModel
{
public:
	CAnimStateMachineGraphViewModel(CGraphViewWidget& view, std::shared_ptr<CryGraphEditor::INodeGraphRuntimeContext> pContext);

	void SetDefinition(const SAnimationStateMachineDef& def);
	const SAnimationStateMachineDef& GetDefinition() const { return m_def; }
	SAnimationStateMachineDef& GetDefinition() { return m_def; }

	virtual CryGraphEditor::INodeGraphRuntimeContext& GetRuntimeContext() override;
	virtual QString GetGraphName() override;
	virtual uint32 GetNodeItemCount() const override;
	virtual CryGraphEditor::CAbstractNodeItem* GetNodeItemByIndex(uint32 index) const override;
	virtual CryGraphEditor::CAbstractNodeItem* GetNodeItemById(QVariant id) const override;
	virtual CryGraphEditor::CAbstractNodeItem* CreateNode(QVariant typeId, const QPointF& position) override;
	virtual bool RemoveNode(CryGraphEditor::CAbstractNodeItem& node) override;
	virtual uint32 GetConnectionItemCount() const override;
	virtual CryGraphEditor::CAbstractConnectionItem* GetConnectionItemByIndex(uint32 index) const override;
	virtual CryGraphEditor::CAbstractConnectionItem* GetConnectionItemById(QVariant id) const override;
	virtual CryGraphEditor::CAbstractConnectionItem* CreateConnection(CryGraphEditor::CAbstractPinItem& sourcePin, CryGraphEditor::CAbstractPinItem& targetPin) override;
	virtual bool RemoveConnection(CryGraphEditor::CAbstractConnectionItem& connection) override;

private:
	std::shared_ptr<CryGraphEditor::INodeGraphRuntimeContext> m_pContext;
	SAnimationStateMachineDef m_def;
	std::vector<std::unique_ptr<CryGraphEditor::CAbstractNodeItem>> m_nodes;
	std::vector<std::unique_ptr<CryGraphEditor::CAbstractConnectionItem>> m_connections;
	CGraphViewWidget& m_view;
};
