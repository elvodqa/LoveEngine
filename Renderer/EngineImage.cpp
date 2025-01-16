//
// Created by YAHAY on 17/12/2024.
//

#include "EngineImage.h"
#include <volk.h>
#include <bit>
#include <stb_image.h>
#include "../love_resource_locator.h"
#include "../debug_panic.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include <vk_mem_alloc.h>

EngineImage * EngineImage::createImage_unallocated(uint32_t width, uint32_t height, VkFormat format,
    VkImageUsageFlags usage, bool has_mips) {
    EngineImage * image = new EngineImage();
    uint32_t mipcount = has_mips?std::bit_width(std::max(width,height)):1;
    image->width = width;
    image->height = height;
    image->format = format;
    image->mipcount = mipcount;
    image->imageLayout.resize(mipcount);
    std::fill(image->imageLayout.begin(), image->imageLayout.end(), VK_IMAGE_LAYOUT_UNDEFINED);
    VkImageCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format,
        .extent = {width,height,1},
        .mipLevels = mipcount,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 1,
        .pQueueFamilyIndices = &renderer::g_QueueFamily,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };
    vkCreateImage(renderer::device, &info, renderer::g_vk_Allocator,&image->deviceImage);
    return image;
}

VkImageAspectFlags aspectFromFormat(VkFormat format) {
    switch (format) {
        case VK_FORMAT_D32_SFLOAT:
        case VK_FORMAT_D16_UNORM:
        return VK_IMAGE_ASPECT_DEPTH_BIT;
        default:
            return VK_IMAGE_ASPECT_COLOR_BIT;


    }
}

/**
 * creates image with backing allocated and a imageview
 */
EngineImage *EngineImage::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage,
                                      const VmaAllocationCreateInfo &alloc_info, bool has_mips) {
    auto* image=new EngineImage();

    uint32_t mipcount = has_mips?std::bit_width(std::max(width,height)):1;
    image->width = width;
    image->height = height;
    image->format = format;
    image->mipcount = mipcount;
    image->imageLayout.resize(mipcount);
    std::fill(image->imageLayout.begin(), image->imageLayout.end(), VK_IMAGE_LAYOUT_UNDEFINED);
    VkImageCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format,
        .extent = {width,height,1},
        .mipLevels = mipcount,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 1,
        .pQueueFamilyIndices = &renderer::g_QueueFamily,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };
    vmaCreateImage(renderer::vma_allocator,&info,&alloc_info,&image->deviceImage,&image->allocation,&image->allocInfo);
    VkImageViewCreateInfo viewInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .components = VK_COMPONENT_SWIZZLE_IDENTITY,
        .format = image->format,
        .image = *image,
        .subresourceRange = VkImageSubresourceRange{
            .aspectMask = aspectFromFormat(format),
            .baseMipLevel = 0,
            .levelCount = image->mipcount,
            .layerCount = 1,
            .baseArrayLayer = 0,
        },
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
    };
    vkCreateImageView(renderer::device,&viewInfo,renderer::g_vk_Allocator,&image->imageView);
    return  image;
}

VkImageView EngineImage::createAdditionalImageView(EngineImage &image) {
    VkImageViewCreateInfo viewInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .components = VK_COMPONENT_SWIZZLE_IDENTITY,
        .format = image.format,
        .image = image,
        .subresourceRange = VkImageSubresourceRange{
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = image.mipcount,
            .layerCount = 1,
            .baseArrayLayer = 0,
        },
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
    };
    VkImageView out;
    vkCreateImageView(renderer::device,&viewInfo,renderer::g_vk_Allocator,&out);
    return out;
}

EngineImage* EngineImage::make(VkCommandBuffer cb, ResourceLocator image_source, VkImageUsageFlags usage,bool generate_mips) {
    uint32_t width, height, channels;
    usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    stbi_info(image_source.path, (int*)&width, (int*)&height, (int*)&channels);


    auto format   = channels==4?VK_FORMAT_R8G8B8A8_SRGB:
                    channels==3?VK_FORMAT_R8G8B8_SRGB:
                    channels==2?VK_FORMAT_R8G8_SRGB:
                                VK_FORMAT_R8G8B8_SRGB;
    VmaAllocationCreateInfo vmaInfo = {
        .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
    };
    auto* image= createImage(width, height, format, usage, vmaInfo, generate_mips);


    uint8_t* lmem = stbi_load(image_source.path, (int*)&width, (int*)&height, (int*)&channels,15);

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
    memcpy(tmp_info.pMappedData,lmem,1/*8 bit srgb*/*width*height*channels);
    vmaFlushAllocation(renderer::vma_allocator,local_tmp_alloc,tmp_info.offset,tmp_info.size);
    stbi_image_free(lmem);
    image->ChangeImageLayout(cb,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,VK_PIPELINE_STAGE_TRANSFER_BIT,VK_ACCESS_TRANSFER_WRITE_BIT);
    VkBufferImageCopy region = {
        .bufferOffset = 0,
        .bufferRowLength = width,
        .bufferImageHeight = height,
        .imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT,0,0,1},
        .imageOffset = {0,0,0},
        .imageExtent = {width,height,1},
    };
    vkCmdCopyBufferToImage(cb,local_tmp,image->deviceImage,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,1, &region);

    return image;
}

void EngineImage::ChangeImageLayout(VkCommandBuffer cb, VkImageLayout newLayout,
                                    VkPipelineStageFlags dst_stage, VkAccessFlags dst_access, uint32_t mip_start, uint32_t mip_count) {
    auto oldlayout = imageLayout[mip_start];
    if (mip_count == (uint32_t)-1) mip_count = this->mipcount;
    VkImageMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = last_used_access,
        .dstAccessMask = dst_access,
        .oldLayout = oldlayout,
        .newLayout = newLayout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = deviceImage,
        .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT,mip_start,mip_count,0,1},
    };
    vkCmdPipelineBarrier(cb,last_used_stage,dst_stage,0,0,nullptr,0,nullptr,1,&barrier);
    for(uint32_t i=0;i<mip_count;i++) {
        if (oldlayout!=imageLayout[mip_start + i]) panic();
        imageLayout[mip_start + i] = newLayout;
    }
    last_used_access = dst_access;
    last_used_stage = dst_stage;
}