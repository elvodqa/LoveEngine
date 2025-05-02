#pragma once

#include <filesystem>
#include "Loaders/Image.h"
struct MemMesh;
class EngineImage;

namespace love {

enum class AssetType {
    texture=1,
    mesh=2,
    objectpack,
};

class Asset {
    public:
    AssetType type;
    std::filesystem::path relative_path;
    uint64_t hash;
    void* mem_ptr;
    // void* engine_object_ptr;
    void updated();

    static void loadObject(Asset* asset);
    static void unloadObject(Asset* asset);
    static void updateObject(Asset* asset);

    [[nodiscard]] __inline MemImage* asImage() const {return (MemImage*)mem_ptr;}
    [[nodiscard]] __inline MemMesh *asMesh() const {return (MemMesh*)mem_ptr;}
};

} // love


