#include "view_matrix.h"

ViewMatrix::ViewMatrix() noexcept
{
	matrix[0][0] = 1.0f;
	matrix[1][1] = 1.0f;
	matrix[2][2] = 1.0f;
	matrix[3][3] = 1.0f;
}

float* ViewMatrix::operator[](std::size_t i) noexcept
{
	return matrix[i];
}

const float* ViewMatrix::operator[](std::size_t i) const noexcept
{
	return matrix[i];
}