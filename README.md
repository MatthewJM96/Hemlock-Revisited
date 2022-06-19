# Hemlock Game Engine

Hemlock is a game engine developed as a project of learning, it is not expected to do everything, nor any particular thing particularly well. Where it does do things well, that's great though.

## Building

Building Hemlock is done via CMake, using Conan for package management where possible.

### Requirements

On any platform, therefore, CMake and Conan must be installed. CMake version 3.22 is required at minimum right now.

In the location where this codebase is cloned, one create a directory, `deps`, inside which the following must be installed:
#### Dependencies Outside Conan
* [Fast Noise 2](https://github.com/Auburn/FastNoise2/releases/tag/v0.9.4-alpha)

The structure expected is the usual `deps/bin`, `deps/include`, `deps/lib`, and can be achieved by passing `--install-prefix=<path/to/deps>` to CMake for projects with a CMake build routine, or `--prefix=<path/to/deps>` in the case of a project with a `configure` script. 

#### Linux
In addition, on Linux, the following packages are required:
##### Arch
* `base-devel`
* `xtrans`
##### Debian
* `build-essential`
* `pkg-config`
* `xtrans-dev`

There are included build options for instrumenting with Address/Thread/Memory Santitizers, as well as with the Gperf profiler. The former require nothing that meeting the C++20 requirement doesn't cover in choice of compiler versions, but instrumenting with the Gperf profiler requires:
##### Arch
* `extra/gperftools`

It is also recommended to install:
##### Arch
* `extra/kcachegrind`

#### Windows
Windows requires some version of the MSVC compiler that supports C++20. This can be obtained by, for example, installing the Visual Studio 2019 Community Edition IDE.
