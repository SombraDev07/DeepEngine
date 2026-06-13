#pragma once
#include <QWidget>

class CAnimStateMachineEditor;

class CPreviewWidget : public QWidget
{
	Q_OBJECT
public:
	CPreviewWidget(CAnimStateMachineEditor& editor);

private:
	CAnimStateMachineEditor& m_editor;
};
