//
// Created by YAHAY on 17/12/2024.
//

#ifndef ENGINEIMAGE_H
#define ENGINEIMAGE_H
#include <vector>
#include <volk.h>


class EngineImage {
    public:
    VkImage deviceImage;
    VkImageView imageView;
    VkDeviceMemory deviceMemory;
    uint32_t width, height;
    std::vector<VkImageLayout> imageLayout;
    private:
    EngineImage(): deviceImage(nullptr), imageView(nullptr), deviceMemory(nullptr), width(0), height(0) {};

    EngineImage make();
};



#endif //ENGINEIMAGE_H
