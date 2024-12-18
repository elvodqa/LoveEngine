cmake_minimum_required(VERSION 3.28)
project(LoveEngine)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(external/SDL EXCLUDE_FROM_ALL)
add_subdirectory(external/VulkanMemoryAllocator EXCLUDE_FROM_ALL)
add_subdirectory(external/entt EXCLUDE_FROM_ALL)
add_subdirectory(external/portaudio EXCLUDE_FROM_ALL)
add_subdirectory(external/volk EXCLUDE_FROM_ALL)
add_subdirectory(external/freetype EXCLUDE_FROM_ALL)
add_subdirectory(external/nlohmann_json EXCLUDE_FROM_ALL)
add_subdirectory(external/glm EXCLUDE_FROM_ALL)


set(VULKAN_COMPONENTS
        "glslc"
        "glslangValidator"
        "glslang"
        "shaderc_combined"
        "SPIRV-Tools"
        "dxc"
)

add_executable(LoveEngine
        main.cpp

        # editor
        editor/editor.cpp
        editor/editor_style.cpp

        # imgui
        external/imgui/imgui.cpp
        external/imgui/imgui_demo.cpp
        external/imgui/imgui_draw.cpp
        external/imgui/imgui_tables.cpp
        external/imgui/imgui_widgets.cpp
        external/imgui/backends/imgui_impl_sdl3.cpp
        external/imgui/backends/imgui_impl_vulkan.cpp
        Renderer/Renderer.cpp
        debug_panic.h
        Renderer/ResourceManager.h
        Renderer/renderer_constants.h
        Renderer/EngineImage.cpp
        Renderer/EngineImage.h

        external/imgui/misc/freetype/imgui_freetype.cpp
        love_resource_locator.h
)

if (TARGET freetype)
    target_link_libraries(LoveEngine PRIVATE freetype)
    target_include_directories(LoveEngine PRIVATE external/freetype/include)
endif()

target_include_directories(LoveEngine PRIVATE external/imgui)
target_include_directories(LoveEngine PRIVATE external/IconFontCppHeaders)


if (TARGET EnTT::EnTT)
    target_link_libraries(LoveEngine PRIVATE EnTT)
    target_include_directories(LoveEngine PRIVATE external/entt/single_include)
endif()

if (TARGET PortAudio::portaudio)
    target_link_libraries(LoveEngine PRIVATE PortAudio::portaudio)
    target_include_directories(LoveEngine PRIVATE external/portaudio/include)
endif()

if (TARGET SDL3::SDL3)
    target_link_libraries(LoveEngine PRIVATE SDL3::SDL3)
    target_include_directories(LoveEngine PRIVATE external/SDL/include)
endif()

if (TARGET VulkanMemoryAllocator)
    target_link_libraries(LoveEngine PRIVATE GPUOpen::VulkanMemoryAllocator)
    target_include_directories(LoveEngine PRIVATE external/VulkanMemoryAllocator/include)
endif()

if (TARGET nlohmann_json::nlohmann_json)
    target_link_libraries(LoveEngine PRIVATE nlohmann_json::nlohmann_json)
    target_include_directories(LoveEngine PRIVATE external/nlohmann_json/include)
    set(JSON_BuildTests OFF CACHE INTERNAL "")
endif()
if (TARGET glm::glm-header-only)
    target_link_libraries(LoveEngine PRIVATE glm::glm-header-only)
    target_include_directories(LoveEngine PRIVATE external/glm/include)
endif()
find_package(Vulkan REQUIRED)

if (VULKAN_FOUND)
    message(STATUS "Found Vulkan, Including and Linking now")

    target_link_libraries(LoveEngine PRIVATE volk)
    target_include_directories(LoveEngine PRIVATE external/volk)

    target_compile_definitions(LoveEngine PRIVATE IMGUI_IMPL_VULKAN_USE_VOLK)
endif()

if (WIN32)
    target_compile_definitions(LoveEngine PRIVATE VK_USE_PLATFORM_WIN32_KHR)
elseif (APPLE)
    target_compile_definitions(LoveEngine PRIVATE VK_USE_PLATFORM_METAL_EXT)
elseif (UNIX)
    target_compile_definitions(LoveEngine PRIVATE VK_USE_PLATFORM_XLIB_KHR)
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    # target_link_libraries(VKEngine PRIVATE Vulkan::MoltenVK) error?
endif()
