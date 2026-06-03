cmake_minimum_required (VERSION 3.10)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_C_COMPILER_LAUNCHER   ${CCACHE_PROGRAM})
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
endif()

project(Diligent C CXX)

find_package(SDL3 REQUIRED)

set(DILIGENT_NO_GL ON CACHE INTERNAL "Disable OpenGL Support")
set(DILIGENT_NO_D3D11 ON CACHE INTERNAL "Disable D3D11 Support")
set(DILIGENT_NO_D3D12 ON CACHE INTERNAL "Disable D3D12 Support")
set(DILIGENT_NO_METAL ON CACHE INTERNAL "" FORCE) # you should disable this if youre building for macos

set(DILIGENT_BUILD_TESTS OFF CACHE INTERNAL "" FORCE)
set(DILIGENT_BUILD_TOOLS OFF CACHE INTERNAL "" FORCE)
set(DILIGENT_BUILD_SAMPLES OFF CACHE INTERNAL "" FORCE)

set(DILIGENT_BUILD_TEXTURE_LOADER OFF CACHE INTERNAL "Disable Texture Loader")
set(DILIGENT_BUILD_ASSET_LOADER OFF CACHE INTERNAL "Disable Asset Loader")
set(DILIGENT_BUILD_NATIVE_APP OFF CACHE INTERNAL "Disable Native App")
set(DILIGENT_BUILD_HLSL2GLSL_CONVERTER OFF CACHE INTERNAL "Disable HLSL Converter")
set(DILIGENT_BUILD_RENDER_STATE_NOTATION_PARSER OFF CACHE INTERNAL "Disable RSN Parser")
set(DILIGENT_BUILD_RENDER_STATE_PACKAGER OFF CACHE INTERNAL "Disable RS Packager")
set(DILIGENT_BUILD_IMGUI ON CACHE INTERNAL "Ensure ImGui is built")

add_compile_definitions(VULKAN_SUPPORTED=1)
add_subdirectory(DiligentCore)
add_subdirectory(DiligentTools)

set(SOURCE
    src/main.cpp
    src/shader.cpp
    DiligentTools/ThirdParty/imgui/backends/imgui_impl_sdl3.cpp
)

add_executable(diligent ${SOURCE})
target_compile_options(diligent PRIVATE -DENGINE_DLL)
set_target_properties(diligent PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}"
)


target_include_directories(diligent PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/DiligentCore"
    "${CMAKE_CURRENT_SOURCE_DIR}/DiligentCore/Primitives"
    "${CMAKE_CURRENT_SOURCE_DIR}/DiligentCore/Common"
    "${CMAKE_CURRENT_SOURCE_DIR}/DiligentCore/Graphics"
    "${CMAKE_CURRENT_SOURCE_DIR}/DiligentTools"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

set_target_properties(diligent PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
)

target_link_libraries(diligent PRIVATE
    Diligent-BuildSettings
    Diligent-GraphicsEngineVk-shared
    Diligent-Imgui
    SDL3::SDL3
)
