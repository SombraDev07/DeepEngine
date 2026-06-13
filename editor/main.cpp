#include <windows.h>
#undef min
#undef max
#include <d3d11.h>
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <Logger.h>
#include <ECS.h>
#include <JobSystem.h>
#include <RenderSystem.h>
#include <Mesh.h>
#include <PhysicsSystem.h>
#include <thread>
#include <cmath>
#include <algorithm>
#include <string>
#include <chrono>
#include <commdlg.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

static ID3D11Device* g_dev = nullptr;
static ID3D11DeviceContext* g_ctx = nullptr;
static IDXGISwapChain* g_swap = nullptr;
static ID3D11RenderTargetView* g_rtv = nullptr;
static bool g_demo = false;
static float g_fps = 0;
static std::vector<Entity> g_ents;
static std::vector<JPH::BodyID> g_bodies;
static RenderSystem* g_render = nullptr;
static Camera g_cam;
static ID3D11ShaderResourceView* g_srv = nullptr;
static PhysicsSystem* g_phys = nullptr;
static Mesh* g_loadedMesh = nullptr;
static Vec3 g_loadedPos(0, 5, 0);
static float g_loadedScale = 0.1f;
static std::string g_loadedName;
static bool g_showSky = true;

bool MakeDevice(HWND h) {
	DXGI_SWAP_CHAIN_DESC sd = {};
	sd.BufferCount = 2; sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; sd.BufferDesc.RefreshRate.Numerator = 60; sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; sd.OutputWindow = h; sd.SampleDesc.Count = 1; sd.Windowed = TRUE; sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	D3D_FEATURE_LEVEL fl; const D3D_FEATURE_LEVEL fla[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
	HRESULT hr = D3D11CreateDeviceAndSwapChain(0, D3D_DRIVER_TYPE_HARDWARE, 0, 0, fla, 2, D3D11_SDK_VERSION, &sd, &g_swap, &g_dev, &fl, &g_ctx);
	if (FAILED(hr)) hr = D3D11CreateDeviceAndSwapChain(0, D3D_DRIVER_TYPE_WARP, 0, 0, fla, 2, D3D11_SDK_VERSION, &sd, &g_swap, &g_dev, &fl, &g_ctx);
	return SUCCEEDED(hr);
}
void MakeRTV() { ID3D11Texture2D* bb; g_swap->GetBuffer(0, IID_PPV_ARGS(&bb)); g_dev->CreateRenderTargetView(bb, 0, &g_rtv); bb->Release(); }
void KillRTV() { if (g_rtv) { g_rtv->Release(); g_rtv = 0; } }

LRESULT CALLBACK WndProc(HWND h, UINT m, WPARAM w, LPARAM l) {
	if (ImGui_ImplWin32_WndProcHandler(h, m, w, l)) return true;
	switch (m) {
	case WM_SIZE: if (g_dev && w != SIZE_MINIMIZED) { KillRTV(); g_swap->ResizeBuffers(0, LOWORD(l), HIWORD(l), DXGI_FORMAT_UNKNOWN, 0); MakeRTV(); } return 0;
	case WM_SYSCOMMAND: if ((w & 0xfff0) == SC_KEYMENU) return 0; break;
	case WM_DESTROY: PostQuitMessage(0); return 0;
	}
	return DefWindowProcW(h, m, w, l);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nShow) {
	LOG_INFO("Deep Engine starting..."); JobSystem::Init(2);
	WNDCLASSEXW wc = {}; wc.cbSize = sizeof(wc); wc.style = CS_CLASSDC; wc.lpfnWndProc = WndProc; wc.hInstance = hInst; wc.lpszClassName = L"DeepEngine";
	RegisterClassExW(&wc);
	HWND hwnd = CreateWindowW(L"DeepEngine", L"Deep Engine v0.2.1", WS_OVERLAPPEDWINDOW, 100, 100, 1600, 900, 0, 0, hInst, 0);
	if (!MakeDevice(hwnd)) { LOG_ERROR("D3D11 failed"); return 1; }
	MakeRTV(); ShowWindow(hwnd, nShow); UpdateWindow(hwnd);
	IMGUI_CHECKVERSION(); ImGui::CreateContext(); ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(hwnd); ImGui_ImplDX11_Init(g_dev, g_ctx);
	g_render = new RenderSystem(); g_render->Init(g_dev, 1600, 900);
	g_phys = new PhysicsSystem(); g_phys->Init();
	g_cam.position = Vec3(0, 5, 10);
	g_cam.pitch = -0.4f;

	auto last = std::chrono::high_resolution_clock::now();
	bool done = false;

	while (!done) {
		MSG msg;
		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) { TranslateMessage(&msg); DispatchMessage(&msg); if (msg.message == WM_QUIT) done = true; }
		if (done) break;

		auto now = std::chrono::high_resolution_clock::now();
		float dt = (float)std::chrono::duration<double>(now - last).count();
		last = now; g_fps = dt > 0 ? 1.0f / dt : 0;
		if (g_phys) g_phys->Update(std::min(dt, 0.1f));

		if (g_render) {
			g_render->BeginFrame();
			for (auto e : g_ents) { auto& t = ECS::Get<TransformComponent>(e); g_render->DrawMesh(*MeshCache::Get().GetCube(), t.position, Vec3(1), Vec3(0.3f + ((u32)e%7)*0.1f, 0.4f + ((u32)e%5)*0.1f, 0.6f + ((u32)e%3)*0.1f)); }
			if (g_loadedMesh) g_render->DrawMeshCached(*g_loadedMesh, g_cam, g_loadedPos, Vec3(g_loadedScale), Vec3(1,1,1));
			g_render->FlushLit(g_cam);
			g_render->DrawGrid(g_cam);
			g_render->RenderViewport(g_cam, &g_srv);
		}

		ImGui_ImplDX11_NewFrame(); ImGui_ImplWin32_NewFrame(); ImGui::NewFrame();

		// Viewport
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0)); ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
		ImGui::SetNextWindowPos(ImVec2(0, 18)); ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y - 18));
		if (ImGui::Begin("##VP", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBringToFrontOnFocus)) {
			bool hov = ImGui::IsWindowHovered();
			if (hov && ImGui::IsMouseDown(ImGuiMouseButton_Right)) { auto d = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right); g_cam.yaw -= d.x * 0.003f; g_cam.pitch -= d.y * 0.003f; g_cam.pitch = std::max(-1.3f, std::min(1.3f, g_cam.pitch)); ImGui::ResetMouseDragDelta(); }
			float spd = 5.0f * dt; if (ImGui::IsKeyDown(ImGuiKey_LeftShift)) spd *= 3.0f;
			Vec3 fwd(cosf(g_cam.pitch) * sinf(g_cam.yaw), 0, cosf(g_cam.pitch) * cosf(g_cam.yaw));
			Vec3 rgt(-fwd.z, 0, fwd.x); Vec3 up(0, 1, 0); fwd = fwd.Normalized(); rgt = rgt.Normalized();
			if (hov || ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow)) {
				if (ImGui::IsKeyDown(ImGuiKey_W)) g_cam.position = g_cam.position + fwd * spd;
				if (ImGui::IsKeyDown(ImGuiKey_S)) g_cam.position = g_cam.position - fwd * spd;
				if (ImGui::IsKeyDown(ImGuiKey_A)) g_cam.position = g_cam.position - rgt * spd;
				if (ImGui::IsKeyDown(ImGuiKey_D)) g_cam.position = g_cam.position + rgt * spd;
				if (ImGui::IsKeyDown(ImGuiKey_Q)) g_cam.position = g_cam.position - up * spd;
				if (ImGui::IsKeyDown(ImGuiKey_E)) g_cam.position = g_cam.position + up * spd;
			}
			ImVec2 avail = ImGui::GetContentRegionAvail(); g_cam.aspect = avail.x / std::max(avail.y, 1.0f); g_cam.Update();
			if (g_srv) ImGui::Image((ImTextureID)g_srv, avail);
			char buf[128]; snprintf(buf, sizeof(buf), "FPS: %.0f  Ents: %zu  Pos: (%.0f,%.0f,%.0f)", g_fps, g_ents.size(), g_cam.position.x, g_cam.position.y, g_cam.position.z);
			auto* dl = ImGui::GetWindowDrawList(); auto wp = ImGui::GetWindowPos(); dl->AddText(ImVec2(wp.x + avail.x - 420, wp.y + 30), ImColor(0, 255, 0, 200), buf);
		}
		ImGui::End(); ImGui::PopStyleVar(2);

		// Menu
		if (ImGui::BeginMainMenuBar()) { if (ImGui::BeginMenu("File")) { if (ImGui::MenuItem("Exit")) PostQuitMessage(0); ImGui::EndMenu(); } if (ImGui::BeginMenu("View")) { ImGui::MenuItem("ImGui Demo", 0, &g_demo); ImGui::EndMenu(); } ImGui::EndMainMenuBar(); }

		// Panel
		ImGui::SetNextWindowPos(ImVec2(0, 18)); ImGui::SetNextWindowSize(ImVec2(240, ImGui::GetIO().DisplaySize.y - 18));
		if (ImGui::Begin("##Panel", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse)) {
			ImGui::TextColored(ImVec4(0.3f, 0.7f, 1, 1), "DEEP ENGINE v0.2.1"); ImGui::Separator();
			if (ImGui::Button("Spawn 100", ImVec2(-1, 0)))
				for (int i = 0; i < 100; ++i) { auto e = ECS::Create(); int r = i / 10, c = i % 10; Vec3 pos(c*2.0f-9.0f, r*0.8f+0.5f+8.0f, r*2.0f-9.0f); ECS::Add<TransformComponent>(e, pos); ECS::Get<TagComponent>(e).name = "Spawned"; g_ents.push_back(e); if (g_phys) { auto id = g_phys->AddBox(pos, Vec3(0.5f)); g_phys->RegisterEntity(e, id); g_bodies.push_back(id); } }
			if (ImGui::Button("Clear All", ImVec2(-1, 0))) { for (auto e : g_ents) { if (g_phys) g_phys->UnregisterEntity(e); ECS::Destroy(e); } for (auto id : g_bodies) if (g_phys) g_phys->RemoveBody(id); g_ents.clear(); g_bodies.clear(); g_loadedMesh = nullptr; }
			ImGui::Separator();
			if (ImGui::Button("Load Model...", ImVec2(-1, 0))) { char path[MAX_PATH] = {}; OPENFILENAMEA ofn = {}; ofn.lStructSize = sizeof(ofn); ofn.hwndOwner = hwnd; ofn.lpstrFilter = "3D Models\0*.obj;*.gltf;*.glb\0\0"; ofn.lpstrFile = path; ofn.nMaxFile = MAX_PATH; ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST; if (GetOpenFileNameA(&ofn)) { g_loadedMesh = new Mesh(); std::string p(path); auto d = p.find_last_of('.'); bool ok = false; if (d != std::string::npos) { auto e = p.substr(d); if (e == ".gltf" || e == ".glb") ok = g_loadedMesh->LoadGLTF(path); else ok = g_loadedMesh->LoadOBJ(path); } else ok = g_loadedMesh->LoadOBJ(path); if (ok) { g_loadedMesh->UploadToGPU(g_dev); g_loadedPos = Vec3(0, 3, 0); g_loadedScale = 0.1f; g_loadedName = path; } else { delete g_loadedMesh; g_loadedMesh = nullptr; } } }
			if (g_loadedMesh) { ImGui::SliderFloat("Scale", &g_loadedScale, 0.01f, 5.0f, "%.2f"); ImGui::SliderFloat3("Position", &g_loadedPos.x, -50.0f, 50.0f, "%.1f"); }
			ImGui::Separator(); ImGui::Text("Systems: Math/Memory/Jobs(%u)/ECS/Render/Phys/Mesh/GLTF", JobSystem::GetWorkerCount());
			if (!g_ents.empty()) { ImGui::Separator(); ImGui::Text("Entities: %zu", g_ents.size()); ImGui::BeginChild("List", ImVec2(0, 200), true); static int sel = -1; for (size_t i = 0; i < g_ents.size(); ++i) { auto e = g_ents[i]; if (!ECS::Has<TagComponent>(e)) continue; auto& t = ECS::Get<TransformComponent>(e); char b[64]; snprintf(b, sizeof(b), "[%zu] (%.0f,%.0f,%.0f)", i, t.position.x, t.position.y, t.position.z); if (ImGui::Selectable(b, sel == (int)i)) sel = (int)i; } ImGui::EndChild(); }
		}
		ImGui::End();

		if (g_demo) ImGui::ShowDemoWindow(&g_demo);
		ImGui::Render(); float cc[4] = {0.12f,0.12f,0.13f,1}; g_ctx->OMSetRenderTargets(1, &g_rtv, 0); g_ctx->ClearRenderTargetView(g_rtv, cc); ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData()); g_swap->Present(1, 0);
	}

	ImGui_ImplDX11_Shutdown(); ImGui_ImplWin32_Shutdown(); ImGui::DestroyContext();
	if (g_render) { g_render->Shutdown(); delete g_render; }
	if (g_phys) { g_phys->Shutdown(); delete g_phys; }
	KillRTV(); if (g_swap) g_swap->Release(); if (g_ctx) g_ctx->Release(); if (g_dev) g_dev->Release();
	DestroyWindow(hwnd); UnregisterClassW(L"DeepEngine", hInst);
	for (auto e : g_ents) ECS::Destroy(e); if (g_loadedMesh) delete g_loadedMesh;
	JobSystem::Shutdown(); return 0;
}
