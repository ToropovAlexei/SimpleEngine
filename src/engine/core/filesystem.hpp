#pragma once

#include <array>
#include <filesystem>
#include <stdexcept>

#if defined(_WIN32)
#include <windows.h>
#elif defined(__linux__)
#include <unistd.h>
#else
#error Unsupported platform
#endif

namespace engine {
namespace core {

constexpr size_t BUFFER_SIZE = 1024;

inline std::filesystem::path getExecutablePath() {
  static const std::filesystem::path executablePath = []() {
    std::array<char, BUFFER_SIZE> buffer{};

#if defined(_WIN32)
    DWORD size = GetModuleFileNameA(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    if (size == 0) {
      throw std::runtime_error("Error getting executable path on Windows");
    }
#elif defined(__linux__)
    ssize_t len = readlink("/proc/self/exe", buffer.data(), buffer.size() - 1);
    if (len == -1) {
      throw std::runtime_error("Error getting executable path on Linux");
    }
    buffer[static_cast<size_t>(len)] = '\0';
#endif

    return std::filesystem::path(buffer.data());
  }();

  return executablePath;
}

inline std::filesystem::path getAbsolutePath(std::filesystem::path path) {
  return getExecutablePath().parent_path() / path;
}

} // namespace core
} // namespace engine