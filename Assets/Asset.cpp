
#include "Asset.h"
#include "Loaders/Image.h"
#include "debug_panic.h"
#include "Loaders/Mesh.h"

// #include "../../Renderer/EngineImage.h"
namespace love {
    void Asset::updated() {
        (void)this->hash;
        this->hash = this->hash;
        __asm__("nop");
    }

    void Asset::loadObject(Asset *asset) {
        switch (asset->type) {
            case AssetType::texture:
                asset->mem_ptr=MemImage::loadImagefromAsset(asset);
                break;
            case AssetType::objectpack:
                break;
            case AssetType::mesh:
                asset->mem_ptr=MemMesh::loadMeshfromAsset(asset);
                break;
            default:
                panic("Unsupported Asset Type");
            return;
        }
    }

}
