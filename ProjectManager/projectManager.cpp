//
// Created by YAHAY on 31/01/2025.
//
#include "projectManager.h"
#include <filesystem>

#include <fstream>

#include "../debug_panic.h"
#include "../Renderer/EngineImage.h"


love::Asset * love::project::getAsset(const fs::path& path) {
    return path_cache.contains(path)?&path_cache[path]:nullptr;
}

love::Asset * love::project::getOrLoadAsset(const fs::path &path) {
    if (path_cache.contains(path)) {
        return &path_cache[path];
    }
    return LoadAsset(path);
}

love::AssetType love::project::TypeForPath(const fs::path &path) {
    auto ext = path.extension();
    if (ext == ".png" || ext == ".jpg") return  love::AssetType::texture;
    if (ext == ".gltf" || ext == ".ply") return  love::AssetType::mesh;

    return (love::AssetType)0;
}
love::Asset* love::project::LoadAsset(const fs::path &path) {
    if (!exists(loaded_project_path/path)) panic("added nonexistent file {}", path.string());
    if (path_cache.contains(path)) panic("already existing file {}", path.string());
    love::project::path_cache[path] = love::Asset{
        .type = TypeForPath(path),
        .relative_path = path,
        .hash = love::project::hash(path)
    };
    return &love::project::path_cache[path];
}

void love::project::loadProject(const fs::path& folder) {
    //todo fix
    loaded_project_path = folder;
}

// void love::project::createProject(const char *folder) {
//     fs::path p = folder;
//     std::error_code ec;
//     if(!fs::create_directory(p,ec)&&ec) {
//         panic("Error creating directory");
//     }
//     // if (!fs::is_empty(p)) {
//     //     panic("folder is not empty");
//     // }
//     // if (!fs::create_directory(p/love::project::engine_folder))
//     //     panic("Error creating folder");
//
//     love::project::cache=std::fstream((p/love::project::engine_folder/love::project::cache_name).c_str());
//     loaded_project_path=p;
//     cache.flush();
//     cache<<"t"<<std::endl;
//     if (!cache.is_open())
//         panic("Error opening file");
//
//     file_watcher.addWatch(( const std::string &)p,&love::project::update_listener,true);
//
// }

// void love::project::saveProject() {
//
// }
//
// void love::project::unloadProject() {
//     file_watcher.removeWatch((const std::string&)loaded_project_path);
//     project::cache.close();
//     loaded_project_path=fs::path();
// }


void love::project::importObject(Asset *asset) {

}

uint64_t love::project::hash(const fs::path& src) {
    return 0;
}

bool is_subpath(const fs::path &path,
                const fs::path &base)
{
    auto rel = fs::relative(path, base);
    return !rel.empty() && rel.native()[0] != '.';
}

void UpdateListener::handleFileAction(efsw::WatchID watchid, const std::string &dir, const std::string &filename,
    efsw::Action action, std::string oldFilename) {
    fs::path path = fs::path(dir) / filename;
    fs::path oldPath =fs::path(dir) / oldFilename;
    switch ( action ) {
        case efsw::Action::Add:
            if (!exists(path)) panic("added nonexistent file {}", path.string());
            love::project::path_cache[path] = love::Asset{
                .type = (love::AssetType)0,
                .relative_path = path,
                .hash = love::project::hash(path)
            };

            break;
        case efsw::Action::Delete:
            love::project::path_cache.erase(path);
            break;
        case efsw::Action::Modified:
            // love::project::path_cache[path].updated();//todo
            love::project::path_cache.erase(path);
            love::project::path_cache[path] = love::Asset{
                .type = (love::AssetType)0,
                .relative_path = path,
                .hash = love::project::hash(path)
            };
            break;
        case efsw::Action::Moved:
            __asm__("nop");//todo
        love::project::path_cache.erase(oldPath);
        love::project::path_cache[path] = love::Asset{
            .type = (love::AssetType)0,
            .relative_path = path,
            .hash = love::project::hash(path)
        };
            break;
    }
}
