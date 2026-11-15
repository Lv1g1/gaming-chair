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
		// Zero-initialised so a failed read returns a deterministic value instead
		// of whatever happened to be on the stack.
		T buffer{};

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

	// Reads a whole block in a single call. Returns false when the block could not
	// be read completely. Reading an array in one go (for example the entity
	// list) is much cheaper than one ReadProcessMemory() per element.
	bool read_memory_block(uintptr_t address, void* buffer, size_t size) noexcept {
		if (size == 0)
			return false;

		if constexpr (MODE == KERNEL_MODE) {
			driver::Request r;
			r.target = reinterpret_cast<PVOID>(address);
			r.buffer = buffer;
			r.size = size;
			return driver::read_memory(handle, r);
		}
		else {
			SIZE_T read = 0;
			return ReadProcessMemory(handle, reinterpret_cast<LPCVOID>(address), buffer, size, &read) != 0
				&& read == size;
		}
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