#pragma once


namespace esp
{
	bool metavision(HANDLE driver, auto entityList, auto localPlayerPawn, auto viewMatrix, bool espChecked)
	{
		
		if (!espChecked)
			return false;

		for (int i = 0; i < 32; i++)
		{
			uintptr_t listEntry = driver::read_memory<uintptr_t>(driver, entityList + (8 * (i & 0x7FFF) >> 9) + 16);
			if (!listEntry)
				continue;

			uintptr_t entityController = driver::read_memory<uintptr_t>(driver, listEntry + 120 * (i & 0x1FF));
			if (!entityController)
				continue;

			uintptr_t entityControllerPawn = driver::read_memory<uintptr_t>(driver, entityController + cs2_dumper::schemas::client_dll::CBasePlayerController::m_hPawn);
			if (!entityControllerPawn)
				continue;

			uintptr_t entityPawn = driver::read_memory<uintptr_t>(driver, listEntry + 120 * (entityControllerPawn & 0x1FF));
			if (!entityPawn)
				continue;

			int entityTeam = driver::read_memory<int>(driver, entityPawn + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iTeamNum);
			/*
			if (entityTeam == localTeam)
				continue;
			*/
			int health = driver::read_memory<int>(driver, entityPawn + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iHealth);
			if (health <= 0 || health > 101)
				continue;


			Vector3 entityPos = driver::read_memory<Vector3>(driver, entityPawn + cs2_dumper::schemas::client_dll::C_BasePlayerPawn::m_vOldOrigin);
			Vector3 entityHead = { entityPos.x, entityPos.y, entityPos.z + 75.f };

			Vector3 localPos = driver::read_memory<Vector3>(driver, localPlayerPawn + cs2_dumper::schemas::client_dll::C_BasePlayerPawn::m_vOldOrigin);

			Vector2 screenFeetPos;
			Vector2 screenHeadPos;


			if (!math::WorldToScreen(viewMatrix, entityHead, screenHeadPos))
				continue;


			ImGui::GetBackgroundDrawList()->AddCircleFilled({ screenHeadPos.x, screenHeadPos.y }, 5.f, ImColor(1.f, 0.f, 0.f));

			if (!math::WorldToScreen(viewMatrix, entityPos, screenFeetPos))
				continue;


			float headHeight = (screenFeetPos.y - screenHeadPos.y) / 8;
			float height = screenFeetPos.y - screenHeadPos.y;
			float width = height / 2.4f;


			ImGui::GetBackgroundDrawList()->AddRect({ screenHeadPos.x - width / 2, screenHeadPos.y }, { screenHeadPos.x - width / 2 + width, screenHeadPos.y + height }, ImColor(1.f, 0.f, 0.f), 0, 0, 1.5f);

		}
		return true;
	}
}