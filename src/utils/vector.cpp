#include "vector.h"

Vector::Vector() noexcept : x{ 0.0f }, y{ 0.0f }, z{ 0.0f } {}
Vector::Vector(float x, float y, float z) noexcept : x{ x }, y{ y }, z{ z } {}

Vector Vector::operator-(const Vector& v) const noexcept
{
	return Vector{ x - v.x, y - v.y, z - v.z };
}

Vector Vector::operator+(const Vector& v) const noexcept
{
	return Vector{ x + v.x, y + v.y, z + v.z };
}

Vector Vector::operator*(float f) const noexcept
{
	return Vector{ x * f, y * f, z * f };
}

Vector& Vector::operator+=(const Vector& v) noexcept
{
	x += v.x;
	y += v.y;
	z += v.z;
	return *this;
}

Vector& Vector::operator-=(const Vector& v) noexcept
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	return *this;
}

Vector& Vector::operator*=(float f) noexcept
{
	x *= f;
	y *= f;
	z *= f;
	return *this;
}