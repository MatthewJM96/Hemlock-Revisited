#ifndef __hemlock_stdafx_h
#define __hemlock_stdafx_h

// Basics
#include <cstdlib>

// Containers
#include "blockingconcurrentqueue.h"
#include "concurrentqueue.h"
#include <map>
#include <queue>
#include <set>
#include <span>
#include <tuple>
#include <unordered_map>
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

// OpenGL
#include <GL/glew.h>

// GL Maths
#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Image Libs
#include <png.h>

// Our Types and Other Hints
#include "basic_concepts.hpp"
#include "decorators.hpp"
#include "types.hpp"

// Our Thread Handling
#include "thread_pool.hpp"

// Our File Handling Interface
#include "io/filesystem.hpp"
#include "io/path.hpp"

// Our Cache
#include "cache.hpp"

// Our Events
#include "event.hpp"

#endif // __hemlock_stdafx_h
