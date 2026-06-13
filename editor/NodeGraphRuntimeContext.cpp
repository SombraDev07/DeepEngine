#include "StdAfx.h"
#include "NodeGraphRuntimeContext.h"

CNodeGraphRuntimeContext::CNodeGraphRuntimeContext()
{
	m_pStyle = std::make_unique<CryGraphEditor::CNodeGraphViewStyle>("AnimationStateMachine");
}
