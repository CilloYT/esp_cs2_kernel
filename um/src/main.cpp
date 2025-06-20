#include <iostream>

#include <Windows.h>
#include <TlHelp32.h>

#include "offset/offsets.hpp"
#include "offset/client_dll.hpp"
#include "offset/buttons.hpp"

#include <../imgui/imgui.h>
#include <../imgui/imgui_impl_dx11.h>
#include <../imgui/imgui_impl_win32.h>


#include "calc/math.h"
#include "gui/gui.h"
#include "ioctl/drv.h"
#include "cheeto/bhop.h"
#include "cheeto/esp.h"


static DWORD get_process_id(const wchar_t* process_name)
{
	DWORD pID = 0;

	HANDLE snap_shot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (snap_shot == INVALID_HANDLE_VALUE)
	{
		return pID;
	}

	PROCESSENTRY32W entry = {};
	entry.dwSize = sizeof(decltype(entry));

	if (Process32FirstW(snap_shot, &entry))
	{
		if (_wcsicmp(process_name, entry.szExeFile) == 0)
		{
			pID = entry.th32ProcessID;
		}
		else 
		{
			while (Process32NextW(snap_shot, &entry))
			{
				if (_wcsicmp(process_name, entry.szExeFile) == 0)
				{
					pID = entry.th32ProcessID;
					break;
				}
			}
		}
	}

	CloseHandle(snap_shot);

	return pID;
}

static std::uintptr_t get_module_base(const DWORD pid, const wchar_t* module_name)
{
	std::uintptr_t module_base = 0;

	HANDLE snap_shot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);

	if (snap_shot == INVALID_HANDLE_VALUE)
	{
		return module_base;
	}

	MODULEENTRY32W entry = {};
	entry.dwSize = sizeof(decltype(entry));

	if (Module32FirstW(snap_shot, &entry))
	{
		if (wcsstr(module_name, entry.szModule) != nullptr)
		{
			module_base = reinterpret_cast<std::uintptr_t>(entry.modBaseAddr);
		}
		else
		{
			while (Module32NextW(snap_shot, &entry))
			{
				if (wcsstr(module_name, entry.szModule) != nullptr)
				{
					module_base = reinterpret_cast<std::uintptr_t>(entry.modBaseAddr);
					break;
				}
			}
		}
	}

	CloseHandle(snap_shot);

	return module_base;
}



void sleepovicSec(int sec)
{
	for (int i = sec; i > 0; i--)
	{
		std::cout << i << std::flush;
		Sleep(1000);
		std::cout << "\b \b" << std::flush;
	}
}

void sleepovicSecWin(int sec)
{
	for (int i = sec; i >= 1; i--)
	{
		std::cout << "\rClosing window in " << i << "s... chui kurz" << std::flush;
		Sleep(1000);
	}
	std::cout << "\n";
	// Zeile ganz löschen:
	std::cout << "\x1b[1A\x1b[2K"; // 1 Zeile hoch, löschen
	std::cout << "\x1b[1A\x1b[2K"; // 1 Zeile hoch, löschen
	std::cout << "\x1b[1A\x1b[2K"; // 1 Zeile hoch, löschen
	std::cout << "\x1b[1A\x1b[2K"; // 1 Zeile hoch, löschen
	std::cout << "\x1b[1A\x1b[2K"; // 1 Zeile hoch, löschen
}

void enableVTMode()
{
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD dwMode = 0;

	if (hOut == INVALID_HANDLE_VALUE)
		return;

	if (!GetConsoleMode(hOut, &dwMode))
		return;

	dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;

	SetConsoleMode(hOut, dwMode);
}

int main(int, char**)
{

	enableVTMode();

	DWORD pid = get_process_id(L"cs2.exe");

	
	if (pid == 0)
	{
		std::cout << "Waiting for cs2\n";
		while (pid == 0)
		{
			Sleep(10);
			pid = get_process_id(L"cs2.exe");
		}
		std::cout << "Found cs2... holup\n";
		Sleep(15000);
	}

	

	const HANDLE driver = CreateFile(
		L"\\\\.\\crazydriver",
		GENERIC_READ,
		0,
		nullptr,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		nullptr
		);

	if (driver == INVALID_HANDLE_VALUE)
	{
		std::cout << "Failed to create driver handle.\n";
		std::cin.get();
		return 1;
	}

	HWND windowovic = GetConsoleWindow();

	if (driver::attach_to_process(pid, driver))
	{

		std::cout << "Attached to process successfully\n";

		if (const std::uintptr_t base = get_module_base(pid, L"client.dll"); base != 0)
		{
			std::cout << "Client found. Base: " << base << "\n\n";
			sleepovicSecWin(3);
			std::cout << "\n\n";
			ShowWindow(windowovic, SW_HIDE);


			Overlay overlay(L"jovanomel");
			
			bool espChecked = false;
			bool hopChecked = false;


			MSG msg;
			ZeroMemory(&msg, sizeof(msg));

			while (msg.message != WM_QUIT)
			{
				if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
				{
					if (msg.message == WM_HOTKEY)
					{
						if (msg.wParam == 1)
						{
							overlay.toggleMenu();
						}
					}
					TranslateMessage(&msg);
					DispatchMessageW(&msg);
				}

				if (GetAsyncKeyState(VK_END) && 0x1)
				{
					msg.message = WM_QUIT;
				}
					
				overlay.startRender();
				overlay.render(espChecked, hopChecked);

				//start ESP or whatever
					
				const auto localPlayerPawn = driver::read_memory<std::uintptr_t>(driver, base + cs2_dumper::offsets::client_dll::dwLocalPlayerPawn);
				view_matrix viewMatrix = driver::read_memory<view_matrix>(driver, base + cs2_dumper::offsets::client_dll::dwViewMatrix);
				uintptr_t entityList = driver::read_memory<uintptr_t>(driver, base + cs2_dumper::offsets::client_dll::dwEntityList);
				int localTeam = driver::read_memory<int>(driver, base + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iTeamNum);

				//hopovic func
				bhop::hopovic(driver, localPlayerPawn, base, hopChecked);

				//metavision func
				esp::metavision(driver, entityList, localPlayerPawn, viewMatrix, espChecked);

				//ending rendering
				overlay.endRender();
				overlay.d3dSwapChain->Present(1, 0);

			}
		}
	}

	ShowWindow(windowovic, SW_SHOW);

	std::cout << "Stopped.\n";

	CloseHandle(driver);

	sleepovicSec(3);

	return 0;
}







