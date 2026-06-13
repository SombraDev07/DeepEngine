#include "StdAfx.h"
#include "StateNodeItem.h"
#include "PinItem.h"
#include <NodeGraph/NodeWidget.h>
#include <NodeGraph/PinGridNodeContentWidget.h>

CStateNodeItem::CStateNodeItem(CAnimStateMachineGraphViewModel& model, const QPointF& position)
	: CAbstractNodeItem(*(m_pData = new CryGraphEditor::CNodeEditorData()), model)
	, m_id(reinterpret_cast<quintptr>(this))
	, m_name("NewState")
	, m_position(position)
{
	auto pInputPin = std::make_unique<CPinItem>(*this, model, false);
	auto pOutputPin = std::make_unique<CPinItem>(*this, model, true);
	m_ownedPins.push_back(std::move(pInputPin));
	m_ownedPins.push_back(std::move(pOutputPin));
	for (auto& pin : m_ownedPins) m_pins.push_back(pin.get());
}

CryGraphEditor::CNodeWidget* CStateNodeItem::CreateWidget(CryGraphEditor::CNodeGraphView& view)
{
	auto pWidget = new CryGraphEditor::CNodeWidget(*this, view);
	new CryGraphEditor::CPinGridNodeContentWidget(*pWidget, view);
	return pWidget;
}

CryGraphEditor::CAbstractPinItem* CStateNodeItem::GetPinItemById(QVariant id) const
{
	for (auto& pin : m_ownedPins)
	{
		if (pin->HasId(id))
			return pin.get();
	}
	return nullptr;
}
