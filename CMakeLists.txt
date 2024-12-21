cmake_minimum_required(VERSION 3.29)

include(FetchContent)
include(FindVulkan)

project(SimpleEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(GLFW_BUILD_WAYLAND ON)
set(GLFW_BUILD_X11 OFF)
option(PROFILING "" OFF)

if(NOT PROFILING)
  add_definitions(-U TRACY_ENABLE)
endif()

file(
  DOWNLOAD
  https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.40.2/CPM.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake
)
include(${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake)
CPMAddPackage("gh:libsdl-org/SDL#main")

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
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)

target_compile_options(${PROJECT_NAME} PRIVATE
 -Wall               # Все базовые предупреждения
 -Wextra             # Дополнительные предупреждения
 -Wpedantic          # Соответствие стандартам C++
 -Wshadow            # Предупреждение при затенении переменных
 -Wnon-virtual-dtor  # Виртуальные деструкторы
 -Wcast-align        # Несоответствие выравнивания при приведении типов
 -Woverloaded-virtual # Перекрытие виртуальных функций
 -Wconversion        # Предупреждение при опасных преобразованиях типов
 -Wsign-conversion   # Предупреждение при изменении знака
 -Wnull-dereference  # Отключение предупреждения о разыменовании nullptr
 -Wdouble-promotion  # Автоматическое преобразование float в double
 -Wformat=2          # Проверка форматирования строк
 -Wredundant-decls   # Повторяющиеся объявления
 -Wunreachable-code  # Невыполнимый код
)
set_source_files_properties(${IMGUI_SOURCES} PROPERTIES COMPILE_FLAGS "-w")