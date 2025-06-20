#pragma once


namespace bhop
{
	bool hopovic(HANDLE driver, auto localPlayerPawn, auto base, bool hopChecked)
	{

		if (!hopChecked)
			return false;

		if (localPlayerPawn == 0)
			return false;

		const auto flags = driver::read_memory<std::uint32_t>(driver, localPlayerPawn + cs2_dumper::schemas::client_dll::C_BaseEntity::m_fFlags);

		const bool in_air = flags & (1 << 0);
		const bool space_pressed = GetAsyncKeyState(VK_SPACE);
		const auto force_jump = driver::read_memory<DWORD>(driver, base + cs2_dumper::buttons::jump);


		if (space_pressed && in_air)
		{
			Sleep(8);
			driver::write_memory(driver, base + cs2_dumper::buttons::jump, 65537);
		}
		else if (space_pressed && !in_air)
		{
			driver::write_memory(driver, base + cs2_dumper::buttons::jump, 256);
		}
		else if (!space_pressed && force_jump == 65537)
		{
			driver::write_memory(driver, base + cs2_dumper::buttons::jump, 256);
		}

		return true;
	}
}