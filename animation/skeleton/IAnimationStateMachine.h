#pragma once

#include <CryExtension/CryGUID.h>
#include <CrySerialization/Forward.h>
#include <CryMath/Cry_Math.h>

struct ICharacterInstance;


enum class EAnimParamType : uint8
{
	Float,
	Bool,
	Trigger,
	Int,
	COUNT
};

enum class EAnimComparison : uint8
{
	Equal,
	NotEqual,
	Greater,
	Less,
	GreaterOrEqual,
	LessOrEqual,
	Triggered
};

struct SAnimParameter
{
	string name;
	EAnimParamType type = EAnimParamType::Float;
	float defaultValue = 0.0f;
	bool bDefaultValue = false;

	void Serialize(Serialization::IArchive& ar)
	{
		ar(name, "name", "Name");
		ar(type, "type", "Type");
		if (type == EAnimParamType::Float || type == EAnimParamType::Int)
			ar(defaultValue, "default", "Default");
		else if (type == EAnimParamType::Bool)
			ar(bDefaultValue, "default", "Default");
	}
};

struct SAnimCondition
{
	string parameterName;
	EAnimComparison comparison = EAnimComparison::Greater;
	float value = 0.0f;

	void Serialize(Serialization::IArchive& ar)
	{
		ar(parameterName, "param", "Parameter");
		ar(comparison, "comp", "Comparison");
		ar(value, "value", "Value");
	}
};

struct SAnimTransition
{
	string name;
	string fromState;
	string toState;
	float duration = 0.2f;
	float exitTimeNormalized = -1.0f;
	DynArray<SAnimCondition> conditions;

	void Serialize(Serialization::IArchive& ar)
	{
		ar(name, "name", "Name");
		ar(fromState, "from", "From");
		ar(toState, "to", "To");
		ar(duration, "duration", "Duration");
		ar(exitTimeNormalized, "exitTime", "Exit Time");
		ar(conditions, "conditions", "Conditions");
	}
};

struct SAnimState
{
	string name;
	string animationPath;
	string blendSpacePath;
	float speed = 1.0f;
	bool bLoop = true;
	bool bOneShot = false;
	DynArray<SAnimTransition> transitions;

	void Serialize(Serialization::IArchive& ar)
	{
		ar(name, "name", "Name");
		ar(animationPath, "anim", "Animation");
		ar(blendSpacePath, "blendspace", "Blend Space");
		ar(speed, "speed", "Speed");
		ar(bLoop, "loop", "Loop");
		ar(bOneShot, "oneshot", "One Shot");
		ar(transitions, "transitions", "Transitions");
	}
};

struct SAnimLayer
{
	string name;
	int layerIndex = 0;
	string defaultState;
	DynArray<SAnimState> states;

	void Serialize(Serialization::IArchive& ar)
	{
		ar(name, "name", "Name");
		ar(layerIndex, "layer", "Layer Index");
		ar(defaultState, "default", "Default State");
		ar(states, "states", "States");
	}
};

struct SAnimationStateMachineDef
{
	CryGUID guid;
	string name;
	DynArray<SAnimParameter> parameters;
	DynArray<SAnimLayer> layers;

	void Serialize(Serialization::IArchive& ar)
	{
		ar(guid, "guid", "GUID");
		ar(name, "name", "Name");
		ar(parameters, "parameters", "Parameters");
		ar(layers, "layers", "Layers");
	}
};

struct IAnimationStateMachine : public _i_reference_target_t
{
	virtual ~IAnimationStateMachine() {}

	virtual void Update(float deltaTime) = 0;

	virtual void SetFloatParam(const char* paramName, float value) = 0;
	virtual void SetBoolParam(const char* paramName, bool value) = 0;
	virtual void FireTrigger(const char* paramName) = 0;
	virtual float GetFloatParam(const char* paramName) const = 0;
	virtual bool GetBoolParam(const char* paramName) const = 0;

	virtual const char* GetCurrentStateName(int layerIndex) const = 0;
	virtual float GetStateProgress(int layerIndex) const = 0;

	virtual bool LoadDefinition(const SAnimationStateMachineDef& def) = 0;
	virtual void SetCharacterInstance(ICharacterInstance* pCharacter) = 0;
};
