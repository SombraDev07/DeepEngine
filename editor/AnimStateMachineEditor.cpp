#include "StdAfx.h"
#include "AnimStateMachineEditor.h"
#include "GraphViewWidget.h"
#include "ParametersWidget.h"
#include "PreviewWidget.h"
#include <QWidget>

REGISTER_VIEWPANE_FACTORY_AND_MENU(CAnimStateMachineEditor, "Animation State Machine Editor", "Tools", false, "")

CAnimStateMachineEditor::CAnimStateMachineEditor()
	: CAssetEditor(QStringList { "AnimationStateMachine" })
{
	setAttribute(Qt::WA_DeleteOnClose);
	RegisterActions();
	InitMenu();

}

void CAnimStateMachineEditor::RegisterActions()
{
}

void CAnimStateMachineEditor::InitMenu()
{
	const CEditor::MenuItems items[] = {
		CEditor::MenuItems::FileMenu, CEditor::MenuItems::Save,
	};
	AddToMenu(items, sizeof(items) / sizeof(CEditor::MenuItems));
}

bool CAnimStateMachineEditor::OnOpenAsset(CAsset* pAsset)
{
	return true;
}

bool CAnimStateMachineEditor::OnSaveAsset(CEditableAsset& editAsset)
{
	return true;
}

void CAnimStateMachineEditor::OnCloseAsset()
{

}

void CAnimStateMachineEditor::OnInitialize()
{
	auto pGraphView = [this]()
	{
		return new CGraphViewWidget(*this);
	};
	RegisterDockableWidget("Graph View", pGraphView, true, false);

	auto pParams = [this]()
	{
		return new CParametersWidget(*this);
	};
	RegisterDockableWidget("Parameters", pParams, true, false);

	auto pPreview = [this]()
	{
		return new CPreviewWidget(*this);
	};
	RegisterDockableWidget("Preview", pPreview, true, false);
}

void CAnimStateMachineEditor::OnCreateDefaultLayout(CDockableContainer* pSender, QWidget* pAssetBrowser)
{
	auto pGraph = pSender->SpawnWidget("Graph View");
	pSender->SpawnWidget("Parameters", pGraph, QToolWindowAreaReference::VSplitRight);
	pSender->SpawnWidget("Preview", pGraph, QToolWindowAreaReference::HSplitBottom);
}
