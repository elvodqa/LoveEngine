#pragma once
#include <fstream>
#include <filesystem>
#include <unordered_map>
#include <efsw/efsw.hpp>
#include "../debug_panic.h"
#include "../Assets/Asset.h"
namespace fs = std::filesystem;

class UpdateListener final : public efsw::FileWatchListener {
public:
    void handleFileAction( efsw::WatchID watchid, const std::string& dir,
                           const std::string& filename, efsw::Action action,
                           std::string oldFilename ) override;
};


namespace love::project {

    inline fs::path loaded_project_path;
    inline std::unordered_map<fs::path, Asset> path_cache;

    ::love::AssetType TypeForPath(const fs::path &path);
    Asset* getAsset(const fs::path& path);
    Asset* getOrLoadAsset(const fs::path& path);
    Asset* LoadAsset(const fs::path &path);
    void loadProject(const fs::path& folder);
    void importObject(Asset* asset);
    uint64_t hash(const fs::path& src);



}
