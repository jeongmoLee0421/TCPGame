#pragma once

// 2023 08 02 이정모 home

// x, y, z 3개 성분을 나타내기 위한 class

namespace math
{
	class Vector3
	{
	public:
		Vector3();
		Vector3(float x, float y, float z);
		Vector3(const Vector3& ref);
		~Vector3();

	public:
		Vector3& operator=(const Vector3& ref);

	public:
		float GetX();
		float GetY();
		float GetZ();

	private:
		float mX;
		float mY;
		float mZ;
	};
}