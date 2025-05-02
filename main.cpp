// Dear ImGui: standalone example application for SDL3 + Vulkan

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

// Important note to the reader who wish to integrate imgui_impl_vulkan.cpp/.h in their own engine/app.
// - Common ImGui_ImplVulkan_XXX functions and structures are used to interface with imgui_impl_vulkan.cpp/.h.
//   You will use those if you want to use this rendering backend in your engine/app.
    // - Helper ImGui_ImplVulkanH_XXX functions and structures are only used by this example (main.cpp) and by
//   the backend itself (imgui_impl_vulkan.cpp), but should PROBABLY NOT be used by your own engine/app code.
// Read comments in imgui_impl_vulkan.h.
//#define VK_USE_PLATFORM_WIN32_KHR
#define STB_IMAGE_IMPLEMENTATION
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#define IMGUI_IMPL_VULKAN_USE_VOLK
#define VOLK_IMPLEMENTATION
#define VMA_IMPLEMENTATION
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#ifdef _DEBUG
#define APP_USE_VULKAN_DEBUG_REPORT
#endif
#include "imgui.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_vulkan.h"
#include <volk.h>
#include "vk_mem_alloc.h"
#include "Renderer/first_renderer.h"
#include <cstdio>
#include <SDL3/SDL.h>
#include "debug_panic.h"
#include <iostream>
#include <entt/entt.hpp>
#include "love.h"
#include "Assets/Loaders/Mesh.h"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "assimp/Importer.hpp"
#include "ECS/Camera.h"
#include "ECS/Mesh.h"
#include "ECS/Transform.h"
#include "ECS/Tree.h"
#include "editor/editor.hpp"
#include "ProjectManager/projectManager.h"
#include "Renderer/ImageManager.h"
#include "Renderer/MeshManager.h"
#include "Renderer/Renderer.h"
#include "Renderer/ResourceManager.h"


static love::Editor* editor;

static void check_vk_result(VkResult err)
{

    if (err == 0)
        return;
    panic("[vulkan] Error: VkResult = {}", (int)err);

}

static void FrameRender(ImGui_ImplVulkanH_Window* wd, ImDrawData* draw_data)
{
    VkResult err;

    VkSemaphore image_acquired_semaphore  = wd->FrameSemaphores[wd->SemaphoreIndex].ImageAcquiredSemaphore;
    VkSemaphore render_complete_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
    err = vkAcquireNextImageKHR(renderer::device, wd->Swapchain, UINT64_MAX, image_acquired_semaphore, VK_NULL_HANDLE, &wd->FrameIndex);
    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
    {
        renderer::g_SwapChainRebuild = true;
        return;
    }
    check_vk_result(err);

    ImGui_ImplVulkanH_Frame* fd = &wd->Frames[wd->FrameIndex];
    {
        err = vkWaitForFences(renderer::device, 1, &fd->Fence, VK_TRUE, UINT64_MAX);    // wait indefinitely instead of periodically checking
        check_vk_result(err);

        err = vkResetFences(renderer::device, 1, &fd->Fence);
        check_vk_result(err);
    }
    {
        err = vkResetCommandPool(renderer::device, fd->CommandPool, 0);
        check_vk_result(err);
        VkCommandBufferBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        err = vkBeginCommandBuffer(fd->CommandBuffer, &info);
        check_vk_result(err);
    }
    int xoff;
    int yoff;
    int w,h;
    {
        xoff = love::editor::wxmn;
        yoff = love::editor::wymn;
        h = (((int)love::editor::wymx-yoff));
        w = (((int)love::editor::wxmx-xoff));
        if (xoff < 0||yoff < 0||xoff+w > wd->Width||yoff+h > wd->Height) {
            xoff = 0;
            yoff = 0;
            w = wd->Width;
            h = wd->Height;
        }

    }
    {
        renderer::first_renderer::drawFrame(w,h,get_frame_no()==0?VK_NULL_HANDLE:renderer::first_renderer::r2r,renderer::first_renderer::r2b);
        VkImageMemoryBarrier barrier = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .srcAccessMask = VK_ACCESS_MEMORY_READ_BIT,
            .dstAccessMask = VK_ACCESS_MEMORY_WRITE_BIT,
            .image = fd->Backbuffer,
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        };
        vkCmdPipelineBarrier(fd->CommandBuffer,VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,VK_PIPELINE_STAGE_TRANSFER_BIT,0,0,nullptr,0,nullptr,1,&barrier);//trasition to dst opt
        VkImageBlit blit={
            .srcOffsets = {{0,0,0},{w,h,1}},
            .srcSubresource = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 1},
            .dstOffsets = {{xoff,yoff,0},{w+xoff,h+yoff,1}},
            .dstSubresource = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 1},

        };
        vkCmdBlitImage(fd->CommandBuffer,*renderer::first_renderer::target,VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,fd->Backbuffer,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,1,&blit,VK_FILTER_NEAREST);
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        vkCmdPipelineBarrier(fd->CommandBuffer,VK_PIPELINE_STAGE_TRANSFER_BIT,VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT|VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,0,0,nullptr,0,nullptr,1,&barrier);//trasition to dst opt

    }



    {
        VkRenderPassBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        info.renderPass = wd->RenderPass;
        info.framebuffer = fd->Framebuffer;
        info.renderArea.extent.width = wd->Width;
        info.renderArea.extent.height = wd->Height;
        info.clearValueCount = 1;
        info.pClearValues = &wd->ClearValue;
        vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
    }

    // Record dear imgui primitives into command buffer
    ImGui_ImplVulkan_RenderDrawData(draw_data, fd->CommandBuffer);

    // Submit command buffer
    vkCmdEndRenderPass(fd->CommandBuffer);
    {
        VkPipelineStageFlags wait_stage[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT|VK_PIPELINE_STAGE_TRANSFER_BIT,VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT|VK_PIPELINE_STAGE_TRANSFER_BIT};
        VkSubmitInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        info.waitSemaphoreCount = 2;
         VkSemaphore waits[] = {image_acquired_semaphore,renderer::first_renderer::r2b};
        info.pWaitSemaphores = waits;

        info.pWaitDstStageMask = wait_stage;
        info.commandBufferCount = 1;
        info.pCommandBuffers = &fd->CommandBuffer;
        info.signalSemaphoreCount = 2;
         VkSemaphore signals[] = {render_complete_semaphore,renderer::first_renderer::r2r};
        info.pSignalSemaphores = signals;

        err = vkEndCommandBuffer(fd->CommandBuffer);
        check_vk_result(err);
        err = vkQueueSubmit(renderer::g_Queue, 1, &info, fd->Fence);
        check_vk_result(err);
    }
}

static void FramePresent(ImGui_ImplVulkanH_Window* wd)
{
    if (renderer::g_SwapChainRebuild)
        return;
    VkSemaphore render_complete_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
    VkPresentInfoKHR info = {};
    info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    info.waitSemaphoreCount = 1;
    info.pWaitSemaphores = &render_complete_semaphore;
    info.swapchainCount = 1;
    info.pSwapchains = &wd->Swapchain;
    info.pImageIndices = &wd->FrameIndex;
    VkResult err = vkQueuePresentKHR(renderer::g_Queue, &info);
    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
    {
        renderer::g_SwapChainRebuild = true;
        return;
    }
    check_vk_result(err);
    wd->SemaphoreIndex = (wd->SemaphoreIndex + 1) % wd->SemaphoreCount; // Now we can use the next set of semaphores
}

// Main code
int main(int arg_count, char** args)
{
    fs::path pj;
    for (int i = 0; i < arg_count; ++i) {
        if (strcmp(args[i],"--project")==0&&i < arg_count - 1) {
                        pj = args[i+1];
        }
            std::cout<<args[i]<<std::endl;
    }
    // entt::registry registry;

    SDL_SetLogPriorities(SDL_LogPriority::SDL_LOG_PRIORITY_DEBUG);

    // Setup SDL
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD) != 0)
    {
        SDL_Log("Error: SDL_Init(): %s\n", SDL_GetError());
        return -1;
    }
    renderer::init();
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForVulkan(renderer::window);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = renderer::vk_Instance;
    init_info.PhysicalDevice = renderer::g_PhysicalDevice;
    init_info.Device = renderer::device;
    init_info.QueueFamily = renderer::g_QueueFamily;
    init_info.Queue = renderer::g_Queue;
    init_info.PipelineCache = renderer::g_PipelineCache;
    init_info.DescriptorPool = renderer::imgui_DescriptorPool;
    init_info.RenderPass = renderer::imgui::wd->RenderPass;
    init_info.Subpass = 0;
    init_info.MinImageCount = renderer::g_MinImageCount;
    init_info.ImageCount = renderer::imgui::wd->ImageCount;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator = renderer::g_vk_Allocator;
    init_info.CheckVkResultFn = check_vk_result;
    ImGui_ImplVulkan_Init(&init_info);


    // ENGINE
    renderer::mesh_manager::init();
    renderer::image_manager::init();
    renderer::first_renderer::init();
    love::editor::editor_init();

    Assimp::Importer Importer;

    auto cb = make_cb_for_frame();
    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    vkBeginCommandBuffer(cb, &begin_info);
    // assert(file->mNumMeshes==1);
    // for (auto i = 0; i < file->mNumMeshes; i++) {
    //     auto mesh = file->mMeshes[i];
    //     auto IB = new uint32_t[mesh->mNumFaces*3];
    //     for (int j = 0; j < mesh->mNumFaces; ++j) {
    //         auto face = mesh->mFaces[j];
    //         for (int k = 0; k < 3; ++k) {
    //             IB[3*j+k]=face.mIndices[k];
    //         }
    //     }
    //     float* zart = new float[mesh->mNumVertices*2];
    //     for (int j = 0; j < mesh->mNumVertices; ++j) {zart[2*j]=mesh->mVertices[j].x;zart[2*j+1]=mesh->mVertices[j].y;}
    //     delete[] zart;
    //     delete[] IB;
    // }
    love::project::loadProject(pj);
    auto asset = love::project::getOrLoadAsset(R"(dragon_recon\blender out1.ply)");
    love::Asset::loadObject(asset);
    auto mesh = asset->asMesh();
    uint32_t mesh_ID = mesh->load_via(cb,renderer::mesh_manager::load_static_mesh)[0];
    auto UVTexAsset=love::project::getOrLoadAsset("UV.png");
    love::Asset::loadObject(UVTexAsset);
    auto UvTexID= renderer::image_manager::register_image(UVTexAsset->asImage()->load(cb,VK_IMAGE_USAGE_SAMPLED_BIT,false,{.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE}),VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    renderer::first_renderer::DBG_UVTEX=UvTexID;
    vkEndCommandBuffer(cb);
    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &cb,
    };
    vkQueueSubmit(renderer::g_Queue, 1, &submit_info, renderer::thread0_load_fence);
    vkWaitForFences(renderer::device, 1, &renderer::thread0_load_fence, VK_TRUE, UINT64_MAX);
    vkResetFences(renderer::device, 1, &renderer::thread0_load_fence);

    auto entity = love::registry.create();
    love::registry.emplace<Transform>(entity,glm::vec3(0,0,0),glm::quat(0,0,0,1),glm::vec3(1,1,1));
    love::registry.emplace<Mesh>(entity,mesh_ID);
    for (int i = 0; i < 24; ++i) {
        auto entity = love::registry.create();
        love::registry.emplace<Transform>(
            entity,glm::vec3(2.5*sin(3.1415*i/12.f),-2.5*cos(3.1415*i/12.f), 0),
            glm::angleAxis(3.1415f*i/12.f,glm::vec3(0,0,1.f)),
            glm::vec3(.25,.25,.25));
        love::registry.emplace<Mesh>(entity,mesh_ID);
    }
    auto cam = love::registry.create();
    love::registry.emplace<Transform>(cam,glm::vec3(0,0,0),glm::quat(0,0,0,1),glm::vec3(1,1,1));
    float deg=90;
    float fovy=deg * 3.1415f/180.f;
    love::registry.emplace<Camera>(cam,fovy,0.01f,10000.f);
    // /ENGINE



    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != nullptr);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    editor = new love::Editor(renderer::window);
    editor->log(love::editor::LogType::Trace, "I'm a trace message");
    editor->log(love::editor::LogType::Warn, "I'm a warning!! message");
    editor->log(love::editor::LogType::Info, "I'm informing you");
    editor->log(love::editor::LogType::Error, "THIS PROGRAM IS BLOWING UP ERROR");
    editor->log(love::editor::LogType::Debug, "Debugging started");

    int fr=0;
    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        static bool low_fps = true;//checkbox @ IMGUI scope down
        SDL_Event event;
        if (low_fps)SDL_WaitEvent(nullptr);
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL3_ProcessEvent(&event);
            editor->check_events(&event);
            switch (event.type) {
                case SDL_EVENT_KEY_UP:
                case SDL_EVENT_KEY_DOWN:
                case SDL_EVENT_MOUSE_WHEEL:
                case SDL_EVENT_MOUSE_MOTION:
                case SDL_EVENT_MOUSE_BUTTON_UP:
                case SDL_EVENT_MOUSE_BUTTON_DOWN:
                    break;
                default:
                    break;
            }
            if (event.type == SDL_EVENT_QUIT)
                done = true;
            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(renderer::window))
                done = true;
        }
        if (SDL_GetWindowFlags(renderer::window) & SDL_WINDOW_MINIMIZED)
        {
            SDL_Delay(10);
            continue;
        }

        // Resize swap chain?
        int fb_width, fb_height;
        SDL_GetWindowSize(renderer::window, &fb_width, &fb_height);
        if (fb_width > 0 && fb_height > 0 && (renderer::g_SwapChainRebuild || renderer::imgui::imgui_MainWindowData.Width != fb_width || renderer::imgui::imgui_MainWindowData.Height != fb_height))
        {
            ImGui_ImplVulkan_SetMinImageCount(renderer::g_MinImageCount);
            ImGui_ImplVulkanH_CreateOrResizeWindow(renderer::vk_Instance, renderer::g_PhysicalDevice, renderer::device, &renderer::imgui::imgui_MainWindowData, renderer::g_QueueFamily, renderer::g_vk_Allocator, fb_width, fb_height, renderer::g_MinImageCount);
            renderer::imgui::imgui_MainWindowData.FrameIndex = 0;
            renderer::g_SwapChainRebuild = false;
        }

        {
            love::registry.get<Transform>(cam)={(glm::vec3(2*sin(get_frame_no()/240.), -2*cos(get_frame_no()/240.), 0)),glm::quatLookAt((glm::vec3(-sin(get_frame_no()/240.), cos(get_frame_no()/240.), 0)),glm::vec3(0.f,0.f,1.f)),glm::vec3(1.f)};
        }
        // Start the Dear ImGui frame
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
        ImGui::Begin("DBG");
        ImGui::Checkbox("DBG_EVENT_DRIVEN", &low_fps);
        ImGui::End();
        editor->draw(done);


        ImGui::Render();
        ImDrawData* draw_data = ImGui::GetDrawData();
        const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
        if (!is_minimized)
        {
            renderer::imgui::wd->ClearValue.color.float32[0] = clear_color.x * clear_color.w;
            renderer::imgui::wd->ClearValue.color.float32[1] = clear_color.y * clear_color.w;
            renderer::imgui::wd->ClearValue.color.float32[2] = clear_color.z * clear_color.w;
            renderer::imgui::wd->ClearValue.color.float32[3] = clear_color.w;
            FrameRender(renderer::imgui::wd, draw_data);
            FramePresent(renderer::imgui::wd);
        }
        advance_frame_and_execute_cleanups();
    }

    // Cleanup
    auto err = vkDeviceWaitIdle(renderer::device);
    check_vk_result(err);
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    renderer::CleanupVulkanWindow();
    renderer::CleanupVulkan();

    SDL_DestroyWindow(renderer::window);
    SDL_Quit();


    return 0;
}