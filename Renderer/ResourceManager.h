//
// Created by YAHAY on 17/12/2024.
//

#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H
#include <functional>
#include <vector>

#include "renderer_constants.h"
#include <volk.h>
#include <vk_mem_alloc.h>

#include "Renderer.h"
static std::vector<std::function<void()>> cleanups[MAX_INFLIGHT_FRAMES];
static VkCommandPool commandPools[MAX_INFLIGHT_FRAMES];
static int currentFrame = 0;
static void default_cleanup();
/**
 * defer for frame lifetime
 * @param fn cleanup function to run after all proccessing with current frame finishes
 */
void deferffl(std::function<void()> &&fn);
void advance_frame_and_execute_cleanups();
VkCommandBuffer make_cb_for_frame();
void init_frame_resource_manager();
#endif //RESOURCEMANAGER_H
