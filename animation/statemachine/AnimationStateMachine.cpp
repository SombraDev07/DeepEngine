#include "StdAfx.h"
#include "AnimationStateMachine.h"
#include <CryAnimation/IAnimationStateMachine.h>
#include <CryAnimation/ICryAnimation.h>
#include <CrySystem/ISystem.h>
#include <CrySystem/ConsoleRegistration.h>

CAnimationStateMachine::CAnimationStateMachine()
{
}

CAnimationStateMachine::~CAnimationStateMachine()
{
}

void CAnimationStateMachine::Update(float deltaTime)
{
	for (auto& layer : m_layers)
	{
		UpdateLayer(layer, deltaTime);
	}
}

void CAnimationStateMachine::SetFloatParam(const char* paramName, float value)
{
	for (auto& param : m_parameters)
	{
		if (param.name == paramName && param.type == EAnimParamType::Float)
		{
			param.currentValue = value;
			return;
		}
	}
}

void CAnimationStateMachine::SetBoolParam(const char* paramName, bool value)
{
	for (auto& param : m_parameters)
	{
		if (param.name == paramName && param.type == EAnimParamType::Bool)
		{
			param.bCurrentValue = value;
			return;
		}
	}
}

void CAnimationStateMachine::FireTrigger(const char* paramName)
{
	for (auto& param : m_parameters)
	{
		if (param.name == paramName && param.type == EAnimParamType::Trigger)
		{
			param.bTriggered = true;
			return;
		}
	}
}

float CAnimationStateMachine::GetFloatParam(const char* paramName) const
{
	for (const auto& param : m_parameters)
	{
		if (param.name == paramName && param.type == EAnimParamType::Float)
			return param.currentValue;
	}
	return 0.0f;
}

bool CAnimationStateMachine::GetBoolParam(const char* paramName) const
{
	for (const auto& param : m_parameters)
	{
		if (param.name == paramName && param.type == EAnimParamType::Bool)
			return param.bCurrentValue;
	}
	return false;
}

const char* CAnimationStateMachine::GetCurrentStateName(int layerIndex) const
{
	if (layerIndex < 0 || layerIndex >= m_layers.size())
		return "";
	return m_layers[layerIndex].currentStateName.c_str();
}

float CAnimationStateMachine::GetStateProgress(int layerIndex) const
{
	if (layerIndex < 0 || layerIndex >= m_layers.size())
		return 0.0f;

	const auto& layer = m_layers[layerIndex];
	return layer.elapsedTime / max(layer.currentDuration, 0.001f);
}

bool CAnimationStateMachine::LoadDefinition(const SAnimationStateMachineDef& def)
{
	m_name = def.name;
	m_guid = def.guid;

	m_parameters.clear();
	for (const auto& paramDef : def.parameters)
	{
		FAnimParam param;
		param.name = paramDef.name;
		param.type = paramDef.type;
		param.currentValue = paramDef.defaultValue;
		param.bCurrentValue = paramDef.bDefaultValue;
		param.bTriggered = false;
		m_parameters.push_back(param);
	}

	m_layers.clear();
	m_layers.resize(def.layers.size());
	for (size_t i = 0; i < def.layers.size(); ++i)
	{
		const auto& layerDef = def.layers[i];
		auto& layer = m_layers[i];

		layer.name = layerDef.name;
		layer.layerIndex = layerDef.layerIndex;
		layer.defaultState = layerDef.defaultState;
		layer.currentStateName = layerDef.defaultState;
		layer.nextStateName = "";
		layer.blendTime = 0.0f;
		layer.elapsedTime = 0.0f;
		layer.currentDuration = 1.0f;

		for (const auto& stateDef : layerDef.states)
		{
			FAnimState state;
			state.name = stateDef.name;
			state.animationPath = stateDef.animationPath;
			state.blendSpacePath = stateDef.blendSpacePath;
			state.speed = stateDef.speed;
			state.bLoop = stateDef.bLoop;
			state.bOneShot = stateDef.bOneShot;

			for (const auto& transDef : stateDef.transitions)
			{
				FAnimTransition trans;
				trans.name = transDef.name;
				trans.fromState = transDef.fromState;
				trans.toState = transDef.toState;
				trans.duration = transDef.duration;
				trans.exitTimeNormalized = transDef.exitTimeNormalized;
				trans.conditions = transDef.conditions;
				state.transitions.push_back(trans);
			}

			layer.states.push_back(state);
		}
	}

	return true;
}

void CAnimationStateMachine::SetCharacterInstance(ICharacterInstance* pCharacter)
{
	m_pCharacter = pCharacter;
}

void CAnimationStateMachine::UpdateLayer(SLayerInstance& layer, float deltaTime)
{
	layer.elapsedTime += deltaTime;

	if (!layer.nextStateName.empty())
	{
		layer.blendTime += deltaTime;
		float t = clamp_tpl(layer.blendTime / max(layer.blendDuration, 0.001f), 0.0f, 1.0f);

		if (m_pCharacter)
		{
			ISkeletonAnim* pSkeletonAnim = m_pCharacter->GetISkeletonAnim();
			if (pSkeletonAnim)
			{
				pSkeletonAnim->SetLayerBlendWeight(layer.layerIndex, 1.0f - t * 0.5f);
			}
		}

		if (t >= 1.0f)
		{
			layer.currentStateName = layer.nextStateName;
			layer.nextStateName = "";
			layer.blendTime = 0.0f;
			layer.elapsedTime = 0.0f;

			PlayState(layer);
		}
		return;
	}

	const FAnimState* pCurrentState = FindState(layer, layer.currentStateName);
	if (!pCurrentState)
	{
		if (!layer.defaultState.empty())
		{
			layer.currentStateName = layer.defaultState;
			PlayState(layer);
		}
		return;
	}

	if (pCurrentState->bOneShot && !pCurrentState->bLoop)
	{
		float duration = max(layer.currentDuration, 0.001f);
		if (layer.elapsedTime >= duration)
		{
			EvaluateTransitions(layer, *pCurrentState);
			return;
		}
	}

	EvaluateTransitions(layer, *pCurrentState);
}

void CAnimationStateMachine::EvaluateTransitions(SLayerInstance& layer, const FAnimState& state)
{
	for (const auto& transition : state.transitions)
	{
		if (EvaluateConditions(transition.conditions))
		{
			layer.nextStateName = transition.toState;
			layer.blendDuration = transition.duration;
			layer.blendTime = 0.0f;
			return;
		}
	}
}

bool CAnimationStateMachine::EvaluateConditions(const DynArray<SAnimCondition>& conditions) const
{
	if (conditions.empty())
		return false;

	for (const auto& condition : conditions)
	{
		bool result = false;

		for (const auto& param : m_parameters)
		{
			if (param.name != condition.parameterName)
				continue;

			switch (condition.comparison)
			{
			case EAnimComparison::Equal:
				if (param.type == EAnimParamType::Float || param.type == EAnimParamType::Int)
					result = fabs(param.currentValue - condition.value) < 0.001f;
				else if (param.type == EAnimParamType::Bool)
					result = param.bCurrentValue == (condition.value != 0.0f);
				break;
			case EAnimComparison::NotEqual:
				if (param.type == EAnimParamType::Float || param.type == EAnimParamType::Int)
					result = fabs(param.currentValue - condition.value) >= 0.001f;
				else if (param.type == EAnimParamType::Bool)
					result = param.bCurrentValue != (condition.value != 0.0f);
				break;
			case EAnimComparison::Greater:
				result = param.currentValue > condition.value;
				break;
			case EAnimComparison::Less:
				result = param.currentValue < condition.value;
				break;
			case EAnimComparison::GreaterOrEqual:
				result = param.currentValue >= condition.value;
				break;
			case EAnimComparison::LessOrEqual:
				result = param.currentValue <= condition.value;
				break;
			case EAnimComparison::Triggered:
				result = param.bTriggered;
				break;
			}
			break;
		}

		if (!result)
			return false;
	}

	return true;
}

void CAnimationStateMachine::PlayState(SLayerInstance& layer)
{
	const FAnimState* pState = FindState(layer, layer.currentStateName);
	if (!pState || !m_pCharacter)
		return;

	ISkeletonAnim* pSkeletonAnim = m_pCharacter->GetISkeletonAnim();
	if (!pSkeletonAnim)
		return;

	IAnimationSet* pAnimSet = m_pCharacter->GetIAnimationSet();

	if (!pState->animationPath.empty() && pAnimSet)
	{
		CryCharAnimationParams animParams;
		animParams.m_nLayerID = layer.layerIndex;
		animParams.m_fPlaybackSpeed = pState->speed;
		animParams.m_nFlags = pState->bLoop ? CA_LOOP_ANIMATION : 0;

		int animId = pAnimSet->GetAnimIDByName(pState->animationPath);
		if (animId >= 0)
		{
			pSkeletonAnim->StartAnimationById(animId, animParams);
		}
	}

	if (!pState->blendSpacePath.empty() && pAnimSet)
	{
		CryCharAnimationParams animParams;
		animParams.m_nLayerID = layer.layerIndex;
		animParams.m_fPlaybackSpeed = pState->speed;
		animParams.m_nFlags = CA_LOOP_ANIMATION;

		int animId = pAnimSet->GetAnimIDByName(pState->blendSpacePath);
		if (animId >= 0)
		{
			pSkeletonAnim->StartAnimationById(animId, animParams);
		}
	}

	layer.elapsedTime = 0.0f;
}

const CAnimationStateMachine::FAnimState* CAnimationStateMachine::FindState(const SLayerInstance& layer, const string& stateName) const
{
	for (const auto& state : layer.states)
	{
		if (state.name == stateName)
			return &state;
	}
	return nullptr;
}

void CAnimationStateMachine::ClearTriggeredParams()
{
	for (auto& param : m_parameters)
	{
		if (param.type == EAnimParamType::Trigger)
			param.bTriggered = false;
	}
}
