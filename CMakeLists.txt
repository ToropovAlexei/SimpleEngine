cmake_minimum_required(VERSION 3.29)

include(FetchContent)
include(FindVulkan)

project(VulkanMine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(GLFW_BUILD_WAYLAND ON)
set(GLFW_BUILD_X11 OFF)
option(PROFILING "" OFF)

if(NOT PROFILING)
  add_definitions(-U TRACY_ENABLE)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -flto -funroll-loops -fomit-frame-pointer -ffast-math -fvisibility=hidden -DNDEBUG")

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)

set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/imgui)

file(GLOB IMGUI_SOURCES ${IMGUI_DIR}/*.cpp)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_SHALLOW ON
)

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG master
    GIT_SHALLOW ON
)

FetchContent_Declare(
    tinygltf
    GIT_REPOSITORY https://github.com/syoyo/tinygltf.git
    GIT_TAG release
    GIT_SHALLOW ON
)

FetchContent_Declare(
    FastNoise2
    GIT_REPOSITORY https://github.com/Auburn/FastNoise2.git
    GIT_TAG master
    GIT_SHALLOW ON
)

FetchContent_Declare(
	glm
	GIT_REPOSITORY	https://github.com/g-truc/glm.git
	GIT_TAG 	bf71a834948186f4097caa076cd2663c69a10e1e #refs/tags/1.0.1
)

FetchContent_Declare(
  stb
  GIT_REPOSITORY https://github.com/nothings/stb.git
  GIT_TAG master
)

FetchContent_Declare (
  tracy
  GIT_REPOSITORY https://github.com/wolfpld/tracy.git
  GIT_TAG v0.11.1
  GIT_SHALLOW TRUE
  GIT_PROGRESS TRUE
)

FetchContent_Declare (
  VulkanMemoryAllocator
  GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
  GIT_TAG master
  GIT_SHALLOW TRUE
  GIT_PROGRESS TRUE
)

# FetchContent_Declare (
#   Taskflow
#   GIT_REPOSITORY https://github.com/taskflow/taskflow.git
#   GIT_TAG master
#   GIT_SHALLOW TRUE
# )

find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})
# TODO REMOVE LIBS/VulkanMemoryAllocator
# add_subdirectory(libs/VulkanMemoryAllocator)

FetchContent_MakeAvailable(glm glfw stb tracy nlohmann_json FastNoise2 VulkanMemoryAllocator tinygltf)

file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp)
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES} ${IMGUI_SOURCES})

# add_definitions(-DGLM_FORCE_RADIANS -DGLM_FORCE_DEPTH_ZERO_TO_ONE -DGLFW_INCLUDE_VULKAN)

# find_package(VulkanMemoryAllocator CONFIG REQUIRED PATHS /usr/include/vk_mem_alloc.h)
find_program(GLSL_VALIDATOR NAMES glslangValidator)
# target_link_libraries(${PROJECT_NAME} PUBLIC glfw glm::glm ${Vulkan_LIBRARIES} GPUOpen::VulkanMemoryAllocator nlohmann_json::nlohmann_json FastNoise2 tinygltf) # TracyClient
# target_include_directories(${PROJECT_NAME} PRIVATE ${stb_SOURCE_DIR} ${IMGUI_DIR})
# target_compile_definitions(${PROJECT_NAME} PRIVATE STB_IMAGE_IMPLEMENTATION STB_IMAGE_RESIZE_IMPLEMENTATION VULKAN_HPP_NO_CONSTRUCTORS)
# target_link_libraries (${PROJECT_NAME} PUBLIC Tracy::TracyClient)

#target_compile_options(${PROJECT_NAME} PRIVATE
#  -Wall               # Все базовые предупреждения
#  -Wextra             # Дополнительные предупреждения
#  -Wpedantic          # Соответствие стандартам C++
#  -Wshadow            # Предупреждение при затенении переменных
#  -Wnon-virtual-dtor  # Виртуальные деструкторы
#  -Wcast-align        # Несоответствие выравнивания при приведении типов
#  -Woverloaded-virtual # Перекрытие виртуальных функций
#  -Wconversion        # Предупреждение при опасных преобразованиях типов
#  -Wsign-conversion   # Предупреждение при изменении знака
#  -Wnull-dereference  # Отключение предупреждения о разыменовании nullptr
#  -Wdouble-promotion  # Автоматическое преобразование float в double
#  -Wformat=2          # Проверка форматирования строк
#  -Wredundant-decls   # Повторяющиеся объявления
#  -Wunreachable-code  # Невыполнимый код
#)

# target_compile_options(glfw PRIVATE -w)

# file(GLOB_RECURSE GLSL_SOURCE_FILES "shaders/*.frag" "shaders/*.vert")

# foreach(GLSL ${GLSL_SOURCE_FILES})
#   get_filename_component(FILE_NAME ${GLSL} NAME)
#   set(SPIRV "${PROJECT_BINARY_DIR}/shaders/${FILE_NAME}.spv")
#   add_custom_command(
#     OUTPUT ${SPIRV}
#     COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/shaders/"
#     COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
#     DEPENDS ${GLSL})
#   list(APPEND SPIRV_BINARY_FILES ${SPIRV})
# endforeach(GLSL)

# add_custom_target(Shaders DEPENDS ${SPIRV_BINARY_FILES})

# add_dependencies(${PROJECT_NAME} Shaders)

# add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders/"
#     COMMAND ${CMAKE_COMMAND} -E copy_directory
#         "${PROJECT_BINARY_DIR}/shaders"
#         "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders"
#         )
# file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/res DESTINATION ${CMAKE_CURRENT_BINARY_DIR})