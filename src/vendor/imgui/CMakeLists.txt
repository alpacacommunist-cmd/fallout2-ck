cmake_minimum_required(VERSION 3.17)
project(imgui_vendor CXX)

set(IMGUI_SOURCES
    imgui.cpp
    imgui_draw.cpp
    imgui_widgets.cpp
    imgui_tables.cpp
    imgui_demo.cpp
)

set(IMGUI_BACKEND_SOURCES
    backends/imgui_impl_sdl2.cpp
    backends/imgui_impl_sdlrenderer2.cpp
)

add_library(imgui_vendor STATIC ${IMGUI_SOURCES} ${IMGUI_BACKEND_SOURCES})

target_include_directories(imgui_vendor PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/backends"
)

target_link_libraries(imgui_vendor PRIVATE SDL2-static)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(imgui_vendor PRIVATE "-w" "-Os")
elseif(MSVC)
    target_compile_options(imgui_vendor PRIVATE "/W0")
endif()
