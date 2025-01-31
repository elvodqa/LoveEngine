#include <imgui.h>
#include <entt/entt.hpp>

#include "../ECS/Camera.h"
#include "InpectorRepr.h"
#include "../ECS/EditorSelected.h"
#include "../ECS/Mesh.h"
#include "../ECS/Transform.h"
//
// Created by YAHAY on 20/01/2025.
//
template<>
void inspectorWindow<Camera>(Camera* cam) {
    if (ImGui::BeginChild("Camera")) {
        ImGui::SliderAngle("Fov",&cam->fovy,1,180);
        ImGui::DragFloat("Near Clip Plane",&cam->nearPlane, 1, 0.00001,1,"%.5f",ImGuiSliderFlags_Logarithmic|ImGuiSliderFlags_NoRoundToFormat);
        ImGui::DragFloat("Near Clip Plane",&cam->farPlane,1 ,1,1000000,"%.5f",ImGuiSliderFlags_Logarithmic|ImGuiSliderFlags_NoRoundToFormat);
    }
    ImGui::EndChild();
}
template void inspectorWindow<Camera>(Camera*);
template<>
void inspectorWindow<EditorSelected>(EditorSelected* a) {}
template void inspectorWindow<EditorSelected>(EditorSelected*);
template<>
void inspectorWindow<Mesh>(Mesh* a) {}
template void inspectorWindow<Mesh>(Mesh*);
template<>
void inspectorWindow<Transform>(Transform* a) {}
template void inspectorWindow<Transform>(Transform*);
// template<>
// void inspectorWindow<entt::entity>(entt::entity* a) {}
// template void inspectorWindow<entt::entity>(entt::entity*);