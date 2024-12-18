#include "editor.hpp"

namespace fs = std::filesystem;

love::Editor::Editor(SDL_Window* window) {
    SetupImGuiStyle(editor::Theme::EverForest);
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGuiIO& io = ImGui::GetIO();
    int display_w, display_h, w, h;
    SDL_GetWindowSize(window, &display_w, &display_h);

    SDL_GetWindowSizeInPixels(window, &w, &h);
    io.DisplayFramebufferScale = ImVec2(0.5f, 0.5f);
    io.DisplaySize = ImVec2((float)display_w, (float)display_h);

}

love::Editor::~Editor() {

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

                ImGui::EndMenu();
            }

            if (ImGui::Checkbox("Explorer", &b_showExplorer)) {

            }

            ImGui::EndMenu();
        }




        ImGui::EndMainMenuBar();
    }


    ImGuiWindowFlags topControlWindowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove;

    ImGui::Begin("##top_contasdrols", NULL, topControlWindowFlags);

    ImGui::End();

    if (b_showExplorer) {
        showExplorer(&b_showExplorer);
    }

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

    if (ImGui::BeginChild("FileGrid", ImVec2(0, 0), true)) { // BEGIN FILE GRID
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