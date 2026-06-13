#include "RenderSystem.h"
#include <d3dcompiler.h>
#include <Logger.h>
#include <DirectXMath.h>
using namespace DirectX;

// ============================================================
//  INIT / SHUTDOWN / RESIZE
// ============================================================

bool RenderSystem::Init(ID3D11Device* device, u32 width, u32 height)
{
	m_device = device;
	m_device->GetImmediateContext(&m_ctx);
	m_width = width; m_height = height;

	LOG_INFO("Render: device OK, compiling shaders...");
	if (!CompileShaders()) {
		LOG_ERROR("Shader compilation failed");
		return false;
	}

	LOG_INFO("Render: shaders OK, resizing...");
	Resize(width, height);

	LOG_INFO("RenderSystem initialized (%ux%u)", width, height);
	return true;
}

void RenderSystem::Shutdown()
{
	if (m_viewportRTV)   m_viewportRTV->Release();
	if (m_viewportSRV)   m_viewportSRV->Release();
	if (m_viewportTex)   m_viewportTex->Release();
	if (m_viewportDSV)   m_viewportDSV->Release();
	if (m_depthTex)      m_depthTex->Release();
	if (m_shadowTex)     m_shadowTex->Release();
	if (m_shadowDSV)     m_shadowDSV->Release();
	if (m_shadowSRV)     m_shadowSRV->Release();
	if (m_shadowSampler) m_shadowSampler->Release();
	if (m_wireVS)        m_wireVS->Release();
	if (m_wirePS)        m_wirePS->Release();
	if (m_wireLayout)    m_wireLayout->Release();
	if (m_litVS)         m_litVS->Release();
	if (m_litPS)         m_litPS->Release();
	if (m_litLayout)     m_litLayout->Release();
	if (m_skyVS)         m_skyVS->Release();
	if (m_skyPS)         m_skyPS->Release();
	if (m_cbCamera)      m_cbCamera->Release();
	if (m_cbLighting)    m_cbLighting->Release();
	if (m_cbShadow)      m_cbShadow->Release();
	if (m_cbSky)         m_cbSky->Release();
	if (m_ctx)           m_ctx->Release();
}

void RenderSystem::Resize(u32 w, u32 h)
{
	m_width = w; m_height = h;
	if (m_viewportRTV) { m_viewportRTV->Release(); m_viewportRTV = nullptr; }
	if (m_viewportSRV) { m_viewportSRV->Release(); m_viewportSRV = nullptr; }
	if (m_viewportTex) { m_viewportTex->Release(); m_viewportTex = nullptr; }
	if (m_viewportDSV) { m_viewportDSV->Release(); m_viewportDSV = nullptr; }
	if (m_depthTex)    { m_depthTex->Release();    m_depthTex    = nullptr; }

	D3D11_TEXTURE2D_DESC td = {};
	td.Width = w; td.Height = h; td.MipLevels = 1; td.ArraySize = 1;
	td.Format = DXGI_FORMAT_R8G8B8A8_UNORM; td.SampleDesc.Count = 1;
	td.Usage = D3D11_USAGE_DEFAULT;
	td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	m_device->CreateTexture2D(&td, nullptr, &m_viewportTex);
	m_device->CreateRenderTargetView(m_viewportTex, nullptr, &m_viewportRTV);
	m_device->CreateShaderResourceView(m_viewportTex, nullptr, &m_viewportSRV);

	td.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; td.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	m_device->CreateTexture2D(&td, nullptr, &m_depthTex);
	m_device->CreateDepthStencilView(m_depthTex, nullptr, &m_viewportDSV);
}

void RenderSystem::BeginFrame()
{
	float c[4] = {0.1f,0.3f,0.8f,1};
	m_ctx->ClearRenderTargetView(m_viewportRTV, c);
	m_ctx->ClearDepthStencilView(m_viewportDSV, D3D11_CLEAR_DEPTH, 1, 0);
	m_litVerts.clear(); m_litIndices.clear(); m_wireVerts.clear();
}

void RenderSystem::EndFrame() {}

void RenderSystem::RenderViewport(const Camera& cam, ID3D11ShaderResourceView** out) {
	if (out) *out = m_viewportSRV;
}

// ============================================================
//  BUFFER HELPERS
// ============================================================

ID3D11Buffer* RenderSystem::CreateVB(const void* data, u32 stride, u32 count)
{
	D3D11_BUFFER_DESC bd = {}; bd.Usage = D3D11_USAGE_DEFAULT; bd.ByteWidth = stride * count; bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	D3D11_SUBRESOURCE_DATA sd = { data }; ID3D11Buffer* buf = nullptr; m_device->CreateBuffer(&bd, &sd, &buf); return buf;
}

ID3D11Buffer* RenderSystem::CreateIB(const u32* data, u32 count)
{
	D3D11_BUFFER_DESC bd = {}; bd.Usage = D3D11_USAGE_DEFAULT; bd.ByteWidth = sizeof(u32) * count; bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	D3D11_SUBRESOURCE_DATA sd = { data }; ID3D11Buffer* buf = nullptr; m_device->CreateBuffer(&bd, &sd, &buf); return buf;
}

// ============================================================
//  DRAW HELPERS
// ============================================================

void RenderSystem::DrawMesh(const Mesh& mesh, const Vec3& pos, const Vec3& scale, const Vec3& color)
{
	if (mesh.vertices.empty()) return;
	u32 base = (u32)m_litVerts.size();
	for (auto& v : mesh.vertices)
		m_litVerts.push_back({v.pos * Vec3(scale.x,scale.y,scale.z) + pos, v.normal, color});
	for (auto idx : mesh.indices)
		m_litIndices.push_back(base + idx);
}

void RenderSystem::DrawGrid(const Camera& cam)
{
	Vec3 gray(0.3f,0.3f,0.3f), red(0.5f,0.1f,0.1f), green(0.1f,0.5f,0.1f);
	float size = 20, spacing = 1;
	for (float i = -size; i <= size; i += spacing)
	{
		m_wireVerts.push_back({{i,0,-size}, i==0?red:gray}); m_wireVerts.push_back({{i,0,size}, i==0?red:gray});
		m_wireVerts.push_back({{-size,0,i}, i==0?green:gray}); m_wireVerts.push_back({{size,0,i}, i==0?green:gray});
	}
	FlushWire(cam);
}

void RenderSystem::DrawWireCube(const Vec3& p, const Vec3& scale, const Vec3& color)
{
	Vec3 s(scale.x*0.5f, scale.y*0.5f, scale.z*0.5f);
	Vec3 v[8] = {{p.x-s.x,p.y-s.y,p.z-s.z},{p.x+s.x,p.y-s.y,p.z-s.z},{p.x+s.x,p.y+s.y,p.z-s.z},{p.x-s.x,p.y+s.y,p.z-s.z},
		{p.x-s.x,p.y-s.y,p.z+s.z},{p.x+s.x,p.y-s.y,p.z+s.z},{p.x+s.x,p.y+s.y,p.z+s.z},{p.x-s.x,p.y+s.y,p.z+s.z}};
	int e[12][2] = {{0,1},{1,2},{2,3},{3,0},{4,5},{5,6},{6,7},{7,4},{0,4},{1,5},{2,6},{3,7}};
	for (auto& ed : e) { m_wireVerts.push_back({v[ed[0]],color}); m_wireVerts.push_back({v[ed[1]],color}); }
}

// ============================================================
//  FLUSH WIRE
// ============================================================

void RenderSystem::FlushWire(const Camera& cam)
{
	if (m_wireVerts.empty()) return;
	auto* vb = CreateVB(m_wireVerts.data(), sizeof(Vertex), (u32)m_wireVerts.size());
	UINT stride = sizeof(Vertex), offset = 0;
	m_ctx->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
	m_ctx->IASetInputLayout(m_wireLayout);
	m_ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	m_ctx->VSSetShader(m_wireVS, nullptr, 0);
	m_ctx->PSSetShader(m_wirePS, nullptr, 0);

	XMMATRIX v = XMLoadFloat4x4((XMFLOAT4X4*)cam.viewMatrix.m);
	XMMATRIX p = XMLoadFloat4x4((XMFLOAT4X4*)cam.projMatrix.m);
	XMMATRIX vp = XMMatrixMultiply(v, p);
	CameraCB cb; cb.viewProj = vp;
	m_ctx->UpdateSubresource(m_cbCamera, 0, nullptr, &cb, 0, 0);
	m_ctx->VSSetConstantBuffers(0, 1, &m_cbCamera);

	D3D11_VIEWPORT vp3d = {0,0,(float)m_width,(float)m_height,0,1};
	m_ctx->RSSetViewports(1, &vp3d);
	m_ctx->OMSetRenderTargets(1, &m_viewportRTV, m_viewportDSV);
	m_ctx->Draw((UINT)m_wireVerts.size(), 0);
	vb->Release(); m_wireVerts.clear();
}

// ============================================================
//  FLUSH LIT — Per-pixel lighting + PCF shadow
// ============================================================

void RenderSystem::FlushLit(const Camera& cam)
{
	if (m_litVerts.empty()) return;
	auto* vb = CreateVB(m_litVerts.data(), sizeof(LtVertex), (u32)m_litVerts.size());
	auto* ib = CreateIB(m_litIndices.data(), (u32)m_litIndices.size());

	UINT stride = sizeof(LtVertex), offset = 0;
	m_ctx->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
	m_ctx->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);
	m_ctx->IASetInputLayout(m_litLayout);
	m_ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_ctx->VSSetShader(m_litVS, nullptr, 0);
	m_ctx->PSSetShader(m_litPS, nullptr, 0);

	// Camera CB (b0)
	XMMATRIX v = XMLoadFloat4x4((XMFLOAT4X4*)cam.viewMatrix.m);
	XMMATRIX p = XMLoadFloat4x4((XMFLOAT4X4*)cam.projMatrix.m);
	XMMATRIX vp = XMMatrixMultiply(v, p);
	CameraCB camCB; camCB.viewProj = vp;
	m_ctx->UpdateSubresource(m_cbCamera, 0, nullptr, &camCB, 0, 0);
	m_ctx->VSSetConstantBuffers(0, 1, &m_cbCamera);

	// Light CB (b1) — identity world, sun light direction, camera pos for specular
	XMMATRIX world = XMMatrixIdentity();
	LightCB lit = {};
	lit.world       = world;
	lit.lightDir    = {-0.4f, -0.8f, -0.4f};
	lit.objectColor = {1.0f, 1.0f, 1.0f};
	lit.camPos      = {cam.position.x, cam.position.y, cam.position.z};
	m_ctx->UpdateSubresource(m_cbLighting, 0, nullptr, &lit, 0, 0);
	m_ctx->VSSetConstantBuffers(1, 1, &m_cbLighting);
	m_ctx->PSSetConstantBuffers(1, 1, &m_cbLighting);

	// Shadow CB (b2) — light view-proj for shadow lookup
	if (m_cbShadow) {
		ShadowCB scb; scb.lightVP = m_lightViewProj;
		m_ctx->UpdateSubresource(m_cbShadow, 0, nullptr, &scb, 0, 0);
		m_ctx->VSSetConstantBuffers(2, 1, &m_cbShadow);
		m_ctx->PSSetConstantBuffers(2, 1, &m_cbShadow);
	}

	// Bind shadow map to PS slot 0
	if (m_shadowSRV && m_shadowSampler) {
		m_ctx->PSSetShaderResources(0, 1, &m_shadowSRV);
		m_ctx->PSSetSamplers(0, 1, &m_shadowSampler);
	}

	D3D11_VIEWPORT vp3d = {0,0,(float)m_width,(float)m_height,0,1};
	m_ctx->RSSetViewports(1, &vp3d);

	D3D11_RASTERIZER_DESC rd = {};
	rd.FillMode = D3D11_FILL_SOLID; rd.CullMode = D3D11_CULL_BACK; rd.DepthClipEnable = TRUE;
	ID3D11RasterizerState* rs = nullptr; m_device->CreateRasterizerState(&rd, &rs);
	m_ctx->RSSetState(rs); rs->Release();

	m_ctx->OMSetRenderTargets(1, &m_viewportRTV, m_viewportDSV);
	m_ctx->DrawIndexed((UINT)m_litIndices.size(), 0, 0);

	// Unbind shadow SRV to avoid hazard on next frame's shadow pass
	ID3D11ShaderResourceView* nullSRV = nullptr;
	m_ctx->PSSetShaderResources(0, 1, &nullSRV);

	vb->Release(); ib->Release();
	m_litVerts.clear(); m_litIndices.clear();
}

// ============================================================
//  DRAW MESH CACHED — for single meshes with explicit world
// ============================================================

void RenderSystem::DrawMeshCached(const Mesh& mesh, const Camera& cam, const Vec3& pos, const Vec3& scale, const Vec3& color)
{
	if (!mesh.GetVB() || !mesh.GetIB()) return;
	ID3D11Buffer* vb = mesh.GetVB(); ID3D11Buffer* ib = mesh.GetIB();
	UINT s = sizeof(MeshVertex), o = 0;
	m_ctx->IASetVertexBuffers(0, 1, &vb, &s, &o); m_ctx->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);
	m_ctx->IASetInputLayout(m_litLayout); m_ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_ctx->VSSetShader(m_litVS, nullptr, 0); m_ctx->PSSetShader(m_litPS, nullptr, 0);

	// Camera CB (b0)
	XMMATRIX v = XMLoadFloat4x4((XMFLOAT4X4*)cam.viewMatrix.m), p2 = XMLoadFloat4x4((XMFLOAT4X4*)cam.projMatrix.m);
	XMMATRIX vp = XMMatrixMultiply(v, p2);
	CameraCB camCB; camCB.viewProj = vp;
	m_ctx->UpdateSubresource(m_cbCamera, 0, nullptr, &camCB, 0, 0); m_ctx->VSSetConstantBuffers(0, 1, &m_cbCamera);

	// Light CB (b1) — per-object world transform
	XMMATRIX w = XMMatrixScaling(scale.x, scale.y, scale.z) * XMMatrixTranslation(pos.x, pos.y, pos.z);
	LightCB lit = {};
	lit.world       = w;
	lit.lightDir    = {-0.4f, -0.8f, -0.4f};
	lit.objectColor = {color.x, color.y, color.z};
	lit.camPos      = {cam.position.x, cam.position.y, cam.position.z};
	m_ctx->UpdateSubresource(m_cbLighting, 0, nullptr, &lit, 0, 0);
	m_ctx->VSSetConstantBuffers(1, 1, &m_cbLighting);
	m_ctx->PSSetConstantBuffers(1, 1, &m_cbLighting);

	// Shadow CB (b2)
	if (m_cbShadow) {
		ShadowCB scb; scb.lightVP = m_lightViewProj;
		m_ctx->UpdateSubresource(m_cbShadow, 0, nullptr, &scb, 0, 0);
		m_ctx->VSSetConstantBuffers(2, 1, &m_cbShadow);
		m_ctx->PSSetConstantBuffers(2, 1, &m_cbShadow);
	}

	// Bind shadow map
	if (m_shadowSRV && m_shadowSampler) {
		m_ctx->PSSetShaderResources(0, 1, &m_shadowSRV);
		m_ctx->PSSetSamplers(0, 1, &m_shadowSampler);
	}

	D3D11_VIEWPORT dv = {0,0,(float)m_width,(float)m_height,0,1}; m_ctx->RSSetViewports(1, &dv);

	D3D11_RASTERIZER_DESC rd = {};
	rd.FillMode = D3D11_FILL_SOLID; rd.CullMode = D3D11_CULL_BACK; rd.DepthClipEnable = TRUE;
	ID3D11RasterizerState* rs = nullptr; m_device->CreateRasterizerState(&rd, &rs);
	m_ctx->RSSetState(rs); rs->Release();

	m_ctx->OMSetRenderTargets(1, &m_viewportRTV, m_viewportDSV);
	m_ctx->DrawIndexed((UINT)mesh.indices.size(), 0, 0);

	// Unbind shadow SRV
	ID3D11ShaderResourceView* nullSRV = nullptr;
	m_ctx->PSSetShaderResources(0, 1, &nullSRV);
}

// ============================================================
//  SHADOW MAP PASS
// ============================================================

void RenderSystem::RenderShadowMap(const Camera& cam)
{
	if (!m_shadowDSV) return;
	m_ctx->ClearDepthStencilView(m_shadowDSV, D3D11_CLEAR_DEPTH, 1, 0);
	if (m_litVerts.empty()) return;
	m_ctx->OMSetRenderTargets(0, nullptr, m_shadowDSV);
	D3D11_VIEWPORT svp = {0,0,2048,2048,0,1};
	m_ctx->RSSetViewports(1, &svp);
	// Use lit VS (accepts POSITION+NORMAL+COLOR layout) with null PS for depth-only pass
	m_ctx->VSSetShader(m_litVS, nullptr, 0); m_ctx->PSSetShader(nullptr, nullptr, 0);

	// Sun light orthographic view (matches sky sun direction)
	XMVECTOR lp = XMVectorSet(40,60,20,1), lt = XMVectorZero(), up = XMVectorSet(0,1,0,0);
	XMMATRIX lv = XMMatrixLookAtLH(lp,lt,up);
	XMMATRIX lp2 = XMMatrixOrthographicLH(120,120,1,200);
	m_lightViewProj = XMMatrixMultiply(lv, lp2);   // <-- saved for FlushLit

	// Camera CB (b0) — light VP
	CameraCB cb; cb.viewProj = m_lightViewProj;
	m_ctx->UpdateSubresource(m_cbCamera, 0, nullptr, &cb, 0, 0);
	m_ctx->VSSetConstantBuffers(0, 1, &m_cbCamera);

	// Light CB (b1) — identity world, dummy values (depth only, pos is all that matters)
	LightCB litcb = {}; litcb.world = XMMatrixIdentity(); litcb.lightDir = {0,1,0}; litcb.objectColor = {1,1,1}; litcb.camPos = {0,0,0};
	m_ctx->UpdateSubresource(m_cbLighting, 0, nullptr, &litcb, 0, 0);
	m_ctx->VSSetConstantBuffers(1, 1, &m_cbLighting);

	// Shadow CB (b2) — identity for depth pass
	ShadowCB scb; scb.lightVP = XMMatrixIdentity();
	m_ctx->UpdateSubresource(m_cbShadow, 0, nullptr, &scb, 0, 0);
	m_ctx->VSSetConstantBuffers(2, 1, &m_cbShadow);

	auto* vb = CreateVB(m_litVerts.data(), sizeof(LtVertex), (u32)m_litVerts.size());
	auto* ib = CreateIB(m_litIndices.data(), (u32)m_litIndices.size());
	UINT s = sizeof(LtVertex), o = 0;
	m_ctx->IASetVertexBuffers(0, 1, &vb, &s, &o);
	m_ctx->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);
	m_ctx->IASetInputLayout(m_litLayout);
	m_ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_ctx->DrawIndexed((UINT)m_litIndices.size(), 0, 0);
	vb->Release(); ib->Release();
}

// ============================================================
//  SKY PASS
// ============================================================

void RenderSystem::RenderSky(const Camera& cam)
{
	if (!m_skyVS || !m_skyPS) return;
	m_ctx->VSSetShader(m_skyVS, nullptr, 0);
	m_ctx->PSSetShader(m_skyPS, nullptr, 0);
	m_ctx->IASetInputLayout(nullptr);
	m_ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	struct SkyCB { float sd[3]; float p1; float c1[4]; float c2[4]; };
	SkyCB sc = {{0.4f, 0.8f, 0.2f}, 0, {0.1f,0.3f,0.8f,1}, {0.4f,0.6f,1.0f,1}};
	m_ctx->UpdateSubresource(m_cbSky, 0, nullptr, &sc, 0, 0);
	m_ctx->PSSetConstantBuffers(0, 1, &m_cbSky);
	D3D11_VIEWPORT dv = {0,0,(float)m_width,(float)m_height,0,1};
	m_ctx->RSSetViewports(1, &dv);
	m_ctx->OMSetRenderTargets(1, &m_viewportRTV, nullptr); // no depth — sky always visible
	m_ctx->Draw(3, 0);
}

// ============================================================
//  COMPILE SHADERS
// ============================================================

bool RenderSystem::CompileShaders()
{
	// ------------------------------------------------------------------
	// Wire vertex shader (used also for shadow depth pass)
	// ------------------------------------------------------------------
	const char* vs = R"(
		cbuffer Cam : register(b0) { float4x4 vp; }
		struct VI { float3 p : POSITION; float3 c : COLOR; };
		struct VO { float4 p : SV_POSITION; float3 c : COLOR; };
		VO main(VI i) { VO o; o.p = mul(vp, float4(i.p,1)); o.c = i.c; return o; }
	)";
	const char* ps = R"(
		struct PI { float4 p : SV_POSITION; float3 c : COLOR; };
		float4 main(PI i) : SV_Target { return float4(i.c,1); }
	)";

	// ------------------------------------------------------------------
	// Lit vertex shader — outputs world-space pos, normal and light-space pos
	// ------------------------------------------------------------------
	const char* litVS = R"(
		cbuffer Cam    : register(b0) { float4x4 vp; }
		cbuffer Lit    : register(b1) {
			float4x4 world;
			float3   lightDir; float pad1;
			float3   objColor; float pad2;
			float3   camPos;   float pad3;
		}
		cbuffer Shadow : register(b2) { float4x4 lightVP; }

		struct VI { float3 p : POSITION; float3 n : NORMAL; float3 c : COLOR; };
		struct VO {
			float4 p    : SV_POSITION;
			float3 wpos : TEXCOORD0;
			float3 wnor : TEXCOORD1;
			float4 lpos : TEXCOORD2;
			float3 col  : COLOR;
		};

		VO main(VI i) {
			VO o;
			float4 wp   = mul(world, float4(i.p, 1.0));
			o.p         = mul(vp,    wp);
			o.wpos      = wp.xyz;
			o.wnor      = normalize(mul((float3x3)world, i.n));
			o.lpos      = mul(lightVP, wp);
			o.col       = i.c * objColor;
			return o;
		}
	)";

	// ------------------------------------------------------------------
	// Lit pixel shader — per-pixel Lambert + Blinn-Phong + PCF shadow
	// ------------------------------------------------------------------
	const char* litPS = R"(
		cbuffer Lit    : register(b1) {
			float4x4 world;
			float3   lightDir; float pad1;
			float3   objColor; float pad2;
			float3   camPos;   float pad3;
		}
		cbuffer Shadow : register(b2) { float4x4 lightVP; }

		Texture2D       shadowMap    : register(t0);
		SamplerComparisonState shadowSmp : register(s0);

		struct PI {
			float4 p    : SV_POSITION;
			float3 wpos : TEXCOORD0;
			float3 wnor : TEXCOORD1;
			float4 lpos : TEXCOORD2;
			float3 col  : COLOR;
		};

		float ShadowPCF(float4 lpos)
		{
			// Perspective divide -> NDC
			float3 proj = lpos.xyz / lpos.w;
			// NDC [-1,1] -> UV [0,1]
			float2 uv = proj.xy * float2(0.5, -0.5) + 0.5;
			// If outside shadow frustum, fully lit
			if (uv.x < 0 || uv.x > 1 || uv.y < 0 || uv.y > 1) return 1.0;
			float depth = proj.z - 0.002; // bias to avoid acne
			// 3x3 PCF
			float shadow = 0;
			float2 texel = 1.0 / float2(2048, 2048);
			[unroll] for (int dx = -1; dx <= 1; dx++)
			[unroll] for (int dy = -1; dy <= 1; dy++)
				shadow += shadowMap.SampleCmpLevelZero(shadowSmp, uv + float2(dx,dy)*texel, depth);
			return shadow / 9.0;
		}

		float4 main(PI i) : SV_Target
		{
			float3 N = normalize(i.wnor);
			float3 L = normalize(-lightDir);
			float3 V = normalize(camPos - i.wpos);
			float3 H = normalize(L + V);

			// Diffuse (Lambert)
			float diff = saturate(dot(N, L));
			// Specular (Blinn-Phong)
			float spec = pow(saturate(dot(N, H)), 32.0) * 0.3;
			// Ambient
			float ambient = 0.15;

			float shadow = ShadowPCF(i.lpos);

			float3 col = i.col * (ambient + shadow * (diff + spec));
			return float4(col, 1.0);
		}
	)";

	// ------------------------------------------------------------------
	// Sky shaders (unchanged)
	// ------------------------------------------------------------------
	const char* skyVS = R"(
		struct VO { float4 p : SV_POSITION; };
		VO main(uint id : SV_VertexID) {
			VO o; o.p = float4(id==0?-1:(id==1?3:-1), id==0?1:(id==1?3:-3), 0, 1); return o;
		}
	)";
	const char* skyPS = R"(
		cbuffer Sky : register(b0) { float3 sunDir; float pad; float4 c1; float4 c2; }
		float4 main(float4 p : SV_POSITION) : SV_Target {
			float3 d = normalize(float3(p.x, 0.5 + abs(p.y) * 0.5, p.y));
			float sunAngle = dot(d, sunDir);
			float rayleigh = pow(1.0 - abs(d.y), 8.0);
			float mie = pow(saturate(sunAngle), 64.0);
			float3 skyCol = lerp(float3(0.3,0.5,1.0), float3(0.5,0.7,1.0), d.y * 0.5 + 0.5);
			float3 sunCol = float3(1.0, 0.9, 0.5) * mie * 2.0;
			float3 horizon = float3(0.9, 0.8, 0.6) * pow(1.0 - abs(d.y), 4.0);
			float3 col = skyCol * (1.0 - rayleigh * 0.5) + sunCol + horizon * 0.3;
			return float4(col, 1);
		}
	)";

	ID3DBlob *b = nullptr, *e = nullptr;
	HRESULT hr;

	// Wire VS
	hr = D3DCompile(vs, strlen(vs), "vs", nullptr, nullptr, "main", "vs_5_0", 0, 0, &b, &e);
	if (FAILED(hr)) { LOG_ERROR("Wire VS: %s", e ? (char*)e->GetBufferPointer() : "?"); if (e) e->Release(); return false; }
	m_device->CreateVertexShader(b->GetBufferPointer(), b->GetBufferSize(), nullptr, &m_wireVS);
	D3D11_INPUT_ELEMENT_DESC wl[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
	m_device->CreateInputLayout(wl, 2, b->GetBufferPointer(), b->GetBufferSize(), &m_wireLayout);
	b->Release();

	// Wire PS
	hr = D3DCompile(ps, strlen(ps), "ps", nullptr, nullptr, "main", "ps_5_0", 0, 0, &b, &e);
	if (FAILED(hr)) { LOG_ERROR("Wire PS: %s", e ? (char*)e->GetBufferPointer() : "?"); if (e) e->Release(); return false; }
	m_device->CreatePixelShader(b->GetBufferPointer(), b->GetBufferSize(), nullptr, &m_wirePS);
	b->Release();

	// Lit VS
	hr = D3DCompile(litVS, strlen(litVS), "litvs", nullptr, nullptr, "main", "vs_5_0", 0, 0, &b, &e);
	if (FAILED(hr)) { LOG_ERROR("Lit VS: %s", e ? (char*)e->GetBufferPointer() : "?"); if (e) e->Release(); return false; }
	m_device->CreateVertexShader(b->GetBufferPointer(), b->GetBufferSize(), nullptr, &m_litVS);
	D3D11_INPUT_ELEMENT_DESC ll[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
	m_device->CreateInputLayout(ll, 3, b->GetBufferPointer(), b->GetBufferSize(), &m_litLayout);
	b->Release();

	// Lit PS
	hr = D3DCompile(litPS, strlen(litPS), "litps", nullptr, nullptr, "main", "ps_5_0", 0, 0, &b, &e);
	if (FAILED(hr)) { LOG_ERROR("Lit PS: %s", e ? (char*)e->GetBufferPointer() : "?"); if (e) e->Release(); return false; }
	m_device->CreatePixelShader(b->GetBufferPointer(), b->GetBufferSize(), nullptr, &m_litPS);
	b->Release();

	// Sky VS
	hr = D3DCompile(skyVS, strlen(skyVS), "skyvs", nullptr, nullptr, "main", "vs_5_0", 0, 0, &b, &e);
	if (FAILED(hr)) { LOG_ERROR("Sky VS: %s", e ? (char*)e->GetBufferPointer() : "?"); if (e) e->Release(); }
	else { m_device->CreateVertexShader(b->GetBufferPointer(), b->GetBufferSize(), nullptr, &m_skyVS); b->Release(); }

	// Sky PS
	hr = D3DCompile(skyPS, strlen(skyPS), "skyps", nullptr, nullptr, "main", "ps_5_0", 0, 0, &b, &e);
	if (FAILED(hr)) { LOG_ERROR("Sky PS: %s", e ? (char*)e->GetBufferPointer() : "?"); if (e) e->Release(); }
	else { m_device->CreatePixelShader(b->GetBufferPointer(), b->GetBufferSize(), nullptr, &m_skyPS); b->Release(); }

	// ------------------------------------------------------------------
	// Constant buffers
	// ------------------------------------------------------------------
	D3D11_BUFFER_DESC cbd = {}; cbd.Usage = D3D11_USAGE_DEFAULT; cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	cbd.ByteWidth = sizeof(CameraCB);
	m_device->CreateBuffer(&cbd, nullptr, &m_cbCamera);

	cbd.ByteWidth = sizeof(LightCB);
	m_device->CreateBuffer(&cbd, nullptr, &m_cbLighting);

	cbd.ByteWidth = sizeof(ShadowCB);
	m_device->CreateBuffer(&cbd, nullptr, &m_cbShadow);

	struct SkyCB { float sunDir[3]; float p1; float c1[4]; float c2[4]; };
	cbd.ByteWidth = sizeof(SkyCB);
	m_device->CreateBuffer(&cbd, nullptr, &m_cbSky);

	// ------------------------------------------------------------------
	// PCF Comparison Sampler for shadow map
	// ------------------------------------------------------------------
	D3D11_SAMPLER_DESC sd = {};
	sd.Filter         = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	sd.AddressU       = D3D11_TEXTURE_ADDRESS_BORDER;
	sd.AddressV       = D3D11_TEXTURE_ADDRESS_BORDER;
	sd.AddressW       = D3D11_TEXTURE_ADDRESS_BORDER;
	sd.BorderColor[0] = 1.0f; // outside shadow frustum = fully lit
	sd.BorderColor[1] = 1.0f;
	sd.BorderColor[2] = 1.0f;
	sd.BorderColor[3] = 1.0f;
	sd.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	sd.MaxLOD         = D3D11_FLOAT32_MAX;
	m_device->CreateSamplerState(&sd, &m_shadowSampler);

	// ------------------------------------------------------------------
	// Shadow map texture (2048x2048 depth)
	// ------------------------------------------------------------------
	D3D11_TEXTURE2D_DESC smd = {};
	smd.Width = 2048; smd.Height = 2048; smd.MipLevels = 1; smd.ArraySize = 1;
	smd.Format = DXGI_FORMAT_R24G8_TYPELESS; smd.SampleDesc.Count = 1;
	smd.Usage = D3D11_USAGE_DEFAULT; smd.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	if (SUCCEEDED(m_device->CreateTexture2D(&smd, nullptr, &m_shadowTex))) {
		D3D11_DEPTH_STENCIL_VIEW_DESC dsvd = {}; dsvd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		m_device->CreateDepthStencilView(m_shadowTex, &dsvd, &m_shadowDSV);
		D3D11_SHADER_RESOURCE_VIEW_DESC srvd = {}; srvd.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS; srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D; srvd.Texture2D.MipLevels = 1;
		m_device->CreateShaderResourceView(m_shadowTex, &srvd, &m_shadowSRV);
	}

	// Initialize light view-proj to identity so first frame before shadow pass has valid data
	m_lightViewProj = XMMatrixIdentity();

	return true;
}
