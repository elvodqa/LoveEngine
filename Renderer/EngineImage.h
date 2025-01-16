//
// Created by YAHAY on 17/12/2024.
//

#ifndef ENGINEIMAGE_H
#define ENGINEIMAGE_H
#include <vector>
#include <volk.h>
#include <vk_mem_alloc.h>

#include "Renderer.h"
#include "../love_resource_locator.h"


class EngineImage {
    public:
    VkImage deviceImage;
    VkImageView imageView;
    VmaAllocation allocation;
    uint32_t width, height;
    std::vector<VkImageLayout> imageLayout;
    VmaAllocationInfo  allocInfo;
    VkFormat format;
    uint32_t mipcount;
    VkExtent2D size;
    VkPipelineStageFlags last_used_stage;
    VkAccessFlags last_used_access;
    // uint32_t dirty_offset,dirty_mip_count;


    EngineImage() {
        last_used_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        last_used_access = VK_ACCESS_NONE;
    }
    operator VkImage&() {return deviceImage;}
    operator VkImageView&() {return imageView;}
    static EngineImage *createImage_unallocated(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, bool has_mips);
    static EngineImage *createImage(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, const VmaAllocationCreateInfo &alloc_info, bool has_mips);
    static VkImageView createAdditionalImageView(EngineImage &image);
    static void destroyImage(EngineImage *image);
    void ChangeImageLayout(VkCommandBuffer cb, VkImageLayout newLayout,
                           VkPipelineStageFlags dst_stage, VkAccessFlags dst_access, uint32_t mip_start=0, uint32_t mip_count=-1);
private:
    ::EngineImage *make(struct ::VkCommandBuffer_T *cb, ResourceLocator image_source, VkImageUsageFlags usage, bool generate_mips);


};



#endif //ENGINEIMAGE_H
