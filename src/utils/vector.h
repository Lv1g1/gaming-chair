#pragma once

struct Vector {
    Vector() noexcept;
    Vector(float x, float y, float z) noexcept;

    Vector operator-(const Vector& v) const noexcept;

    Vector operator+(const Vector& v) const noexcept;

    Vector operator*(float f) const noexcept;

    Vector& operator+=(const Vector& v) noexcept;

    Vector& operator-=(const Vector& v) noexcept;

    Vector& operator*=(float f) noexcept;

    float x, y, z;
};
