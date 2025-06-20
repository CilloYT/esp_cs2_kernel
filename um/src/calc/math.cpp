#include "math.h"
#include <Windows.h>
#include <cmath>

float ScreenWidth = GetSystemMetrics(SM_CXSCREEN);
float ScreenHeight = GetSystemMetrics(SM_CYSCREEN);



float getDst(Vector3 entityPos, Vector3 localPos)
{
	return sqrtf(
		pow(entityPos.x - localPos.x, 2) +
		pow(entityPos.y - localPos.y, 2) +
		pow(entityPos.z - localPos.z, 2)
	);
}

bool math::WorldToScreen(view_matrix viewMatrix, Vector3 entityPos, Vector2& screenPos)
{

	float x = viewMatrix[0][0] * entityPos.x + viewMatrix[0][1] * entityPos.y + viewMatrix[0][2] * entityPos.z + viewMatrix[0][3];
	float y = viewMatrix[1][0] * entityPos.x + viewMatrix[1][1] * entityPos.y + viewMatrix[1][2] * entityPos.z + viewMatrix[1][3];

	float w = viewMatrix[3][0] * entityPos.x + viewMatrix[3][1] * entityPos.y + viewMatrix[3][2] * entityPos.z + viewMatrix[3][3];

	if (w < 0.01f)
		return false;
		
	float inv_w = 1.f / w;
	x *= inv_w;
	y *= inv_w;

	screenPos.x = (ScreenWidth / 2.0f) + (x * ScreenWidth / 2.0f);
	screenPos.y = (ScreenHeight / 2.0f) - (y * ScreenHeight / 2.0f);

	return true;
	
}


bool math::getRectPos(Vector2 screenPos, Vector2& rStartPos, Vector2& rEndPos, Vector3 entityPos, Vector3 localPos)
{
	float distance = getDst(entityPos, localPos);

	float scale = 100.f / distance;

	float height = scale * 50.f;
	float width = height / 2.4f;

	rStartPos.x = screenPos.x - (width / 2);
	rStartPos.y = screenPos.y;

	rEndPos.x = screenPos.x + (width / 2);
	rEndPos.y = screenPos.y + height;

	return true;
}