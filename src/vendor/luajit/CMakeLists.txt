# src/vendor/luajit/CMakeLists.txt
include(ExternalProject)

set(LUAJIT_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

if(WIN32)
    # --- WINDOWS (MSVC) ---
    # msvcbuild.bat hardcodes "lua51.lib"
    set(LUAJIT_BYPRODUCT "${LUAJIT_SRC_DIR}/src/lua51.lib")

    set(LUAJIT_BUILD_CMD cmd /c "cd /d ${LUAJIT_SRC_DIR}/src && msvcbuild.bat static")
else()
    set(LUAJIT_BYPRODUCT "${LUAJIT_SRC_DIR}/src/libluajit.a")
    set(LUAJIT_BUILD_CMD make -C "${LUAJIT_SRC_DIR}/src" -j${nproc})
endif()

ExternalProject_Add(luajit_project
    SOURCE_DIR "${LUAJIT_SRC_DIR}"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ${LUAJIT_BUILD_CMD}
    INSTALL_COMMAND ""
    BUILD_BYPRODUCTS "${LUAJIT_BYPRODUCT}"
)

add_library(libluajit INTERFACE)
target_include_directories(libluajit INTERFACE "${LUAJIT_SRC_DIR}/src")
target_link_libraries(libluajit INTERFACE "${LUAJIT_BYPRODUCT}")
add_dependencies(libluajit luajit_project)
