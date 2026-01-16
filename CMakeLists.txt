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

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/data
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/data ${CMAKE_BINARY_DIR}/data
    COMMENT "Copying files..."
)

add_custom_target(CopyFiles ALL DEPENDS ${CMAKE_BINARY_DIR}/data)

add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/glfw)
add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/bgfx)
add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/catch2)
add_subdirectory(${CMAKE_SOURCE_DIR}/src)
add_subdirectory(${CMAKE_SOURCE_DIR}/unit_test)
add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/openmesh)

set(WITH_examples OFF CACHE BOOL "Disable CGAL examples")
set(WITH_demos OFF CACHE BOOL "Disable CGAL tests")
set(CGAL_ENABLE_TESTING OFF CACHE BOOL "Disable CGAL demos")
set(RUNNING_CGAL_AUTO_TEST ON CACHE BOOL " ")
add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/cgal)
