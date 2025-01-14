cmake_minimum_required(VERSION 3.31)

include(FetchContent)
include(FindVulkan)

project(SimpleEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(
  DOWNLOAD
  https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.40.2/CPM.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake
)
include(${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake)
CPMAddPackage(
  NAME SDL3
  GIT_TAG main
  GITHUB_REPOSITORY libsdl-org/SDL
  OPTIONS
    "SDL_X11 ON"
)
CPMAddPackage("gh:GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator#master")
CPMAddPackage(
  NAME glm
  GIT_TAG master
  GITHUB_REPOSITORY g-truc/glm
  OPTIONS
   "GLM_ENABLE_CXX_20 ON"
   "GLM_ENABLE_SIMD_AVX2 ON"
)

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -flto -funroll-loops -fomit-frame-pointer -ffast-math -fvisibility=hidden -DNDEBUG")

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)

set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/imgui)

file(GLOB IMGUI_SOURCES ${IMGUI_DIR}/*.cpp)

find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})

file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp)
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES} ${IMGUI_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_include_directories(${PROJECT_NAME} PRIVATE ${IMGUI_DIR})

find_program(GLSL_VALIDATOR NAMES glslangValidator)
target_link_libraries(${PROJECT_NAME} PRIVATE ${Vulkan_LIBRARIES} SDL3::SDL3 GPUOpen::VulkanMemoryAllocator glm)
target_compile_definitions(${PROJECT_NAME} PRIVATE
  STB_IMAGE_IMPLEMENTATION
  STB_IMAGE_RESIZE_IMPLEMENTATION
  VULKAN_HPP_NO_CONSTRUCTORS
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