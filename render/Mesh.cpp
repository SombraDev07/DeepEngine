#include "Mesh.h"
#include <Logger.h>
#include <cstdio>
#include <cstring>
#include <cmath>

bool Mesh::LoadOBJ(const char* path)
{
	FILE* f = fopen(path, "r");
	if (!f) { LOG_ERROR("Cannot open: %s", path); return false; }

	std::vector<Vec3> positions;
	std::vector<Vec3> normals;
	vertices.clear();
	indices.clear();

	char line[512];
	while (fgets(line, sizeof(line), f))
	{
		if (strncmp(line, "v ", 2) == 0)
		{
			Vec3 v;
			sscanf(line, "v %f %f %f", &v.x, &v.y, &v.z);
			positions.push_back(v);
		}
		else if (strncmp(line, "vn ", 3) == 0)
		{
			Vec3 n;
			sscanf(line, "vn %f %f %f", &n.x, &n.y, &n.z);
			normals.push_back(n);
		}
		else if (strncmp(line, "f ", 2) == 0)
		{
			u32 vi[4] = {}, ni[4] = {};
			int nParsed = sscanf(line, "f %u//%u %u//%u %u//%u %u//%u",
				&vi[0], &ni[0], &vi[1], &ni[1], &vi[2], &ni[2], &vi[3], &ni[3]);

			u32 base = (u32)vertices.size();
			for (int i = 0; i < (nParsed >= 12 ? 4 : 3); ++i)
			{
				MeshVertex mv;
				mv.pos = (vi[i] > 0 && vi[i] <= positions.size()) ? positions[vi[i]-1] : Vec3();
				mv.normal = (ni[i] > 0 && ni[i] <= normals.size()) ? normals[ni[i]-1] : Vec3(0, 1, 0);
				mv.color = Vec3(0.7f, 0.7f, 0.8f);
				vertices.push_back(mv);
			}
			indices.insert(indices.end(), {base, base+1, base+2, base, base+2, base+3});
		}
	}
	fclose(f);

	if (vertices.empty()) return false;

	if (indices.empty())
	{
		for (u32 i = 0; i < vertices.size(); ++i) indices.push_back(i);
	}

	LOG_INFO("Loaded %s: %zu verts, %zu tris", path, vertices.size(), indices.size()/3);
	return true;
}

void Mesh::CreateCube(const Vec3& halfExtent)
{
	Vec3 h = halfExtent;
	vertices = {
		{{-h.x,-h.y,-h.z},{0,-1,0},{0.7f,0.7f,0.8f}},
		{{ h.x,-h.y,-h.z},{0,-1,0},{0.7f,0.7f,0.8f}},
		{{ h.x,-h.y, h.z},{0,-1,0},{0.7f,0.7f,0.8f}},
		{{-h.x,-h.y, h.z},{0,-1,0},{0.7f,0.7f,0.8f}},
		{{-h.x, h.y,-h.z},{0, 1,0},{0.7f,0.7f,0.8f}},
		{{ h.x, h.y,-h.z},{0, 1,0},{0.7f,0.7f,0.8f}},
		{{ h.x, h.y, h.z},{0, 1,0},{0.7f,0.7f,0.8f}},
		{{-h.x, h.y, h.z},{0, 1,0},{0.7f,0.7f,0.8f}},
	};
	indices = {0,1,2,0,2,3, 4,6,5,4,7,6, 0,4,5,0,5,1, 1,5,6,1,6,2, 2,6,7,2,7,3, 3,7,4,3,4,0};
}

void Mesh::CreateGrid(float size, float spacing)
{
	Vec3 gray(0.25f, 0.25f, 0.25f);
	for (float i = -size; i <= size; i += spacing)
	{
		vertices.push_back({{i, 0, -size}, {0,1,0}, gray});
		vertices.push_back({{i, 0,  size}, {0,1,0}, gray});
		vertices.push_back({{-size, 0, i}, {0,1,0}, gray});
		vertices.push_back({{ size, 0, i}, {0,1,0}, gray});
	}
	for (u32 i = 0; i < vertices.size(); ++i) indices.push_back(i);
}

MeshCache::MeshCache()
{
	m_cube.CreateCube(Vec3(0.5f, 0.5f, 0.5f));
	m_grid.CreateGrid(20.0f, 2.0f);
}

Mesh* MeshCache::GetOrLoad(const char* path)
{
	for (auto& [p, m] : m_loaded)
		if (p == path) return m;
	auto* mesh = new Mesh();
	if (mesh->LoadOBJ(path))
	{
		m_loaded.push_back({path, mesh});
		return mesh;
	}
	delete mesh;
	return &m_cube;
}
