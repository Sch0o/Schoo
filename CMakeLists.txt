cmake_minimum_required(VERSION 3.28)
project(Schoo LANGUAGES CXX DESCRIPTION "a 3D Renderer made by schoo")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)

#set(GLSLC_PROGRAM "D:/APP/VulkanSDK/Bin/glslc.exe")
find_package(Vulkan REQUIRED COMPONENTS glslc)
find_program(GLSLC_PROGRAM glslc REQUIRED)

if(NOT GLSLC_PROGRAM)
    message(FATAL_ERROR "glslc not found. This is needed to compile shaders. Please install shaderc or load the Vulkan SDK.")
else()
    message("-- Found glslc: ${Vulkan_GLSLC_EXECUTABLE}")
endif()

execute_process(COMMAND ${GLSLC_PROGRAM} ${CMAKE_SOURCE_DIR}/shader/shader.vert -o ${CMAKE_SOURCE_DIR}/shader/vert.spv)
execute_process(COMMAND ${GLSLC_PROGRAM} ${CMAKE_SOURCE_DIR}/shader/shader.frag -o ${CMAKE_SOURCE_DIR}/shader/frag.spv)


aux_source_directory(source SRC)
add_library(schoo_lib STATIC ${SRC})
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


