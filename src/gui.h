#pragma once

#include <Windows.h>
#include <iostream>
#include <dwmapi.h>
#include <d3d11.h>
#include <TlHelp32.h>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_dx11.h>
#include <imgui/imgui_impl_win32.h>

#include "utils/vector.h"
#include "utils/view_matrix.h"
#include "utils/projection.h"

#define Vec ImVec2

// Overlay drawing helpers.
//
// These used to be macros, which silently mapped the fourth argument of
// drawRect() onto ImGui's `rounding` parameter instead of the outline
// thickness: boxes were drawn with the configured thickness used as corner
// rounding and an always-1px border. Wrapping the calls keeps every argument in
// the right place.
inline void drawLine(const ImVec2& a, const ImVec2& b, ImU32 color, float thickness) noexcept {
	ImGui::GetBackgroundDrawList()->AddLine(a, b, color, thickness);
}

inline void drawRect(const ImVec2& min, const ImVec2& max, ImU32 color, float thickness) noexcept {
	// A thickness below one pixel produces a degenerate outline that ImGui does
	// not render at all, which looks exactly like "the box is missing".
	if (thickness < 1.0f)
		thickness = 1.0f;
	ImGui::GetBackgroundDrawList()->AddRect(min, max, color, 0.0f, ImDrawFlags_None, thickness);
}

inline void drawCircle(const ImVec2& center, float radius, ImU32 color, int segments) noexcept {
	ImGui::GetBackgroundDrawList()->AddCircle(center, radius, color, segments);
}

inline void drawText(const ImVec2& position, ImU32 color, const char* text) noexcept {
	ImGui::GetBackgroundDrawList()->AddText(position, color, text);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class GUI {
private:
	HINSTANCE instance;
	INT cmd_show;

	// Window
    WNDCLASSEXW wc;
	
    HWND hWnd;
    
    DXGI_SWAP_CHAIN_DESC sdc;

    ID3D11Device* device;
    ID3D11DeviceContext* context;
    IDXGISwapChain* swap_chain;
    ID3D11RenderTargetView* render_target_view;
    D3D_FEATURE_LEVEL feature_level;

    ID3D11Texture2D* back_buffer;

public:
	GUI(HINSTANCE hInstance, INT nCmdShow) noexcept;

	bool init() noexcept;

    void cleanup() noexcept;

	void render() noexcept;

	bool is_running() noexcept;

	void new_frame() noexcept;
};

// Size of the overlay in pixels (the coordinate space used for drawing).
void get_display_size(float& width, float& height) noexcept;

// Projects a world-space point to overlay coordinates.
bool world_to_screen(const Vector& origin, Vector& screen, const ViewMatrix& matrix) noexcept;

// Builds the on-screen box of a player from the head position; see
// projection::compute_box().
bool compute_screen_box(const Vector& head, float box_height, float width_to_height_ratio,
	const ViewMatrix& matrix, projection::ScreenBox& box) noexcept;
