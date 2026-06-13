#pragma once
#include <AssetSystem/AssetEditor.h>

class CGraphViewWidget;
class CParametersWidget;
class CPreviewWidget;

class CAnimStateMachineEditor : public CAssetEditor
{
	Q_OBJECT
public:
	CAnimStateMachineEditor();
	const char* GetEditorName() const override { return "Animation State Machine Editor"; }

protected:
	virtual bool OnOpenAsset(CAsset* pAsset) override;
	virtual bool OnSaveAsset(CEditableAsset& editAsset) override;
	virtual void OnCloseAsset() override;
	virtual void OnInitialize() override;
	virtual void OnCreateDefaultLayout(CDockableContainer* pSender, QWidget* pAssetBrowser) override;

private:
	void RegisterActions();
	void InitMenu();
};
