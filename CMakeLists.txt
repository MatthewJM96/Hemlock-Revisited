cmake_minimum_required(VERSION 3.22)

project(Hemlock_Revisited)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_FLAGS_DEBUG "-g -fconcepts-diagnostics-depth=3")

# if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
#     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmodules-ts")
# elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
#     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmodules -fprebuilt-module-path=${PROJECT_BINARY_DIR}/clm/${CMAKE_BUILD_TYPE}/")
# endif()

add_executable(Hemlock_Revisited
    "${PROJECT_SOURCE_DIR}/src/stdafx.cpp"
    "${PROJECT_SOURCE_DIR}/src/timing.cpp"
    "${PROJECT_SOURCE_DIR}/src/app/app.cpp"
    "${PROJECT_SOURCE_DIR}/src/app/screen.cpp"
    "${PROJECT_SOURCE_DIR}/src/graphics/font.cpp"
    "${PROJECT_SOURCE_DIR}/src/graphics/window.cpp"
    "${PROJECT_SOURCE_DIR}/src/graphics/window_manager.cpp"
    "${PROJECT_SOURCE_DIR}/src/io/image.cpp"
    "${PROJECT_SOURCE_DIR}/src/ui/input/dispatcher.cpp"
    "${PROJECT_SOURCE_DIR}/src/ui/input/manager.cpp"
    "${PROJECT_SOURCE_DIR}/tests/main.cpp"
)

target_precompile_headers(Hemlock_Revisited
    PUBLIC
        include/stdafx.h
)

target_include_directories(Hemlock_Revisited
    SYSTEM
    PUBLIC
    "${PROJECT_SOURCE_DIR}/deps/include"
    "${PROJECT_SOURCE_DIR}/include"
)

target_link_directories(Hemlock_Revisited
    PUBLIC
    "${PROJECT_SOURCE_DIR}/deps/lib"
)

target_link_libraries(Hemlock_Revisited
    GL
    GLEW
    glob
    png
    SDL2main
    SDL2
    SDL2_ttf
)
