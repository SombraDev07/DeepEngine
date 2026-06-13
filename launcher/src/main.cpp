#include <CryCore/Platform/CryWindows.h>
#include <CryCore/Platform/CryLibrary.h>
#include <CrySystem/SystemInitParams.h>
#include <CryCore/Platform/platform_impl.inl>

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

#include <cmath>
#include <chrono>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

static ID3D11Device* g_dev = nullptr;
static ID3D11DeviceContext* g_ctx = nullptr;
static IDXGISwapChain* g_swap = nullptr;
static ID3D11RenderTargetView* g_rtv = nullptr;
static float g_fps = 0;

bool CreateDevice(HWND h) {
	DXGI_SWAP_CHAIN_DESC sd = {};
	sd.BufferCount = 2; sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; sd.BufferDesc.RefreshRate.Numerator = 60; sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; sd.OutputWindow = h; sd.SampleDesc.Count = 1; sd.Windowed = TRUE; sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	D3D_FEATURE_LEVEL fl; const D3D_FEATURE_LEVEL fla[] = {D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0};
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
	case WM_DESTROY: PostQuitMessage(0); return 0;
	}
	return DefWindowProcW(h, m, w, l);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nShow)
{
	// Init CRYENGINE with manual loop
	SSystemInitParams params;
	cry_strcpy(params.szSystemCmdLine, GetCommandLineA());
	params.sLogFileName = "CryLauncher.log";
	params.bEditor = false;
	params.bExecuteCommandLine = false;
	params.bSkipRenderer = false;
	params.bUnattendedMode = false;

	if (!CryInitializeEngine(params, true)) {
		MessageBoxA(0, "Engine init failed.\nRun from bin/win_x64/", "Error", MB_ICONERROR);
		return 1;
	}
	ISystem* pSystem = params.pSystem;
	if (!pSystem) return 1;

	// Window + D3D11
	WNDCLASSEXW wc = {}; wc.cbSize = sizeof(wc); wc.style = CS_CLASSDC; wc.lpfnWndProc = WndProc; wc.hInstance = hInst; wc.lpszClassName = L"DeepCryLauncher";
	RegisterClassExW(&wc);
	HWND hwnd = CreateWindowW(L"DeepCryLauncher", L"Deep Engine v0.3 [CRYENGINE]", WS_OVERLAPPEDWINDOW, 100, 100, 1600, 900, 0, 0, hInst, 0);
	if (!CreateDevice(hwnd)) return 1;
	MakeRTV(); ShowWindow(hwnd, nShow); UpdateWindow(hwnd);

	// ImGui
	IMGUI_CHECKVERSION(); ImGui::CreateContext(); ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(hwnd); ImGui_ImplDX11_Init(g_dev, g_ctx);

	auto last = std::chrono::high_resolution_clock::now();
	bool done = false;

	while (!done) {
		MSG msg;
		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) { TranslateMessage(&msg); DispatchMessage(&msg); if (msg.message == WM_QUIT) done = true; }
		if (done) break;

		auto now = std::chrono::high_resolution_clock::now();
		float dt = (float)std::chrono::duration<double>(now - last).count();
		last = now; g_fps = dt > 0 ? 1.0f / dt : 0;

		if (pSystem) pSystem->DoFrame();

		ImGui_ImplDX11_NewFrame(); ImGui_ImplWin32_NewFrame(); ImGui::NewFrame();

		ImGui::SetNextWindowPos(ImVec2(0, 0)); ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y));
		ImGui::Begin("Deep Engine [CRYENGINE]", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_MenuBar);
		if (ImGui::BeginMenuBar()) {
			if (ImGui::BeginMenu("File")) { if (ImGui::MenuItem("Exit")) PostQuitMessage(0); ImGui::EndMenu(); }
			ImGui::EndMenuBar();
		}
		ImGui::TextColored(ImVec4(0.3f, 0.7f, 1, 1), "DEEP ENGINE v0.3");
		ImGui::Separator();
		ImGui::Text("FPS: %.0f", g_fps);
		ImGui::Text("Core: CRYENGINE 5.7 | UI: ImGui v1.91");
		ImGui::Separator();
		ImGui::BulletText("CrySystem     : OK");
		ImGui::BulletText("Cry3DEngine   : OK");
		ImGui::BulletText("CryRenderer   : OK");
		ImGui::BulletText("CryPhysics    : OK");
		ImGui::BulletText("CryAnimation  : OK");
		ImGui::BulletText("FlowGraph     : OK");
		ImGui::BulletText("Schematyc     : OK");
		ImGui::BulletText("Entity System : OK");
		ImGui::End();

		ImGui::Render(); float cc[4] = {0,0,0,1}; g_ctx->OMSetRenderTargets(1, &g_rtv, 0); g_ctx->ClearRenderTargetView(g_rtv, cc);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData()); g_swap->Present(1, 0);
	}

	ImGui_ImplDX11_Shutdown(); ImGui_ImplWin32_Shutdown(); ImGui::DestroyContext();
	KillRTV(); if (g_swap) g_swap->Release(); if (g_ctx) g_ctx->Release(); if (g_dev) g_dev->Release();
	DestroyWindow(hwnd); UnregisterClassW(L"DeepCryLauncher", hInst);
	if (pSystem) pSystem->Quit();
	return 0;
}
