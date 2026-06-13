#pragma once
#include <d3d11.h>
#include <Platform.h>
#include <Vec3.h>
#include <Camera.h>
#include <vector>
#include <Mesh.h>
#include <DirectXMath.h>
using namespace DirectX;

struct Vertex   { Vec3 pos; Vec3 color; };
struct LtVertex { Vec3 pos; Vec3 normal; Vec3 color; };

// Constant buffer layouts (must match HLSL, 16-byte aligned)
struct CameraCB { XMMATRIX viewProj; };
struct LightCB  {
    XMMATRIX  world;
    XMFLOAT3  lightDir;   float pad1;
    XMFLOAT3  objectColor; float pad2;
    XMFLOAT3  camPos;     float pad3;
};
struct ShadowCB { XMMATRIX lightVP; };

class RenderSystem
{
public:
    bool Init(ID3D11Device* device, u32 width, u32 height);
    void Shutdown();
    void Resize(u32 width, u32 height);
    void BeginFrame();
    void EndFrame();
    void RenderViewport(const Camera& cam, ID3D11ShaderResourceView** outSRV);
    void RenderSky(const Camera& cam);
    void RenderShadowMap(const Camera& cam);
    void DrawGrid(const Camera& cam);
    void DrawWireCube(const Vec3& pos, const Vec3& scale, const Vec3& color);
    void DrawMesh(const Mesh& mesh, const Vec3& pos, const Vec3& scale, const Vec3& color);
    void DrawMeshCached(const Mesh& mesh, const Camera& cam, const Vec3& pos, const Vec3& scale, const Vec3& color);
    void FlushWire(const Camera& cam);
    void FlushLit(const Camera& cam);
    ID3D11Texture2D* GetViewportTexture() { return m_viewportTex; }

private:
    ID3D11Device*        m_device  = nullptr;
    ID3D11DeviceContext* m_ctx     = nullptr;

    // Shaders
    ID3D11VertexShader *m_wireVS=nullptr, *m_litVS=nullptr, *m_skyVS=nullptr;
    ID3D11PixelShader  *m_wirePS=nullptr, *m_litPS=nullptr, *m_skyPS=nullptr;

    // Constant buffers
    ID3D11Buffer *m_cbSky=nullptr, *m_cbCamera=nullptr, *m_cbLighting=nullptr, *m_cbShadow=nullptr;

    // Input layouts
    ID3D11InputLayout *m_wireLayout=nullptr, *m_litLayout=nullptr;

    // Textures & views
    ID3D11Texture2D         *m_viewportTex=nullptr, *m_depthTex=nullptr, *m_shadowTex=nullptr;
    ID3D11RenderTargetView  *m_viewportRTV=nullptr;
    ID3D11ShaderResourceView*m_viewportSRV=nullptr, *m_shadowSRV=nullptr;
    ID3D11DepthStencilView  *m_viewportDSV=nullptr, *m_shadowDSV=nullptr;

    // Sampler for PCF shadow comparison
    ID3D11SamplerState* m_shadowSampler = nullptr;

    // Light view-proj saved from RenderShadowMap for use in FlushLit
    XMMATRIX m_lightViewProj = {};

    u32 m_width = 0, m_height = 0;

    std::vector<Vertex>   m_wireVerts;
    std::vector<LtVertex> m_litVerts;
    std::vector<u32>      m_litIndices;

    ID3D11Buffer* CreateVB(const void* data, u32 stride, u32 count);
    ID3D11Buffer* CreateIB(const u32* data, u32 count);
    bool CompileShaders();
};
