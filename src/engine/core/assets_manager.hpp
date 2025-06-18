#pragma once

#include <filesystem>
#include <string_view>

namespace engine::core {
struct Texture {
  uint32_t width = 0;
  uint32_t height = 0;
  uint8_t channels = 0;
  std::unique_ptr<std::byte[]> data = nullptr;
};

class AssetsManager {
public:
  static Texture loadTexture(std::string_view path);

private:
  static Texture createErrorTexture(uint32_t size = 16);

private:
  static std::filesystem::path assetsPath;
  static std::filesystem::path shadersPath;
  static std::filesystem::path texturesPath;
};
} // namespace engine::core