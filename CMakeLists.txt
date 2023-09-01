cmake_minimum_required(VERSION 3.10)
project(TRIAD-6 VERSION 0.1 LANGUAGES C)

find_package(SDL2 REQUIRED)

add_executable(TRIAD-6 src/bct.c src/bvs1.c src/cpu.c src/machine.c src/main.c)
target_include_directories(TRIAD-6 PUBLIC "${PROJECT_SOURCE_DIR}/include" "${SDL2_INCLUDE_DIRS}")
target_link_libraries(TRIAD-6 SDL2::SDL2)
