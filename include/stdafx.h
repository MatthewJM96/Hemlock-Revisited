#ifndef __hemlock_stdafx_h
#define __hemlock_stdafx_h

// Basics
#include <cstdlib>

// Containers
#include <moodycamel/blockingconcurrentqueue.h>
#include <moodycamel/concurrentqueue.h>
#include <map>
#include <queue>
#include <set>
#include <span>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// Views
#include <ranges>
#include <iterator>

// Strings
#include <cstring>
#include <string>
#include <regex>

// Generics
#include <algorithm>
#include <functional>
#include <limits>
#include <memory>
#include <type_traits>
#include <utility>

// Thread Handling
#include <thread>

// Error Handling
#include <stdexcept>

// File Handling
#include <cstdio>
#include <filesystem>

// Streams
#include <iostream>
#include <fstream>
#include <sstream>

// SDL2
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#if defined(HEMLOCK_USING_VULKAN)
#include <SDL2/SDL_vulkan.h>
#endif // defined(HEMLOCK_USING_VULKAN)

#if defined(HEMLOCK_USING_OPENGL)
// OpenGL SDK
#include <GL/glew.h>
#endif // defined(HEMLOCK_USING_OPENGL)

#if defined(HEMLOCK_USING_VULKAN)
// Vulkan SDK
#include <vulkan/vulkan.h>
#endif // defined(HEMLOCK_USING_VULKAN)

// GL Maths
#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Image Libs
#include <png.h>

// Our generic preprocessor macros.
#include "preprocessor.hpp"

// Our constants.
#include "constants.hpp"
#include "decorators.hpp"
#include "debug.hpp"

// Our Types and Other Hints
#include "basic_concepts.hpp"
#include "types.hpp"

// Our Thread Handling
#include "thread/thread_pool.hpp"
#include "thread/thread_workflow_builder.h"
#include "thread/thread_workflow.hpp"

// Our File Handling Interface
#include "io/filesystem.hpp"
#include "io/path.hpp"

// Our Cache
#include "cache.hpp"

// Our Events
#include "event.hpp"

#endif // __hemlock_stdafx_h
