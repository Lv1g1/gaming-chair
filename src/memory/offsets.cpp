#include "offsets.h"

#include <Windows.h>
#include <winhttp.h>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#pragma comment(lib, "winhttp.lib")

namespace offsets {

// ---------------------------------------------------------------------------
// Runtime values, initialized to the last known-good fallback values.
// Keep these in sync with reset() below.
// ---------------------------------------------------------------------------

uintptr_t dwLocalPlayerPawn = 0x2553D38;
uintptr_t dwEntityList      = 0x2709010;
uintptr_t dwViewMatrix      = 0x2559640;

uintptr_t m_vecViewOffset    = 0xF60;
uintptr_t m_hPlayerPawn      = 0x92C;
uintptr_t m_iHealth          = 0x34C;
uintptr_t m_iTeamNum         = 0x3E7;
uintptr_t m_lifeState        = 0x354;
uintptr_t m_vecOrigin        = 0x80;
uintptr_t m_vOldOrigin       = 0x1324;
uintptr_t m_bDormant         = 0x103;

uintptr_t m_pGameSceneNode   = 0x330;
uintptr_t m_CBodyComponent   = 0x30;
uintptr_t m_skeletonInstance = 0x80;
uintptr_t m_modelState       = 0x140;
uintptr_t m_entitySpottedState = 0x23D0;
uintptr_t m_bSpotted         = 0x8;

uintptr_t entityListEntrySize   = 0x70;
uintptr_t entityListChunkOffset = 0x10;
uintptr_t entityListChunkSize   = 512;
uintptr_t modelStateBoneArray   = 0x80;

void reset() noexcept {
    dwLocalPlayerPawn = 0x2553D38;
    dwEntityList      = 0x2709010;
    dwViewMatrix      = 0x2559640;

    m_vecViewOffset    = 0xF60;
    m_hPlayerPawn      = 0x92C;
    m_iHealth          = 0x34C;
    m_iTeamNum         = 0x3E7;
    m_lifeState        = 0x354;
    m_vecOrigin        = 0x80;
    m_vOldOrigin       = 0x1324;
    m_bDormant         = 0x103;

    m_pGameSceneNode   = 0x330;
    m_CBodyComponent   = 0x30;
    m_skeletonInstance = 0x80;
    m_modelState       = 0x140;
    m_entitySpottedState = 0x23D0;
    m_bSpotted         = 0x8;

    entityListEntrySize   = 0x70;
    entityListChunkOffset = 0x10;
    entityListChunkSize   = 512;
    modelStateBoneArray   = 0x80;
}

// ---------------------------------------------------------------------------
// JSON resolution
// ---------------------------------------------------------------------------

namespace {

    struct OffsetEntry {
        const char* name;
        uintptr_t* target;
    };

    // Name -> address-of-offset. The resolver updates *target for every entry
    // whose name is found in the JSON source.
    constexpr OffsetEntry kOffsets[] = {
        { "dwLocalPlayerPawn",    &dwLocalPlayerPawn },
        { "dwEntityList",         &dwEntityList },
        { "dwViewMatrix",         &dwViewMatrix },

        { "m_vecViewOffset",      &m_vecViewOffset },
        { "m_hPlayerPawn",        &m_hPlayerPawn },
        { "m_iHealth",            &m_iHealth },
        { "m_iTeamNum",           &m_iTeamNum },
        { "m_lifeState",          &m_lifeState },
        { "m_vecOrigin",          &m_vecOrigin },
        { "m_vOldOrigin",         &m_vOldOrigin },
        { "m_bDormant",           &m_bDormant },

        { "m_pGameSceneNode",     &m_pGameSceneNode },
        { "m_CBodyComponent",     &m_CBodyComponent },
        { "m_skeletonInstance",   &m_skeletonInstance },
        { "m_modelState",         &m_modelState },
        { "m_entitySpottedState", &m_entitySpottedState },
        { "m_bSpotted",           &m_bSpotted },

        { "entityListEntrySize",   &entityListEntrySize },
        { "entityListChunkOffset", &entityListChunkOffset },
        { "entityListChunkSize",   &entityListChunkSize },
        { "modelStateBoneArray",   &modelStateBoneArray },
    };

    bool is_space(char c) noexcept {
        return c == ' ' || c == '\t' || c == '\r' || c == '\n';
    }

    // Characters that end a bare JSON value: "," "}" "]" and whitespace.
    bool is_value_delimiter(char c) noexcept {
        return is_space(c) || c == ',' || c == '}' || c == ']';
    }

    // Parses a value token: "12345", "0x1A146E8" (quoted or bare) or a bare
    // decimal number. Returns false when the token is not a valid offset, so an
    // invalid entry can never be applied as 0.
    bool parse_value(const std::string& token, uintptr_t& out) noexcept {
        std::string s = token;
        if (s.size() >= 2 && s.front() == '"' && s.back() == '"')
            s = s.substr(1, s.size() - 2);

        if (s.empty())
            return false;

        // Offsets are never negative; rejecting the sign also stops strtoull
        // from wrapping a "-1" into a huge value.
        if (s.find('-') != std::string::npos || s.find('+') != std::string::npos)
            return false;

        const bool hex = s.size() > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X');
        const char* digits = hex ? s.c_str() + 2 : s.c_str();
        if (*digits == '\0')
            return false;

        errno = 0;
        char* end = nullptr;
        const unsigned long long value = strtoull(digits, &end, hex ? 16 : 10);
        if (end == digits || *end != '\0' || errno == ERANGE)
            return false;

        out = static_cast<uintptr_t>(value);
        return true;
    }

    // Finds `"name": value` in the JSON text and writes `value` into `out`.
    // Returns false when the name is not present as a valid key/value pair.
    bool extract_offset(const std::string& json, const char* name, uintptr_t& out) noexcept {
        const std::string key = "\"" + std::string(name) + "\"";

        size_t pos = 0;
        while ((pos = json.find(key, pos)) != std::string::npos) {
            const size_t after_key = pos + key.size();

            // The name must be a key: followed (after whitespace) by ':'.
            size_t colon = after_key;
            while (colon < json.size() && is_space(json[colon]))
                ++colon;
            if (colon >= json.size() || json[colon] != ':') {
                pos = after_key;
                continue;
            }

            size_t i = colon + 1;
            while (i < json.size() && is_space(json[i]))
                ++i;
            if (i >= json.size())
                return false;

            std::string token;
            if (json[i] == '"') {
                const size_t end = json.find('"', i + 1);
                if (end == std::string::npos)
                    return false;
                token = json.substr(i + 1, end - i - 1);
            }
            else {
                size_t end = i;
                while (end < json.size() && !is_value_delimiter(json[end]))
                    ++end;
                token = json.substr(i, end - i);
            }

            uintptr_t parsed = 0;
            if (!parse_value(token, parsed)) {
                // Not a number (or a broken hex literal): keep looking for another
                // occurrence instead of applying a bogus 0.
                pos = after_key;
                continue;
            }

            out = parsed;
            return true;
        }

        return false;
    }

    // Applies every offset found in `json`; increments `resolved` per match.
    // Returns true when at least one offset was updated.
    bool apply_json(const std::string& json, size_t& resolved) noexcept {
        if (json.empty())
            return false;

        bool any = false;
        for (const OffsetEntry& entry : kOffsets) {
            uintptr_t value = 0;
            if (!extract_offset(json, entry.name, value))
                continue;

            // A zeroed offset would make every read hit address 0 and break the
            // whole tool silently: never accept it.
            if (value == 0) {
                OutputDebugStringA("[offsets] ignoring a zero value\n");
                continue;
            }

            *entry.target = value;
            ++resolved;
            any = true;
        }
        return any;
    }

    bool read_file(const std::string& path, std::string& out) noexcept {
        FILE* file = nullptr;
        if (fopen_s(&file, path.c_str(), "rb") != 0 || !file)
            return false;

        fseek(file, 0, SEEK_END);
        const long size = ftell(file);
        fseek(file, 0, SEEK_SET);

        if (size <= 0) {
            fclose(file);
            return false;
        }

        out.resize(static_cast<size_t>(size));
        const size_t read = fread(&out[0], 1, static_cast<size_t>(size), file);
        fclose(file);

        out.resize(read);
        return read > 0;
    }

    std::wstring to_wide(const std::string& s) {
        if (s.empty())
            return {};
        const int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), nullptr, 0);
        if (n <= 0)
            return {};
        std::wstring w(static_cast<size_t>(n), L'\0');
        MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), &w[0], n);
        return w;
    }

    // Downloads `url` (http or https) into `out`. Returns false on any failure.
    bool http_get(const std::string& url, std::string& out) noexcept {
        const size_t scheme_end = url.find("://");
        if (scheme_end == std::string::npos)
            return false;

        const std::string scheme = url.substr(0, scheme_end);
        const bool secure = (scheme == "https");

        const size_t host_start = scheme_end + 3;
        const size_t path_start = url.find('/', host_start);

        std::string host_port;
        std::string path = "/";
        if (path_start == std::string::npos) {
            host_port = url.substr(host_start);
        }
        else {
            host_port = url.substr(host_start, path_start - host_start);
            path = url.substr(path_start);
        }

        std::string host = host_port;
        INTERNET_PORT port = secure ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT;
        const size_t colon = host_port.rfind(':');
        if (colon != std::string::npos) {
            host = host_port.substr(0, colon);

            const char* port_str = host_port.c_str() + colon + 1;
            char* end = nullptr;
            const long parsed = strtol(port_str, &end, 10);
            if (end != port_str && *end == '\0' && parsed > 0 && parsed <= 65535)
                port = static_cast<INTERNET_PORT>(parsed);
        }

        const std::wstring w_host = to_wide(host);
        const std::wstring w_path = to_wide(path);

        HINTERNET session = WinHttpOpen(
            L"gaming-chair/1.0",
            WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
            WINHTTP_NO_PROXY_NAME,
            WINHTTP_NO_PROXY_BYPASS,
            0);
        if (!session)
            return false;

        constexpr DWORD kTimeoutMs = 5000;
        WinHttpSetTimeouts(session, kTimeoutMs, kTimeoutMs, kTimeoutMs, kTimeoutMs);

        HINTERNET connect = WinHttpConnect(session, w_host.c_str(), port, 0);
        HINTERNET request = nullptr;
        bool ok = false;

        if (connect) {
            request = WinHttpOpenRequest(
                connect,
                L"GET",
                w_path.c_str(),
                nullptr,
                WINHTTP_NO_REFERER,
                WINHTTP_DEFAULT_ACCEPT_TYPES,
                secure ? WINHTTP_FLAG_SECURE : 0);
        }

        if (request &&
            WinHttpSendRequest(request, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                WINHTTP_NO_REQUEST_DATA, 0, 0, 0) &&
            WinHttpReceiveResponse(request, nullptr)) {

            out.clear();
            DWORD size = 0;
            DWORD downloaded = 0;
            for (;;) {
                size = 0;
                if (!WinHttpQueryDataAvailable(request, &size) || size == 0)
                    break;

                std::vector<char> buffer(size);
                if (!WinHttpReadData(request, buffer.data(), size, &downloaded) || downloaded == 0)
                    break;

                out.append(buffer.data(), downloaded);
            }
            ok = !out.empty();
        }

        if (request) WinHttpCloseHandle(request);
        if (connect) WinHttpCloseHandle(connect);
        WinHttpCloseHandle(session);

        return ok;
    }

    void debug_log(const char* message) noexcept {
        OutputDebugStringA(message);
    }

    // Absolute canonical form, so the same file found through two different
    // relative paths is only read once.
    std::string absolute_path(const std::string& path) {
        char buffer[MAX_PATH] = {};
        if (GetFullPathNameA(path.c_str(), MAX_PATH, buffer, nullptr) == 0)
            return path;
        return buffer;
    }

} // namespace

bool resolve() noexcept {
    reset();

    size_t resolved = 0;

    // Candidate files, from the least to the most specific one: a value found in
    // a later file overrides the same value found earlier.
    //
    //   1. working directory  (where the tool was started from)
    //   2. project root       (one level above build\, where the repository keeps
    //                          offsets.json - this is the case when the exe is
    //                          launched from Explorer instead of Visual Studio)
    //   3. next to the exe    (per-installation override)
    std::vector<std::string> candidates;

    std::string exe_dir;
    {
        char exe_path[MAX_PATH] = {};
        if (GetModuleFileNameA(nullptr, exe_path, MAX_PATH) > 0) {
            std::string path(exe_path);
            const size_t slash = path.find_last_of("\\/");
            if (slash != std::string::npos) {
                exe_dir = path.substr(0, slash + 1);                    // "...\build\"

                std::string parent = exe_dir;
                if (!parent.empty() && (parent.back() == '\\' || parent.back() == '/'))
                    parent.pop_back();
                const size_t parent_slash = parent.find_last_of("\\/");
                if (parent_slash != std::string::npos)
                    candidates.push_back(parent.substr(0, parent_slash + 1) + OFFSETS_LOCAL_FILENAME);
            }
        }
    }

    candidates.insert(candidates.begin(), OFFSETS_LOCAL_FILENAME);
    if (!exe_dir.empty())
        candidates.push_back(exe_dir + OFFSETS_LOCAL_FILENAME);

    size_t sources_read = 0;
    std::string last_source;
    std::vector<std::string> seen;

    for (const std::string& candidate : candidates) {
        const std::string path = absolute_path(candidate);

        bool duplicate = false;
        for (const std::string& previous : seen) {
            if (_stricmp(previous.c_str(), path.c_str()) == 0) {
                duplicate = true;
                break;
            }
        }
        if (duplicate)
            continue;
        seen.push_back(path);

        std::string content;
        if (!read_file(path, content))
            continue;

        ++sources_read;
        last_source = path;
        apply_json(content, resolved);
    }

    // Remote endpoint (optional; skipped when OFFSETS_URL is empty at build time).
#if OFFSETS_REMOTE_ENABLED
    {
        std::string content;
        if (http_get(OFFSETS_URL, content) && !content.empty()) {
            const size_t before = resolved;
            apply_json(content, resolved);
            if (resolved > before) {
                ++sources_read;
                last_source = OFFSETS_URL;
            }
        }
    }
#endif

    const size_t total = sizeof(kOffsets) / sizeof(kOffsets[0]);

    if (sources_read > 0) {
        std::cout << "[offsets] " << resolved << "/" << total << " resolved from "
            << sources_read << " source(s), last: " << last_source << "\n";
    }
    else {
        std::cout << "[offsets] no " << OFFSETS_LOCAL_FILENAME
            << " found (working dir, project root, exe dir): using the compiled-in defaults\n";
    }

    // The values that matter most, so it is obvious what the tool is using.
    std::printf("[offsets] dwEntityList=0x%llX dwLocalPlayerPawn=0x%llX "
        "dwViewMatrix=0x%llX m_iHealth=0x%llX entityListEntrySize=0x%llX\n",
        static_cast<unsigned long long>(dwEntityList),
        static_cast<unsigned long long>(dwLocalPlayerPawn),
        static_cast<unsigned long long>(dwViewMatrix),
        static_cast<unsigned long long>(m_iHealth),
        static_cast<unsigned long long>(entityListEntrySize));

    {
        char buffer[128];
        sprintf_s(buffer, "[offsets] resolved %zu/%zu offsets\n", resolved, total);
        debug_log(buffer);
    }

    return resolved > 0;
}

} // namespace offsets
