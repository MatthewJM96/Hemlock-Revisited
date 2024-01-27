# Hemlock Game Engine

Hemlock is a game engine developed as a project of learning, it is not expected to do everything, nor any particular thing particularly well. Where it does do things well, that's great though.

## Building

Building Hemlock is done via CMake, using Conan for package management where possible. Some dependencies may be provided by submodules, currently only libheatmap is provided in this manner.

### Requirements

On any platform, therefore, git, CMake and Conan must be installed. CMake version 3.22 is required at minimum right now.

There are included build options for instrumenting with Address/Thread/Memory Santitizers, as well as with the Gperf profiler. The former require nothing that meeting the C++20 requirement doesn't cover in choice of compiler versions, but instrumenting with the Gperf profiler requires:
##### Arch
* `extra/gperftools`

It is also recommended to install:
##### Arch
* `extra/kcachegrind`

#### Windows
Windows requires some version of the MSVC compiler that supports C++20. This can be obtained by, for example, installing the Visual Studio 2019 Community Edition IDE.
