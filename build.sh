#!/bin/bash

BUILD_TYPE=${1:-Debug}
BUILD_DIR=build
CMAKE_ARGS=(
  -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
  -DCMAKE_C_COMPILER=clang
  -DCMAKE_CXX_COMPILER=clang++
)

# Создаем build директорию, если её нет
if [ ! -d "$BUILD_DIR" ]; then
  cmake -S . -B "$BUILD_DIR" "${CMAKE_ARGS[@]}" -G Ninja
fi

# Проверяем, изменились ли CMake-файлы
CMAKELISTS_CHANGED=$(find . -name "CMakeLists.txt" -newer "$BUILD_DIR/CMakeCache.txt" 2>/dev/null)

if [ -n "$CMAKELISTS_CHANGED" ]; then
  echo "CMakeLists.txt changed. Re-running CMake..."
  cmake -S . -B "$BUILD_DIR" "${CMAKE_ARGS[@]}" -G Ninja
fi

# Сборка с ninja
ninja -C "$BUILD_DIR"

# Определяем исполняемый файл (первый найденный ELF-файл)
EXECUTABLE=$(find "$BUILD_DIR" -type f -executable -not -name "*.a" -not -name "*.so" | head -n1)

if [ -z "$EXECUTABLE" ]; then
  echo "Executable not found in $BUILD_DIR"
  exit 1
fi

echo "Running: $EXECUTABLE"
"$EXECUTABLE"