//
// Created by YAHAY on 17/12/2024.
//

#ifndef ENGINEIMAGE_H
#define ENGINEIMAGE_H
#include <vector>
#include <volk.h>
#include <vk_mem_alloc.h>

#include "../love_resource_locator.h"


class EngineImage {
    public:
    VkImage deviceImage;
    VkImageView imageView;
    VmaAllocation allocation;
    uint32_t width, height;
    std::vector<VkImageLayout> imageLayout;
    VmaAllocationInfo  alloc_info;
    VkFormat format;
    uint32_t mipcount;

private:
    ::EngineImage *make(struct ::VkCommandBuffer_T *cb, ResourceLocator image_source, VkImageUsageFlags usage, bool generate_mips);

    void ChangeImageLayout(VkCommandBuffer cb, VkImageLayout newLayout, VkPipelineStageFlags srcstage,
                           VkPipelineStageFlags dststage, uint32_t mipstart=0, uint32_t mipcount=-1);
};



#endif //ENGINEIMAGE_H
