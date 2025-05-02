//
// Created by YAHAY on 20/12/2024.
//

#include "ImageManager.h"

#include <vector>

#include "EngineImage.h"
#include "../debug_panic.h"
#include "Renderer.h"

void renderer::image_manager::init() {

    std::vector<VkDescriptorSetLayoutBinding> bindings ={
        {
            .binding = SAMPLER_BINDING,
            .descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLER,
            .descriptorCount = 10,
            .pImmutableSamplers = nullptr,
            .stageFlags = VK_SHADER_STAGE_ALL,
        },
        {
            .binding = IMAGE_BINDING,
            .descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .descriptorCount = 4000,
            .pImmutableSamplers = nullptr,
            .stageFlags = VK_SHADER_STAGE_ALL,
        },
    };
    VkDescriptorBindingFlags bindingFlags[] =
    {
        (VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT |
        VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT),
        (VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
        VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT)
    };
    VkDescriptorSetLayoutBindingFlagsCreateInfo bindflagCI = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
        .pBindingFlags = &bindingFlags[0],
        .bindingCount = (uint32_t)bindings.size(),
    };
    VkDescriptorSetLayoutCreateInfo imageLayoutCI =
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = (uint32_t)bindings.size(),
        .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT ,
        .pBindings = bindings.data(),
        .pNext = &bindflagCI,

    };

    vkc(vkCreateDescriptorSetLayout(renderer::device, &imageLayoutCI,renderer::g_vk_Allocator, &image_set_layout));
    std::vector poolsizes =
    {
        VkDescriptorPoolSize(VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLER, 16),
        VkDescriptorPoolSize(VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 50000),
    };
    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pPoolSizes = poolsizes.data(),
        .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
        .maxSets = 1,
        .poolSizeCount = (uint32_t)poolsizes.size(),
    };
    vkc(vkCreateDescriptorPool(renderer::device, &descriptorPoolCreateInfo, renderer::g_vk_Allocator, &pool));
    auto descriptorCount = 100;
    VkDescriptorSetVariableDescriptorCountAllocateInfo variableAllocInfo ={
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO,
        .descriptorSetCount = 1,
        .pDescriptorCounts = (uint32_t*) &descriptorCount,
    };
    VkDescriptorSetAllocateInfo descSetAllocInfo =
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = pool,
        .descriptorSetCount = 1,
        .pSetLayouts = &image_set_layout,
        .pNext = &variableAllocInfo,
    };

    vkc(vkAllocateDescriptorSets(renderer::device, &descSetAllocInfo, &descriptorset));


    VkSamplerCreateInfo defSamplerCI{
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .anisotropyEnable = false,
        .compareEnable = false,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .magFilter = VK_FILTER_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .mipLodBias = 0.0f,
        .compareOp = VK_COMPARE_OP_NEVER,
        .minLod = 0.0f,
        .maxLod = 0.0f,
        .unnormalizedCoordinates = VK_FALSE,
    };
    VkSampler sampler;
    vkc(vkCreateSampler(renderer::device,&defSamplerCI,renderer::g_vk_Allocator,&sampler));

        VkDescriptorImageInfo iminfo{.sampler = sampler,};
        VkWriteDescriptorSet writes={
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descriptorset,
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLER,
        .descriptorCount = 1,
        .pImageInfo = &iminfo,
        };
    vkUpdateDescriptorSets(renderer::device,1,&writes,0,nullptr);
}

/**
 * think as if this is free(), there is 0 checks for double freeing, oob freeing and others. have fun!
 * @param index
 */
void renderer::image_manager::evict_image_now(int index) {
    descriptors[index].next_free_slot = free_list_head;
    free_list_head = index;
}

/**
 *
 * @param image mip0 must have the image layout the image is going to be used with
 * @param read_time_image_layout
 * @return
 */
int renderer::image_manager::register_image(const EngineImage *image, VkImageLayout read_time_image_layout) {
    int index;
    if (free_list_head==-1) {
        descriptors.push_back(std::bit_cast<descriptorListSlot>(image));
        index = descriptors.size() - 1;
    }
    else {
        index = free_list_head;
        free_list_head=descriptors[free_list_head].next_free_slot;
        descriptors[index] =std::bit_cast<descriptorListSlot>(image);
    }
    VkDescriptorImageInfo descriptorImageInfo ={
        .sampler = VK_NULL_HANDLE,
        .imageView = image->image_view,
        .imageLayout = (read_time_image_layout!=VkImageLayout{})?read_time_image_layout:image->image_layout[0],
    };
    VkWriteDescriptorSet writeDescriptor ={
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descriptorset,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        .dstBinding = IMAGE_BINDING,
        .dstArrayElement = (uint32_t)index,
        .pImageInfo = &descriptorImageInfo,
    };
    vkUpdateDescriptorSets(renderer::device, 1, &writeDescriptor, 0, nullptr);
    return index;
}





