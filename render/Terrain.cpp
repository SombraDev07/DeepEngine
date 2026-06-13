#include "Terrain.h"
#include <cstring>

f32 Terrain::Noise2D(u32 x, u32 y, u32 seed) const
{
	u32 n = x + y * 57 + seed * 131;
	n = (n << 13) ^ n;
	return 1.0f - (f32)((n * (n * n * 15731u + 789221u) + 1376312589u) & 0x7FFFFFFFu) / 1073741824.0f;
}

f32 Terrain::FractalNoise(f32 x, f32 y, u32 octaves) const
{
	f32 val = 0, amp = 1, freq = 1;
	for (u32 i = 0; i < octaves; ++i) {
		val += Noise2D((u32)(x * freq), (u32)(y * freq), i * 7) * amp;
		amp *= 0.5f; freq *= 2.0f;
	}
	return val * 0.6f;
}

void Terrain::Generate()
{
	heightmap.resize(width * depth);
	vertices.resize(width * depth);
	indices.clear();

	f32 step = size / (f32)(width - 1);

	// Generate heightmap
	for (u32 z = 0; z < depth; ++z) {
		for (u32 x = 0; x < width; ++x) {
			f32 h = FractalNoise((f32)x / (f32)width * 4.0f, (f32)z / (f32)depth * 4.0f, 5) * maxHeight;
			heightmap[z * width + x] = h;
		}
	}

	// Generate vertices
	for (u32 z = 0; z < depth; ++z) {
		for (u32 x = 0; x < width; ++x) {
			TerrainVertex v;
			v.pos = Vec3((f32)x * step - size * 0.5f, heightmap[z * width + x], (f32)z * step - size * 0.5f);
			v.normal = SampleNormal(v.pos.x, v.pos.z);
			// Color based on height (green low, brown mid, white high)
			f32 t = v.pos.y / maxHeight;
			if (t < 0.3f)      v.color = Vec3(0.2f, 0.5f, 0.1f); // Green
			else if (t < 0.6f) v.color = Vec3(0.5f, 0.4f, 0.2f); // Brown
			else               v.color = Vec3(0.7f, 0.7f, 0.6f); // Grey/white
			vertices[z * width + x] = v;
		}
	}

	// Generate indices (triangles)
	for (u32 z = 0; z < depth - 1; ++z) {
		for (u32 x = 0; x < width - 1; ++x) {
			u32 tl = z * width + x;
			u32 tr = z * width + x + 1;
			u32 bl = (z + 1) * width + x;
			u32 br = (z + 1) * width + x + 1;
			indices.push_back(tl); indices.push_back(bl); indices.push_back(tr);
			indices.push_back(tr); indices.push_back(bl); indices.push_back(br);
		}
	}
}

f32 Terrain::SampleHeight(f32 wx, f32 wz) const
{
	f32 fx = (wx + size * 0.5f) / size * (f32)(width - 1);
	f32 fz = (wz + size * 0.5f) / size * (f32)(depth - 1);
	u32 x = (u32)std::min((f32)(width - 2), std::max(0.0f, fx));
	u32 z = (u32)std::min((f32)(depth - 2), std::max(0.0f, fz));
	f32 tx = fx - (f32)x, tz = fz - (f32)z;
	f32 h00 = heightmap[z * width + x];
	f32 h10 = heightmap[z * width + x + 1];
	f32 h01 = heightmap[(z + 1) * width + x];
	f32 h11 = heightmap[(z + 1) * width + x + 1];
	return h00 * (1 - tx) * (1 - tz) + h10 * tx * (1 - tz) + h01 * (1 - tx) * tz + h11 * tx * tz;
}

Vec3 Terrain::SampleNormal(f32 wx, f32 wz) const
{
	f32 d = 0.5f;
	f32 l = SampleHeight(wx - d, wz);
	f32 r = SampleHeight(wx + d, wz);
	f32 b = SampleHeight(wx, wz - d);
	f32 f = SampleHeight(wx, wz + d);
	Vec3 n(-(r - l), 2.0f * d, -(f - b));
	return n.Normalized();
}
