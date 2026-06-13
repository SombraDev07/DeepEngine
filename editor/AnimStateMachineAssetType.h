#pragma once
#include <AssetSystem/AssetType.h>

class CAnimStateMachineAssetType : public CAssetType
{
public:
	DECLARE_ASSET_TYPE_DESC(CAnimStateMachineAssetType);

	static const char* TypeName() { return "AnimationStateMachine"; }

	virtual const char*   GetTypeName() const override       { return TypeName(); }
	virtual const char*   GetUiTypeName() const override     { return QT_TR_NOOP("Animation State Machine"); }
	virtual const char*   GetFileExtension() const override  { return "asmd"; }
	virtual bool          CanBeCreated() const override      { return true; }
	virtual bool          IsImported() const override        { return false; }
	virtual bool          CanBeEdited() const override       { return true; }
	virtual CAssetEditor* Edit(CAsset* pAsset) const override;
	virtual CryIcon       GetIconInternal() const override;

protected:
	virtual bool OnCreate(INewAsset& asset, const SCreateParams* pCreateParams) const override;
};
