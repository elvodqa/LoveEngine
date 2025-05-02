#pragma once
#include <vk_mem_alloc.h>
#include "Image.h"
#include "../../Renderer/EngineImage.h"


namespace love {
  class Asset;
}

struct MemImage{
  uint8_t* stbi_ptr;
  uint32_t width;
  uint32_t height;
  uint32_t channels;
  EngineImage* load(VkCommandBuffer cb, VkImageUsageFlags flags, bool createMips, const VmaAllocationCreateInfo &alloc_info);

  static MemImage* loadImagefromAsset(love::Asset* asset);

  void loadTo(VkCommandBuffer cb, EngineImage* image);
  private:
  ~MemImage();
};

// MemImage* loadImagefromAsset(love::Asset* asset);
