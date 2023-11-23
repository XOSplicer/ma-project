cmake_minimum_required(VERSION 3.19)

project(PhasarUnsafeRs)

set(CMAKE_EXPORT_COMPILE_COMMANDS YES)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# LLMV

find_package(LLVM 14 REQUIRED CONFIG)
find_library(LLVM_LIBRARY NAMES LLVM PATHS ${LLVM_LIBRARY_DIRS} NO_DEFAULT_PATH)
find_library(CLANG_LIBRARY NAMES clang-cpp libclang-cpp HINTS ${LLVM_LIBRARY_DIRS})

include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIB_PATH} ${LLVM_LIBRARY_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

# subdirectories

add_subdirectory(phasar)
add_subdirectory(corrosion)
add_subdirectory(find_unsafe_rs)
add_subdirectory(tools)
