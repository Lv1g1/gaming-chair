#include "process.h"

#include <TlHelp32.h>

Process::Process() noexcept {
	handle = nullptr;
	process_id = 0;
}

Process::~Process() noexcept {
	if constexpr (MODE == USER_MODE) {
		if (handle) {
			CloseHandle(handle);
		}
	}
}

DWORD Process::get_process_id(const wchar_t* process_name) noexcept {
	DWORD pid = 0;

	HANDLE snap_shot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (snap_shot == INVALID_HANDLE_VALUE)
		return pid;

	PROCESSENTRY32W entry = {};
	entry.dwSize = sizeof(decltype(entry));

	if (Process32FirstW(snap_shot, &entry) == TRUE) {
		// Check if the first handle is the one we want.
		if (_wcsicmp(process_name, entry.szExeFile) == 0)
			pid = entry.th32ProcessID;
		else {
			while (Process32NextW(snap_shot, &entry) == TRUE) {
				if (_wcsicmp(process_name, entry.szExeFile) == 0) {
					pid = entry.th32ProcessID;
					break;
				}
			}
		}
	}

	CloseHandle(snap_shot);

	return pid;
}

bool Process::attach(const wchar_t* process_name) noexcept {
	process_id = get_process_id(process_name);
	if (!process_id)
		return false;

	if constexpr (MODE == KERNEL_MODE) {
		handle = driver::init();
		if (!handle)
			return false;
		
		driver::Request r;
		r.process_id = reinterpret_cast<HANDLE>(process_id);
	
		if (!driver::attach_to_process(handle, r))
			return false;
	}
	else {
		handle = OpenProcess(PROCESS_ALL_ACCESS, TRUE, process_id);
		if (!handle)
			return false;
	}
	
	return true;
}

void Process::detach() noexcept {
	if constexpr (MODE == USER_MODE) {
		if (handle) {
			CloseHandle(handle);
			handle = nullptr;
			process_id = 0;
		}
	}
}

uintptr_t Process::get_module_base_address(const wchar_t* module_name) noexcept {
	uintptr_t module_base = 0;

	// Snap-shot of process' modules (dlls).
	HANDLE snap_shot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, process_id);
	if (snap_shot == INVALID_HANDLE_VALUE)
		return module_base;

	MODULEENTRY32W entry = {};
	entry.dwSize = sizeof(decltype(entry));

	if (Module32FirstW(snap_shot, &entry) == TRUE) {
		if (wcsstr(module_name, entry.szModule) != nullptr)
			module_base = reinterpret_cast<uintptr_t>(entry.modBaseAddr);
		else {
			while (Module32NextW(snap_shot, &entry) == TRUE) {
				if (wcsstr(module_name, entry.szModule) != nullptr) {
					module_base = reinterpret_cast<uintptr_t>(entry.modBaseAddr);
					break;
				}
			}
		}
	}

	CloseHandle(snap_shot);

	return module_base;
}