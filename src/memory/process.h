#pragma once

#include <Windows.h>
#include "driver.h"

constexpr bool KERNEL_MODE = true;
constexpr bool USER_MODE = false;

constexpr bool MODE = USER_MODE;

class Process {
private:
	HANDLE handle;
	DWORD process_id;

	DWORD get_process_id(const wchar_t* process_name) noexcept;

public:
	Process() noexcept;
	~Process() noexcept;

	bool attach(const wchar_t* process_name) noexcept;
	void detach() noexcept;

	uintptr_t get_module_base_address(const wchar_t* module_name) noexcept;

	template <typename T>
	T read_memory(uintptr_t address) noexcept {
		T buffer;

		if constexpr (MODE == KERNEL_MODE) {
			driver::Request r;
			r.target = reinterpret_cast<PVOID>(address);
			r.buffer = &buffer;
			r.size = sizeof(T);
			driver::read_memory(handle, r);
		}
		else {
			ReadProcessMemory(handle, reinterpret_cast<LPVOID>(address), &buffer, sizeof(T), NULL);
		}

		return buffer;
	}

	template <typename T>
	void write_memory(uintptr_t address, T buffer) noexcept {
		if constexpr (MODE == KERNEL_MODE) {
			driver::Request r;
			r.target = reinterpret_cast<PVOID>(address);
			r.buffer = (PVOID)&buffer;
			r.size = sizeof(T);
			driver::write_memory(handle, r);
		}
		else {
			WriteProcessMemory(handle, reinterpret_cast<LPVOID>(address), &buffer, sizeof(T), nullptr);
		}
	}
};