#include "StdAfx.h"
#include "AnimStateMachineAssetType.h"
#include "AnimStateMachineEditor.h"

REGISTER_ASSET_TYPE(CAnimStateMachineAssetType)

CAssetEditor* CAnimStateMachineAssetType::Edit(CAsset* pAsset) const
{
	return CAssetEditor::OpenAssetForEdit("Animation State Machine Editor", pAsset);
}

CryIcon CAnimStateMachineAssetType::GetIconInternal() const
{
	return CryIcon("icons:common/animation.ico");
}

bool CAnimStateMachineAssetType::OnCreate(INewAsset& asset, const SCreateParams* pCreateParams) const
{
	return true;
}
