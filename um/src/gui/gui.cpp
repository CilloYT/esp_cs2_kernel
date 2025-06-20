#include "gui.h"
#include <exception>
#include <iostream>

#include "fonts.h"
#include "../offset/offsets.hpp"
#include "../offset/client_dll.hpp"
#include "../offset/buttons.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


LRESULT WINAPI WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	Overlay* overlay = reinterpret_cast<Overlay*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
		return true;

	switch (msg) 
	{
	case WM_NCHITTEST:
		if (overlay && overlay->isMenuVisible())
			return HTCLIENT;
		break;

	case WM_SYSCOMMAND:
		if ((wParam & 0xFFF0) == SC_KEYMENU)
			return 0;
		break;

	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	}
	return ::DefWindowProcW(hwnd, msg, wParam, lParam);


}

Overlay::Overlay(const wchar_t* windowName)
{
	wc = { sizeof(WNDCLASSEXW) };
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = windowName;
	wc.hInstance = GetModuleHandle(nullptr);
	RegisterClassExW(&wc);

	windowHandle = CreateWindowExW
	(WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_TOOLWINDOW, windowName, windowName,
		WS_POPUP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), nullptr, nullptr, wc.hInstance, nullptr);

	SetLayeredWindowAttributes(windowHandle, RGB(0, 0, 0), 255, LWA_ALPHA | LWA_COLORKEY);
	SetWindowPos(windowHandle, HWND_TOPMOST, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE);

	SetWindowLongPtr(windowHandle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

	RegisterHotKey(NULL, 1, MOD_NOREPEAT, VK_INSERT);

	DXGI_SWAP_CHAIN_DESC sd = {};
	sd.BufferDesc.Width = GetSystemMetrics(SM_CXSCREEN);
	sd.BufferDesc.Height = GetSystemMetrics(SM_CYSCREEN);
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = windowHandle;
	sd.SampleDesc.Count = 1;
	sd.Windowed = TRUE;
	sd.BufferCount = 1;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	D3D_FEATURE_LEVEL featureLvl;
	const D3D_FEATURE_LEVEL featureLvlArr[1] = {D3D_FEATURE_LEVEL_11_0};

	HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT, featureLvlArr, 1,
		D3D11_SDK_VERSION, &sd, &d3dSwapChain, &d3dDevice, &featureLvl, &d3dDeviceContext);


	if (FAILED(hr))
	{
		throw std::exception("Failed to create device and swapchain!");
	}


	//load don image
	unsigned char* rgbaData = stbi_load_from_memory(imageData, sizeof(imageData), &imageWidth, &imageHeight, &channels, 4);

	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = imageWidth;
	desc.Height = imageHeight;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA subResource = {};
	subResource.pSysMem = rgbaData;
	subResource.SysMemPitch = imageWidth * 4;

	ID3D11Texture2D* pTexture = nullptr;
	HRESULT hrPic = d3dDevice->CreateTexture2D(&desc, &subResource, &pTexture);

	if (SUCCEEDED(hrPic))
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = desc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;

		hr = d3dDevice->CreateShaderResourceView(pTexture, &srvDesc, &myTexture);
		pTexture->Release();
	}

	stbi_image_free(rgbaData);
	//image loaded

	ID3D11Texture2D* pBackBuffer = nullptr;
	d3dSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	d3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &d3dRenderTargetView);
	pBackBuffer->Release();

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
	io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(windowHandle);
	ImGui_ImplDX11_Init(d3dDevice, d3dDeviceContext);

	ShowWindow(windowHandle, SW_SHOW);

}


Overlay::~Overlay()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	d3dRenderTargetView->Release();
	d3dSwapChain->Release();
	d3dDeviceContext->Release();
	d3dDevice->Release();

	DestroyWindow(windowHandle);
	UnregisterClass(wc.lpszClassName, wc.hInstance);


}

void Overlay::toggleMenu()
{
	showMenu = !showMenu;

	LONG style = GetWindowLong(windowHandle, GWL_EXSTYLE);
	
	if (showMenu)
		style &= ~WS_EX_TRANSPARENT;

	else
		style |= WS_EX_TRANSPARENT;

	SetWindowLongW(windowHandle, GWL_EXSTYLE, style);
	SetLayeredWindowAttributes(windowHandle, RGB(0, 0, 0), 255, LWA_ALPHA | LWA_COLORKEY);


}

void Overlay::startRender()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	float clearColor[4] = { 0, 0, 0, 0 };
	d3dDeviceContext->ClearRenderTargetView(d3dRenderTargetView, clearColor);


}



void Overlay::render(bool& espChecked, bool& hopChecked)
{

	if (showMenu)
	{

		ImGui::SetNextWindowSize(ImVec2(670, 300));
		ImGui::Begin("UNDETEKOVIC", &showMenu);
		ImGui::Image(reinterpret_cast<ImTextureID>(myTexture), ImVec2(imageWidth, imageHeight));

		ImGui::Checkbox("Metavision", &espChecked);
		ImGui::Checkbox("Hopovic", &hopChecked);

		ImGui::End();
	}
		

}



bool Overlay::isMenuVisible()
{
	return showMenu;
}


void Overlay::endRender()
{
	ImGui::Render();
	d3dDeviceContext->OMSetRenderTargets(1, &d3dRenderTargetView, nullptr);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void Overlay::renderLoop(auto driver, auto base)
{
	

}
