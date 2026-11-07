# src/vendor/sqlite3/CMakeLists.txt
add_library(sqlite3 STATIC sqlite3.c)

target_include_directories(sqlite3 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(sqlite3 PRIVATE "-w" "-Os")
elseif(MSVC)
	target_compile_options(sqlite3 PRIVATE "/W0")
endif()
