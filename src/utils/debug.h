#pragma once

constexpr bool DEBUG = false;

char PRINTF_DEBUG_BUFFER[4096];

#define PRINTF_DEBUG(format, ...) if constexpr (DEBUG) { \
	sprintf_s(PRINTF_DEBUG_BUFFER, format, __VA_ARGS__); \
	OutputDebugStringA(PRINTF_DEBUG_BUFFER); \
}

#define PRINT_STRING_DEBUG(str) if constexpr (DEBUG) { \
	OutputDebugStringA(str); \
}