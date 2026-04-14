#add_library(core)
#target_sources()
add_executable(core
    #PRIVATE
        ${CMAKE_SOURCE_DIR}/src/citygen/gen_main.cpp
        ${CMAKE_SOURCE_DIR}/src/system/files.cpp
        ${CMAKE_SOURCE_DIR}/src/core/renderer.cpp
        ${CMAKE_SOURCE_DIR}/src/utils/planLoader.cpp
        ${CMAKE_SOURCE_DIR}/src/core/simulator.cpp
        ${CMAKE_SOURCE_DIR}/src/core/geometry.cpp
        ${CMAKE_SOURCE_DIR}/src/core/ecs.cpp
        ${CMAKE_SOURCE_DIR}/src/core/ecs_systems.cpp
        ${CMAKE_SOURCE_DIR}/src/core/space_analyse.cpp
        ${CMAKE_SOURCE_DIR}/src/core/framework.cpp
    #PUBLIC       
)

target_include_directories(core
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/third_party/bgfx/bx/include
        ${CMAKE_SOURCE_DIR}/third_party/bgfx/bgfx/include
        ${CMAKE_SOURCE_DIR}/third_party/bgfx/bimg/include
        ${CMAKE_SOURCE_DIR}/third_party/bullet3/src
        ${CMAKE_SOURCE_DIR}/third_party/glfw/include
        ${CMAKE_SOURCE_DIR}/third_party/nlohmann/include
        ${CMAKE_SOURCE_DIR}/third_party/eigen
        ${CMAKE_SOURCE_DIR}/third_party/libigl/include
        ${CMAKE_SOURCE_DIR}/third_party/openmesh
        ${CMAKE_SOURCE_DIR}/third_party/entt/single_include
)

target_compile_features(core PUBLIC cxx_std_20)

target_link_libraries(core
    PRIVATE
        bgfx
        bx
        bimg
        glfw
        BulletDynamics
        BulletCollision
        LinearMath
)

set(SHADERC_EXECUTABLE 
    "${CMAKE_SOURCE_DIR}/build/third_party/bgfx/cmake/bgfx/shaderc")

set(BGFX_SHADER_INCLUDE_DIRS 
    "${CMAKE_SOURCE_DIR}/third_party/bgfx/bgfx/examples/common" 
    "${CMAKE_SOURCE_DIR}/third_party/bgfx/bgfx/src" )

set(BGFX_SHADER_SRC_DIR  "${CMAKE_SOURCE_DIR}/src/shaders")
set(BGFX_SHADER_BIN_DIR  "${CMAKE_SOURCE_DIR}/build/src/shaders")

function(bgfx_compile_shader OUT_VAR SHADER_TYPE SHADER_FILE)
    get_filename_component(FILE_NAME_WE ${SHADER_FILE} NAME_WE)
    set(OUTPUT_FILE "${BGFX_SHADER_BIN_DIR}/${FILE_NAME_WE}.bin")

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

bgfx_compile_shader(VS_BASIC_BIN v vs_basic.sc)
bgfx_compile_shader(FS_BASIC_BIN f fs_basic.sc)
bgfx_compile_shader(VS_GRID_BIN  v vs_grid.sc)
bgfx_compile_shader(FS_GRID_BIN  f fs_grid.sc)
bgfx_compile_shader(VS_FIELD_BIN v vs_field.sc)
bgfx_compile_shader(FS_FIELD_BIN f fs_field.sc)

bgfx_compile_shader(VS_BASIC_TEX_BIN v vs_basic_tex.sc)
bgfx_compile_shader(FS_BASIC_TEX_BIN f fs_basic_tex.sc)

add_custom_target(bgfx_shaders
    DEPENDS
        ${VS_BASIC_BIN}
        ${FS_BASIC_BIN}
        ${VS_GRID_BIN}
        ${FS_GRID_BIN}
        ${VS_FIELD_BIN}
        ${FS_FIELD_BIN}
        ${VS_BASIC_TEX_BIN}
        ${FS_BASIC_TEX_BIN}
)

add_dependencies(bgfx_shaders shaderc)
add_dependencies(core bgfx_shaders)

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/data
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/data ${CMAKE_BINARY_DIR}/data
    COMMENT "Copying files..."
)

add_custom_target(CopyFiles ALL DEPENDS ${CMAKE_BINARY_DIR}/data)