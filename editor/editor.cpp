#include "editor.hpp"

namespace fs = std::filesystem;


static void check_vk_result(VkResult err)
{
    if (err == 0)
        return;
    SDL_Log("[vulkan][%s:%d] Error: VkResult = %d", __FILE__, __LINE__, err);
    if (err < 0)
        panic();
}

// Helper function to find Vulkan memory type bits. See ImGui_ImplVulkan_MemoryType() in imgui_impl_vulkan.cpp
uint32_t findMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties mem_properties;
    vkGetPhysicalDeviceMemoryProperties(renderer::g_PhysicalDevice, &mem_properties);

    for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++)
        if ((type_filter & (1 << i)) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties)
            return i;

    return 0xFFFFFFFF; // Unable to find memoryType
}

// Helper function to load an image with common settings and return a MyTextureData with a VkDescriptorSet as a sort of Vulkan pointer
bool LoadTextureFromFile(const char* filename, MyTextureData* tex_data)
{
    // Specifying 4 channels forces stb to load the image in RGBA which is an easy format for Vulkan
    tex_data->Channels = 4;
    unsigned char* image_data = stbi_load(filename, &tex_data->Width, &tex_data->Height, 0, tex_data->Channels);

    if (image_data == NULL)
        return false;

    // Calculate allocation size (in number of bytes)
    size_t image_size = tex_data->Width * tex_data->Height * tex_data->Channels;

    VkResult err;

    // Create the Vulkan image.
    {
        VkImageCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        info.imageType = VK_IMAGE_TYPE_2D;
        info.format = VK_FORMAT_R8G8B8A8_UNORM;
        info.extent.width = tex_data->Width;
        info.extent.height = tex_data->Height;
        info.extent.depth = 1;
        info.mipLevels = 1;
        info.arrayLayers = 1;
        info.samples = VK_SAMPLE_COUNT_1_BIT;
        info.tiling = VK_IMAGE_TILING_OPTIMAL;
        info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        err = vkCreateImage(renderer::device, &info, renderer::g_vk_Allocator, &tex_data->Image);
        check_vk_result(err);
        VkMemoryRequirements req;
        vkGetImageMemoryRequirements(renderer::device, tex_data->Image, &req);
        VkMemoryAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = req.size;
        alloc_info.memoryTypeIndex = findMemoryType(req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        err = vkAllocateMemory(renderer::device, &alloc_info, renderer::g_vk_Allocator, &tex_data->ImageMemory);
        check_vk_result(err);
        err = vkBindImageMemory(renderer::device, tex_data->Image, tex_data->ImageMemory, 0);
        check_vk_result(err);
    }

    // Create the Image View
    {
        VkImageViewCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        info.image = tex_data->Image;
        info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        info.format = VK_FORMAT_R8G8B8A8_UNORM;
        info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        info.subresourceRange.levelCount = 1;
        info.subresourceRange.layerCount = 1;
        err = vkCreateImageView(renderer::device, &info, renderer::g_vk_Allocator, &tex_data->ImageView);
        check_vk_result(err);
    }

    // Create Sampler
    {
        VkSamplerCreateInfo sampler_info{};
        sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sampler_info.magFilter = VK_FILTER_LINEAR;
        sampler_info.minFilter = VK_FILTER_LINEAR;
        sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT; // outside image bounds just use border color
        sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler_info.minLod = -1000;
        sampler_info.maxLod = 1000;
        sampler_info.maxAnisotropy = 1.0f;
        err = vkCreateSampler(renderer::device, &sampler_info, renderer::g_vk_Allocator, &tex_data->Sampler);
        check_vk_result(err);
    }

    // Create Descriptor Set using ImGUI's implementation
    tex_data->DS = ImGui_ImplVulkan_AddTexture(tex_data->Sampler, tex_data->ImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    // Create Upload Buffer
    {
        VkBufferCreateInfo buffer_info = {};
        buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_info.size = image_size;
        buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        err = vkCreateBuffer(renderer::device, &buffer_info, renderer::g_vk_Allocator, &tex_data->UploadBuffer);
        check_vk_result(err);
        VkMemoryRequirements req;
        vkGetBufferMemoryRequirements(renderer::device, tex_data->UploadBuffer, &req);
        VkMemoryAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = req.size;
        alloc_info.memoryTypeIndex = findMemoryType(req.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        err = vkAllocateMemory(renderer::device, &alloc_info, renderer::g_vk_Allocator, &tex_data->UploadBufferMemory);
        check_vk_result(err);
        err = vkBindBufferMemory(renderer::device, tex_data->UploadBuffer, tex_data->UploadBufferMemory, 0);
        check_vk_result(err);
    }

    // Upload to Buffer:
    {
        void* map = NULL;
        err = vkMapMemory(renderer::device, tex_data->UploadBufferMemory, 0, image_size, 0, &map);
        check_vk_result(err);
        memcpy(map, image_data, image_size);
        VkMappedMemoryRange range[1] = {};
        range[0].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        range[0].memory = tex_data->UploadBufferMemory;
        range[0].size = image_size;
        err = vkFlushMappedMemoryRanges(renderer::device, 1, range);
        check_vk_result(err);
        vkUnmapMemory(renderer::device, tex_data->UploadBufferMemory);
    }

    // Release image memory using stb
    stbi_image_free(image_data);

    // Create a command buffer that will perform following steps when hit in the command queue.
    // TODO: this works in the example, but may need input if this is an acceptable way to access the pool/create the command buffer.
    VkCommandPool command_pool = renderer::imgui::imgui_MainWindowData.Frames[ renderer::imgui::imgui_MainWindowData.FrameIndex].CommandPool;
    VkCommandBuffer command_buffer;
    {
        VkCommandBufferAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandPool = command_pool;
        alloc_info.commandBufferCount = 1;

        err = vkAllocateCommandBuffers(renderer::device, &alloc_info, &command_buffer);
        check_vk_result(err);

        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        err = vkBeginCommandBuffer(command_buffer, &begin_info);
        check_vk_result(err);
    }

    // Copy to Image
    {
        VkImageMemoryBarrier copy_barrier[1] = {};
        copy_barrier[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        copy_barrier[0].dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        copy_barrier[0].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        copy_barrier[0].newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        copy_barrier[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        copy_barrier[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        copy_barrier[0].image = tex_data->Image;
        copy_barrier[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copy_barrier[0].subresourceRange.levelCount = 1;
        copy_barrier[0].subresourceRange.layerCount = 1;
        vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, copy_barrier);

        VkBufferImageCopy region = {};
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.layerCount = 1;
        region.imageExtent.width = tex_data->Width;
        region.imageExtent.height = tex_data->Height;
        region.imageExtent.depth = 1;
        vkCmdCopyBufferToImage(command_buffer, tex_data->UploadBuffer, tex_data->Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        VkImageMemoryBarrier use_barrier[1] = {};
        use_barrier[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        use_barrier[0].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        use_barrier[0].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        use_barrier[0].oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        use_barrier[0].newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        use_barrier[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        use_barrier[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        use_barrier[0].image = tex_data->Image;
        use_barrier[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        use_barrier[0].subresourceRange.levelCount = 1;
        use_barrier[0].subresourceRange.layerCount = 1;
        vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, use_barrier);
    }

    // End command buffer
    {
        VkSubmitInfo end_info = {};
        end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        end_info.commandBufferCount = 1;
        end_info.pCommandBuffers = &command_buffer;
        err = vkEndCommandBuffer(command_buffer);
        check_vk_result(err);
        err = vkQueueSubmit(renderer::g_Queue, 1, &end_info, VK_NULL_HANDLE);
        check_vk_result(err);
        err = vkDeviceWaitIdle(renderer::device);
        check_vk_result(err);
    }

    return true;
}

// Helper function to cleanup an image loaded with LoadTextureFromFile
void RemoveTexture(MyTextureData* tex_data)
{
    vkFreeMemory(renderer::device, tex_data->UploadBufferMemory, nullptr);
    vkDestroyBuffer(renderer::device, tex_data->UploadBuffer, nullptr);
    vkDestroySampler(renderer::device, tex_data->Sampler, nullptr);
    vkDestroyImageView(renderer::device, tex_data->ImageView, nullptr);
    vkDestroyImage(renderer::device, tex_data->Image, nullptr);
    vkFreeMemory(renderer::device, tex_data->ImageMemory, nullptr);
    ImGui_ImplVulkan_RemoveTexture(tex_data->DS);
}


love::Editor::Editor(SDL_Window* window) {
    SetupImGuiStyle(editor::Theme::Default);
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGuiIO& io = ImGui::GetIO();
    int display_w, display_h, w, h;
    SDL_GetWindowSize(window, &display_w, &display_h);

    SDL_GetWindowSizeInPixels(window, &w, &h);
    io.DisplayFramebufferScale = ImVec2(0.5f, 0.5f);
    io.DisplaySize = ImVec2((float)display_w, (float)display_h);

    c_assetSearchBuffer = (char*)malloc(sizeof(char) * 1024);
    c_consoleInputBuffer = (char*)malloc(sizeof(char) * t_consoleInputBufferSize);
    // memset(c_consoleInputBuffer, 0, t_consoleInputBufferSize * sizeof(char));

    renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        SDL_Log("Error creating SDL_Renderer for editor: %s", SDL_GetError());
    }

}

love::Editor::~Editor() {

}

void love::Editor::check_events(const SDL_Event* event) {
    switch (event->type) {
        case SDL_EVENT_DROP_FILE:
            b_eventFileDropped = true;
            c_eventFileDroppedName = const_cast<char*>(event->drop.data);
            break;
    }
}

void love::Editor::draw(bool& done) {
    ImGui::DockSpaceOverViewport();

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File"))
        {
            ImGui::MenuItem(ICON_FA_FILE " New", "Cmd+Q");
            ImGui::MenuItem(ICON_FA_FOLDER " Open", "Cmd+Q");
            ImGui::MenuItem(ICON_FA_CUBE " Open", "Cmd+Q");
            ImGui::MenuItem("Create");
            if (ImGui::MenuItem("Exit", "Cmd+Q")) {
                done = true;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View"))
        {
            if (ImGui::BeginMenu("Themes")) {
                if (ImGui::MenuItem(ICON_FA_PAINT_BRUSH "First")) {
                    changeTheme(editor::Theme::First);
                }
                if (ImGui::MenuItem(ICON_FA_PAINT_BRUSH "Gold Source")) {
                    changeTheme(editor::Theme::GoldSource);
                }
                if (ImGui::MenuItem(ICON_FA_PAINT_BRUSH "Ever Forest")) {
                    changeTheme(editor::Theme::EverForest);
                }
                if (ImGui::MenuItem(ICON_FA_PAINT_BRUSH "Microsoft")) {
                    changeTheme(editor::Theme::Microsoft);
                }
                if (ImGui::MenuItem(ICON_FA_PAINT_BRUSH "Moonlight")) {
                    changeTheme(editor::Theme::Moonlight);
                }
                if (ImGui::MenuItem(ICON_FA_PAINT_BRUSH "Purple Comfy")) {
                    changeTheme(editor::Theme::PurpleComfy);
                }
                if (ImGui::MenuItem(ICON_FA_PAINT_BRUSH "Default")) {
                    changeTheme(editor::Theme::Default);
                }

                ImGui::EndMenu();
            }

            ImGui::Checkbox("Explorer", &b_explorerShow);
            ImGui::Checkbox("Console", &b_consoleShow);
            ImGui::Checkbox("Asset Browser", &b_assetBrowserShow);

            ImGui::EndMenu();
        }


        ImGui::EndMainMenuBar();
    }

    if (b_explorerShow) {
        showExplorer(&b_explorerShow);
    }
    if (b_assetBrowserShow) {
        ShowAssetBrowser(&b_assetBrowserShow);
    }
    if (b_consoleShow) {
        showConsole(&b_consoleShow);
    }

    b_eventFileDropped = false;
    c_eventFileDroppedName = nullptr;
}

void love::Editor::ShowAssetBrowser(bool *p_open) {
    ImGui::Begin("Asset Browser");

    if (ImGui::Button(ICON_FA_LIST)) {

    }
    ImGui::SameLine();
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
    if (ImGui::InputText("##AssetSearch", c_assetSearchBuffer, t_assetSearchBufferSize)) {

    }
    ImGui::PopItemWidth();

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyle().Colors[ImGuiCol_FrameBg]);
    auto displayWidth = ImGui::GetContentRegionAvail().x;
    if (ImGui::BeginChild("##FileDisplay", ImVec2(displayWidth, ImGui::GetContentRegionAvail().y))) {
        if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows)) { // check if hovering the fucking item part
            b_assetBrowserHovered = true;
            if (b_eventFileDropped) {
                SDL_Log("Dropped file: %s", c_eventFileDroppedName);

                love::editor::ImageAsset imageAsset;
                if (!LoadTextureFromFile(c_eventFileDroppedName, &imageAsset.data)) {
                    SDL_Log("Error loading file image");
                }
                imageAsset.name = fs::path(c_eventFileDroppedName).filename();
                assetsImage.push_back(imageAsset);
                SDL_Log("Added image to assets");
            }
        }

        /* This actually works except the y of childSize idk why
        ImVec2 parentPos = ImGui::GetCursorScreenPos(); // Position of the parent window (top-left)
        ImVec2 childPosScreenSpace = ImGui::GetItemRectMin(); // Absolute top-left position of the child window
        ImVec2 childPosRelativeToParent = ImVec2(childPosScreenSpace.x - parentPos.x, childPosScreenSpace.y - parentPos.y);
        ImVec2 childSize = ImVec2(ImGui::GetItemRectMax().x - childPosScreenSpace.x, ImGui::GetItemRectMax().y - childPosScreenSpace.y); // Size of the child window

        ImGui::Text("Child window screen position: (%.1f, %.1f)", childPosScreenSpace.x, childPosScreenSpace.y);
        ImGui::Text("Child window relative to parent: (%.1f, %.1f)", childPosRelativeToParent.x, childPosRelativeToParent.y);
        ImGui::Text("Child window size: (%.1f, %.1f)", childSize.x, childSize.y);
         */

        bool thumbLessDetail = true;
        const float itemSize = thumbLessDetail ? 48 : 128;

        int index = 0;
        int columns = displayWidth / (thumbLessDetail ? 256 : 128);
        columns = columns < 1 ? 1 : columns;

        float curX = 0.0f;
        ImGui::Columns(columns, nullptr, false);



        for (int i = 0; i < assetsImage.size(); i++) {
            auto& item = assetsImage[i];
            ImGui::PushID(i);

            // Urho3D::SharedPtr<Texture2D> thumb = GetImage(itemPath);

            //auto filename = item.filename().string();
            ImGui::BeginGroup();

            //std::string extension = item.extension();

            std::string a = "##" + std::to_string(i) + item.name;
            ImGui::ImageButton(a.c_str(), (ImTextureID)item.data.DS, { itemSize - 20, itemSize - 16 });// , { 0, 1 }, { 1,0 });
            if (ImGui::IsMouseDoubleClicked(0) && ImGui::IsItemHovered()) // change target directory
            {
                SDL_Log("%s", item.name.c_str());
            }

            if (asDetail)
                ImGui::SameLine();
            ImGui::TextWrapped("%s", item.name.c_str());
            if (ImGui::IsItemHovered() && !asDetail)
                ImGui::SetTooltip("%s", item.name.c_str());
            ImGui::EndGroup();


            ImGui::PopID();
            ImGui::NextColumn();
        }
    }
    ImGui::EndChild();
    ImGui::PopStyleColor();


    ImGui::End();
}

void love::Editor::showExplorer(bool *p_open) {
    ImGui::Begin("Explorer", p_open); // BEGIN EXPLORER

    if (!b_explorerSearchInputing) {

        if (ImGui::Button(ICON_FA_SEARCH)) {
            c_explorerSearchBuffer = new char[1024];
            c_explorerSearchBuffer[0] = '/';
            b_explorerSearchInputing = true;
        }

        ImGui::SameLine();
        if (ImGui::Button("/")) {
            currentPath = "/";
        }
        ImGui::SameLine();

        for (const auto& part : currentPath) {
            if (part.filename() == "") {
                continue;
            }
            if (ImGui::Button(part.c_str())) {
                if (part.filename() != currentPath.filename()) {
                    auto pos = currentPath.string().find(part.string());
                    if (pos != std::string::npos) {
                        currentPath = currentPath.string().substr(0, pos + part.string().size());
                    }
                }
            }
            ImGui::SameLine();
        }
        ImGui::SameLine();
    }

    if (b_explorerSearchInputing) {
        if (ImGui::InputText("Path", c_explorerSearchBuffer, 1024, ImGuiInputTextFlags_EnterReturnsTrue)) {
            SDL_Log("%s", c_explorerSearchBuffer);
            auto path = fs::path(c_explorerSearchBuffer);
            if (exists(path)) {
                currentPath = path;
                b_explorerSearchInputing = false;
            }
        }
    }

    ImGui::Separator();

    const float buttonWidth = 100.0f;  // Set fixed width for button
    const float buttonHeight = 100.0f; // Set fixed height for button
    const ImVec2 buttonSize(buttonWidth, buttonHeight);

    if (ImGui::BeginChild("ExplorerContext", ImVec2(0, 0), true)) { // BEGIN FILE GRID
        /*if (ImGui::Button("..")) {
            currentPath = currentPath.parent_path();
        }*/

        ImGui::SameLine();
        ImGui::Checkbox("Compact", &asDetail);
        ImGui::SameLine();
        ImGui::Checkbox("Hide dot files", &hideDots);

        if (fs::exists(currentPath) && fs::is_directory(currentPath)) {
            // Create a list of items (files and directories)
            std::vector<fs::path> folders;
            std::vector<fs::path> items;

            for (const auto& entry : fs::directory_iterator(currentPath)) {
                const auto& path = entry.path();
                std::string displayName = path.string();  // Full path

                if (hideDots && path.filename().string().c_str()[0] == '.') {
                    continue;
                }

                if (fs::is_directory(path)) {
                    folders.push_back(path);
                } else if (fs::is_regular_file(path)) {
                    items.push_back(path);
                }
            }

            // show directories
            if (ImGui::BeginListBox("##FolderDisplay", ImVec2(ImGui::GetContentRegionAvail().x / 5, ImGui::GetContentRegionAvail().y))) {
                if (ImGui::Selectable((ICON_FA_FOLDER + std::string(" ") + "..").c_str())) {
                    currentPath = currentPath.parent_path();
                }

                for (const auto& folder : folders) {

                    auto display = ICON_FA_FOLDER + std::string(" ") + folder.filename().string();
                    if (ImGui::Selectable(display.c_str())) {
                        if (fs::is_directory(folder)) {
                            currentPath = folder;
                        }

                        // Handle file selection here if necessary
                    }
                }
                ImGui::EndListBox();
            }

            ImGui::SameLine();

            // TODO: ImGuiChildFlags_ResizeX, Make the folder explorer and file explorer resizable inbetween each other
            float fileDisplayWidth = ImGui::GetContentRegionAvail().x;
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyle().Colors[ImGuiCol_FrameBg]);
            if (ImGui::BeginChild("##FileDisplay", ImVec2(fileDisplayWidth, ImGui::GetContentRegionAvail().y))) {
                const float w = fileDisplayWidth;
                const float itemSize = asDetail ? 48 : 128;

                int index = 0;
                int columns = fileDisplayWidth / (asDetail ? 256 : 128);
                columns = columns < 1 ? 1 : columns;

                float curX = 0.0f;
                ImGui::Columns(columns, nullptr, false);

                static std::string assetPopupPath;

                for (int i = 0; i < items.size(); i++) {
                    auto& item = items[i];
                    ImGui::PushID(i);

                    // Urho3D::SharedPtr<Texture2D> thumb = GetImage(itemPath);

                    auto filename = item.filename().string();
                    ImGui::BeginGroup();

                    std::string extension = item.extension();

                    std::string logo;
                    if (extension == ".lua" || extension == ".cs" || extension == ".odin" || extension == ".txt" || extension == ".md" ) {
                        logo = ICON_FA_FILE_CODE;
                    }
                    else if (extension == ".mp3" || extension == ".wav" || extension == ".ogg") {
                        logo = ICON_FA_FILE_AUDIO;
                    }
                    else if (extension == ".mp4" || extension == ".mov" || extension == ".avi") {
                        logo = ICON_FA_FILE_VIDEO;
                    }
                    else if (extension == ".jpg" || extension == ".png" || extension == ".jpeg" || extension == ".gif" || extension == ".webp") {
                        logo = ICON_FA_FILE_IMAGE;
                    }
                    else {
                        logo = ICON_FA_FILE;
                    }

                    ImGui::Button(logo.c_str(), { itemSize - 20, itemSize - 16 });// , { 0, 1 }, { 1,0 });
                    if (ImGui::IsMouseDoubleClicked(0) && ImGui::IsItemHovered()) // change target directory
                    {
                        SDL_Log("%s", filename.c_str());
                    }

                    if (asDetail)
                        ImGui::SameLine();
                    ImGui::TextWrapped("%s", filename.c_str());
                    if (ImGui::IsItemHovered() && !asDetail)
                        ImGui::SetTooltip("%s", filename.c_str());
                    ImGui::EndGroup();


                    ImGui::PopID();
                    ImGui::NextColumn();
                }
            }
            ImGui::EndChild();
            ImGui::PopStyleColor();
        } else {
            SDL_Log("Provided path is not a valid directory or doesn't exist!");
        }

    }
    ImGui::EndChild();
    ImGui::End();
}

// TODO: ADD FILTER
void love::Editor::showConsole(bool *p_open) {
    ImGui::Begin("Console", p_open);
    auto footerHeightToReserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
    if (ImGui::BeginChild("ScrollRegion##", {0, -footerHeightToReserve}, {}, {})) {
        float timestamp_width = ImGui::CalcTextSize("00:00:00:0000").x;
        int count = 0;

        ImGui::PushTextWrapPos();
        for (auto item : consoleItems) {
            switch (item.type) {
                case editor::LogType::Debug:
                    ImGui::Text("%s", (std::string(ICON_FA_BUG) + std::string(" ") + (item.message)).c_str());
                    break;
                case editor::LogType::Error:
                    ImGui::Text("%s", (std::string(ICON_FA_EXCLAMATION_CIRCLE)+ std::string(" ") + (item.message)).c_str());
                    break;
                case editor::LogType::Info:
                    ImGui::Text("%s", (std::string(ICON_FA_INFO) + std::string(" ")+ (item.message)).c_str());
                    break;
                case editor::LogType::Warn:
                    ImGui::Text("%s", (std::string(ICON_FA_EXCLAMATION_TRIANGLE)+ std::string(" ") + (item.message)).c_str());
                    break;
                case editor::LogType::Trace:
                    ImGui::Text("%s", (std::string(ICON_FA_QUESTION) + std::string(" ")+ (item.message)).c_str());
                    break;

            }
            ImGui::Separator();
        }

        ImGui::PopTextWrapPos();

        if (b_scrollToBottom && (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() /*|| console.autoScroll*/)) {
            ImGui::SetScrollHereY(1);
        }
        b_scrollToBottom = false;
    }
    ImGui::EndChild();

    ImGui::Separator();

    auto flags = ImGuiInputTextFlags_CallbackHistory | ImGuiInputTextFlags_CallbackCharFilter | ImGuiInputTextFlags_EnterReturnsTrue;

    auto reclaimFocus = false;

    ImGui::PushItemWidth(-ImGui::GetStyle().ItemSpacing.x * 7);

    if (ImGui::InputText("Input", c_consoleInputBuffer, t_consoleInputBufferSize, ImGuiInputTextFlags_EnterReturnsTrue)) {
        if (c_consoleInputBuffer[0] != 0) {
            log(editor::LogType::Debug,std::string(c_consoleInputBuffer));
            b_scrollToBottom = true;
        }
        reclaimFocus = true;
        c_consoleInputBuffer = (char*)malloc(sizeof(char) * t_consoleInputBufferSize);
    }

    ImGui::PopItemWidth();

    ImGui::SetItemDefaultFocus();
    if (reclaimFocus) {
        ImGui::SetKeyboardFocusHere(-1);
    }

    ImGui::End();
}

void love::Editor::log(love::editor::LogType type, std::string msg) {
    love::editor::LogItem item;
    item.message = std::string(msg);
    item.type = type;

    consoleItems.push_back(item);
}


/*
if (fs::exists(currentPath) && fs::is_directory(currentPath)) {
        for (const auto& entry : fs::directory_iterator(currentPath)) {
            const auto& path = entry.path();

            if (hideDots) {
                if (entry.path().filename().c_str()[0] == '.') {
                    continue;
                }
            }


            if (fs::is_directory(path)) {
                //std::cout << "[DIR]  " << path.string() << std::endl;
                // If you want to recursively list directories and files, call the function again

                if (ImGui::Button(entry.path().filename().c_str())) {
                    currentPath = entry.path();
                }

                //list_directory(path);
            } else if (fs::is_regular_file(path)) {
                //std::cout << "[FILE] " << path.string() << std::endl;

                ImGui::Button(entry.path().filename().c_str());
            }
        }
    }
 */