#pragma once
#include <Platform.h>
#include <Vec3.h>
#include <vector>
#include <string>
#include <d3d11.h>

struct MeshVertex { Vec3 pos; Vec3 normal; Vec3 color; };

class Mesh
{
public:
	std::vector<MeshVertex> vertices;
	std::vector<u32> indices;

	bool LoadOBJ(const char* path);
	bool LoadGLTF(const char* path);
	void CreateCube(const Vec3& halfExtent);
	void CreateGrid(float size, float spacing);

	void UploadToGPU(ID3D11Device* device);
	void ReleaseGPU();
	ID3D11Buffer* GetVB() const { return m_vb; }
	ID3D11Buffer* GetIB() const { return m_ib; }

private:
	ID3D11Buffer* m_vb = nullptr;
	ID3D11Buffer* m_ib = nullptr;
};

class MeshCache
{
public:
	static MeshCache& Get() { static MeshCache c; return c; }
	Mesh* GetOrLoad(const char* path);
	Mesh* GetGrid() { return &m_grid; }
	Mesh* GetCube() { return &m_cube; }
private:
	MeshCache();
	Mesh m_cube;
	Mesh m_grid;
	std::vector<std::pair<std::string, Mesh*>> m_loaded;
};
