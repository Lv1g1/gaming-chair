#include "gui.h"
#include "settings.h"

// GUI class implementation
constexpr D3D_FEATURE_LEVEL feature_levels[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };

GUI::GUI(HINSTANCE hInstance, INT nCmdShow) noexcept
{
	instance = hInstance;
	cmd_show = nCmdShow;

	ZeroMemory(&wc, sizeof(wc));
	ZeroMemory(&sdc, sizeof(sdc));

	device = nullptr;
	context = nullptr;
	swap_chain = nullptr;
	render_target_view = nullptr;
	feature_level = {};

	back_buffer = nullptr;
	hWnd = nullptr;
}

bool GUI::init() noexcept
{
	// Create window
	wc = { sizeof(WNDCLASSEXW), CS_HREDRAW | CS_VREDRAW, WndProc, 0L, 0L, instance, NULL, NULL, NULL, NULL,  L"Overlay", NULL };
	RegisterClassExW(&wc);

	hWnd = CreateWindowExW(WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST, wc.lpszClassName, L"Overlay", WS_POPUP, 0, 0, settings->DISPLAY_WIDTH, settings->DISPLAY_HEIGHT, nullptr, nullptr, instance, nullptr);

	if (!hWnd) {
		return false;
	}

	SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), BYTE(255), LWA_ALPHA);

	{
		RECT client_area{};
		if (!GetClientRect(hWnd, &client_area)) {
			return false;
		}

		RECT window_area{};
		if (!GetWindowRect(hWnd, &window_area)) {
			return false;
		}

		POINT diff{};
		if (!ClientToScreen(hWnd, &diff)) {
			return false;
		}

		const MARGINS margins{
			window_area.left + (diff.x - window_area.left),
			window_area.top + (diff.y - window_area.top),
			window_area.right,
			window_area.bottom
		};

		DwmExtendFrameIntoClientArea(hWnd, &margins);
	}

	// Create device and swap chain
	sdc = {
		.BufferDesc = {
			.RefreshRate = { 60, 1 },
			.Format = DXGI_FORMAT_R8G8B8A8_UNORM
		},
		.SampleDesc = { 1U, 0U },
		.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
		.BufferCount = 2U,
		.OutputWindow = hWnd,
		.Windowed = TRUE,
		.SwapEffect = DXGI_SWAP_EFFECT_DISCARD,
		.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
	};

	D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0U, feature_levels, 2U, D3D11_SDK_VERSION, &sdc, &swap_chain, &device, &feature_level, &context);

	D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0U, feature_levels, 2U, D3D11_SDK_VERSION, &sdc, &swap_chain, &device, &feature_level, &context);

	swap_chain->GetBuffer(0U, IID_PPV_ARGS(&back_buffer));

	if (back_buffer) {
		device->CreateRenderTargetView(back_buffer, nullptr, &render_target_view);
		back_buffer->Release();
	}
	else {
		return false;
	}

	// Show the window
	ShowWindow(hWnd, cmd_show);
	UpdateWindow(hWnd);

	// Setup Dear ImGui context
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer bindings
	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX11_Init(device, context);

	return true;
}

void GUI::cleanup() noexcept
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();

	ImGui::DestroyContext();

	if (render_target_view) {
		render_target_view->Release();
	}

	if (swap_chain) {
		swap_chain->Release();
	}

	if (context) {
		context->Release();
	}

	if (device) {
		device->Release();
	}

	if (hWnd) {
		DestroyWindow(hWnd);
	}

	UnregisterClassW(wc.lpszClassName, instance);
}

void GUI::render() noexcept
{
	ImGui::Render();

	constexpr float clear_color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	context->OMSetRenderTargets(1, &render_target_view, nullptr);
	context->ClearRenderTargetView(render_target_view, clear_color);

	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	swap_chain->Present(1, 0);
}

bool GUI::is_running() noexcept
{
	MSG msg{};
	while (PeekMessageW(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	return msg.message != WM_QUIT;
}

void GUI::new_frame() noexcept
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

// WndProc implementation
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return 0L;

	if (msg == WM_DESTROY) {
		PostQuitMessage(0);
		return 0L;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

// world_to_screen function implementation
bool world_to_screen(const Vector& origin, Vector& screen, const ViewMatrix& matrix) noexcept
{
	auto w = matrix[3][0] * origin.x + matrix[3][1] * origin.y + matrix[3][2] * origin.z + matrix[3][3];
	if (w < 0.001f)
		return false;

	const float x = matrix[0][0] * origin.x + matrix[0][1] * origin.y + matrix[0][2] * origin.z + matrix[0][3];
	const float y = matrix[1][0] * origin.x + matrix[1][1] * origin.y + matrix[1][2] * origin.z + matrix[1][3];

	w = 1.0f / w;
	float nx = x * w;
	float ny = y * w;

	const ImVec2 size = ImGui::GetIO().DisplaySize;

	screen.x = (size.x / 2.0f * nx) + (nx + size.x / 2.0f);
	screen.y = -(size.y / 2.0f * ny) + (ny + size.y / 2.0f);

	return true;
}