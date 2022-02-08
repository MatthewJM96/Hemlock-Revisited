cmake_minimum_required(VERSION 3.22)

if(NOT EXISTS "${CMAKE_BINARY_DIR}/cmake/conan.cmake")
  message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
  file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/v0.16.1/conan.cmake"
                "${CMAKE_BINARY_DIR}/cmake/conan.cmake"
                EXPECTED_HASH SHA256=396e16d0f5eabdc6a14afddbcfff62a54a7ee75c6da23f32f7a31bc85db23484
                TLS_VERIFY ON)
endif()

include(${CMAKE_BINARY_DIR}/cmake/conan.cmake)

project(Hemlock_Revisited)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

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

conan_cmake_configure(REQUIRES glew/2.2.0
                               glm/0.9.9.8
                               sdl/2.0.18
                               sdl_ttf/2.0.18
                               libpng/1.6.37
                               concurrentqueue/1.0.3
                      GENERATORS cmake_find_package)

conan_cmake_autodetect(settings)

conan_cmake_install(PATH_OR_REFERENCE .
                    BUILD missing
                    REMOTE conancenter
                    SETTINGS ${settings})

list(APPEND CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR}")

find_package(glew REQUIRED)
find_package(glm REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(PNG REQUIRED)
find_package(concurrentqueue REQUIRED)

add_executable(Hemlock_Revisited
    "${PROJECT_SOURCE_DIR}/src/stdafx.cpp"
    "${PROJECT_SOURCE_DIR}/src/timing.cpp"
    "${PROJECT_SOURCE_DIR}/src/app/app.cpp"
    "${PROJECT_SOURCE_DIR}/src/app/screen.cpp"
    "${PROJECT_SOURCE_DIR}/src/graphics/font/font.cpp"
    "${PROJECT_SOURCE_DIR}/src/graphics/font/text_align.cpp"
    "${PROJECT_SOURCE_DIR}/src/graphics/glsl_program.cpp"
    "${PROJECT_SOURCE_DIR}/src/graphics/sprite/batcher.cpp"
    "${PROJECT_SOURCE_DIR}/src/graphics/sprite/string_drawer.cpp"
    "${PROJECT_SOURCE_DIR}/src/graphics/window.cpp"
    "${PROJECT_SOURCE_DIR}/src/graphics/window_manager.cpp"
    "${PROJECT_SOURCE_DIR}/src/io/glob.cpp"
    "${PROJECT_SOURCE_DIR}/src/io/image.cpp"
    "${PROJECT_SOURCE_DIR}/src/io/iomanager.cpp"
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
    ${glew_INCLUDE_DIRS}
    ${glm_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_ttf_INCLUDE_DIRS}
    ${PNG_INCLUDE_DIRS}
    ${concurrentqueue_INCLUDE_DIRS}
    "${PROJECT_SOURCE_DIR}/include"
)

# target_link_directories(Hemlock_Revisited
#     PUBLIC
#     "${PROJECT_SOURCE_DIR}/deps/lib"
# )

target_link_libraries(Hemlock_Revisited
    # GL
    glew::glew
    PNG::PNG
    SDL2::SDL2main
    SDL2::SDL2
    SDL2_ttf::SDL2_ttf
)
