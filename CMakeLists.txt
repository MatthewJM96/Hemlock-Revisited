cmake_minimum_required(VERSION 3.22)

message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/0.18.1/conan.cmake"
            "${CMAKE_BINARY_DIR}/cmake/conan.cmake"
            EXPECTED_HASH SHA256=5cdb3042632da3efff558924eecefd580a0e786863a857ca097c3d1d43df5dcd
            TLS_VERIFY ON
            STATUS DOWNLOAD_STATUS)

list(GET DOWNLOAD_STATUS 0 DOWNLOAD_STATUS_CODE)
if(NOT ${DOWNLOAD_STATUS_CODE} EQUAL 0)
    message(FATAL_ERROR "Error downloading conan.cmake: ${DOWNLOAD_STATUS}")
else()
    message(STATUS "Download result: ${DOWNLOAD_STATUS}")
endif()

include(${CMAKE_BINARY_DIR}/cmake/conan.cmake)

project(Hemlock_Revisited)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

option(HEMLOCK_USING_SDL "Whether Hemlock uses the SDL library." ON)
option(HEMLOCK_USING_SDL_TTF "Whether Hemlock uses the SDL TTF library." ON)
option(HEMLOCK_USING_OPENGL "Whether Hemlock uses OpenGL." ON)
option(HEMLOCK_USING_VULKAN "Whether Hemlock uses Vulkan." OFF)
option(HEMLOCK_USING_LUA "Whether Hemlock uses Lua for scripting." ON)

option(HEMLOCK_FAST_DEBUG "Whether to compile debug builds with some optimisation." OFF)

option(HEMLOCK_ENABLE_ADDRESS_SANITIZER "Whether to compile with address sanitizer." OFF)
option(HEMLOCK_ENABLE_THREAD_SANITIZER "Whether to compile with thread sanitizer." OFF)
option(HEMLOCK_ENABLE_MEMORY_SANITIZER "Whether to compile with memory sanitizer." OFF)
option(HEMLOCK_ENABLE_GPERF_PROFILER "Whether to compile with GPerf profiler." OFF)

if (UNIX OR MINGW)
    set(CMAKE_CXX_FLAGS "-pthread")
    if (HEMLOCK_ENABLE_ADDRESS_SANITIZER)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
        set(CMAKE_CXX_FLAGS_RELEASE "-g -O3")
        set(CMAKE_CXX_FLAGS_DEBUG "-g -O1")
    elseif(HEMLOCK_ENABLE_THREAD_SANITIZER)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread -fno-omit-frame-pointer")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=thread -fno-omit-frame-pointer")
        set(CMAKE_CXX_FLAGS_RELEASE "-g -O2")
        set(CMAKE_CXX_FLAGS_DEBUG "-g -O1")
    elseif(HEMLOCK_ENABLE_MEMORY_SANITIZER)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=memory -fsanitize-memory-track-origins -fPIE -fno-omit-frame-pointer")
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=memory -fsanitize-memory-track-origins -pie -fno-omit-frame-pointer")
            set(CMAKE_CXX_FLAGS_RELEASE "-g -O2")
            set(CMAKE_CXX_FLAGS_DEBUG "-g -O1")
    elseif(HEMLOCK_ENABLE_GPERF_PROFILER)
        set(CMAKE_CXX_FLAGS_RELEASE "-g -O3")
        set(CMAKE_CXX_FLAGS_DEBUG "-g -O1")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wold-style-cast -Wshadow -Wformat=2 -Werror")
        set(CMAKE_CXX_FLAGS_RELEASE "-O3")
        set(CMAKE_CXX_FLAGS_DEBUG "-g")
    endif()

    if (HEMLOCK_FAST_DEBUG)
        set(CMAKE_CXX_FLAGS_DEBUG "-g -O1")
    endif()

    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wduplicated-cond -Wuseless-cast")
        # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmodules-ts -fconcepts-diagnostics-depth=3")
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wused-but-marked-unused")
        # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmodules -fprebuilt-module-path=${PROJECT_BINARY_DIR}/clm/${CMAKE_BUILD_TYPE}/")
    endif()
elseif (MSVC)
    set(CMAKE_CXX_FLAGS "/W3 /WX /EHsc /Zc:preprocessor")
    set(CMAKE_CXX_FLAGS_RELEASE "/MD /O2")
    set(CMAKE_CXX_FLAGS_DEBUG "/MDd /Od /Zi")

    # Don't error on secure warnings.
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS=1)
endif()

# if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
#     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmodules-ts")
# elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
#     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmodules -fprebuilt-module-path=${PROJECT_BINARY_DIR}/clm/${CMAKE_BUILD_TYPE}/")
# endif()

set(Hemlock_Requirements
    glm/0.9.9.8
    sdl/2.0.18
    sdl_ttf/2.0.18
    libpng/1.6.37
    concurrentqueue/1.0.3
    boost/1.78.0
    bullet3/3.24
    entt/3.10.1
)

if (HEMLOCK_USING_OPENGL)
    set(Hemlock_Requirements
        ${Hemlock_Requirements}
        glew/2.2.0
    )
endif()

if (HEMLOCK_USING_LUA)
    set(Hemlock_Requirements
        ${Hemlock_Requirements}
        luajit/2.0.5
    )
endif()

conan_cmake_configure(REQUIRES ${Hemlock_Requirements}
                      GENERATORS cmake_find_package)

conan_cmake_autodetect(settings)

conan_cmake_install(PATH_OR_REFERENCE .
                    BUILD missing
                    REMOTE conancenter
                    SETTINGS ${settings})

list(APPEND CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR}")

if (HEMLOCK_USING_OPENGL)
    find_package(OpenGL REQUIRED)
    find_package(GLEW REQUIRED)
endif()
find_package(glm REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(PNG REQUIRED)
find_package(concurrentqueue REQUIRED)
find_package(Boost COMPONENTS iostreams REQUIRED)
find_package(Bullet REQUIRED)
find_package(EnTT REQUIRED)
if (HEMLOCK_USING_LUA)
    find_package(luajit REQUIRED)
endif()

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

if (HEMLOCK_USING_LUA)
    add_compile_definitions(HEMLOCK_USING_LUA=1)
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
    "${PROJECT_SOURCE_DIR}/src/script/lua/lua_function.cpp"
    "${PROJECT_SOURCE_DIR}/src/thread/thread_workflow_builder.cpp"
    "${PROJECT_SOURCE_DIR}/src/ui/input/dispatcher.cpp"
    "${PROJECT_SOURCE_DIR}/src/ui/input/manager.cpp"
    "${PROJECT_SOURCE_DIR}/src/voxel/chunk.cpp"
    "${PROJECT_SOURCE_DIR}/src/voxel/chunk/grid.cpp"
    "${PROJECT_SOURCE_DIR}/src/voxel/coordinate_system.cpp"
    "${PROJECT_SOURCE_DIR}/src/voxel/graphics/renderer.cpp"
    "${PROJECT_SOURCE_DIR}/src/voxel/graphics/mesh/instance_manager.cpp"
    "${PROJECT_SOURCE_DIR}/src/voxel/io/chunk_file_task.cpp"
    "${PROJECT_SOURCE_DIR}/src/voxel/io/chunk_load_task.cpp"
    "${PROJECT_SOURCE_DIR}/src/voxel/io/chunk_save_task.cpp"
    "${PROJECT_SOURCE_DIR}/src/voxel/ray.cpp"
    "${PROJECT_SOURCE_DIR}/tests/main.cpp"
)

target_precompile_headers(Hemlock_Revisited
    PUBLIC
        include/stdafx.h
)

set(Hemlock_Include_Dirs
    ${glm_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_ttf_INCLUDE_DIRS}
    ${PNG_INCLUDE_DIRS}
    ${concurrentqueue_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    ${Bullet_INCLUDE_DIRS}
    ${EnTT_INCLUDE_DIRS}
)

if (HEMLOCK_USING_OPENGL)
    set(Hemlock_Include_Dirs
        ${Hemlock_Include_Dirs}
        ${glew_INCLUDE_DIRS}
    )
endif()

if (HEMLOCK_USING_LUA)
    set(Hemlock_Include_Dirs
        ${Hemlock_Include_Dirs}
        ${luajit_Include_Dirs}
    )
endif()

target_include_directories(Hemlock_Revisited
    PUBLIC
    "${PROJECT_SOURCE_DIR}/include"
)

target_include_directories(Hemlock_Revisited
    SYSTEM
    PUBLIC
    ${Hemlock_Include_Dirs}
)

if (UNIX OR MINGW)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        if(CMAKE_BUILD_TYPE STREQUAL "Release" OR HEMLOCK_FAST_DEBUG)
            target_include_directories(Hemlock_Revisited
                SYSTEM
                PUBLIC
                "${PROJECT_SOURCE_DIR}/deps/GCC-x64-Release/include"
            )

            target_link_directories(Hemlock_Revisited
                PUBLIC
                "${PROJECT_SOURCE_DIR}/deps/GCC-x64-Release/lib"
            )
        else()
            target_include_directories(Hemlock_Revisited
                SYSTEM
                PUBLIC
                "${PROJECT_SOURCE_DIR}/deps/GCC-x64-Debug/include"
            )

            target_link_directories(Hemlock_Revisited
                PUBLIC
                "${PROJECT_SOURCE_DIR}/deps/GCC-x64-Debug/lib"
            )
        endif()
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        if(CMAKE_BUILD_TYPE STREQUAL "Release" OR HEMLOCK_FAST_DEBUG)
            target_include_directories(Hemlock_Revisited
                SYSTEM
                PUBLIC
                "${PROJECT_SOURCE_DIR}/deps/Clang-x64-Release/include"
            )

            target_link_directories(Hemlock_Revisited
                PUBLIC
                "${PROJECT_SOURCE_DIR}/deps/Clang-x64-Release/lib"
            )
        else()
            target_include_directories(Hemlock_Revisited
                SYSTEM
                PUBLIC
                "${PROJECT_SOURCE_DIR}/deps/Clang-x64-Debug/include"
            )

            target_link_directories(Hemlock_Revisited
                PUBLIC
                "${PROJECT_SOURCE_DIR}/deps/Clang-x64-Debug/lib"
            )
        endif()
    endif()
else()
    target_include_directories(Hemlock_Revisited
    SYSTEM
    PUBLIC
    "${PROJECT_SOURCE_DIR}/deps/include"
    )

    target_link_directories(Hemlock_Revisited
    PUBLIC
    "${PROJECT_SOURCE_DIR}/deps/lib"
    )
endif()

if (HEMLOCK_USING_OPENGL)
    target_link_libraries(Hemlock_Revisited GLEW::GLEW)
    if (UNIX OR MINGW)
        target_link_libraries(Hemlock_Revisited GL)
    elseif (MSVC)
        target_link_libraries(Hemlock_Revisited OpenGL32)
    endif()
elseif (HEMLOCK_USING_VULKAN)
    target_link_libraries(Hemlock_Revisited vulkan)
endif()

if (HEMLOCK_USING_LUA)
    target_link_libraries(Hemlock_Revisited luajit::luajit)
endif()

if(HEMLOCK_ENABLE_GPERF_PROFILER)
    target_link_libraries(Hemlock_Revisited profiler)
endif()

target_link_libraries(Hemlock_Revisited
    PNG::PNG
    SDL2::SDL2main
    SDL2::SDL2
    SDL2_ttf::SDL2_ttf
    Boost::iostreams
    Bullet::Bullet
)

if(CMAKE_BUILD_TYPE STREQUAL "Release" OR HEMLOCK_FAST_DEBUG)
    target_link_libraries(Hemlock_Revisited
        FastNoise
    )
else()
    target_link_libraries(Hemlock_Revisited
        FastNoiseD
    )
endif()
