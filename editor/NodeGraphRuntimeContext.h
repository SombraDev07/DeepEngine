#pragma once
#include <NodeGraph/ICryGraphEditor.h>
#include <NodeGraph/NodeGraphViewStyle.h>
#include <NodeGraph/NodeGraphView.h>
#include <Controls/DictionaryWidget.h>

class CAnimStateMachineNodeDictionaryEntry : public CAbstractDictionaryEntry
{
public:
	CAnimStateMachineNodeDictionaryEntry(const QString& name, const QVariant& typeId)
		: m_name(name), m_typeId(typeId) {}

	virtual uint32    GetType() const override { return Type_Entry; }
	virtual QVariant  GetIdentifier() const override { return m_typeId; }
	virtual QVariant  GetColumnValue(int32 columnIndex) const override { return m_name; }

private:
	QString m_name;
	QVariant m_typeId;
};

class CAnimStateMachineNodeDictionary : public CAbstractDictionary
{
	Q_OBJECT
public:
	CAnimStateMachineNodeDictionary() {}

	virtual const char* GetName() const override { return "Animation State Machine"; }
	virtual int32 GetNumEntries() const override { return 1; }
	virtual const CAbstractDictionaryEntry* GetEntry(int32 index) const override
	{
		if (index == 0)
			return &m_entry;
		return nullptr;
	}
	virtual int32 GetDefaultFilterColumn() const override { return 0; }

private:
	CAnimStateMachineNodeDictionaryEntry m_entry{"State Node", "StateNode"};
};

class CNodeGraphRuntimeContext : public CryGraphEditor::INodeGraphRuntimeContext
{
public:
	CNodeGraphRuntimeContext();
	virtual const char* GetTypeName() const override { return "AnimationStateMachine"; }
	virtual CryGraphEditor::CNodeGraphViewStyle* GetStyle() const override { return m_pStyle.get(); }
	virtual CAbstractDictionary* GetAvailableNodesDictionary() override { return &m_dict; }

private:
	std::unique_ptr<CryGraphEditor::CNodeGraphViewStyle> m_pStyle;
	CAnimStateMachineNodeDictionary m_dict;
};
