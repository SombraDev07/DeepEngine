#include "Mesh.h"
#include <Logger.h>
#include <cgltf.h>
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
			// Parse face: supports "f v1 v2 v3 v4", "f v1//vn1 v2//vn2...", "f v1/vt1/vn1..."
			int vi[4] = {0}, ni[4] = {0};
			int nParsed = 0;
			// Try "f v//vn" format
			nParsed = sscanf(line, "f %d//%d %d//%d %d//%d %d//%d",
				&vi[0], &ni[0], &vi[1], &ni[1], &vi[2], &ni[2], &vi[3], &ni[3]);
			if (nParsed < 6) {
				// Try "f v/vt/vn" format
				int dummy;
				nParsed = sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d",
					&vi[0], &dummy, &ni[0], &vi[1], &dummy, &ni[1],
					&vi[2], &dummy, &ni[2], &vi[3], &dummy, &ni[3]);
			}
			if (nParsed < 6) {
				// Try "f v v v v" format (positions only)
				nParsed = sscanf(line, "f %d %d %d %d", &vi[0], &vi[1], &vi[2], &vi[3]);
				ni[0] = ni[1] = ni[2] = ni[3] = 0;
			}

			int vertCount = (nParsed >= 12) ? 4 : (nParsed >= 6) ? 3 : 0;
			if (vertCount < 3) continue;

			u32 base = (u32)vertices.size();
			for (int i = 0; i < vertCount; ++i)
			{
				MeshVertex mv;
				mv.pos = (vi[i] > 0 && (size_t)vi[i] <= positions.size()) ? positions[vi[i]-1] : Vec3();
				mv.normal = (ni[i] > 0 && (size_t)ni[i] <= normals.size()) ? normals[ni[i]-1] : Vec3(0, 1, 0);
				mv.color = Vec3(0.7f, 0.7f, 0.8f);
				vertices.push_back(mv);
			}

			// Triangulate: tri fans for quads, single triangle for tris
			indices.push_back(base);
			indices.push_back(base + 1);
			indices.push_back(base + 2);
			if (vertCount == 4) {
				indices.push_back(base);
				indices.push_back(base + 2);
				indices.push_back(base + 3);
			}
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

bool Mesh::LoadGLTF(const char* path)
{
	cgltf_options options = {};
	cgltf_data* data = nullptr;
	cgltf_result result = cgltf_parse_file(&options, path, &data);
	if (result != cgltf_result_success) { LOG_ERROR("cgltf parse: %s", path); return false; }
	cgltf_load_buffers(&options, data, path);

	vertices.clear(); indices.clear();

	// Load all meshes into one combined mesh
	for (size_t mi = 0; mi < data->meshes_count; ++mi)
	{
		const auto& mesh = data->meshes[mi];
		for (size_t pi = 0; pi < mesh.primitives_count; ++pi)
		{
			const auto& prim = mesh.primitives[pi];
			const auto* posAcc = prim.attributes[0].data;
			float* posData = nullptr;
			size_t posCount = 0, posStride = 0;

			// Get position attribute
			for (size_t ai = 0; ai < prim.attributes_count; ++ai) {
				if (prim.attributes[ai].type == cgltf_attribute_type_position) {
					cgltf_accessor_unpack_floats(prim.attributes[ai].data, nullptr, 0);
					posCount = prim.attributes[ai].data->count;
					posData = (float*)malloc(posCount * 3 * sizeof(float));
					cgltf_accessor_unpack_floats(prim.attributes[ai].data, posData, posCount * 3);
					break;
				}
			}
			if (!posData) continue;

			// Get normal attribute
			float* nrmData = nullptr;
			for (size_t ai = 0; ai < prim.attributes_count; ++ai) {
				if (prim.attributes[ai].type == cgltf_attribute_type_normal) {
					nrmData = (float*)malloc(prim.attributes[ai].data->count * 3 * sizeof(float));
					cgltf_accessor_unpack_floats(prim.attributes[ai].data, nrmData, prim.attributes[ai].data->count * 3);
					break;
				}
			}

			// Add vertices
			u32 base = (u32)vertices.size();
			for (size_t vi = 0; vi < posCount; ++vi) {
				MeshVertex mv;
				mv.pos = Vec3(posData[vi*3], posData[vi*3+1], posData[vi*3+2]);
				if (nrmData) mv.normal = Vec3(nrmData[vi*3], nrmData[vi*3+1], nrmData[vi*3+2]);
				else mv.normal = Vec3(0, 1, 0);
				mv.color = Vec3(0.7f, 0.7f, 0.8f);
				vertices.push_back(mv);
			}

			// Add indices
			if (prim.indices) {
				u32* idxData = (u32*)malloc(prim.indices->count * sizeof(u32));
				cgltf_accessor_unpack_indices(prim.indices, idxData, sizeof(u32), prim.indices->count);
				for (size_t ii = 0; ii < prim.indices->count; ++ii)
					indices.push_back(base + idxData[ii]);
				free(idxData);
			} else {
				for (u32 ii = 0; ii < (u32)posCount; ++ii) indices.push_back(base + ii);
			}

			free(posData);
			if (nrmData) free(nrmData);
		}
	}

	cgltf_free(data);
	LOG_INFO("Loaded GLTF %s: %zu verts, %zu tris", path, vertices.size(), indices.size() / 3);
	return vertices.empty() ? false : true;
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
	if (mesh->LoadOBJ(path) || mesh->LoadGLTF(path))
	{
		m_loaded.push_back({path, mesh});
		return mesh;
	}
	delete mesh;
	return &m_cube;
}

void Mesh::UploadToGPU(ID3D11Device* device)
{
	ReleaseGPU();
	if (vertices.empty()) return;

	D3D11_BUFFER_DESC vd = {}; vd.Usage = D3D11_USAGE_DEFAULT; vd.ByteWidth = sizeof(MeshVertex) * (u32)vertices.size(); vd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	D3D11_SUBRESOURCE_DATA vsd = { vertices.data() };
	device->CreateBuffer(&vd, &vsd, &m_vb);

	D3D11_BUFFER_DESC id = {}; id.Usage = D3D11_USAGE_DEFAULT; id.ByteWidth = sizeof(u32) * (u32)indices.size(); id.BindFlags = D3D11_BIND_INDEX_BUFFER;
	D3D11_SUBRESOURCE_DATA isd = { indices.data() };
	device->CreateBuffer(&id, &isd, &m_ib);
}

void Mesh::ReleaseGPU()
{
	if (m_vb) { m_vb->Release(); m_vb = nullptr; }
	if (m_ib) { m_ib->Release(); m_ib = nullptr; }
}
