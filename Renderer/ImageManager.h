#ifndef IMAGEMANAGER_H
#define IMAGEMANAGER_H
#include <vector>
#include <volk.h>
#include <vk_mem_alloc.h>

#include "EngineImage.h"
namespace renderer::image_manager {
    typedef union{
        int next_free_slot;
        const EngineImage* ptr;
    }descriptorListSlot;
    static VkDescriptorPool pool;
    inline VkDescriptorSetLayout imageSetLayout;
    static std::vector<descriptorListSlot> descriptors;
    static VkDescriptorSet descriptorset;
    static int free_list_head = -1;
    constexpr int SAMPLER_BINDING=0;
    constexpr int IMAGE_BINDING=1;
    void init();
    void evict_image_now(int image);
    int register_image(const EngineImage* image, VkImageLayout read_time_image_layout={});

}
#endif //IMAGEMANAGER_H
