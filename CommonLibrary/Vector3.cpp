#include "Vector3.h"

math::Vector3::Vector3()
	: mX{}
	, mY{}
	, mZ{}
{
}

math::Vector3::Vector3(float x, float y, float z)
	: mX{ x }
	, mY{ y }
	, mZ{ z }
{
}

math::Vector3::Vector3(const Vector3& ref)
	: mX{ ref.mX }
	, mY{ ref.mY }
	, mZ{ ref.mZ }
{
}

math::Vector3::~Vector3()
{
}

math::Vector3& math::Vector3::operator=(const Vector3& ref)
{
	mX = ref.mX;
	mY = ref.mY;
	mZ = ref.mZ;

	return *this;
}

float math::Vector3::GetX()
{
	return mX;
}

float math::Vector3::GetY()
{
	return mY;
}

float math::Vector3::GetZ()
{
	return mZ;
}
