#ifndef __hemlock_tests_performance_screen_chunk_hpp
#define __hemlock_tests_performance_screen_chunk_hpp

#include "voxel/chunk/chunk.hpp"
#include "voxel/chunk/grid.hpp"

namespace hemlock {
    namespace test {
        namespace performance_screen {
            using Chunk     = hvox::Chunk<>;
            using ChunkGrid = hvox::ChunkGrid<>;
        }  // namespace performance_screen
    }      // namespace test
}  // namespace hemlock
namespace htest = hemlock::test;

#endif  // __hemlock_tests_performance_screen_chunk_hpp
