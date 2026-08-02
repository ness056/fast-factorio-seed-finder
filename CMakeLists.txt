cmake_minimum_required(VERSION 3.30)
project(seed_finder LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(warnings_interface INTERFACE)

target_compile_options(warnings_interface INTERFACE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic -mavx2 -fno-math-errno>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic -mavx2 -fno-math-errno>
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /fp:strict /arch:AVX2>
)

add_subdirectory(core)
add_subdirectory(image_generator)
add_subdirectory(benchmark)

add_subdirectory(seed_finders/example)
add_subdirectory(seed_finders/solo_any)
add_subdirectory(seed_finders/botless_solo_any)
add_subdirectory(seed_finders/mmp)
