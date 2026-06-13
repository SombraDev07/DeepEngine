#include "StdAfx.h"
#include "GraphViewModel.h"
#include "StateNodeItem.h"
#include "TransitionConnection.h"
#include "PinItem.h"

CAnimStateMachineGraphViewModel::CAnimStateMachineGraphViewModel(CGraphViewWidget& view, std::shared_ptr<CryGraphEditor::INodeGraphRuntimeContext> pContext)
	: m_view(view)
	, m_pContext(pContext)
{
}

void CAnimStateMachineGraphViewModel::SetDefinition(const SAnimationStateMachineDef& def)
{
	m_def = def;
}

CryGraphEditor::INodeGraphRuntimeContext& CAnimStateMachineGraphViewModel::GetRuntimeContext()
{
	return *m_pContext;
}

QString CAnimStateMachineGraphViewModel::GetGraphName()
{
	return m_def.name.c_str();
}

uint32 CAnimStateMachineGraphViewModel::GetNodeItemCount() const
{
	return m_nodes.size();
}

CryGraphEditor::CAbstractNodeItem* CAnimStateMachineGraphViewModel::GetNodeItemByIndex(uint32 index) const
{
	if (index < m_nodes.size())
		return m_nodes[index].get();
	return nullptr;
}

CryGraphEditor::CAbstractNodeItem* CAnimStateMachineGraphViewModel::GetNodeItemById(QVariant id) const
{
	for (const auto& node : m_nodes)
	{
		if (node->HasId(id))
			return node.get();
	}
	return nullptr;
}

CryGraphEditor::CAbstractNodeItem* CAnimStateMachineGraphViewModel::CreateNode(QVariant typeId, const QPointF& position)
{
	auto pNode = std::make_unique<CStateNodeItem>(*this, position);
	auto pRaw = pNode.get();
	m_nodes.push_back(std::move(pNode));
	return pRaw;
}

bool CAnimStateMachineGraphViewModel::RemoveNode(CryGraphEditor::CAbstractNodeItem& node)
{
	for (auto it = m_nodes.begin(); it != m_nodes.end(); ++it)
	{
		if (it->get() == &node)
		{
			m_nodes.erase(it);
			return true;
		}
	}
	return false;
}

uint32 CAnimStateMachineGraphViewModel::GetConnectionItemCount() const
{
	return m_connections.size();
}

CryGraphEditor::CAbstractConnectionItem* CAnimStateMachineGraphViewModel::GetConnectionItemByIndex(uint32 index) const
{
	if (index < m_connections.size())
		return m_connections[index].get();
	return nullptr;
}

CryGraphEditor::CAbstractConnectionItem* CAnimStateMachineGraphViewModel::GetConnectionItemById(QVariant id) const
{
	for (const auto& conn : m_connections)
	{
		if (conn->HasId(id))
			return conn.get();
	}
	return nullptr;
}

CryGraphEditor::CAbstractConnectionItem* CAnimStateMachineGraphViewModel::CreateConnection(CryGraphEditor::CAbstractPinItem& sourcePin, CryGraphEditor::CAbstractPinItem& targetPin)
{
	auto pConn = std::make_unique<CTransitionConnection>(sourcePin, targetPin, *this);
	auto pRaw = pConn.get();
	m_connections.push_back(std::move(pConn));
	return pRaw;
}

bool CAnimStateMachineGraphViewModel::RemoveConnection(CryGraphEditor::CAbstractConnectionItem& connection)
{
	for (auto it = m_connections.begin(); it != m_connections.end(); ++it)
	{
		if (it->get() == &connection)
		{
			m_connections.erase(it);
			return true;
		}
	}
	return false;
}
