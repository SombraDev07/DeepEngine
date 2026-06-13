#pragma once
#define NOMINMAX
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>

namespace Layers
{
	static constexpr JPH::ObjectLayer NON_MOVING = 0;
	static constexpr JPH::ObjectLayer MOVING = 1;
	static constexpr JPH::ObjectLayer NUM_LAYERS = 2;

	static constexpr JPH::BroadPhaseLayer BP_NON_MOVING(0);
	static constexpr JPH::BroadPhaseLayer BP_MOVING(1);
	static constexpr JPH::uint NUM_BP_LAYERS(2);
}

class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
{
public:
	BPLayerInterfaceImpl()
	{
		mMapping[Layers::NON_MOVING] = Layers::BP_NON_MOVING;
		mMapping[Layers::MOVING] = Layers::BP_MOVING;
	}
	JPH::uint GetNumBroadPhaseLayers() const override { return Layers::NUM_BP_LAYERS; }
	JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer layer) const override { return mMapping[layer]; }
private:
	JPH::BroadPhaseLayer mMapping[Layers::NUM_LAYERS];
};

class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter
{
public:
	bool ShouldCollide(JPH::ObjectLayer l1, JPH::BroadPhaseLayer l2) const override
	{
		switch (l1)
		{
		case Layers::NON_MOVING: return l2 == Layers::BP_MOVING;
		case Layers::MOVING: return true;
		}
		return false;
	}
};

class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter
{
public:
	bool ShouldCollide(JPH::ObjectLayer l1, JPH::ObjectLayer l2) const override
	{
		if (l1 == Layers::NON_MOVING && l2 == Layers::NON_MOVING) return false;
		return true;
	}
};
