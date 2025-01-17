cmake_minimum_required(VERSION 3.28)
project(Schoo LANGUAGES CXX DESCRIPTION "a 3D Renderer made by schoo")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(IMGUI_DIR "${PROJECT_SOURCE_DIR}/external/imgui")
set(SHADER_SOURCE_DIR "${PROJECT_SOURCE_DIR}/shader")

find_package(Vulkan REQUIRED COMPONENTS glslc)
find_program(GLSLC_PROGRAM glslc REQUIRED)

if(NOT GLSLC_PROGRAM)
    message(FATAL_ERROR "glslc not found. This is needed to compile shaders. Please install shaderc or load the Vulkan SDK.")
else()
    message("-- Found glslc: ${Vulkan_GLSLC_EXECUTABLE}")
endif()

file(GLOB VERT_SHADERS "${SHADER_SOURCE_DIR}/*.vert")
file(GLOB FRAG_SHADERS "${SHADER_SOURCE_DIR}/*.frag")
# 将着色器文件转换为SPV格式
foreach(VERT_SHADER ${VERT_SHADERS})
    # 获取着色器的文件名（不带扩展名）
    get_filename_component(SHADER_NAME ${VERT_SHADER} NAME_WE)
    # 执行着色器转换，生成SPV文件
    execute_process(COMMAND ${GLSLC_PROGRAM} ${VERT_SHADER} -o ${SHADER_SOURCE_DIR}/spv/${SHADER_NAME}_vert.spv)
endforeach()
foreach(FRAG_SHADER ${FRAG_SHADERS})
    # 获取着色器的文件名（不带扩展名）
    get_filename_component(SHADER_NAME ${FRAG_SHADER} NAME_WE)
    # 执行着色器转换，生成SPV文件
    execute_process(COMMAND ${GLSLC_PROGRAM} ${FRAG_SHADER} -o ${SHADER_SOURCE_DIR}/spv/${SHADER_NAME}_frag.spv)
endforeach()

file(GLOB_RECURSE SRC ${PROJECT_SOURCE_DIR}/schoo/*.cpp)
file(GLOB IMGUI_SOURCES ${IMGUI_DIR}/*.cpp)
add_library(schoo_lib STATIC ${SRC} ${IMGUI_SOURCES}
        schoo/function/render/passes/fxaa_pass.hpp
        schoo/function/render/passes/outline_pass.hpp
        schoo/function/reflection/typeDescriptor.hpp
        schoo/function/reflection/memberFunction.hpp
        schoo/function/reflection/registry.hpp)
target_include_directories(schoo_lib PUBLIC
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/external
        ${PROJECT_SOURCE_DIR}/schoo
)
target_link_libraries(schoo_lib PRIVATE ${PROJECT_SOURCE_DIR}/External/lib/glfw3.lib)

target_link_libraries(schoo_lib PUBLIC Vulkan::Vulkan)

add_subdirectory(sandBox)



#include_directories(external/include)

#
#find_package(Vulkan REQUIRED)
#if(Vulkan_FOUND)
#    message("---->Find vulkan success :${Vulkan_INCLUDE_DIRS} ")
#endif()
#target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIRS})
#target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan)


