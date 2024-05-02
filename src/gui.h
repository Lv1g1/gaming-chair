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

#define Vec ImVec2

constexpr ImU32 Color(int r, int g, int b, int a) noexcept {
	return IM_COL32(r, g, b, a);
}

#define drawCircle ImGui::GetBackgroundDrawList()->AddCircle
#define drawRect ImGui::GetBackgroundDrawList()->AddRect
#define drawLine ImGui::GetBackgroundDrawList()->AddLine
#define drawText ImGui::GetBackgroundDrawList()->AddText

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

bool world_to_screen(const Vector& origin, Vector& screen, const ViewMatrix& matrix) noexcept;