#pragma once

#ifndef NDEBUG
#include "efsw/efsw.hpp"
#include <functional>
#include <unordered_map>
#endif
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
#ifndef NDEBUG
  struct CallbackId {
    size_t value;
  };

  using FileChangeCallback = std::function<void(const std::string &)>;
#endif

private:
#ifndef NDEBUG
  class AssetsWatcher : public efsw::FileWatchListener {
  public:
    void handleFileAction([[maybe_unused]] efsw::WatchID watchid, const std::string &dir,
                          [[maybe_unused]] const std::string &filename, efsw::Action action,
                          [[maybe_unused]] std::string oldFilename = "") override {
      if (action == efsw::Actions::Modified) {
        auto subfolder = dir.substr(assetsPath.string().size() + 1);
        auto relativePath = std::filesystem::relative(dir, std::filesystem::path(SOURCE_DIR) / "assets");
        std::filesystem::copy(std::filesystem::path(dir) / filename, assetsPath / relativePath / filename,
                              std::filesystem::copy_options::overwrite_existing);
        onAssetsModified(filename);
      }
    }
  };
#endif

public:
  static Texture loadTexture(std::string_view path);

#ifndef NDEBUG
  [[nodiscard]] static CallbackId subscribe(FileChangeCallback callback);
  static void unsubscribe(CallbackId id);
#endif

private:
  static Texture createErrorTexture(uint32_t size = 16);

#ifndef NDEBUG
  static void onAssetsModified(std::string filename);
#endif

private:
  static std::filesystem::path assetsPath;
  static std::filesystem::path shadersPath;
  static std::filesystem::path texturesPath;

#ifndef NDEBUG
  static std::unique_ptr<efsw::FileWatcher> efswWatcher;
  static std::unique_ptr<AssetsWatcher> watcher;
  static std::unordered_map<size_t, FileChangeCallback> callbacks;
  static size_t nextCallbackId;
#endif
};
} // namespace engine::core