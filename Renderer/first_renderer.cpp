//
// Created by YAHAY on 21/12/2024.
//
#include <entt/entt.hpp>
#include "first_renderer.h"
#include "../love.h"
#include <fstream>
#include <volk.h>
#include "ImageManager.h"
#include "MeshManager.h"
#include "ResourceManager.h"
#include "../ECS/Camera.h"
#include "../ECS/Mesh.h"
#include "../ECS/Transform.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
typedef uint32_t mesh;
typedef uint32_t transform;
typedef uint32_t camera;

void renderer::first_renderer::init() {
    int w,h;
    SDL_GetWindowSize(renderer::window,&w,&h);
    VmaAllocationCreateInfo vmaCreateInfo={
        .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
        .flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
    };
    auto color_format = VK_FORMAT_R8G8B8A8_SRGB;
    target = EngineImage::createImage(w,h,color_format,VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT|VK_IMAGE_USAGE_TRANSFER_SRC_BIT|VK_IMAGE_USAGE_SAMPLED_BIT,vmaCreateInfo,false);
    depth = EngineImage::createImage(w,h,VK_FORMAT_D32_SFLOAT,VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT|VK_IMAGE_USAGE_TRANSFER_SRC_BIT,vmaCreateInfo,false);

    render_output_image_id = image_manager::register_image(target,VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts({
        renderer::image_manager::image_set_layout,
    });
    VkPushConstantRange pushranges[]={{
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .offset = 0,
        .size = 32*sizeof(float)
    },
    {
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        .offset = 32*sizeof(float),
        .size = 4*sizeof(uint32_t)
    }
    };
    VkPipelineLayoutCreateInfo pipelinelayoutcreate={
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .setLayoutCount = (uint32_t)descriptorSetLayouts.size(),
        .pSetLayouts = descriptorSetLayouts.data(),
        .flags = 0,
        .pPushConstantRanges = pushranges,
        .pushConstantRangeCount = 2,
    };
    vkCreatePipelineLayout(renderer::device,&pipelinelayoutcreate,renderer::g_vk_Allocator,&default_pipeline_layout);

    const VkPipelineRenderingCreateInfoKHR pipeline_rendering_create_info {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
        .colorAttachmentCount = 1,
        .depthAttachmentFormat = VK_FORMAT_D32_SFLOAT,
        .pColorAttachmentFormats = &color_format,
        .viewMask = 0,
    };
    std::ifstream vfile("D:\\GitD\\LoveEngine\\shaders\\first.vert.spv", std::ios::binary | std::ios::ate);
    auto vsize = vfile.tellg();
    vfile.seekg(0);
    std::vector<char> vert_spv_buffer(vsize);
    vfile.read(vert_spv_buffer.data(), vsize);

    std::ifstream ffile("D:\\GitD\\LoveEngine\\shaders\\first.frag.spv", std::ios::binary | std::ios::ate);
    auto fsize = ffile.tellg();
    ffile.seekg(0);
    std::vector<char> frag_spv_buffer(fsize);
    ffile.read(frag_spv_buffer.data(), fsize);
    VkShaderModuleCreateInfo vert_mod={
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = vert_spv_buffer.size(),
        .pCode = (uint32_t*)vert_spv_buffer.data(),
    }, frag_mod={
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = frag_spv_buffer.size(),
        .pCode = (uint32_t*)frag_spv_buffer.data(),
    };

    VkPipelineShaderStageCreateInfo stages[2] {
      {
          .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
          .flags = 0,
          .stage = VK_SHADER_STAGE_VERTEX_BIT,
          .pName = "main",
          .module = nullptr,
          .pNext = &vert_mod,
      },
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .flags = 0,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pName = "main",
            .module = nullptr,
            .pNext = &frag_mod,
        }
    };
    VkDynamicState dynamicStateList[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamicStates={
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
    .dynamicStateCount = 2,
    .pDynamicStates = dynamicStateList,
    };
    VkPipelineMultisampleStateCreateInfo multisampleState={
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE,
    };
    VkPipelineRasterizationStateCreateInfo rasterizationState={
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .lineWidth = 1.0f,
        .depthBiasEnable = VK_FALSE,
        .flags = 0,
    };
    VkPipelineTessellationStateCreateInfo tessellationState={
        .sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
        .patchControlPoints = 1,
    };
    VkPipelineViewportStateCreateInfo viewportState={
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount = 1,
    };
    VkPipelineColorBlendAttachmentState colorBlendAttachment={
        .blendEnable = VK_FALSE,
        .colorWriteMask = VK_COLOR_COMPONENT_FLAG_BITS_MAX_ENUM,
    };
    VkPipelineColorBlendStateCreateInfo colorBlendState={
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment,
    };
    VkPipelineDepthStencilStateCreateInfo depthStencilState={
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = VK_FALSE,
        .minDepthBounds = 0.0f,
        .maxDepthBounds = 1.0f,
        .stencilTestEnable = VK_FALSE,
    };
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState={
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
        .flags = 0,
    };

    VkVertexInputBindingDescription bindingDescriptions[4] = {
        {//pos
            .binding = 0,
            .stride = sizeof(float) * 3,
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
        },
        {//UV
            .binding = 1,
            .stride = sizeof(float) * 2,
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
        },
        {//normal
            .binding = 2,
            .stride = sizeof(float) * 3,
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
        },
        {//tangent
            .binding = 3,
            .stride = sizeof(float) * 3,
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
        },
    };
    VkVertexInputAttributeDescription attributeDescriptions[4] = {
        {
            .location = 0,
            .binding = 0,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = 0,
        },
        {
            .location = 1,
            .binding = 1,
            .format = VK_FORMAT_R32G32_SFLOAT,
            .offset = 0,
        },
        {
            .location = 2,
            .binding = 2,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = 0,
        },
        {
            .location = 3,
            .binding = 3,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = 0,
        },
    };

    VkPipelineVertexInputStateCreateInfo vertexInputState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .vertexBindingDescriptionCount = 4,     // Four bindings: position, UV, normal, tangent
        .pVertexBindingDescriptions = bindingDescriptions,
        .vertexAttributeDescriptionCount = 4,   // Four attributes: position, UV, normal, tangent
        .pVertexAttributeDescriptions = attributeDescriptions,
    };

    VkGraphicsPipelineCreateInfo gfxpipelinecreateinfo={
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = &pipeline_rendering_create_info,
        .flags = 0,
        .renderPass = nullptr,
        .layout = default_pipeline_layout,
        .subpass = 0,
        .stageCount = 2,
        .pStages = stages,
        .pDynamicState = &dynamicStates,
        .pMultisampleState = &multisampleState,
        .pRasterizationState = &rasterizationState,
        .pTessellationState = &tessellationState,
        .pViewportState = &viewportState,
        .pColorBlendState = &colorBlendState,
        .pDepthStencilState = &depthStencilState,
        .pInputAssemblyState = &inputAssemblyState,
        .pVertexInputState = &vertexInputState,
    };
    vkCreateGraphicsPipelines(::renderer::device,renderer::g_PipelineCache,1,&gfxpipelinecreateinfo,renderer::g_vk_Allocator,&default_pipeline);

    VkSemaphoreCreateInfo semCI{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
    };
    vkCreateSemaphore(renderer::device,&semCI,renderer::g_vk_Allocator,&renderer::first_renderer::r2r);
    vkCreateSemaphore(renderer::device,&semCI,renderer::g_vk_Allocator,&renderer::first_renderer::r2b);

}

void renderer::first_renderer::drawFrame(uint32_t width, uint32_t height, VkSemaphore wait_semaphore_ready2render, VkSemaphore signal_sempahore_ready2blit) {
    if (width > target->width || height > target->height) {
        uint32_t w,h;
        SDL_GetWindowSize(renderer::window,(int*)&w,(int*)&h);
        w= std::max(w,width);
        h= std::max(h,height);
        VmaAllocationCreateInfo vmaCreateInfo={
            .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
            .flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
        };
        auto color_format = VK_FORMAT_R8G8B8A8_SRGB;
        {
            auto depth=first_renderer::depth;
            auto target=first_renderer::target;
            auto render_output_image_id = first_renderer::render_output_image_id;
            deferffl([target,depth,render_output_image_id]() {
                EngineImage::destroyImage(target);
                EngineImage::destroyImage(depth);
                image_manager::evict_image_now(render_output_image_id);
            });
        }
        target = EngineImage::createImage(w,h,color_format,VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT|VK_IMAGE_USAGE_TRANSFER_SRC_BIT|VK_IMAGE_USAGE_SAMPLED_BIT,vmaCreateInfo,false);
        depth = EngineImage::createImage(w,h,VK_FORMAT_D32_SFLOAT,VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT|VK_IMAGE_USAGE_TRANSFER_SRC_BIT,vmaCreateInfo,false);

        render_output_image_id = image_manager::register_image(target,VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }
    //
    // entt::registry registry;
    auto regview = love::registry.view<Mesh,Transform>();

    // view.each([&](auto entity, auto mesh) {
    // });
    camera cam;
    auto cb = make_cb_for_frame();
    VkCommandBufferBeginInfo beginInfo={
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    vkc(vkBeginCommandBuffer(cb,&beginInfo));
    VkRenderingAttachmentInfo colorAttachment={
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .clearValue = VkClearValue{.4,.5,.8,1},
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .imageView = *target,
    };
    VkRenderingAttachmentInfo depthAttachment={
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .clearValue = VkClearValue{1.f},
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        .imageView = *depth,
    };
    VkRenderingInfo renderingInfo={
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .flags = 0,
        .layerCount = 1,
        .renderArea = {0,0,width,height},
        .viewMask = 0,
        .colorAttachmentCount = 1,
        .pColorAttachments =&colorAttachment,
        .pDepthAttachment = &depthAttachment,
    };
    target->ChangeImageLayout(cb,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
    if (depth->image_layout[0]!=VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) depth->ChangeImageLayout(cb,VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT|VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);
    vkCmdBeginRendering(cb,&renderingInfo);
    vkCmdBindPipeline(cb,VK_PIPELINE_BIND_POINT_GRAPHICS,default_pipeline);
    vkCmdBindIndexBuffer(cb,*mesh_manager::static_ib,0,VK_INDEX_TYPE_UINT32);
    VkBuffer vertexBuffers[4]={*mesh_manager::static_vb,*mesh_manager::static_uvb,*mesh_manager::static_normalb,*mesh_manager::static_tanb};
    constexpr VkDeviceSize offsets[4] = {0, 0, 0, 0};
    vkCmdBindVertexBuffers(cb,0,4,vertexBuffers,offsets);

    VkRect2D sci={.offset = {0,0}, .extent = {width,height}};
    VkViewport viewport={.x=0, .y=0, .width=(float)width, .height=(float)height, .minDepth=0, .maxDepth=1.f};
    vkCmdSetScissor(cb,0,1,&sci);
    vkCmdSetViewport(cb,0,1,&viewport);
    auto[cameraComp,TransformComp]=love::registry.get<Camera,Transform>(love::registry.view<Camera>().front());

    auto proj = glm::perspective(cameraComp.fovy, ((float)width)/(float)height, cameraComp.nearPlane, cameraComp.farPlane);
    proj[1][1] *= -1; // gl -> vk
    auto view = glm::lookAt(TransformComp.translation,TransformComp.forward(),glm::vec3(0.f,0.f,1.f));
    auto viewproj = proj * view;
    vkCmdPushConstants(cb,default_pipeline_layout,VK_SHADER_STAGE_VERTEX_BIT,0,16*sizeof(float),&viewproj);
    auto defaultmodel = glm::mat4(1.f);
if (!image_manager::descriptorset) panic("");
    vkCmdPushConstants(cb,default_pipeline_layout,VK_SHADER_STAGE_VERTEX_BIT,16*sizeof(float), 16*sizeof(float),&defaultmodel);
    vkCmdBindDescriptorSets(cb,VK_PIPELINE_BIND_POINT_GRAPHICS,default_pipeline_layout,0,1, &image_manager::descriptorset,0,nullptr);

    {
        regview.each([&](auto id,Mesh& meshComp,Transform& transform) {
            auto mesh = mesh_manager::meshes[meshComp.meshID];
            auto mod=transform.getTransformMatrix();

            vkCmdPushConstants(cb,default_pipeline_layout,VK_SHADER_STAGE_VERTEX_BIT,16*sizeof(float), 16*sizeof(float),&mod);
            uint32_t texID=0;
            vkCmdPushConstants(cb,default_pipeline_layout,VK_SHADER_STAGE_FRAGMENT_BIT,32*sizeof(float),1*sizeof(uint32_t),&renderer::first_renderer::DBG_UVTEX);

            vkCmdDrawIndexed(cb,mesh.index_count,1,mesh.index_offset,mesh.vertex_offset,0);

        });
    }
    vkCmdEndRendering(cb);
    target->ChangeImageLayout(cb,VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,VK_PIPELINE_STAGE_TRANSFER_BIT,VK_ACCESS_TRANSFER_READ_BIT);

    vkc(vkEndCommandBuffer(cb));
    VkPipelineStageFlags x = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkSubmitInfo subInfo={
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &cb,
        .waitSemaphoreCount = wait_semaphore_ready2render==VK_NULL_HANDLE?0u:1u,
        .pWaitSemaphores = wait_semaphore_ready2render==VK_NULL_HANDLE?nullptr:&wait_semaphore_ready2render,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &signal_sempahore_ready2blit,
        .pWaitDstStageMask = &x,
    };
    vkc(vkQueueSubmit(renderer::g_Queue,1,&subInfo,VK_NULL_HANDLE));



}
