#include "editor.hpp"

love::Editor::Editor() {
    SetupImGuiStyle(true, 1.0);
}

love::Editor::~Editor() {

}

void love::Editor::draw(bool& done) {
    ImGui::DockSpaceOverViewport();

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File"))
        {
            ImGui::MenuItem("New", "Cmd+Q");
            ImGui::MenuItem("Create");
            if (ImGui::MenuItem("Exit", "Cmd+Q")) {
                done = true;
            }
            ImGui::EndMenu();
        }




        ImGui::EndMainMenuBar();
    }

    ImGui::ShowDemoWindow();


}
