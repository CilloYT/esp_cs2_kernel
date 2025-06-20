#pragma once

#include <d3d11.h>
#include <dwmapi.h>

#include <../imgui/imgui.h>
#include <../imgui/imgui_impl_dx11.h>
#include <../imgui/imgui_impl_win32.h>
#include <dinput.h>
#include <d3dcompiler.h>



class Overlay
{
public:
	Overlay(const wchar_t* windowName);
	~Overlay();

	void toggleMenu();
	void startRender();
	void render(bool& espChecked, bool& hopChecked);
	void endRender();
	void renderLoop(auto driver, auto base);
	bool isMenuVisible();


	WNDCLASSEXW wc;
	HWND windowHandle = nullptr;

	ID3D11Device* d3dDevice = nullptr;
	ID3D11DeviceContext* d3dDeviceContext = nullptr;
	IDXGISwapChain* d3dSwapChain = nullptr;
	ID3D11RenderTargetView* d3dRenderTargetView = nullptr;
	ID3D11ShaderResourceView* myTexture = nullptr;

	unsigned int resizedWidth;
	unsigned int resizedHeight;

	bool showMenu = false;

};
