//
// Created by YAHAY on 21/12/2024.
//

#ifndef FIRSTRENDERER_H
#define FIRSTRENDERER_H
#include "EngineBuffer.h"
#include "EngineImage.h"

namespace renderer::first_renderer {
    inline EngineImage *target;
    inline EngineImage *depth;
    inline int render_output_image_id;
    inline VkPipelineLayout default_pipeline_layout;
    inline VkPipeline default_pipeline;
    inline VkSemaphore r2r,r2b;
    void init();
    void drawFrame(uint32_t width, uint32_t height, VkSemaphore wait_semaphore_ready2render, VkSemaphore signal_sempahore_ready2blit);

}
#endif //FIRSTRENDERER_H
