cmake_minimum_required(VERSION 3.28)
project(LoveEngine)

set(CMAKE_CXX_STANDARD 20)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
add_subdirectory(external/SDL EXCLUDE_FROM_ALL)

add_subdirectory(external/VulkanMemoryAllocator EXCLUDE_FROM_ALL)
add_subdirectory(external/entt EXCLUDE_FROM_ALL)
add_subdirectory(external/portaudio EXCLUDE_FROM_ALL)
add_subdirectory(external/volk EXCLUDE_FROM_ALL)
add_subdirectory(external/freetype EXCLUDE_FROM_ALL)
add_subdirectory(external/nlohmann_json EXCLUDE_FROM_ALL)
add_subdirectory(external/glm EXCLUDE_FROM_ALL)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "" FORCE)
#set(ASSIMP_NO_EXPORT ON CACHE BOOL FORCE)
#set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "" FORCE)


add_subdirectory(external/assimp EXCLUDE_FROM_ALL)


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
        Renderer/ResourceManager.cpp
        Renderer/ImageManager.cpp
        Renderer/ImageManager.h
        Renderer/MeshManager.cpp
        Renderer/MeshManager.h
        Renderer/EngineBuffer.cpp
        Renderer/EngineBuffer.h
        Renderer/first_renderer.cpp
        Renderer/first_renderer.h
        Renderer/ShaderManager.cpp
        Renderer/ShaderManager.h
        debug_panic.cpp
        ECS/Transform.h
        ECS/Tree.h
        ECS/Mesh.h
        love.h
        ECS/Camera.h
        ECS/EditorSelected.h
        editor/InpectorRepr.h
        editor/defaultspecializers.cpp
)
if(TARGET assimp::assimp)
    target_link_libraries(LoveEngine PRIVATE assimp::assimp)
    target_include_directories(LoveEngine PRIVATE external/assimp/include)
endif ()
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
#    target_link_libraries(LoveEngine PRIVATE SDL3::Headers)
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
if (TARGET LoveEngine)
    add_compile_definitions(_DEBUG)
    add_compile_definitions(GLM_FORCE_DEPTH_ZERO_TO_ONE)
    target_compile_options(LoveEngine PRIVATE -Wno-reorder-init-list)
    target_compile_options(LoveEngine PRIVATE -Wno-nullability-completeness)

endif ()
target_include_directories(LoveEngine PRIVATE external/stb)

include(FetchContent)
FetchContent_Declare(
        cpptrace
        GIT_REPOSITORY https://github.com/jeremy-rifkin/cpptrace.git
        GIT_TAG        v0.7.5 # <HASH or TAG>
)
FetchContent_MakeAvailable(cpptrace)
target_link_libraries(LoveEngine PRIVATE cpptrace::cpptrace)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    # target_link_libraries(VKEngine PRIVATE Vulkan::MoltenVK) error?
endif()
