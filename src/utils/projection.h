#pragma once

// Screen-space projection helpers.
//
// These are pure math functions so they can be tested without a DirectX device
// or ImGui context.

#include <cmath>

#include "vector.h"
#include "view_matrix.h"

namespace projection {

// Projects a world-space point through a CS2 view-projection matrix.
//
// The matrix is stored the way the game exposes it: the fourth row holds the
// clip-space W row, so the perspective divide is `w` from row 3.
//
// Returns false when the point is behind the camera or when the result is not a
// finite number: callers must check the return value instead of drawing with
// the untouched output.
inline bool world_to_screen(const Vector& world, Vector& screen, const ViewMatrix& matrix,
                            float display_width, float display_height) noexcept {
    const float w = matrix[3][0] * world.x + matrix[3][1] * world.y
                  + matrix[3][2] * world.z + matrix[3][3];

    // `!(w > eps)` also rejects NaN.
    if (!(w > 0.001f))
        return false;

    const float inv_w = 1.0f / w;

    const float x = (matrix[0][0] * world.x + matrix[0][1] * world.y
                   + matrix[0][2] * world.z + matrix[0][3]) * inv_w;
    const float y = (matrix[1][0] * world.x + matrix[1][1] * world.y
                   + matrix[1][2] * world.z + matrix[1][3]) * inv_w;

    if (!std::isfinite(x) || !std::isfinite(y))
        return false;

    // NDC [-1, 1] -> screen pixels. The Y axis is flipped because screen space
    // grows downwards.
    screen.x = (display_width * 0.5f) * x + display_width * 0.5f;
    screen.y = -(display_height * 0.5f) * y + display_height * 0.5f;

    return true;
}

struct ScreenBox {
    float left = 0.0f;
    float top = 0.0f;
    float right = 0.0f;
    float bottom = 0.0f;

    float width() const noexcept { return right - left; }
    float height() const noexcept { return bottom - top; }

    bool valid() const noexcept {
        return std::isfinite(left) && std::isfinite(top) && std::isfinite(right)
            && std::isfinite(bottom) && width() >= 1.0f && height() >= 1.0f;
    }
};

// Builds the on-screen bounding box of a player, anchored on the head position.
//
// The box extends `box_height` world units downwards from `head` (CS2 uses +Z as
// "up"), so it always lines up with whatever the caller draws at the head
// position - typically a bone read from the same skeleton. This is deliberate:
// anchoring the box on a pawn field such as m_vOldOrigin makes every box
// disappear (or jump elsewhere) as soon as that offset is stale after a game
// update, while the skeleton keeps working.
//
// Returns false when either point cannot be projected or when the resulting
// rectangle would be degenerate, so nothing is drawn for enemies that are behind
// the camera or completely outside of the screen.
inline bool compute_box(const Vector& head, float box_height, float width_to_height_ratio,
                        const ViewMatrix& matrix, float display_width, float display_height,
                        ScreenBox& box) noexcept {
    if (!(box_height > 0.0f) || !std::isfinite(box_height))
        return false;
    if (!(width_to_height_ratio > 0.0f) || !std::isfinite(width_to_height_ratio))
        return false;

    const Vector feet = head - Vector{ 0.0f, 0.0f, box_height };

    Vector head_2d;
    Vector feet_2d;

    if (!world_to_screen(head, head_2d, matrix, display_width, display_height))
        return false;
    if (!world_to_screen(feet, feet_2d, matrix, display_width, display_height))
        return false;

    // The head is above the feet on screen, but use min/max so a different Y
    // convention cannot produce an inverted (and therefore invisible) box.
    const float top = std::fmin(head_2d.y, feet_2d.y);
    const float bottom = std::fmax(head_2d.y, feet_2d.y);
    const float height = bottom - top;
    if (!(height >= 1.0f) || !std::isfinite(height))
        return false;

    const float half_width = height * width_to_height_ratio * 0.5f;
    if (!(half_width > 0.0f) || !std::isfinite(half_width))
        return false;

    const float center_x = (head_2d.x + feet_2d.x) * 0.5f;
    box.left = center_x - half_width;
    box.right = center_x + half_width;
    box.top = top;
    box.bottom = bottom;

    if (!box.valid())
        return false;

    // Reject boxes that are completely outside of the overlay: they are a sign
    // of garbage input and would never be visible anyway.
    if (box.right < 0.0f || box.left > display_width || box.bottom < 0.0f || box.top > display_height)
        return false;

    return true;
}

} // namespace projection
