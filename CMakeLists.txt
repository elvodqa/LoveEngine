cmake_minimum_required(VERSION 3.28)
project(LoveEngine)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(external/SDL EXCLUDE_FROM_ALL)
add_subdirectory(external/VulkanMemoryAllocator EXCLUDE_FROM_ALL)
add_subdirectory(external/entt EXCLUDE_FROM_ALL)
add_subdirectory(external/portaudio EXCLUDE_FROM_ALL)

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

        # imgui
        external/imgui/imgui.cpp
        external/imgui/imgui_demo.cpp
        external/imgui/imgui_draw.cpp
        external/imgui/imgui_tables.cpp
        external/imgui/imgui_widgets.cpp
        external/imgui/backends/imgui_impl_sdl3.cpp
        external/imgui/backends/imgui_impl_vulkan.cpp
)

target_include_directories(LoveEngine PRIVATE external/imgui)

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
    target_link_libraries(LoveEngine PRIVATE VulkanMemoryAllocator)
    target_include_directories(LoveEngine PRIVATE external/VulkanMemoryAllocator/include)
endif ()

find_package(Vulkan REQUIRED)
if(VULKAN_FOUND)
    message(STATUS "Found Vulkan, Including and Linking now")
    include_directories(LoveEngine Vulkan_INCLUDE_DIRS)
    target_link_libraries(LoveEngine PRIVATE Vulkan::Vulkan)

    #if (NOT Vulkan_volk_FOUND)
    #    message(SEND_ERROR "Vulkan volk not found")
    #
    #   elseif (Vulkan_volk_FOUND)
    #      target_link_libraries(VKEngine PRIVATE Vulkan::volk)
    # endif ()

endif (VULKAN_FOUND)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    #target_link_libraries(VKEngine PRIVATE Vulkan::MoltenVK) error?
endif ()