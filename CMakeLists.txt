cmake_minimum_required(VERSION 3.22)

project(Hemlock_Revisited)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

option(HEMLOCK_USING_SDL "Whether Hemlock uses the SDL library." ON)
option(HEMLOCK_USING_SDL_TTF "Whether Hemlock uses the SDL TTF library." ON)
option(HEMLOCK_USING_OPENGL "Whether Hemlock uses OpenGL." OFF)
option(HEMLOCK_USING_VULKAN "Whether Hemlock uses Vulkan." ON)

if (UNIX OR MINGW)
    set(CMAKE_CXX_FLAGS "-pthread -Wall -Wextra -Wold-style-cast -Wshadow -Wformat=2 -Werror")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
    set(CMAKE_CXX_FLAGS_DEBUG "-g")

    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wduplicated-cond -Wuseless-cast")
        # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmodules-ts -fconcepts-diagnostics-depth=3")
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wused-but-marked-unused")
        # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmodules -fprebuilt-module-path=${PROJECT_BINARY_DIR}/clm/${CMAKE_BUILD_TYPE}/")
    endif()
elseif (MSVC)
    set(CMAKE_CXX_FLAGS "/W4 /WX /EHsc")
    set(CMAKE_CXX_FLAGS_RELEASE "/O3")
    set(CMAKE_CXX_FLAGS_DEBUG "/Od /Zi")
endif()

# if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
#     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmodules-ts")
# elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
#     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmodules -fprebuilt-module-path=${PROJECT_BINARY_DIR}/clm/${CMAKE_BUILD_TYPE}/")
# endif()

if (HEMLOCK_USING_SDL)
    add_compile_definitions(HEMLOCK_USING_SDL=1)
endif()

if (HEMLOCK_USING_SDL_TTF)
    add_compile_definitions(HEMLOCK_USING_SDL_TTF=1)
endif()

if (HEMLOCK_USING_OPENGL)
    add_compile_definitions(HEMLOCK_USING_OPENGL=1)
endif()

if (HEMLOCK_USING_VULKAN)
    add_compile_definitions(HEMLOCK_USING_VULKAN=1)
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(DEBUG=1)
endif()

add_executable(Hemlock_Revisited
    "${PROJECT_SOURCE_DIR}/src/stdafx.cpp"
    "${PROJECT_SOURCE_DIR}/src/timing.cpp"
    "${PROJECT_SOURCE_DIR}/src/app/app_base.cpp"
    "${PROJECT_SOURCE_DIR}/src/app/screen_base.cpp"
    "${PROJECT_SOURCE_DIR}/src/app/single_window_app.cpp"
    "${PROJECT_SOURCE_DIR}/src/app/process/manager_base.cpp"
    "${PROJECT_SOURCE_DIR}/src/app/process/process_base.cpp"
    "${PROJECT_SOURCE_DIR}/src/app/window/window_base.cpp"
    "${PROJECT_SOURCE_DIR}/src/app/window/window.cpp"
    "${PROJECT_SOURCE_DIR}/src/camera/basic_first_person_camera.cpp"
    "${PROJECT_SOURCE_DIR}/src/camera/basic_orthographic_camera.cpp"
    "${PROJECT_SOURCE_DIR}/src/graphics/font/font.cpp"
    "${PROJECT_SOURCE_DIR}/src/graphics/font/text_align.cpp"
    "${PROJECT_SOURCE_DIR}/src/graphics/glsl_program.cpp"
    "${PROJECT_SOURCE_DIR}/src/graphics/mesh.cpp"
    "${PROJECT_SOURCE_DIR}/src/graphics/pixel.cpp"
    "${PROJECT_SOURCE_DIR}/src/graphics/sprite/batcher.cpp"
    "${PROJECT_SOURCE_DIR}/src/graphics/sprite/string_drawer.cpp"
    "${PROJECT_SOURCE_DIR}/src/io/glob.cpp"
    "${PROJECT_SOURCE_DIR}/src/io/image.cpp"
    "${PROJECT_SOURCE_DIR}/src/io/iomanager.cpp"
    "${PROJECT_SOURCE_DIR}/src/ui/input/dispatcher.cpp"
    "${PROJECT_SOURCE_DIR}/src/ui/input/manager.cpp"
    "${PROJECT_SOURCE_DIR}/src/voxel/chunk.cpp"
    "${PROJECT_SOURCE_DIR}/src/voxel/coordinate_system.cpp"
    "${PROJECT_SOURCE_DIR}/src/voxel/generator.cpp"
    "${PROJECT_SOURCE_DIR}/src/voxel/grid.cpp"
    "${PROJECT_SOURCE_DIR}/src/voxel/mesher.cpp"
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


if (HEMLOCK_USING_OPENGL)
    target_link_libraries(Hemlock_Revisited GL GLEW)
elseif (HEMLOCK_USING_VULKAN)
    target_link_libraries(Hemlock_Revisited vulkan)
endif()

target_link_libraries(Hemlock_Revisited
    png
    SDL2main
    SDL2
    SDL2_ttf
    FastNoise
)
