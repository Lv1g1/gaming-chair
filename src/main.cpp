#include "server/server.h" // Must be included first, as it includes WinSock2.h which conflicts with Windows.h

#include <Windows.h>
#include <cfloat>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>

#include "gui.h"

#include "memory/process.h"
#include "memory/offsets.h"

#include "utils/vector.h"
#include "utils/view_matrix.h"
#include "utils/bone_matrix.h"
#include "utils/debug.h" // For debugging

#include "settings.h"

namespace {

// Beep() blocks the calling thread for the whole duration of the tone: playing
// it inline froze the overlay (and the aimbot) for up to 200 ms per alert, which
// is by far the biggest source of stutter. Play it on a detached thread instead.
void play_beep_async(DWORD frequency, DWORD duration) noexcept {
	try {
		std::thread([frequency, duration]() { Beep(frequency, duration); }).detach();
	}
	catch (...) {
		// Thread creation failed: the alert is not worth terminating for.
	}
}

} // namespace

INT APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, INT nCmdShow) {
	init_console();

	load_settings();

	// Resolve memory offsets from local/remote JSON sources. Falls back to the
	// compiled-in values for anything that cannot be resolved.
	offsets::resolve();
	
	Server server = Server();

	// Run thread for server
	std::thread server_thread([&server]() {
	 		server.start(settings->SERVER_PORT);
		}
	);

	// Create the GUI
	GUI gui{ hInstance, nCmdShow };

	if (!gui.init()) {
		PRINT_STRING_DEBUG("Failed to initialize GUI\n");
		MessageBoxA(nullptr, "Failed to initialize the DirectX11 overlay.", "gaming chair", MB_ICONERROR | MB_OK);
		server.stop();
		server_thread.join();
		return EXIT_FAILURE;
	}

	// Attach to the process
	Process process = Process();
	if (!process.attach(L"cs2.exe")) {
		PRINT_STRING_DEBUG("Failed to attach to process\n");
		MessageBoxA(nullptr, "Failed to attach to cs2.exe.\n\nMake sure Counter-Strike 2 is running, then try running this program as administrator.", "gaming chair", MB_ICONERROR | MB_OK);
		gui.cleanup();
		server.stop();
		server_thread.join();
		return EXIT_FAILURE;
	}

	// Get the client.dll base address
	uintptr_t client_base_address = process.get_module_base_address(L"client.dll");
	if (!client_base_address) {
		PRINT_STRING_DEBUG("Failed to get client.dll base address\n");
		MessageBoxA(nullptr, "Failed to find the client.dll module in cs2.exe.", "gaming chair", MB_ICONERROR | MB_OK);
		gui.cleanup();
		server.stop();
		server_thread.join();
		return EXIT_FAILURE;
	}

	std::cout << "Attached to cs2.exe\n";
	std::cout << "client.dll base address: 0x" << std::hex << client_base_address << std::dec << "\n";

	// Runtime cheat state
	bool wallhack_enabled = true;
	bool wallhack_key_was_down = false;
	ULONGLONG proximity_last_alert[64] = {};
	int boxes_drawn_last_frame = -1;
	ULONGLONG boxes_log_last_ms = 0;

	// Entity list layout. The real values are runtime offsets (offsets.json):
	// kMaxEntityListEntrySize is only an upper bound used to size the stack
	// buffer that receives the controller block.
	constexpr uintptr_t kMaxEntityListEntrySize = 0x80;

	// Number of player slots (same limit the loop below has always used).
	constexpr int kMaxEntitySlots = 64;

	// A bogus value coming from offsets.json would silently produce garbage reads.
	if (offsets::entityListEntrySize == 0 || offsets::entityListEntrySize > kMaxEntityListEntrySize) {
		std::cout << "[esp] entityListEntrySize out of range (" << offsets::entityListEntrySize
			<< "), using 0x" << std::hex << kMaxEntityListEntrySize << std::dec << "\n";
	}

	// main application loop
	bool running = true;
	while (running) {
		//Check if the settings have been changed and update them
		refresh_settings();

		if (!gui.is_running()) {
			break;
		}

		gui.new_frame();

		const auto frame_start = std::chrono::steady_clock::now();

		/*
		// Test drawing
		//draw circle at the center of the screen
		auto diplay_center_x = settings->DISPLAY_WIDTH / 2;
		auto diplay_center_y = settings->DISPLAY_HEIGHT / 2;
		if (settings->AIMBOT_DRAW_FOV) {
			drawCircle(
				Vec(diplay_center_x, diplay_center_y),
				settings->AIMBOT_FOV,
				AIMBOT_FOV_COLOR,
				32
			);
		}*/


		//Cheat logic here
		const float diplay_center_x = settings->DISPLAY_WIDTH * 0.5f;
		const float diplay_center_y = settings->DISPLAY_HEIGHT * 0.5f;

		// Wallhack toggle (edge-triggered key press)
		const bool wallhack_key_down = (GetAsyncKeyState(settings->WALLHACK_TOGGLE_KEY) & 0x8000) != 0;
		if (wallhack_key_down && !wallhack_key_was_down) {
			wallhack_enabled = !wallhack_enabled;
			std::cout << "[esp] wallhack " << (wallhack_enabled ? "ON" : "OFF") << "\n";
			play_beep_async(800, 100);
		}
		wallhack_key_was_down = wallhack_key_down;

		// Proximity alert circle (bigger than the aimbot FOV circle)
		if (settings->PROXIMITY_ALERT_ENABLED && settings->PROXIMITY_ALERT_DRAW) {
			drawCircle(
				Vec(diplay_center_x, diplay_center_y),
				settings->PROXIMITY_ALERT_RADIUS,
				PROXIMITY_ALERT_COLOR,
				64
			);
		}

		auto local_player = process.read_memory<uintptr_t>(client_base_address + offsets::dwLocalPlayerPawn);
		PRINTF_DEBUG("Local player: 0x%llx\n", local_player);
		
		if (local_player) {
			Vector nearest_head_position_2d;
			float min_distance_sq = FLT_MAX;
			bool has_target = false;

			// m_iTeamNum is a single byte; reading it as an int also pulled in the
			// neighbouring field, which made the team comparison unreliable.
			const auto local_player_team = process.read_memory<uint8_t>(local_player + offsets::m_iTeamNum);
			PRINTF_DEBUG("Local player team: %d\n", local_player_team);

			auto entity_list = process.read_memory<uintptr_t>(client_base_address + offsets::dwEntityList);

			auto list_entry = entity_list
				? process.read_memory<uintptr_t>(entity_list + 0x10)
				: static_cast<uintptr_t>(0);

			auto view_matrix = process.read_memory<ViewMatrix>(client_base_address + offsets::dwViewMatrix);

			// Entity list geometry comes from the runtime offsets, so a game update
			// that changes the entry stride can be handled by editing offsets.json
			// instead of rebuilding.
			const uintptr_t entry_size =
				(offsets::entityListEntrySize > 0 && offsets::entityListEntrySize <= kMaxEntityListEntrySize)
					? offsets::entityListEntrySize
					: kMaxEntityListEntrySize;
			const uintptr_t chunk_size = (offsets::entityListChunkSize > 0) ? offsets::entityListChunkSize : 512;

			// The controller slots are contiguous, so read the whole identity block
			// with a single ReadProcessMemory() instead of one call per slot. Falls
			// back to per-slot reads if the block cannot be read completely.
			uint8_t controller_block[kMaxEntitySlots * kMaxEntityListEntrySize] = {};
			const bool have_controller_block = list_entry != 0
				&& process.read_memory_block(list_entry + entry_size,
					controller_block, static_cast<size_t>(kMaxEntitySlots) * entry_size);

			int boxes_drawn = 0;

			for (int i = 0; i < kMaxEntitySlots && list_entry != 0; i++) {

				uintptr_t current_controller = 0;
				if (have_controller_block) {
					std::memcpy(&current_controller,
						controller_block + static_cast<size_t>(i) * entry_size,
						sizeof(current_controller));
				}
				else {
					current_controller = process.read_memory<uintptr_t>(list_entry + (i + 1) * entry_size);
				}

				if (current_controller == 0) {
					continue;
				}

				const auto pawn_handle = process.read_memory<uint32_t>(current_controller + offsets::m_hPlayerPawn);
				if (pawn_handle == 0) {
					continue;
				}

				// The handle stores the entity index: chunk = index / chunk_size,
				// slot = index % chunk_size.
				const uintptr_t entity_index = pawn_handle & 0x7FFF;
				auto list_entry_2 = process.read_memory<uintptr_t>(
					entity_list + sizeof(uintptr_t) * (entity_index / chunk_size) + offsets::entityListChunkOffset);
				if (list_entry_2 == 0) {
					continue;
				}

				auto current_pawn = process.read_memory<uintptr_t>(
					list_entry_2 + entry_size * (entity_index % chunk_size));
				if (current_pawn == 0) {
					continue;
				}

				// m_bDormant lives in CGameSceneNode, not directly on the pawn.
				auto node_scene = process.read_memory<uintptr_t>(current_pawn + offsets::m_pGameSceneNode);
				if (node_scene == 0) {
					continue;
				}

				const auto dormant = process.read_memory<bool>(node_scene + offsets::m_bDormant);
				PRINTF_DEBUG("Dormant: %d\n", dormant);
				if (dormant) {
					continue;
				}

				auto health = process.read_memory<int>(current_pawn + offsets::m_iHealth);
				PRINTF_DEBUG("Health: %d\n", health);
				if (health <= 0) {
					continue;
				}

				// m_lifeState is a single byte and LIFE_ALIVE is 0. Reading it as an
				// int pulled in the following field, and the old `!= 256` check only
				// worked while that byte happened to be 1: after a game update the
				// check matched nothing and no enemy was ever drawn.
				const auto life_state = process.read_memory<uint8_t>(current_pawn + offsets::m_lifeState);
				PRINTF_DEBUG("Life state: %d\n", life_state);
				if (life_state != 0) {
					continue;
				}

				const auto team_num = process.read_memory<uint8_t>(current_pawn + offsets::m_iTeamNum);
				PRINTF_DEBUG("Team num: %d\n", team_num);
				if (team_num == local_player_team) {
					continue;
				}

				auto spotted = process.read_memory<bool>(current_pawn + offsets::m_entitySpottedState + offsets::m_bSpotted);
				PRINTF_DEBUG("Spotted: %d\n", spotted);

				auto bone_matrix_ptr = process.read_memory<uintptr_t>(
					node_scene + offsets::m_modelState + offsets::modelStateBoneArray);
				auto bone_matrix = process.read_memory<BoneMatrix>(bone_matrix_ptr);

				// Everything that is drawn on a player is anchored on the head bone:
				// it comes from the skeleton (the same data that makes the bones and
				// the head marker work) and not from a pawn field such as
				// m_vOldOrigin, whose offset changes with every game update. When that
				// offset is stale the box is silently projected somewhere else on the
				// map, which looks exactly like "no boxes are drawn" while the
				// skeleton keeps working.
				const Vector head_world = bone_matrix[bone_id::head];

				Vector head_position_2d;
				if (!world_to_screen(head_world, head_position_2d, view_matrix)) {
					continue;
				}

				projection::ScreenBox box;
				if (!compute_screen_box(head_world, settings->BOX_HEIGHT, settings->BOX_HEIGHT_WIDTH_RATIO, view_matrix, box)) {
					continue;
				}

				// Proximity alert: beep when an enemy (even behind walls) enters the circle.
				// Throttled: one beep per enemy every PROXIMITY_ALERT_INTERVAL seconds.
				if (settings->PROXIMITY_ALERT_ENABLED) {
					const float px = (box.left + box.right) * 0.5f - diplay_center_x;
					const float py = box.bottom - diplay_center_y;
					const float radius = settings->PROXIMITY_ALERT_RADIUS;

					// Squared distance: no sqrt() in the per-enemy path.
					if (px * px + py * py <= radius * radius) {
						const ULONGLONG now = GetTickCount64();

						// Never below 250 ms: a zeroed interval would otherwise start a
						// beep thread on every single frame.
						ULONGLONG interval_ms = static_cast<ULONGLONG>(settings->PROXIMITY_ALERT_INTERVAL * 1000.0f);
						if (interval_ms < 250) {
							interval_ms = 250;
						}

						if (now - proximity_last_alert[i] >= interval_ms) {
							proximity_last_alert[i] = now;
							play_beep_async(1000, 200);
						}
					}
				}

				const float half_width = box.width() * 0.5f;

				// Draw the box
				if (wallhack_enabled && settings->DRAW_BOX) {
					if (!spotted || settings->DRAW_BOX_WHEN_SPOTTED) {
						++boxes_drawn;
						drawRect(
							Vec(box.left, box.top),
							Vec(box.right, box.bottom),
							BOX_COLOR,
							settings->BOX_THICKNESS
						);
					}
				}

				// Draw the health bar
				if (wallhack_enabled && settings->DRAW_HEALTH_BAR) {
					if (!spotted || settings->DRAW_HEALTH_BAR_WHEN_SPOTTED) {
						uint8_t red = static_cast<uint8_t>(255 - (health * 2.55f) * 0.5f);
						uint8_t green = static_cast<uint8_t>(health * 2.55f);

						const float health_ratio = (health > 100) ? 1.0f : (static_cast<float>(health) / 100.0f);
						const float health_top_y = box.bottom - health_ratio * box.height();

						drawLine(
							Vec(box.left, box.bottom),
							Vec(box.left, health_top_y),
							IM_COL32(red, green, 0, 255),
							settings->HEALTH_BAR_THICKNESS
						);
					}
				}

				// Write the health
				if (wallhack_enabled && settings->DRAW_TEXT) {
					if (!spotted || settings->DRAW_TEXT_WHEN_SPOTTED) {
						// Stack buffer instead of std::to_string(): no heap allocation
						// for every player on every frame.
						char health_text[16];
						sprintf_s(health_text, "%d", health);
						drawText(
							Vec(box.left, box.top - 10),
							TEXT_COLOR,
							health_text
						);
					}
				}

				// Draw the bones
				if (wallhack_enabled && settings->DRAW_BONES) {
					if (!spotted || settings->DRAW_BONES_WHEN_SPOTTED) {
						for (BonePair pair : bone_pairs) {
							Vector bone_position_3d_1 = bone_matrix[pair.bone1];
							Vector bone_position_3d_2 = bone_matrix[pair.bone2];

							Vector bone_position_2d_1;
							Vector bone_position_2d_2;

							if (!world_to_screen(bone_position_3d_1, bone_position_2d_1, view_matrix))
								continue;
							if (!world_to_screen(bone_position_3d_2, bone_position_2d_2, view_matrix))
								continue;

							drawLine(
								Vec(bone_position_2d_1.x, bone_position_2d_1.y),
								Vec(bone_position_2d_2.x, bone_position_2d_2.y),
								BONES_COLOR,
								settings->BONES_THICKNESS
							);
						}
					}
				}

				if (wallhack_enabled && settings->DRAW_HEAD) {
					if (!spotted || settings->DRAW_HEAD_WHEN_SPOTTED) {
						// Draw the Head
						auto head_radius = half_width * 0.5f;

						drawCircle(
							Vec(head_position_2d.x, head_position_2d.y),
							head_radius,
							HEAD_COLOR,
							32
						);
					}
				}	

				if (settings->AIMBOT_ENABLED) {
					if (spotted || settings->AIMBOT_ENABLED_WHEN_NOT_SPOTTED) {
						// Nearest head, compared on squared distances so the per-enemy
						// path does not call sqrt()/pow().
						const float dx = head_position_2d.x - diplay_center_x;
						const float dy = head_position_2d.y - diplay_center_y;
						const float distance_sq = dx * dx + dy * dy;
						if (distance_sq < min_distance_sq) {
							min_distance_sq = distance_sq;
							nearest_head_position_2d = head_position_2d;
							has_target = true;
						}
					}
				}
			}

			// Report box count changes in the console: it makes "no boxes on
			// screen" diagnosable without a debugger (0 = nothing reached the draw
			// call, N > 0 = the boxes are being emitted but may be invisible).
			// Throttled to one line per second: console output is slow and the count
			// changes every time a player enters or leaves the view.
			if (boxes_drawn != boxes_drawn_last_frame) {
				const ULONGLONG now = GetTickCount64();
				if (now - boxes_log_last_ms >= 1000) {
					std::cout << "[esp] boxes drawn: " << boxes_drawn << "\n";
					boxes_log_last_ms = now;
					boxes_drawn_last_frame = boxes_drawn;
				}
			}

			if (settings->AIMBOT_ENABLED) {
				// Aim at the nearest head.
				//
				// The smoothing loop used to call Sleep(1) between the steps: that
				// stalled the render loop (and therefore the whole overlay) for
				// several milliseconds on every single frame while aiming. The steps
				// are now applied back to back with no sleep at all.
				if (has_target) {
					const float delta_x = diplay_center_x - nearest_head_position_2d.x;
					const float delta_y = diplay_center_y - nearest_head_position_2d.y;
					const float fov = settings->AIMBOT_FOV;

					if (delta_x * delta_x + delta_y * delta_y <= fov * fov) {
						// One mouse move per step, all in the same frame. The number of
						// steps is capped so a silly AIMBOT_SMOOTH value cannot turn
						// into thousands of input calls (it used to be thousands of
						// Sleep(1) calls, which froze the overlay).
						int steps = settings->AIMBOT_SMOOTH;
						if (steps < 1) {
							steps = 1;
						}
						else if (steps > 16) {
							steps = 16;
						}

						const float step_x = delta_x / static_cast<float>(steps);
						const float step_y = delta_y / static_cast<float>(steps);

						for (int step = 0; step < steps; ++step) {
							mouse_event(MOUSEEVENTF_MOVE,
								-static_cast<LONG>(std::lround(step_x)),
								-static_cast<LONG>(std::lround(step_y)),
								0, 0);
						}
					}
				}

				if (settings->AIMBOT_DRAW_FOV) {
					drawCircle(
						Vec(diplay_center_x, diplay_center_y),
						settings->AIMBOT_FOV,
						AIMBOT_FOV_COLOR,
						32
					);
				}
			}
		}

		// Warn (at most every 2 s) when the work of a frame does not fit in a 60 Hz
		// budget: after the blocking calls were removed, anything left here is a
		// real hotspot (measured without the vsync wait of Present()).
		{
			static ULONGLONG last_warning_ms = 0;
			const float work_ms = std::chrono::duration<float, std::milli>(
				std::chrono::steady_clock::now() - frame_start).count();
			if (work_ms > 8.0f) {
				const ULONGLONG now = GetTickCount64();
				if (now - last_warning_ms >= 2000) {
					last_warning_ms = now;
					std::printf("[esp] slow frame: %.1f ms\n", work_ms);
				}
			}
		}

		gui.render();
	}

	PRINT_STRING_DEBUG("Exiting\n");

	// Cleanup
	gui.cleanup();

	server.stop();
	server_thread.join();

	// Save the settings
	save_settings();

	return EXIT_SUCCESS;
}