#ifndef __hemlock_voxel_chunk_navmesh_hpp
#define __hemlock_voxel_chunk_navmesh_hpp

#include "algorithm/acs/graph/state.hpp"
#include "voxel/coordinate_system.h"

namespace hemlock {
    namespace voxel {
        namespace ai {
            struct ChunkNavmeshNode {
                BlockChunkPosition block_pos;
                ChunkGridPosition  chunk_pos;

                bool operator==(const ChunkNavmeshNode& rhs) const {
                    return block_pos == rhs.block_pos && chunk_pos == rhs.chunk_pos;
                }
            };

            using ChunkNavmesh = algorithm::GraphMap<ChunkNavmeshNode, false>;
            using ChunkNavmeshVertexDescriptor
                = algorithm::VertexDescriptor<ChunkNavmeshNode, false>;
        }  // namespace ai
    }      // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

namespace std {
    template <>
    struct hash<hvox::ai::ChunkNavmeshNode> {
        size_t operator()(const hvox::ai::ChunkNavmeshNode& node) const {
            // TODO(Matthew): Here and elsewhere, is packing done portably? We can
            //                likely not directly serialise bit-fields due to endian
            //                concerns, but at least within runtime does it achieve
            //                the same as explicit bit masking/shifting across
            //                platforms?
            union {
                HEMLOCK_PACKED_STRUCT(struct {
                    i64 block_x : 5;
                    i64 block_y : 5;
                    i64 block_z : 5;
                    i64 chunk_x : 19;
                    i64 chunk_y : 11;
                    i64 chunk_z : 19;
                }) coords;

                ui64 id;
            } packed_node;

            packed_node.coords.block_x = node.block_pos.x;
            packed_node.coords.block_y = node.block_pos.y;
            packed_node.coords.block_z = node.block_pos.z;

            packed_node.coords.chunk_x = node.chunk_pos.x;
            packed_node.coords.chunk_y = node.chunk_pos.y;
            packed_node.coords.chunk_z = node.chunk_pos.z;

            return packed_node.id;
        }
    };
}  // namespace std

#endif  // __hemlock_voxel_chunk_navmesh_hpp
