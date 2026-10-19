# src/vendor/sqlite3/CMakeLists.txt
add_library(sqlite3_vendor STATIC sqlite3.c)

target_include_directories(sqlite3_vendor PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(sqlite3_vendor PRIVATE "-w" "-Os")
    # target_compile_options(sqlite3_vendor PRIVATE "-w" "$<$<CONFIG:Release,MinSizeRel>:-Os>")
elseif(MSVC)
	target_compile_options(sqlite3_vendor PRIVATE "/W0")
endif()
