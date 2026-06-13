#pragma once
#include <Vec3.h>
#include <vector>
#include <cmath>

struct TerrainVertex { Vec3 pos; Vec3 normal; Vec3 color; };

class Terrain
{
public:
	u32 width = 128, depth = 128;
	f32 size = 100.0f;
	f32 maxHeight = 10.0f;
	std::vector<f32> heightmap;
	std::vector<TerrainVertex> vertices;
	std::vector<u32> indices;

	void Generate();
	f32 SampleHeight(f32 wx, f32 wz) const;
	Vec3 SampleNormal(f32 wx, f32 wz) const;
	f32 GetHeightAt(u32 x, u32 z) const { return heightmap[z * width + x]; }
private:
	f32 Noise2D(u32 x, u32 y, u32 seed) const;
	f32 FractalNoise(f32 x, f32 y, u32 octaves = 4) const;
};
