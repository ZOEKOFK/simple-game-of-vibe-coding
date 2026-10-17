# ============================================================
# CMakeLists.txt —— 顶层构建脚本
# 构建：cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake
# 依赖库 raylib 由 vcpkg manifest 模式（vcpkg.json）自动安装。
# ============================================================
cmake_minimum_required(VERSION 3.20)
project(arena_survivor VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- raylib：优先走 vcpkg 导出的 CMake 配置；失败则手动定位兜底 ----
find_package(raylib CONFIG QUIET)
if(NOT raylib_FOUND)
    find_path(RAYLIB_INCLUDE_DIR raylib.h REQUIRED)
    find_library(RAYLIB_LIBRARY NAMES raylib REQUIRED)
    add_library(raylib UNKNOWN IMPORTED)
    set_target_properties(raylib PROPERTIES IMPORTED_LOCATION "${RAYLIB_LIBRARY}")
    target_include_directories(raylib INTERFACE "${RAYLIB_INCLUDE_DIR}")
    if(WIN32)
        target_link_libraries(raylib INTERFACE winmm gdi32 opengl32 shell32)
    endif()
endif()

# ---- 源文件：收集 src/ 下全部 .cpp（后续任务新增文件自动纳入）----
file(GLOB_RECURSE ARENA_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable(arena_survivor ${ARENA_SOURCES})
target_include_directories(arena_survivor PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(arena_survivor PRIVATE raylib)

# ---- 源码编码：全部源文件以 UTF-8 保存（含中文注释），MSVC 需显式按 UTF-8 解析 ----
if(MSVC)
    target_compile_options(arena_survivor PRIVATE /utf-8)
endif()
