#include "assets_manager.hpp"
#include "stb_image.h"
#include <cstring>
#include <engine/core/assert.hpp>
#include <engine/core/filesystem.hpp>
#include <engine/core/logger.hpp>
#include <fstream>
#include <memory>

namespace engine::core {
Texture AssetsManager::loadTexture(std::string_view path)
{
  Texture texture;
  std::string absPath = std::string(texturesPath / path);

  int width;
  int height;
  int channels;
  std::byte *rawData = reinterpret_cast<std::byte *>(stbi_load(absPath.c_str(), &width, &height, &channels, 0));

  if (!rawData) {
    LOG_ERROR("Failed to load texture: {}", absPath);
    return createErrorTexture();
  }

  texture.width = static_cast<uint32_t>(width);
  texture.height = static_cast<uint32_t>(height);
  texture.channels = static_cast<uint8_t>(channels);
  texture.data.reset(rawData);

  return texture;
};

std::vector<char> AssetsManager::loadShader(std::string_view path)
{
  std::unordered_set<std::string> includedFiles;
  std::string code = loadShaderWithIncludes(std::filesystem::path(shadersPath / path.data()), includedFiles);
  return std::vector<char>(code.begin(), code.end());
}

std::string AssetsManager::loadShaderWithIncludes(const std::filesystem::path &path,
  std::unordered_set<std::string> &includedFiles,
  int depth)
{
  SE_ASSERT(depth <= 32, "Maximum include depth reached (possible cyclic include)");

  std::string canonicalPath = std::filesystem::canonical(path).string();
  if (includedFiles.count(canonicalPath)) { return ""; }
  includedFiles.insert(canonicalPath);

  std::ifstream file(path);
  SE_ASSERT(file.is_open(), "Failed to open shader file: {}", path.string());

  std::stringstream buffer;
  std::string line;
  std::filesystem::path parentDir = path.parent_path();

  while (std::getline(file, line)) {
    if (line.find("#include") == 0) {
      size_t start = line.find('"');
      if (start == std::string::npos) { start = line.find('<'); }
      if (start == std::string::npos) { continue; }

      size_t end = line.find_last_of(start == line.find('"') ? '"' : '>');
      if (end == std::string::npos) { continue; }

      std::string includePath = line.substr(start + 1, end - start - 1);
      std::filesystem::path fullIncludePath = parentDir / includePath;

      std::string includedContent = loadShaderWithIncludes(fullIncludePath, includedFiles, depth + 1);
      buffer << includedContent << "\n";
    } else {
      buffer << line << "\n";
    }
  }

  return buffer.str();
}

std::vector<char> AssetsManager::readFile(std::string_view filename)
{
  std::ifstream file(shadersPath / filename.data(), std::ios::ate | std::ios::binary);

  SE_ASSERT(file.is_open(), "Failed to open {}!", filename);

  size_t fileSize = static_cast<size_t>(file.tellg());
  std::vector<char> buffer(fileSize);

  file.seekg(0);
  file.read(buffer.data(), static_cast<std::streamsize>(fileSize));

  file.close();
  return buffer;
}

Texture AssetsManager::createErrorTexture(uint32_t size)
{
  Texture texture;
  texture.width = size;
  texture.height = size;
  texture.channels = 4;

  texture.data = std::make_unique<std::byte[]>(size * size * 4);
  const std::byte pink[] = { std::byte(0xFF), std::byte(0x00), std::byte(0xFF), std::byte(0xFF) };
  const std::byte black[] = { std::byte(0x00), std::byte(0x00), std::byte(0x00), std::byte(0xFF) };

  for (uint32_t y = 0; y < size; ++y) {
    for (uint32_t x = 0; x < size; ++x) {
      bool isPink = (x / (size / 2)) % 2 == (y / (size / 2)) % 2;

      uint32_t pixelOffset = (y * size + x) * 4;
      const std::byte *color = isPink ? pink : black;
      std::memcpy(&texture.data[pixelOffset], color, 4);
    }
  }

  return texture;
}

#ifndef NDEBUG
AssetsManager::CallbackId AssetsManager::subscribe(FileChangeCallback callback)
{
  if (!efswWatcher) {
    efswWatcher = std::make_unique<efsw::FileWatcher>();
    watcher = std::make_unique<AssetsWatcher>();
    efswWatcher->addWatch(std::filesystem::path(SOURCE_DIR) / "assets", watcher.get(), true);
    efswWatcher->watch();
  }
  CallbackId id = { nextCallbackId++ };
  callbacks[id.value] = callback;
  return id;
}

void AssetsManager::unsubscribe(CallbackId id) { callbacks.erase(id.value); }

void AssetsManager::onAssetsModified(std::string filename)
{
  for (auto &callback : callbacks) { callback.second(filename); }
}

std::unordered_map<size_t, AssetsManager::FileChangeCallback> AssetsManager::callbacks{};
std::unique_ptr<efsw::FileWatcher> AssetsManager::efswWatcher{};
std::unique_ptr<AssetsManager::AssetsWatcher> AssetsManager::watcher{};
size_t AssetsManager::nextCallbackId = 0;
#endif

std::filesystem::path AssetsManager::assetsPath = getAbsolutePath("assets");
std::filesystem::path AssetsManager::shadersPath = AssetsManager::assetsPath / "shaders";
std::filesystem::path AssetsManager::texturesPath = AssetsManager::assetsPath / "textures";
}// namespace engine::core