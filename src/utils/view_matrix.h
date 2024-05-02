#pragma once

#include <iostream>
#include "vector.h"

struct ViewMatrix {
    ViewMatrix() noexcept;

    float* operator[](std::size_t i) noexcept;

    const float* operator[](std::size_t i) const noexcept;

    float matrix[4][4];
};