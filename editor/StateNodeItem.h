#pragma once
#include <NodeGraph/AbstractNodeItem.h>

#include "GraphViewModel.h"
#include <NodeGraph/AbstractNodeItem.h>
#include <NodeGraph/NodeEditorData.h>
class CPinItem;

class CStateNodeItem : public CryGraphEditor::CAbstractNodeItem
{
public:
	CStateNodeItem(CAnimStateMachineGraphViewModel& model, const QPointF& position);

	virtual CryGraphEditor::CNodeWidget* CreateWidget(CryGraphEditor::CNodeGraphView& view) override;
	virtual const char* GetStyleId() const override { return "StateNode"; }
	virtual QVariant GetId() const override { return m_id; }
	virtual bool HasId(QVariant id) const override { return id == m_id; }
	virtual QVariant GetTypeId() const override { return "StateNode"; }
	virtual const CryGraphEditor::PinItemArray& GetPinItems() const override { return m_pins; }
	virtual QString GetName() const override { return m_name; }
	virtual void SetName(const QString& name) override { m_name = name; SignalNameChanged(); }
	virtual QPointF GetPosition() const override { return m_position; }
	virtual void SetPosition(QPointF position) override { m_position = position; SignalPositionChanged(); }
	virtual CryGraphEditor::CAbstractPinItem* GetPinItemById(QVariant id) const override;

	void SetAnimationPath(const QString& path) { m_animationPath = path; }
	QString GetAnimationPath() const { return m_animationPath; }

private:
	QVariant m_id;
	QString m_name;
	QPointF m_position;
	QString m_animationPath;
	CryGraphEditor::PinItemArray m_pins;
	std::vector<std::unique_ptr<CPinItem>> m_ownedPins;
	CryGraphEditor::CNodeEditorData* m_pData = nullptr;
};
