#cmake_minimum_required(VERSION 3.10.0)
#project(Laymann VERSION 0.1.0 LANGUAGES C CXX)

#set(CMAKE_CXX_STANDARD 20)
add_executable(UT_Geometry 
geometry_test.cpp
)

target_link_libraries(UT_Geometry
    PRIVATE
        Catch2::Catch2WithMain
)

target_include_directories(UT_Geometry
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/third_party/eigen
        ${CMAKE_SOURCE_DIR}/third_party/catch2
        ${CMAKE_SOURCE_DIR}/third_party/libig/include
)