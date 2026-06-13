#pragma once

#include <CryAnimation/IAnimationStateMachine.h>
#include <CryCore/smartptr.h>
#include <CryString/CryString.h>
#include <vector>

struct ICharacterInstance;

class CAnimationStateMachine : public IAnimationStateMachine
{
public:
	CAnimationStateMachine();
	virtual ~CAnimationStateMachine();

	void Update(float deltaTime) override;

	void SetFloatParam(const char* paramName, float value) override;
	void SetBoolParam(const char* paramName, bool value) override;
	void FireTrigger(const char* paramName) override;
	float GetFloatParam(const char* paramName) const override;
	bool GetBoolParam(const char* paramName) const override;

	const char* GetCurrentStateName(int layerIndex) const override;
	float GetStateProgress(int layerIndex) const override;

	bool LoadDefinition(const SAnimationStateMachineDef& def) override;
	void SetCharacterInstance(ICharacterInstance* pCharacter) override;

private:
	struct FAnimParam
	{
		string name;
		EAnimParamType type = EAnimParamType::Float;
		float currentValue = 0.0f;
		bool bCurrentValue = false;
		bool bTriggered = false;
	};

	struct FAnimTransition
	{
		string name;
		string fromState;
		string toState;
		float duration = 0.2f;
		float exitTimeNormalized = -1.0f;
		DynArray<SAnimCondition> conditions;
	};

	struct FAnimState
	{
		string name;
		string animationPath;
		string blendSpacePath;
		float speed = 1.0f;
		bool bLoop = true;
		bool bOneShot = false;
		DynArray<FAnimTransition> transitions;
	};

	struct SLayerInstance
	{
		string name;
		int layerIndex = 0;
		string defaultState;
		string currentStateName;
		string nextStateName;
		float blendTime = 0.0f;
		float blendDuration = 0.2f;
		float elapsedTime = 0.0f;
		float currentDuration = 1.0f;
		DynArray<FAnimState> states;
	};

	void UpdateLayer(SLayerInstance& layer, float deltaTime);
	void EvaluateTransitions(SLayerInstance& layer, const FAnimState& state);
	bool EvaluateConditions(const DynArray<SAnimCondition>& conditions) const;
	void PlayState(SLayerInstance& layer);
	const FAnimState* FindState(const SLayerInstance& layer, const string& stateName) const;
	void ClearTriggeredParams();

	string m_name;
	CryGUID m_guid;
	ICharacterInstance* m_pCharacter = nullptr;
	DynArray<FAnimParam> m_parameters;
	DynArray<SLayerInstance> m_layers;
};
