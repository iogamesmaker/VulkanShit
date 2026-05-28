// utils.hpp
#pragma once
#include <filesystem>
#if defined (_WIN32)
#include <windows.h>
#elif defined (__linux__)
#include <unistd.h>
#include <limits.h>
#endif

inline std::filesystem::path root_dir() {
#if defined (_WIN32)
    wchar_t path[MAX_PATH] = { 0 };
    GetModuleFileNameW(NULL, path, MAX_PATH);
    return std::filesystem::path(path).parent_path();
#elif defined (__linux__)
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    return std::filesystem::path(std::string(result, (count > 0) ? count : 0)).parent_path();
#else
    return std::filesystem::current_path()
#endif
}
