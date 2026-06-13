#pragma once
#include <NodeGraph/AbstractPinItem.h>

#include "GraphViewModel.h"
#include "StateNodeItem.h"

class CPinItem : public CryGraphEditor::CAbstractPinItem
{
public:
	CPinItem(CStateNodeItem& node, CAnimStateMachineGraphViewModel& model, bool isOutput);

	virtual CryGraphEditor::CPinWidget* CreateWidget(CryGraphEditor::CNodeWidget& nodeWidget, CryGraphEditor::CNodeGraphView& view) override;
	virtual const char* GetStyleId() const override { return "Pin"; }
	virtual CryGraphEditor::CAbstractNodeItem& GetNodeItem() const override { return m_node; }
	virtual QString GetName() const override { return m_isOutput ? "Out" : "In"; }
	virtual QString GetDescription() const override { return {}; }
	virtual QString GetTypeName() const override { return "transition"; }
	virtual QVariant GetId() const override { return m_id; }
	virtual bool HasId(QVariant id) const override { return id == m_id; }
	virtual bool IsInputPin() const override { return !m_isOutput; }
	virtual bool IsOutputPin() const override { return m_isOutput; }
	virtual bool CanConnect(const CryGraphEditor::CAbstractPinItem* pOtherPin) const override;

private:
	CStateNodeItem& m_node;
	QVariant m_id;
	bool m_isOutput;
};
