#include "StdAfx.h"
#include "PinItem.h"
#include <NodeGraph/PinWidget.h>

static int s_pinIdCounter = 0;

CPinItem::CPinItem(CStateNodeItem& node, CAnimStateMachineGraphViewModel& model, bool isOutput)
	: CAbstractPinItem(model)
	, m_node(node)
	, m_id(++s_pinIdCounter)
	, m_isOutput(isOutput)
{
}

CryGraphEditor::CPinWidget* CPinItem::CreateWidget(CryGraphEditor::CNodeWidget& nodeWidget, CryGraphEditor::CNodeGraphView& view)
{
	return new CryGraphEditor::CPinWidget(*this, nodeWidget, view);
}

bool CPinItem::CanConnect(const CryGraphEditor::CAbstractPinItem* pOtherPin) const
{
	if (!pOtherPin)
		return false;
	return (IsInputPin() && pOtherPin->IsOutputPin()) || (IsOutputPin() && pOtherPin->IsInputPin());
}
