#include "StdAfx.h"
#include "TransitionConnection.h"
#include <NodeGraph/ConnectionWidget.h>

CTransitionConnection::CTransitionConnection(CryGraphEditor::CAbstractPinItem& sourcePin, CryGraphEditor::CAbstractPinItem& targetPin, CAnimStateMachineGraphViewModel& model)
	: CAbstractConnectionItem(model)
	, m_pSourcePin(&sourcePin)
	, m_pTargetPin(&targetPin)
	, m_id(reinterpret_cast<quintptr>(this))
{
}

CryGraphEditor::CConnectionWidget* CTransitionConnection::CreateWidget(CryGraphEditor::CNodeGraphView& view)
{
	return new CryGraphEditor::CConnectionWidget(this, view);
}
