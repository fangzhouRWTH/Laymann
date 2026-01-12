#cmake_minimum_required(VERSION 3.10.0)
#project(Laymann VERSION 0.1.0 LANGUAGES C CXX)

#set(CMAKE_CXX_STANDARD 20)

if (WIN32)
    add_compile_definitions(PLATFORM_WINDOWS)
elseif (APPLE)
    add_compile_definitions(PLATFORM_MACOS)
elseif (UNIX)
    add_compile_definitions(PLATFORM_LINUX)
endif()

add_executable(UT_Geometry geometry_test.cpp)

target_include_directories(UT_Geometry
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/eigen
)

# target_link_libraries(${PROJECT_NAME}
#     PRIVATE
# )


