cmake_minimum_required(VERSION 3.10.0)
project(Laymann VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

# bgfx shaderc 可执行文件路径：你需要改成自己机器上的实际路径
set(SHADERC_EXECUTABLE "${CMAKE_SOURCE_DIR}/third_party/bgfx/.build/linux64_gcc/bin/shadercRelease" CACHE FILEPATH "bgfx shaderc executable")

# bgfx 自带 shader include 路径（common.sh、bgfx_shader.sh 等）
# 通常在 bgfx 仓库的 examples/common 或 src 下面，按你的结构改：
set(BGFX_SHADER_INCLUDE_DIRS 
    "${CMAKE_SOURCE_DIR}/third_party/bgfx/examples/common" 
    "${CMAKE_SOURCE_DIR}/third_party/bgfx/src" )

# 我们自己的 shader 源码目录 和 输出目录
set(BGFX_SHADER_SRC_DIR  "${CMAKE_SOURCE_DIR}/src/shaders")
set(BGFX_SHADER_BIN_DIR  "${CMAKE_SOURCE_DIR}/src/shaders")

function(bgfx_compile_shader OUT_VAR SHADER_TYPE SHADER_FILE)
    get_filename_component(FILE_NAME_WE ${SHADER_FILE} NAME_WE)
    set(OUTPUT_FILE "${BGFX_SHADER_BIN_DIR}/${FILE_NAME_WE}.bin")

    # 把 BGFX_SHADER_INCLUDE_DIRS 展开成: -i dir1 -i dir2 ...
    set(_include_dirs "")
    foreach(dir ${BGFX_SHADER_INCLUDE_DIRS})
        list(APPEND _include_dirs -i ${dir})
    endforeach()

    add_custom_command(
        OUTPUT ${OUTPUT_FILE}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${BGFX_SHADER_BIN_DIR}
        COMMAND ${SHADERC_EXECUTABLE}
            -f ${BGFX_SHADER_SRC_DIR}/${SHADER_FILE}
            -o ${OUTPUT_FILE}
            --type ${SHADER_TYPE}
            --platform linux
            --profile 120
            --varyingdef ${BGFX_SHADER_SRC_DIR}/varying.def.sc
            ${_include_dirs}
        DEPENDS
            ${BGFX_SHADER_SRC_DIR}/${SHADER_FILE}
            ${BGFX_SHADER_SRC_DIR}/varying.def.sc
        COMMENT "Compiling bgfx shader ${SHADER_FILE} -> ${OUTPUT_FILE}"
        VERBATIM
    )

    set(${OUT_VAR} ${OUTPUT_FILE} PARENT_SCOPE)
endfunction()

# 调用刚才的函数，为每个 shader 生成一个 .bin
bgfx_compile_shader(VS_BASIC_BIN v vs_basic.sc)
bgfx_compile_shader(FS_BASIC_BIN f fs_basic.sc)
# bgfx_compile_shader(VS_GRID_BIN  v vs_grid.sc)
# bgfx_compile_shader(FS_GRID_BIN  f fs_grid.sc)

# 聚合成一个自定义 target，方便依赖
add_custom_target(bgfx_shaders
    DEPENDS
        ${VS_BASIC_BIN}
        ${FS_BASIC_BIN}
        #${VS_GRID_BIN}
        #${FS_GRID_BIN}
)

add_library(bgfx::bgfx STATIC IMPORTED GLOBAL)

set_target_properties(bgfx::bgfx PROPERTIES
    IMPORTED_LOCATION_DEBUG   "${CMAKE_CURRENT_SOURCE_DIR}/third_party/bgfx/.build/linux64_gcc/bin/libbgfxDebug.a"
    IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/third_party/bgfx/.build/linux64_gcc/bin/libbgfxRelease.a"
    # IMPORTED_LOCATION_RELWITHDEBINFO "${CMAKE_CURRENT_SOURCE_DIR}/third_party/bgfx/lib/linux/libbgfxRelWithDebInfo.a"
    # IMPORTED_LOCATION_MINSIZEREL     "${CMAKE_CURRENT_SOURCE_DIR}/third_party/bgfx/lib/linux/libbgfxMinSizeRel.a"
)

add_library(bx::bx STATIC IMPORTED GLOBAL)
set_target_properties(bx::bx PROPERTIES
    IMPORTED_LOCATION_DEBUG   "${CMAKE_CURRENT_SOURCE_DIR}/third_party/bgfx/.build/linux64_gcc/bin/libbxDebug.a"
    IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/third_party/bgfx/.build/linux64_gcc/bin/libbxRelease.a"
)

add_library(bimg::bimg STATIC IMPORTED GLOBAL)
set_target_properties(bimg::bimg PROPERTIES
    IMPORTED_LOCATION_DEBUG   "${CMAKE_CURRENT_SOURCE_DIR}/third_party/bgfx/.build/linux64_gcc/bin/libbimgDebug.a"
    IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/third_party/bgfx/.build/linux64_gcc/bin/libbimgRelease.a"
)

add_executable(Laymann 
    src/test/bgfx_test.cpp

)

add_dependencies(Laymann bgfx_shaders)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(Laymann PRIVATE BX_CONFIG_DEBUG=1)
else()
    target_compile_definitions(Laymann PRIVATE BX_CONFIG_DEBUG=0)
endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third_party/glfw)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/bx/include
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/bgfx/include
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/bimg/include
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/glfw/include
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        bgfx::bgfx
        bx::bx
        bimg::bimg
        glfw
)


