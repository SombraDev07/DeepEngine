#pragma once
#include <NodeGraph/AbstractConnectionItem.h>

#include "GraphViewModel.h"

class CTransitionConnection : public CryGraphEditor::CAbstractConnectionItem
{
public:
	CTransitionConnection(CryGraphEditor::CAbstractPinItem& sourcePin, CryGraphEditor::CAbstractPinItem& targetPin, CAnimStateMachineGraphViewModel& model);

	virtual CryGraphEditor::CConnectionWidget* CreateWidget(CryGraphEditor::CNodeGraphView& view) override;
	virtual const char* GetStyleId() const override { return "TransitionConnection"; }
	virtual CryGraphEditor::CAbstractPinItem& GetSourcePinItem() const override { return *m_pSourcePin; }
	virtual CryGraphEditor::CAbstractPinItem& GetTargetPinItem() const override { return *m_pTargetPin; }
	virtual QVariant GetId() const override { return m_id; }
	virtual bool HasId(QVariant id) const override { return id == m_id; }

	float GetDuration() const { return m_duration; }
	void SetDuration(float duration) { m_duration = duration; }

private:
	CryGraphEditor::CAbstractPinItem* m_pSourcePin;
	CryGraphEditor::CAbstractPinItem* m_pTargetPin;
	QVariant m_id;
	float m_duration = 0.2f;
};
