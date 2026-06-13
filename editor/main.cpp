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
#include <cmath>
#include <string>
#include <chrono>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

static ID3D11Device* g_dev = nullptr;
static ID3D11DeviceContext* g_ctx = nullptr;
static IDXGISwapChain* g_swap = nullptr;
static ID3D11RenderTargetView* g_rtv = nullptr;
static bool g_demo = false;
static float g_fps = 0;
static std::vector<Entity> g_ents;
static RenderSystem* g_render = nullptr;
static Camera g_cam;
static ID3D11ShaderResourceView* g_srv = nullptr;

bool MakeDevice(HWND h) {
	DXGI_SWAP_CHAIN_DESC sd = {};
	sd.BufferCount = 2; sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60; sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; sd.OutputWindow = h;
	sd.SampleDesc.Count = 1; sd.Windowed = TRUE; sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
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
	LOG_INFO("Deep Engine starting...");
	JobSystem::Init(2);

	WNDCLASSEXW wc = {}; wc.cbSize = sizeof(wc); wc.style = CS_CLASSDC; wc.lpfnWndProc = WndProc; wc.hInstance = hInst; wc.lpszClassName = L"DeepEngine";
	RegisterClassExW(&wc);
	HWND hwnd = CreateWindowW(L"DeepEngine", L"Deep Engine v0.1", WS_OVERLAPPEDWINDOW, 100, 100, 1600, 900, 0, 0, hInst, 0);
	if (!MakeDevice(hwnd)) { LOG_ERROR("D3D11 failed"); return 1; }
	MakeRTV(); ShowWindow(hwnd, nShow); UpdateWindow(hwnd);

	IMGUI_CHECKVERSION(); ImGui::CreateContext(); ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(hwnd); ImGui_ImplDX11_Init(g_dev, g_ctx);

	g_render = new RenderSystem();
	g_render->Init(g_dev, 1600, 900);
	g_cam.position = Vec3(0, 5, 10);

	auto last = std::chrono::high_resolution_clock::now();
	bool done = false;
	float yaw = 0, pitch = -0.4f;

	while (!done) {
		MSG msg;
		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) { TranslateMessage(&msg); DispatchMessage(&msg); if (msg.message == WM_QUIT) done = true; }
		if (done) break;

		auto now = std::chrono::high_resolution_clock::now();
		float dt = (float)std::chrono::duration<double>(now - last).count();
		last = now; g_fps = dt > 0 ? 1.0f / dt : 0;

		if (g_render) {
			g_render->BeginFrame();
			for (auto e : g_ents) {
				auto& t = ECS::Get<TransformComponent>(e);
				g_render->DrawMesh(*MeshCache::Get().GetCube(), t.position, Vec3(1), Vec3(0.3f + ((u32)e%7)*0.1f, 0.4f + ((u32)e%5)*0.1f, 0.6f + ((u32)e%3)*0.1f));
			}
			g_render->FlushLit(g_cam);
			g_render->DrawGrid(g_cam);
			g_render->RenderViewport(g_cam, &g_srv);
		}

		ImGui_ImplDX11_NewFrame(); ImGui_ImplWin32_NewFrame(); ImGui::NewFrame();

		// Viewport fills screen
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
		ImGui::SetNextWindowPos(ImVec2(0, 18));
		ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y - 18));
		if (ImGui::Begin("##VP", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBringToFrontOnFocus)) {
			bool hovered = ImGui::IsWindowHovered();

			// Mouse look with right button
			if (hovered && ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
				auto d = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
				yaw -= d.x * 0.003f; pitch -= d.y * 0.003f;
				ImGui::ResetMouseDragDelta(ImGuiMouseButton_Right);
			}

			// WASD movement
			float speed = 5.0f * dt;
			if (ImGui::IsKeyDown(ImGuiKey_LeftShift)) speed *= 3.0f;
			Vec3 forward(cosf(pitch) * sinf(yaw), 0, cosf(pitch) * cosf(yaw));
			Vec3 right(-forward.z, 0, forward.x);
			Vec3 up(0, 1, 0);
			forward = forward.Normalized(); right = right.Normalized();

			if (hovered || ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow))
			{
				if (ImGui::IsKeyDown(ImGuiKey_W)) g_cam.position = g_cam.position + forward * speed;
				if (ImGui::IsKeyDown(ImGuiKey_S)) g_cam.position = g_cam.position - forward * speed;
				if (ImGui::IsKeyDown(ImGuiKey_A)) g_cam.position = g_cam.position - right * speed;
				if (ImGui::IsKeyDown(ImGuiKey_D)) g_cam.position = g_cam.position + right * speed;
				if (ImGui::IsKeyDown(ImGuiKey_Q)) g_cam.position = g_cam.position - up * speed;
				if (ImGui::IsKeyDown(ImGuiKey_E)) g_cam.position = g_cam.position + up * speed;
			}

				Vec3 fwd(cosf(g_cam.pitch) * sinf(g_cam.yaw), sinf(g_cam.pitch), cosf(g_cam.pitch) * cosf(g_cam.yaw)); (void)fwd;

			ImVec2 avail = ImGui::GetContentRegionAvail();
			g_cam.aspect = avail.x / std::max(avail.y, 1.0f); g_cam.Update();
			if (g_srv) ImGui::Image((ImTextureID)g_srv, avail);
			char buf[64]; snprintf(buf, sizeof(buf), "FPS: %.0f  Entities: %zu  Pos: (%.0f,%.0f,%.0f)", g_fps, g_ents.size(), g_cam.position.x, g_cam.position.y, g_cam.position.z);
			auto* dl = ImGui::GetWindowDrawList(); auto wp = ImGui::GetWindowPos();
			dl->AddText(ImVec2(wp.x + avail.x - 350, wp.y + 30), ImColor(0, 255, 0, 200), buf);
		}
		ImGui::End();
		ImGui::PopStyleVar(2);

		// Menu
		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("File")) { if (ImGui::MenuItem("Exit")) PostQuitMessage(0); ImGui::EndMenu(); }
			if (ImGui::BeginMenu("View")) { ImGui::MenuItem("ImGui Demo", 0, &g_demo); ImGui::EndMenu(); }
			ImGui::EndMainMenuBar();
		}

		// Left panel
		ImGui::SetNextWindowPos(ImVec2(0, 18)); ImGui::SetNextWindowSize(ImVec2(240, ImGui::GetIO().DisplaySize.y - 18));
		if (ImGui::Begin("##Panel", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse)) {
			ImGui::TextColored(ImVec4(0.3f, 0.7f, 1, 1), "DEEP ENGINE v0.1"); ImGui::Separator();
			if (ImGui::Button("Spawn 100 Entities", ImVec2(-1, 0)))
				for (int i = 0; i < 100; ++i) {
					auto e = ECS::Create(); int r = i / 10, c = i % 10;
					ECS::Add<TransformComponent>(e, Vec3(c*2.0f-9.0f, r*0.8f+0.5f, r*2.0f-9.0f));
					ECS::Get<TagComponent>(e).name = "Spawned"; g_ents.push_back(e);
				}
			if (ImGui::Button("Clear All", ImVec2(-1, 0))) { for (auto e : g_ents) ECS::Destroy(e); g_ents.clear(); }
			if (ImGui::Button("Run Heavy Job", ImVec2(-1, 0)))
				JobSystem::Run([] { volatile double x = 0; for (int i = 0; i < 100000000; ++i) x += sqrt((double)i); });
			ImGui::Separator(); ImGui::Text("Systems");
			ImGui::BulletText("Math (Vec3, Vec4, Quat)"); ImGui::BulletText("Memory (Pool)");
			ImGui::BulletText("Jobs (%u)", JobSystem::GetWorkerCount()); ImGui::BulletText("ECS (EnTT)");
			ImGui::BulletText("Renderer (D3D11 + HLSL)"); ImGui::BulletText("Mesh (.obj loader)");
			if (!g_ents.empty()) {
				ImGui::Separator(); ImGui::Text("Entities (%zu)", g_ents.size());
				ImGui::BeginChild("List", ImVec2(0, 200), true);
				static int sel = -1;
				for (size_t i = 0; i < g_ents.size(); ++i) {
					auto e = g_ents[i]; if (!ECS::Has<TagComponent>(e)) continue;
					auto& t = ECS::Get<TransformComponent>(e); char b[64];
					snprintf(b, sizeof(b), "[%zu] (%.0f,%.0f,%.0f)", i, t.position.x, t.position.y, t.position.z);
					if (ImGui::Selectable(b, sel == (int)i)) sel = (int)i;
				}
				ImGui::EndChild();
			}
		}
		ImGui::End();

		if (g_demo) ImGui::ShowDemoWindow(&g_demo);
		ImGui::Render(); float cc[4] = {0.12f,0.12f,0.13f,1}; g_ctx->OMSetRenderTargets(1, &g_rtv, 0); g_ctx->ClearRenderTargetView(g_rtv, cc);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData()); g_swap->Present(1, 0);
	}

	if (g_render) { g_render->Shutdown(); delete g_render; }
	ImGui_ImplDX11_Shutdown(); ImGui_ImplWin32_Shutdown(); ImGui::DestroyContext();
	KillRTV(); if (g_swap) { g_swap->Release(); g_swap = 0; }
	if (g_ctx) { g_ctx->Release(); g_ctx = 0; }
	if (g_dev) { g_dev->Release(); g_dev = 0; }
	DestroyWindow(hwnd); UnregisterClassW(L"DeepEngine", hInst);
	for (auto e : g_ents) ECS::Destroy(e);
	JobSystem::Shutdown(); return 0;
}
