#pragma once
#include <QWidget>

class CAnimStateMachineEditor;
class QTreeWidget;

class CParametersWidget : public QWidget
{
	Q_OBJECT
public:
	CParametersWidget(CAnimStateMachineEditor& editor);

private:
	CAnimStateMachineEditor& m_editor;
	QTreeWidget* m_pTree;
};
