#pragma once

#include "vector.h"

namespace bone_id
{
	constexpr auto waist = 0;
	constexpr auto neck = 5;
	constexpr auto head = 6;
	constexpr auto left_shoulder = 8;
	constexpr auto left_elbow = 9;
	constexpr auto left_hand = 11;
	constexpr auto right_shoulder = 13;
	constexpr auto right_elbow = 14;
	constexpr auto right_hand = 16;
	constexpr auto left_knee = 23;
	constexpr auto left_foot = 24;
	constexpr auto right_knee = 26;
	constexpr auto right_foot = 27;
}

constexpr int bone_count = 13;
constexpr int bone_ids[] = {
	bone_id::waist,
	bone_id::neck,
	bone_id::head,
	bone_id::left_shoulder,
	bone_id::left_elbow,
	bone_id::left_hand,
	bone_id::right_shoulder,
	bone_id::right_elbow,
	bone_id::right_hand,
	bone_id::left_knee,
	bone_id::left_foot,
	bone_id::right_knee,
	bone_id::right_foot
};


typedef struct {
	int bone1;
	int bone2;
} BonePair;

constexpr BonePair bone_pairs[] = {
	{ bone_id::waist, bone_id::neck },
	{ bone_id::neck, bone_id::head },
	{ bone_id::left_shoulder, bone_id::left_elbow },
	{ bone_id::left_elbow, bone_id::left_hand },
	{ bone_id::right_shoulder, bone_id::right_elbow },
	{ bone_id::right_elbow, bone_id::right_hand },
	{ bone_id::waist, bone_id::left_knee },
	{ bone_id::left_knee, bone_id::left_foot },
	{ bone_id::waist, bone_id::right_knee },
	{ bone_id::right_knee, bone_id::right_foot }
};

struct BoneMatrix
{
	char bones[27 * 32 + 16];

public:
	Vector& operator[](std::size_t i) noexcept
	{
		return *reinterpret_cast<Vector*>(&bones[i * 32]);
	}

	const Vector& operator[](std::size_t i) const noexcept
	{
		return *reinterpret_cast<const Vector*>(&bones[i * 32]);
	}
};