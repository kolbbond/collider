cmake_minimum_required(VERSION 3.30)
project(collider)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# find SDL2
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)

# add sources
set(source_list 
    src/board.cpp
    src/bitboards.cpp
    src/board.cpp
    src/collider.cpp
    src/debug.cpp
    src/display.cpp
    src/eval.cpp
    src/initial.cpp
    src/input.cpp
    src/legal.cpp
    src/move.cpp
    #    src/pv.cpp
    src/sdlp.cpp
    src/search.cpp
    )

add_executable(main src/main.cpp
    ${source_list}
    )

target_link_libraries(main
    SDL2
    SDL2_image
    )
target_include_directories(main PUBLIC
include
)
