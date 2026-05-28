cmake_minimum_required (VERSION 3.10)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_C_COMPILER_LAUNCHER   ${CCACHE_PROGRAM})
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
endif()

project(Diligent CXX)

find_package(SDL3 REQUIRED)

set(DILIGENT_NO_GL ON CACHE INTERNAL "Disable OpenGL Support")
set(DILIGENT_NO_D3D11 ON CACHE INTERNAL "Disable D3D11 Support")
set(DILIGENT_NO_D3D12 ON CACHE INTERNAL "Disable D3D12 Support")
set(DILIGENT_BUILD_TESTS OFF CACHE INTERNAL "" FORCE)
set(DILIGENT_BUILD_TOOLS OFF CACHE INTERNAL "" FORCE)
set(DILIGENT_BUILD_SAMPLES OFF CACHE INTERNAL "" FORCE)
set(DILIGENT_NO_METAL ON CACHE INTERNAL "" FORCE)

add_compile_definitions(VULKAN_SUPPORTED=1)

set(SOURCE
    src/main.cpp
    src/shader.cpp
)
add_subdirectory(DiligentCore)
add_executable(diligent ${SOURCE})
target_compile_options(diligent PRIVATE -DENGINE_DLL)
set_target_properties(diligent PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(XCB REQUIRED xcb)

target_include_directories(diligent PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/DiligentCore"
    "${CMAKE_CURRENT_SOURCE_DIR}/DiligentCore/Primitives"
    "${CMAKE_CURRENT_SOURCE_DIR}/DiligentCore/Common"
    "${CMAKE_CURRENT_SOURCE_DIR}/DiligentCore/Graphics"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

set_target_properties(diligent PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
)

target_link_libraries(diligent PRIVATE
    Diligent-BuildSettings
    Diligent-GraphicsEngineVk-shared
    SDL3::SDL3
)
