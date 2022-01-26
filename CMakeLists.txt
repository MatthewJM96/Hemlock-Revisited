cmake_minimum_required(VERSION 3.22)

project(Hemlock_Revisited)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -fmodules-ts")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_FLAGS_DEBUG "-g")

add_executable(Hemlock_Revisited
    "${PROJECT_SOURCE_DIR}/src/event.cpp"
    "${PROJECT_SOURCE_DIR}/src/types.cpp"
    "${PROJECT_SOURCE_DIR}/tests/main.cpp"
)

target_include_directories(Hemlock_Revisited
    SYSTEM
    PUBLIC
    "${PROJECT_SOURCE_DIR}/deps"
    "${PROJECT_SOURCE_DIR}/include"
)

# target_link_libraries(Hemlock_Revisited
# 
# )
