cmake_minimum_required(VERSION 4.0.1)

include(FetchContent)
include(FindVulkan)

project(SimpleEngine LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DSOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

file(
  DOWNLOAD
  https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.40.8/CPM.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake
)
include(${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake)
CPMAddPackage(
  NAME SDL3
  GIT_TAG main
  GITHUB_REPOSITORY libsdl-org/SDL
  OPTIONS
    "SDL_X11 ON"
    "SDL_OPENGL ON"
)

CPMAddPackage("gh:KhronosGroup/Vulkan-Hpp#main")
CPMAddPackage("gh:GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator#master")

CPMAddPackage(
    NAME spirv-reflect
    GITHUB_REPOSITORY KhronosGroup/SPIRV-Reflect
    GIT_TAG main
    OPTIONS
    "SPIRV_REFLECT_STATIC_LIB ON"
    "SPIRV_REFLECT_EXECUTABLE OFF"
)

CPMAddPackage(
  NAME glm
  GIT_TAG master
  GITHUB_REPOSITORY g-truc/glm
  OPTIONS
   "GLM_ENABLE_CXX_20 ON"
   "GLM_ENABLE_SIMD_AVX2 ON"
)

CPMAddPackage(
  NAME efsw
  GIT_TAG master
  GITHUB_REPOSITORY SpartanJ/efsw
)

CPMAddPackage(
  stb
  GITHUB_REPOSITORY nothings/stb
  GIT_TAG master
)

set(GLAD_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glad/include)
add_library(glad STATIC ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glad/src/gl.c)
target_include_directories(glad PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glad/include)

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -flto -funroll-loops -fomit-frame-pointer -ffast-math -fvisibility=hidden -DNDEBUG")

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)

set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/imgui)

file(GLOB IMGUI_SOURCES ${IMGUI_DIR}/*.cpp)

find_package(OpenGL REQUIRED)
find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})

file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp)
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_executable(${PROJECT_NAME}
    ${HEADERS}
    ${SOURCES}
    ${IMGUI_SOURCES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${stb_SOURCE_DIR}
    ${IMGUI_DIR}
    ${GLAD_INCLUDE_DIR}
)

find_program(GLSL_VALIDATOR NAMES glslangValidator)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${Vulkan_LIBRARIES}
    SDL3::SDL3
    GPUOpen::VulkanMemoryAllocator
    glm
    spirv-reflect-static
    OpenGL::GL
    glad
    efsw
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
  STB_IMAGE_IMPLEMENTATION
  STB_IMAGE_RESIZE_IMPLEMENTATION
  VULKAN_HPP_DISPATCH_LOADER_DYNAMIC
  VULKAN_HPP_NO_CONSTRUCTORS
  VULKAN_HPP_NO_EXCEPTIONS
  GLM_ENABLE_EXPERIMENTAL
  GLM_FORCE_DEPTH_ZERO_TO_ONE
  GLM_FORCE_RADIANS)

target_compile_options(${PROJECT_NAME} PRIVATE
 -Wall
 -Wextra
 -Wpedantic
 -Wshadow
 -Wnon-virtual-dtor
 -Wcast-align
 -Woverloaded-virtual
 -Wconversion
 -Wsign-conversion
 -Wnull-dereference
 -Wdouble-promotion
 -Wformat=2
 -Wredundant-decls
 -Wunreachable-code
)
set_source_files_properties(${IMGUI_SOURCES} PROPERTIES COMPILE_FLAGS "-w")

file(GLOB_RECURSE GLSL_SOURCE_FILES "shaders/*.frag" "shaders/*.vert")

foreach(GLSL ${GLSL_SOURCE_FILES})
  get_filename_component(FILE_NAME ${GLSL} NAME)
  set(SPIRV "${PROJECT_BINARY_DIR}/shaders/${FILE_NAME}.spv")
  add_custom_command(
    OUTPUT ${SPIRV}
    COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/shaders/"
    # COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
    COMMAND ${CMAKE_COMMAND} -E copy ${GLSL} "${PROJECT_BINARY_DIR}/shaders/${FILE_NAME}"
    DEPENDS ${GLSL})
  list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(Shaders DEPENDS ${SPIRV_BINARY_FILES})

add_dependencies(${PROJECT_NAME} Shaders)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders/"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_BINARY_DIR}/shaders"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders"
        )

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})