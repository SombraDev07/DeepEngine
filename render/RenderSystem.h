#pragma once
#include <d3d11.h>
#include <Platform.h>
#include <Vec3.h>
#include <Camera.h>
#include <vector>
#include <Mesh.h>

struct Vertex { Vec3 pos; Vec3 color; };
struct LtVertex { Vec3 pos; Vec3 normal; Vec3 color; };

class RenderSystem
{
public:
	bool Init(ID3D11Device* device, u32 width, u32 height);
	void Shutdown();
	void Resize(u32 width, u32 height);

	void BeginFrame();
	void EndFrame();
	void RenderViewport(const Camera& cam, ID3D11ShaderResourceView** outSRV);

	void DrawGrid(const Camera& cam);
	void DrawWireCube(const Vec3& pos, const Vec3& scale, const Vec3& color);
	void DrawMesh(const Mesh& mesh, const Vec3& pos, const Vec3& scale, const Vec3& color);
	void FlushWire(const Camera& cam);
	void FlushLit(const Camera& cam);

	ID3D11Texture2D* GetViewportTexture() { return m_viewportTex; }

private:
	ID3D11Device* m_device = nullptr;
	ID3D11DeviceContext* m_ctx = nullptr;

	// Wire shaders
	ID3D11VertexShader* m_wireVS = nullptr;
	ID3D11PixelShader*  m_wirePS = nullptr;
	ID3D11InputLayout*  m_wireLayout = nullptr;
	ID3D11Buffer*       m_cbCamera = nullptr;

	// Lit shaders
	ID3D11VertexShader* m_litVS = nullptr;
	ID3D11PixelShader*  m_litPS = nullptr;
	ID3D11InputLayout*  m_litLayout = nullptr;
	ID3D11Buffer*       m_cbLighting = nullptr;

	ID3D11Texture2D* m_viewportTex = nullptr;
	ID3D11RenderTargetView* m_viewportRTV = nullptr;
	ID3D11ShaderResourceView* m_viewportSRV = nullptr;
	ID3D11DepthStencilView* m_viewportDSV = nullptr;
	ID3D11Texture2D* m_depthTex = nullptr;

	u32 m_width = 0, m_height = 0;

	std::vector<Vertex> m_wireVerts;
	std::vector<LtVertex> m_litVerts;
	std::vector<u32> m_litIndices;

	ID3D11Buffer* CreateVB(const void* data, u32 stride, u32 count);
	ID3D11Buffer* CreateIB(const u32* data, u32 count);
	bool CompileShaders();
};
