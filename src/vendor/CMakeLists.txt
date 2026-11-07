add_subdirectory(luajit)

if(CK_ENABLE_EDITOR)
    add_subdirectory(sqlite3)
    add_subdirectory(imgui)
endif()
