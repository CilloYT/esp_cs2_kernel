#pragma once
#include <array>

struct Vector2
{
	float x, y;
};

struct Vector3
{

	constexpr Vector3(
		const float x = 0.f,
		const float y = 0.f,
		const float z = 0.f
	) noexcept:
		x(x), y(y), z(z) { }


	constexpr const Vector3& operator+(const Vector3& other) const noexcept
	{
		return Vector3(x + other.x, y + other.y, z + other.z);
	}

	constexpr const Vector3& operator*(const float factor) const noexcept
	{
		return Vector3(x * factor, y * factor, z * factor);
	}

	float x, y, z;
};

struct view_matrix
{

	float* operator[](int index)
	{
		return matrix[index];
	}
	float matrix[4][4];
};


namespace math
{
	bool WorldToScreen(view_matrix viewMatrix, Vector3 entityPos, Vector2& screenPos);
	bool getRectPos(Vector2 screenPos, Vector2& rStartPos, Vector2& rEndPos, Vector3 entityPos, Vector3 localPos);

}

	





