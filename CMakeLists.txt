cmake_minimum_required(VERSION 3.10.0)
project(Laymann VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

if (WIN32)
    add_compile_definitions(PLATFORM_WINDOWS)
elseif (APPLE)
    add_compile_definitions(PLATFORM_MACOS)
elseif (UNIX)
    add_compile_definitions(PLATFORM_LINUX)
endif()

add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/glfw)
add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/bgfx)
add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/catch2)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(USE_DOUBLE_PRECISION OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/bullet3)
add_subdirectory(${CMAKE_SOURCE_DIR}/src)
add_subdirectory(${CMAKE_SOURCE_DIR}/unit_test)
