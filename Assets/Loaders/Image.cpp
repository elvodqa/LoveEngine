#include "../Asset.h"
#include <stb_image.h>
#include "Image.h"
#include "../../Renderer/EngineImage.h"
#include "../../debug_panic.h"
#include "../../Renderer/ResourceManager.h"
#include "ProjectManager/projectManager.h"
#include "Renderer/ImageManager.h"

MemImage* MemImage::loadImagefromAsset(love::Asset* asset) {
    auto first = asset->relative_path.begin();
    if (first == asset->relative_path.end()) {
        panic("no path to load");
    }
    if (*first==":mem:"||*first==":pak:"||*first==":net:") panic("not implemented");
    if (asset->relative_path.empty())panic("no path to load");
    uint32_t width, height, channels;
    auto ptr = stbi_load((char*)((love::project::loaded_project_path/asset->relative_path).u8string().c_str()), (int*)&width, (int*)&height, (int*)&channels,0);
    if (!ptr) panic("failed to load image {}",stbi_failure_reason() );
    return new MemImage{
        .stbi_ptr=ptr,
        .width=width,
        .height=height,
        .channels=channels};

}
EngineImage *MemImage::load(VkCommandBuffer cb, VkImageUsageFlags flags, bool createMips, const VmaAllocationCreateInfo &alloc_info) {
    auto format   = channels==4?VK_FORMAT_R8G8B8A8_SRGB:
                channels==3?VK_FORMAT_R8G8B8_SRGB:
                channels==2?VK_FORMAT_R8G8_SRGB:
                            VK_FORMAT_R8G8B8_SRGB;

    auto image = EngineImage::createImage(width,height,format,flags|VK_IMAGE_USAGE_TRANSFER_DST_BIT,alloc_info,createMips);
    loadTo(cb,image);
    return image;
}

void MemImage::loadTo(VkCommandBuffer cb, EngineImage *image) {
    if ((image->height=!height||image->width!=width)) panic("size mismatch");
    auto format   = channels==4?VK_FORMAT_R8G8B8A8_SRGB:
            channels==3?VK_FORMAT_R8G8B8_SRGB:
            channels==2?VK_FORMAT_R8G8_SRGB:
                        VK_FORMAT_R8G8B8_SRGB;
    if (image->format!=format) panic("format mismatch");
    VmaAllocationCreateInfo vmaInfo={};
    VkBuffer local_tmp;
    VmaAllocation local_tmp_alloc;
    VmaAllocationInfo tmp_info;
    vmaInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
    vmaInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT|VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
    vmaInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
    VkBufferCreateInfo buffer_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .flags = 0,
        .size = width*height*channels,
        .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 1,
        .pQueueFamilyIndices = &renderer::g_QueueFamily,
    };
    vmaCreateBuffer(renderer::vma_allocator,&buffer_info,&vmaInfo,&local_tmp,&local_tmp_alloc,&tmp_info);
    deferffl([local_tmp, local_tmp_alloc]{vmaDestroyBuffer(renderer::vma_allocator,local_tmp,local_tmp_alloc);});
    memcpy(tmp_info.pMappedData,stbi_ptr,1/*8 bit srgb*/*width*height*channels);
    vmaFlushAllocation(renderer::vma_allocator,local_tmp_alloc,tmp_info.offset,tmp_info.size);
    image->ChangeImageLayout(cb,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,VK_PIPELINE_STAGE_TRANSFER_BIT,VK_ACCESS_TRANSFER_WRITE_BIT);
    VkBufferImageCopy region = {
        .bufferOffset = 0,
        .bufferRowLength = width,
        .bufferImageHeight = height,
        .imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT,0,0,1},
        .imageOffset = {0,0,0},
        .imageExtent = {width,height,1},
    };
    vkCmdCopyBufferToImage(cb,local_tmp,image->device_image,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,1, &region);

}

MemImage::~MemImage() {
    stbi_image_free(stbi_ptr);
}
