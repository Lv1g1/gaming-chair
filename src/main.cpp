#include <Windows.h>
#include <iostream>
#include <string>

#include "gui.h"

#include "memory/process.h"
#include "memory/offsets.h"

#include "utils/vector.h"
#include "utils/view_matrix.h"
#include "utils/bone_matrix.h"

#include "settings.h"

constexpr bool DEBUG = false;

#define PRINT_DEBUG(format, ...) if constexpr (DEBUG) { \
	char output[256]; \
	sprintf_s(output, format, __VA_ARGS__); \
	OutputDebugStringA(output); \
}

INT APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, INT nCmdShow) {
	// Create the GUI
	GUI gui{ hInstance, nCmdShow };

	if (!gui.init()) {
		std::cerr << "Failed to initialize GUI" << std::endl;
		return EXIT_FAILURE;
	}

	// Attach to the process
	Process process = Process();
	if (!process.attach(L"cs2.exe")) {
		std::cerr << "Failed to attach to process" << std::endl;
		return EXIT_FAILURE;
	}

	// Get the client.dll base address
	uintptr_t client_base_address = process.get_module_base_address(L"client.dll");
	if (!client_base_address) {
		std::cerr << "Failed to get client.dll base address" << std::endl;
		return EXIT_FAILURE;
	}

	PRINT_DEBUG("Client base address: 0x%llx\n", client_base_address);

	// main application loop
	bool running = true;
	while (running) {
		if (!gui.is_running()) {
			break;
		}

		gui.new_frame();

		//Cheat logic here
		auto diplay_center_x = DISPLAY_WIDTH / 2;
		auto diplay_center_y = DISPLAY_HEIGHT / 2;

		auto local_player = process.read_memory<uintptr_t>(client_base_address + offsets::dwLocalPlayerPawn);
		PRINT_DEBUG("Local player: 0x%llx\n", local_player);
		
		if (local_player) {
			Vector nearest_head_position_2d;
			auto min_distance = 999999.0f;

			auto local_player_team = process.read_memory<int>(local_player + offsets::m_iTeamNum);
			PRINT_DEBUG("Local player team: %d\n", local_player_team);

			auto entity_list = process.read_memory<uintptr_t>(client_base_address + offsets::dwEntityList);

			auto list_entry = process.read_memory<uintptr_t>(entity_list + 0x10);

			auto view_matrix = process.read_memory<ViewMatrix>(client_base_address + offsets::dwViewMatrix);

			for (int i = 0; i < 64; i++) {

				auto current_controller = process.read_memory<uintptr_t>(list_entry + i * 0x78);
				if (current_controller == 0) {
					continue;
				}

				auto pawn_handle = process.read_memory<uintptr_t>(current_controller + offsets::m_hPlayerPawn);
				if (pawn_handle == 0) {
					continue;
				}

				auto list_entry_2 = process.read_memory<uintptr_t>(entity_list + 0x8 * ((pawn_handle & 0x7FFF) >> 9) + 0x10);
				auto current_pawn = process.read_memory<uintptr_t>(list_entry_2 + 0x78 * (pawn_handle & 0x1FF));

				const auto dormant = process.read_memory<bool>(current_pawn + offsets::m_bDormant);
				PRINT_DEBUG("Dormant: %d\n", dormant);
				if (dormant) {
					continue;
				}

				auto health = process.read_memory<int>(current_pawn + offsets::m_iHealth);
				PRINT_DEBUG("Health: %d\n", health);
				if (health <= 0) {
					continue;
				}

				int life_state = process.read_memory<int>(current_pawn + offsets::m_lifeState);
				PRINT_DEBUG("Life state: %d\n", life_state);
				if (life_state != 256) {
					continue;
				}

				auto team_num = process.read_memory<int>(current_pawn + offsets::m_iTeamNum);
				PRINT_DEBUG("Team num: %d\n", team_num);
				if (team_num == local_player_team) {
					continue;
				}

				auto spotted = process.read_memory<bool>(current_pawn + offsets::m_entitySpottedState + offsets::m_bSpotted);
				PRINT_DEBUG("Spotted: %d\n", spotted);
				
				auto entity_origin = process.read_memory<Vector>(current_pawn + offsets::m_vOldOrigin);
				auto view_offset = process.read_memory<Vector>(current_pawn + offsets::m_vecViewOffset);

				auto node_scene = process.read_memory<uintptr_t>(current_pawn + offsets::m_pGameSceneNode);
				auto bone_matrix_ptr = process.read_memory<uintptr_t>(node_scene + offsets::m_modelState + 0x80);
				auto bone_matrix = process.read_memory<BoneMatrix>(bone_matrix_ptr);
				
				// x axis: + move right, - move left
				// y axis: + move down, - move up
				Vector top;
				Vector bottom;
				world_to_screen(entity_origin + Vector{ 0, 0, WALLHACK_BOX_HEIGHT }, top, view_matrix);
				world_to_screen(entity_origin, bottom, view_matrix);

				auto height = top.y - bottom.y;
				auto half_width = -height * WALLHACK_BOX_HEIGHT_WIDTH_RATIO / 2; // y axis is inverted

				// Draw the box
				if constexpr (WALLHACK_DRAW_BOX) {
					if (!spotted || WALLHACK_DRAW_BOX_WHEN_SPOTTED) {
						drawRect(
							Vec(bottom.x - half_width, bottom.y),
							Vec(top.x + half_width, top.y),
							Color(WALLHACK_BOX_COLOR),
							WALLHACK_BOX_THICKNESS
						);
					}
				}

				// Draw the health bar
				if constexpr (WALLHACK_DRAW_HEALTH_BAR) {
					if (!spotted || WALLHACK_DRAW_HEALTH_BAR_WHEN_SPOTTED) {
						uint8_t red = 255 - (health * 2.55f) * 0.5;
						uint8_t green = health * 2.55f;

						float health_ratio = health / 100.0f;

						auto health_top_x = bottom.x - half_width;
						auto health_top_y = bottom.y + health_ratio * height;

						drawLine(
							Vec(bottom.x - half_width, bottom.y),
							Vec(health_top_x, health_top_y),
							Color(red, green, 0, 255),
							WALLHACK_HEALTH_BAR_THICKNESS
						);
					}
				}

				// Write the health
				if constexpr (WALLHACK_DRAW_TEXT) {
					if (!spotted || WALLHACK_DRAW_TEXT_WHEN_SPOTTED) {
						drawText(
							Vec(top.x, top.y - 10),
							Color(WALLHACK_TEXT_COLOR),
							std::to_string(health).c_str()
						);
					}
				}

				// Draw the bones
				if constexpr (WALLHACK_DRAW_BONES) {
					if (!spotted || WALLHACK_DRAW_BONES_WHEN_SPOTTED) {
						for (BonePair pair : bone_pairs) {
							Vector bone_position_3d_1 = bone_matrix[pair.bone1];
							Vector bone_position_3d_2 = bone_matrix[pair.bone2];

							Vector bone_position_2d_1;
							Vector bone_position_2d_2;

							world_to_screen(bone_position_3d_1, bone_position_2d_1, view_matrix);
							world_to_screen(bone_position_3d_2, bone_position_2d_2, view_matrix);

							drawLine(
								Vec(bone_position_2d_1.x, bone_position_2d_1.y),
								Vec(bone_position_2d_2.x, bone_position_2d_2.y),
								Color(WALLHACK_BONES_COLOR),
								WALLHACK_BONES_THICKNESS
							);
						}
					}
				}

				// Calculate the head position
				Vector head_position_3d = bone_matrix[bone_id::head];
				Vector head_position_2d;
				world_to_screen(head_position_3d, head_position_2d, view_matrix);

				if constexpr (WALLHACK_DRAW_HEAD) {
					if (!spotted || WALLHACK_DRAW_HEAD_WHEN_SPOTTED) {
						// Draw the Head
						auto head_radius = half_width * 0.5f;

						drawCircle(
							Vec(head_position_2d.x, head_position_2d.y),
							head_radius,
							Color(WALLHACK_HEAD_COLOR),
							32
						);
					}
				}	

				if constexpr (AIMBOT_ENABLED) {
					if (spotted || AIMBOT_ENABLED_WHEN_NOT_SPOTTED) {
						// Check if the head is the nearest
						auto distance = sqrt(pow(head_position_2d.x - DISPLAY_WIDTH / 2, 2) + pow(head_position_2d.y - DISPLAY_HEIGHT / 2, 2));
						if (distance < min_distance) {
							min_distance = distance;
							nearest_head_position_2d = head_position_2d;
						}
					}
				}
			}

			if constexpr (AIMBOT_ENABLED) {
				// aimbot
				auto delta_x = diplay_center_x - nearest_head_position_2d.x;
				auto delta_y = diplay_center_y - nearest_head_position_2d.y;

				auto distance = sqrt(delta_x * delta_x + delta_y * delta_y);
				if (distance < AIMBOT_FOV) {
					mouse_event(MOUSEEVENTF_MOVE, -delta_x, -delta_y, 0, 0);
				}

				if (AIMBOT_DRAW_FOV) {
					drawCircle(
						Vec(diplay_center_x, diplay_center_y),
						AIMBOT_FOV,
						Color(AIMBOT_DRAW_FOV_COLOR),
						32);
				}
			}
		}

		gui.render();
	}

	gui.cleanup();

	return EXIT_SUCCESS;
}