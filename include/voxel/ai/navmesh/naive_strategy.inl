#include "memory/handle.hpp"
#include "voxel/chunk/chunk.h"
#include "voxel/coordinate_system.h"

// TODO(Matthew): right now we are hardcoding in what gets added to the navmesh. This is
//                fine for now, but perhaps we would like to avoid requiring all the
//                boilerplate including handling of race conditions we do here in
//                game-specific implementations and so actually template on the strategy
//                used for choosing nodes of the navmesh.
// TODO(Matthew): we have here chosen to implement a any-gap navmesh with no annotation
//                of height of the gap. Do we want different implementations? As part of
//                the above provide each as optionals with a default?
// TODO(Matthew): features to consider support for:
//                  - diagnonal movement
//                  - liquids
//                  - jumping with horizontal distance as well as vertical,
//                  - climbing (ladders, or generally),
//                  - LODing navmeshes
//                  - gravity direction
// TODO(Matthew): The boost graph data structure is better replaced with a custom data
//                structure, especially as then can ensure locality while doing a graph
//                separately in each chunk, which lets us reduce memory usage and
//                improve performance of chunk unloading. (Maybe avoid the hash limits
//                meaning fewer chunks can be navmeshed as can be theoretically loaded -
//                not that this is a big issue).
// TODO(Matthew): In any case, the following algorithm is very inefficient.
// TODO(Matthew): To efficiently handle changes in a neighbour affecting bulk
//                navmeshing, implement an on_X_face_change event in chunks?

// TODO(Matthew): Notably slowed down stitch time in access pattern change... visit
// this.

namespace hemlock::voxel::ai::impl {
    inline ChunkNavmeshVertexDescriptor
    get_vertex(hvox::ai::ChunkNavmesh& navmesh, const ChunkNavmeshNode& coord) {
        try {
            return navmesh.coord_vertex_map.at(coord);
        } catch (std::out_of_range&) {
            auto vertex                     = boost::add_vertex(navmesh.graph);
            navmesh.coord_vertex_map[coord] = vertex;
            return vertex;
        }
    }
}  // namespace hemlock::voxel::ai::impl

template <hvox::IdealBlockConstraint IsSolid>
void hvox::ai::NaiveNavmeshStrategy<IsSolid>::do_bulk(
    hmem::Handle<ChunkGrid>, hmem::Handle<Chunk> chunk
) const {
    auto chunk_pos = chunk->position;

    std::shared_lock<std::shared_mutex> block_lock;
    const Block*                        chunk_blocks = chunk->blocks.get(block_lock);
    std::unique_lock<std::shared_mutex> navmesh_lock;
    hvox::ai::ChunkNavmesh& chunk_navmesh = chunk->navmesh.get(navmesh_lock);

    const IsSolid is_solid{};

    //----------------------------------------------------------------------------------
    //
    // Navmesh within this chunk.
    //----------------------------------------------------------------------------------

    // We don't navmesh up to the very top face of the chunk, as this is handled in
    // stitching phase. To step onto "top face" is to step into the above-neighbouring
    // chunk.

    /*****************************\
     * In-chunk navigable check. *
    \*****************************/

    // TODO(Matthew): Is this optimised well by compiler?
    auto do_navigable_check = [&](const ChunkNavmeshVertexDescriptor& block_vertex,
                                  BlockChunkPosition                  start_offset,
                                  BlockChunkPosition                  offset,
                                  i64                                 start,
                                  i64                                 end) {
        for (i64 y_off = start; y_off > end; --y_off) {
            BlockIndex above_candidate_index
                = hvox::block_index(static_cast<i64v3>(offset) + i64v3{ 0, y_off, 0 });
            const Block* above_candidate_block = &chunk_blocks[above_candidate_index];

            BlockIndex candidate_index = hvox::block_index(
                static_cast<i64v3>(offset) + i64v3{ 0, y_off - 1, 0 }
            );
            const Block* candidate_block = &chunk_blocks[candidate_index];

            if (is_solid(candidate_block) && !is_solid(above_candidate_block)) {
                // TODO(Matthew): Put this in to make up for forgetting to ask if air
                //                gap exists to allow step up or down.
                //                  / _   _ /
                //                  _|     |_
                //                I.e. the slashed blocks in these two examples.
                //                Hardcoded for one step as that is all we're doing for
                //                now.
                if (y_off - 1 == 1) {
                    BlockIndex twice_above_start_index = hvox::block_index(
                        start_offset + BlockChunkPosition{ 0, 2, 0 }
                    );
                    const Block* twice_above_start_block
                        = &chunk_blocks[twice_above_start_index];

                    if (is_solid(twice_above_start_block)) continue;
                } else if (y_off - 1 == -1) {
                    BlockIndex twice_above_candidate_index
                        = hvox::block_index(offset + BlockChunkPosition{ 0, 1, 0 });
                    const Block* twice_above_candidate_block
                        = &chunk_blocks[twice_above_candidate_index];

                    if (is_solid(twice_above_candidate_block)) continue;
                }

                ChunkNavmeshNode candidate_block_coord = {
                    static_cast<i64v3>(offset) + i64v3{0, y_off - 1, 0},
                      chunk_pos
                };

                // Ensure node exists for this block.
                ChunkNavmeshVertexDescriptor candidate_block_vertex
                    = impl::get_vertex(chunk_navmesh, candidate_block_coord);

                boost::add_edge(
                    block_vertex, candidate_block_vertex, chunk_navmesh.graph
                );
                boost::add_edge(
                    candidate_block_vertex, block_vertex, chunk_navmesh.graph
                );
            }
        }
    };

    /*******************************\
     * Navmesh bulk of this chunk. *
     *   i.e. not faces of chunk.  *
    \*******************************/

    for (BlockChunkPositionCoord x = 1; x < CHUNK_LENGTH - 1; ++x) {
        for (BlockChunkPositionCoord z = 1; z < CHUNK_LENGTH - 1; ++z) {
            for (BlockChunkPositionCoord y = 1; y < CHUNK_LENGTH - 2; ++y) {
                BlockIndex   block_index = hvox::block_index({ x, y, z });
                const Block* block       = &chunk_blocks[block_index];

                // Only consider block if it is solid.
                if (!is_solid(block)) continue;

                BlockIndex   block_above_index = hvox::block_index({ x, y + 1, z });
                const Block* block_above       = &chunk_blocks[block_above_index];

                // Only consider block if it is not covered above.
                if (is_solid(block_above)) continue;

                // Ensure node exists for this block.
                ChunkNavmeshNode block_coord = {
                    {x, y, z},
                    chunk_pos
                };
                ChunkNavmeshVertexDescriptor block_vertex
                    = impl::get_vertex(chunk_navmesh, block_coord);

                // Left
                do_navigable_check(block_vertex, { x, y, z }, { x - 1, y, z }, 2, -1);

                // Right
                do_navigable_check(block_vertex, { x, y, z }, { x + 1, y, z }, 2, -1);

                // Front
                do_navigable_check(block_vertex, { x, y, z }, { x, y, z + 1 }, 2, -1);

                // Back
                do_navigable_check(block_vertex, { x, y, z }, { x, y, z - 1 }, 2, -1);
            }

            // Second-to-top case.
            {
                BlockIndex block_index = hvox::block_index({ x, CHUNK_LENGTH - 2, z });
                const Block* block     = &chunk_blocks[block_index];

                // Only consider block if it is solid.
                if (!is_solid(block)) continue;

                BlockIndex block_above_index
                    = hvox::block_index({ x, CHUNK_LENGTH - 1, z });
                const Block* block_above = &chunk_blocks[block_above_index];

                // Only consider block if it is not covered above.
                if (is_solid(block_above)) continue;

                // Ensure node exists for this block.
                ChunkNavmeshNode block_coord = {
                    {x, CHUNK_LENGTH - 2, z},
                    chunk_pos
                };
                ChunkNavmeshVertexDescriptor block_vertex
                    = impl::get_vertex(chunk_navmesh, block_coord);

                // Left
                do_navigable_check(
                    block_vertex,
                    { x, CHUNK_LENGTH - 2, z },
                    { x - 1, CHUNK_LENGTH - 2, z },
                    1,
                    -1
                );

                // Right
                do_navigable_check(
                    block_vertex,
                    { x, CHUNK_LENGTH - 2, z },
                    { x + 1, CHUNK_LENGTH - 2, z },
                    1,
                    -1
                );

                // Front
                do_navigable_check(
                    block_vertex,
                    { x, CHUNK_LENGTH - 2, z },
                    { x, CHUNK_LENGTH - 2, z + 1 },
                    1,
                    -1
                );

                // Back
                do_navigable_check(
                    block_vertex,
                    { x, CHUNK_LENGTH - 2, z },
                    { x, CHUNK_LENGTH - 2, z - 1 },
                    1,
                    -1
                );
            }
        }
    }

    /********************************\
     * Navmesh left and right faces *
     *   except for edges.          *
    \********************************/

    for (BlockChunkPositionCoord z = 1; z < CHUNK_LENGTH - 1; ++z) {
        for (BlockChunkPositionCoord y = 1; y < CHUNK_LENGTH - 2; ++y) {
            // Left Face
            {
                BlockIndex   block_index = hvox::block_index({ 0, y, z });
                const Block* block       = &chunk_blocks[block_index];

                // Only consider block if it is solid.
                if (!is_solid(block)) continue;

                BlockIndex   block_above_index = hvox::block_index({ 0, y + 1, z });
                const Block* block_above       = &chunk_blocks[block_above_index];

                // Only consider block if it is not covered above.
                if (is_solid(block_above)) continue;

                // Ensure node exists for this block.
                ChunkNavmeshNode block_coord = {
                    {0, y, z},
                    chunk_pos
                };
                ChunkNavmeshVertexDescriptor block_vertex
                    = impl::get_vertex(chunk_navmesh, block_coord);

                // Right
                do_navigable_check(block_vertex, { 0, y, z }, { 1, y, z }, 2, -1);

                // Front
                do_navigable_check(block_vertex, { 0, y, z }, { 0, y, z + 1 }, 2, -1);

                // Back
                do_navigable_check(block_vertex, { 0, y, z }, { 0, y, z - 1 }, 2, -1);
            }

            // Right Face
            {
                BlockIndex block_index = hvox::block_index({ CHUNK_LENGTH - 1, y, z });
                const Block* block     = &chunk_blocks[block_index];

                // Only consider block if it is solid.
                if (!is_solid(block)) continue;

                BlockIndex block_above_index
                    = hvox::block_index({ CHUNK_LENGTH - 1, y + 1, z });
                const Block* block_above = &chunk_blocks[block_above_index];

                // Only consider block if it is not covered above.
                if (is_solid(block_above)) continue;

                // Ensure node exists for this block.
                ChunkNavmeshNode block_coord = {
                    {CHUNK_LENGTH - 1, y, z},
                    chunk_pos
                };
                ChunkNavmeshVertexDescriptor block_vertex
                    = impl::get_vertex(chunk_navmesh, block_coord);

                // Left
                do_navigable_check(
                    block_vertex,
                    { CHUNK_LENGTH - 1, y, z },
                    { CHUNK_LENGTH - 2, y, z },
                    2,
                    -1
                );

                // Front
                do_navigable_check(
                    block_vertex,
                    { CHUNK_LENGTH - 1, y, z },
                    { CHUNK_LENGTH - 1, y, z + 1 },
                    2,
                    -1
                );

                // Back
                do_navigable_check(
                    block_vertex,
                    { CHUNK_LENGTH - 1, y, z },
                    { CHUNK_LENGTH - 1, y, z - 1 },
                    2,
                    -1
                );
            }
        }

        // Second-to-top case.
        {
            // Left Face
            {
                BlockIndex block_index = hvox::block_index({ 0, CHUNK_LENGTH - 2, z });
                const Block* block     = &chunk_blocks[block_index];

                // Only consider block if it is solid.
                if (!is_solid(block)) continue;

                BlockIndex block_above_index
                    = hvox::block_index({ 0, CHUNK_LENGTH - 1, z });
                const Block* block_above = &chunk_blocks[block_above_index];

                // Only consider block if it is not covered above.
                if (is_solid(block_above)) continue;

                // Ensure node exists for this block.
                ChunkNavmeshNode block_coord = {
                    {0, CHUNK_LENGTH - 2, z},
                    chunk_pos
                };
                ChunkNavmeshVertexDescriptor block_vertex
                    = impl::get_vertex(chunk_navmesh, block_coord);

                // Right
                do_navigable_check(
                    block_vertex,
                    { 0, CHUNK_LENGTH - 2, z },
                    { 1, CHUNK_LENGTH - 2, z },
                    1,
                    -1
                );

                // Front
                do_navigable_check(
                    block_vertex,
                    { 0, CHUNK_LENGTH - 2, z },
                    { 0, CHUNK_LENGTH - 2, z + 1 },
                    1,
                    -1
                );

                // Back
                do_navigable_check(
                    block_vertex,
                    { 0, CHUNK_LENGTH - 2, z },
                    { 0, CHUNK_LENGTH - 2, z - 1 },
                    1,
                    -1
                );
            }

            // Right Face
            {
                BlockIndex block_index
                    = hvox::block_index({ CHUNK_LENGTH - 1, CHUNK_LENGTH - 2, z });
                const Block* block = &chunk_blocks[block_index];

                // Only consider block if it is solid.
                if (!is_solid(block)) continue;

                BlockIndex block_above_index
                    = hvox::block_index({ CHUNK_LENGTH - 1, CHUNK_LENGTH - 1, z });
                const Block* block_above = &chunk_blocks[block_above_index];

                // Only consider block if it is not covered above.
                if (is_solid(block_above)) continue;

                // Ensure node exists for this block.
                ChunkNavmeshNode block_coord = {
                    {CHUNK_LENGTH - 1, CHUNK_LENGTH - 2, z},
                    chunk_pos
                };
                ChunkNavmeshVertexDescriptor block_vertex
                    = impl::get_vertex(chunk_navmesh, block_coord);

                // Left
                do_navigable_check(
                    block_vertex,
                    { CHUNK_LENGTH - 1, CHUNK_LENGTH - 2, z },
                    { CHUNK_LENGTH - 2, CHUNK_LENGTH - 2, z },
                    1,
                    -1
                );

                // Front
                do_navigable_check(
                    block_vertex,
                    { CHUNK_LENGTH - 1, CHUNK_LENGTH - 2, z },
                    { CHUNK_LENGTH - 1, CHUNK_LENGTH - 2, z + 1 },
                    1,
                    -1
                );

                // Back
                do_navigable_check(
                    block_vertex,
                    { CHUNK_LENGTH - 1, CHUNK_LENGTH - 2, z },
                    { CHUNK_LENGTH - 1, CHUNK_LENGTH - 2, z - 1 },
                    1,
                    -1
                );
            }
        }
    }

    /********************************\
     * Navmesh front and back faces *
     *   except for edges.          *
    \********************************/

    for (BlockChunkPositionCoord x = 1; x < CHUNK_LENGTH - 1; ++x) {
        for (BlockChunkPositionCoord y = 1; y < CHUNK_LENGTH - 2; ++y) {
            // Front Face
            {
                BlockIndex block_index = hvox::block_index({ x, y, CHUNK_LENGTH - 1 });
                const Block* block     = &chunk_blocks[block_index];

                // Only consider block if it is solid.
                if (!is_solid(block)) continue;

                BlockIndex block_above_index
                    = hvox::block_index({ x, y + 1, CHUNK_LENGTH - 1 });
                const Block* block_above = &chunk_blocks[block_above_index];

                // Only consider block if it is not covered above.
                if (is_solid(block_above)) continue;

                // Ensure node exists for this block.
                ChunkNavmeshNode block_coord = {
                    {x, y, CHUNK_LENGTH - 1},
                    chunk_pos
                };
                ChunkNavmeshVertexDescriptor block_vertex
                    = impl::get_vertex(chunk_navmesh, block_coord);

                // Left
                do_navigable_check(
                    block_vertex,
                    { x, y, CHUNK_LENGTH - 1 },
                    { x - 1, y, CHUNK_LENGTH - 1 },
                    2,
                    -1
                );

                // Right
                do_navigable_check(
                    block_vertex,
                    { x, y, CHUNK_LENGTH - 1 },
                    { x + 1, y, CHUNK_LENGTH - 1 },
                    2,
                    -1
                );

                // Back
                do_navigable_check(
                    block_vertex,
                    { x, y, CHUNK_LENGTH - 1 },
                    { x, y, CHUNK_LENGTH - 2 },
                    2,
                    -1
                );
            }

            // Back Face
            {
                BlockIndex   block_index = hvox::block_index({ x, y, 0 });
                const Block* block       = &chunk_blocks[block_index];

                // Only consider block if it is solid.
                if (!is_solid(block)) continue;

                BlockIndex   block_above_index = hvox::block_index({ x, y + 1, 0 });
                const Block* block_above       = &chunk_blocks[block_above_index];

                // Only consider block if it is not covered above.
                if (is_solid(block_above)) continue;

                // Ensure node exists for this block.
                ChunkNavmeshNode block_coord = {
                    {x, y, 0},
                    chunk_pos
                };
                ChunkNavmeshVertexDescriptor block_vertex
                    = impl::get_vertex(chunk_navmesh, block_coord);

                // Left
                do_navigable_check(block_vertex, { x, y, 0 }, { x - 1, y, 0 }, 2, -1);

                // Right
                do_navigable_check(block_vertex, { x, y, 0 }, { x + 1, y, 0 }, 2, -1);

                // Front
                do_navigable_check(block_vertex, { x, y, 0 }, { x, y, 1 }, 2, -1);
            }
        }

        // Second-to-top case
        {
            // Front Face
            {
                BlockIndex block_index
                    = hvox::block_index({ x, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1 });
                const Block* block = &chunk_blocks[block_index];

                // Only consider block if it is solid.
                if (!is_solid(block)) continue;

                BlockIndex block_above_index
                    = hvox::block_index({ x, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1 });
                const Block* block_above = &chunk_blocks[block_above_index];

                // Only consider block if it is not covered above.
                if (is_solid(block_above)) continue;

                // Ensure node exists for this block.
                ChunkNavmeshNode block_coord = {
                    {x, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1},
                    chunk_pos
                };
                ChunkNavmeshVertexDescriptor block_vertex
                    = impl::get_vertex(chunk_navmesh, block_coord);

                // Left
                do_navigable_check(
                    block_vertex,
                    { x, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1 },
                    { x - 1, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1 },
                    1,
                    -1
                );

                // Right
                do_navigable_check(
                    block_vertex,
                    { x, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1 },
                    { x + 1, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1 },
                    1,
                    -1
                );

                // Back
                do_navigable_check(
                    block_vertex,
                    { x, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1 },
                    { x, CHUNK_LENGTH - 2, CHUNK_LENGTH - 2 },
                    1,
                    -1
                );
            }

            // Ensure node exists for this block.
            ChunkNavmeshNode block_coord = {
                {x, CHUNK_LENGTH - 2, 0},
                chunk_pos
            };
            ChunkNavmeshVertexDescriptor block_vertex
                = impl::get_vertex(chunk_navmesh, block_coord);

            // Left
            do_navigable_check(
                block_vertex,
                { x, CHUNK_LENGTH - 2, 0 },
                { x - 1, CHUNK_LENGTH - 2, 0 },
                1,
                -1
            );

            // Right
            do_navigable_check(
                block_vertex,
                { x, CHUNK_LENGTH - 2, 0 },
                { x + 1, CHUNK_LENGTH - 2, 0 },
                1,
                -1
            );

            // Front
            do_navigable_check(
                block_vertex,
                { x, CHUNK_LENGTH - 2, 0 },
                { x, CHUNK_LENGTH - 2, 1 },
                1,
                -1
            );
        }
    }

    /********************************\
     * Navmesh bottom face          *
     *   except for edges.          *
    \********************************/

    for (BlockChunkPositionCoord x = 1; x < CHUNK_LENGTH - 1; ++x) {
        for (BlockChunkPositionCoord z = 1; z < CHUNK_LENGTH - 1; ++z) {
            // Bottom Face
            {
                BlockIndex   block_index = hvox::block_index({ x, 0, z });
                const Block* block       = &chunk_blocks[block_index];

                // Only consider block if it is solid.
                if (!is_solid(block)) continue;

                BlockIndex   block_above_index = hvox::block_index({ x, 1, z });
                const Block* block_above       = &chunk_blocks[block_above_index];

                // Only consider block if it is not covered above.
                if (is_solid(block_above)) continue;

                // Ensure node exists for this block.
                ChunkNavmeshNode block_coord = {
                    {x, 0, z},
                    chunk_pos
                };
                ChunkNavmeshVertexDescriptor block_vertex
                    = impl::get_vertex(chunk_navmesh, block_coord);

                // Left
                do_navigable_check(block_vertex, { x, 0, z }, { x - 1, 0, z }, 2, 0);

                // Right
                do_navigable_check(block_vertex, { x, 0, z }, { x + 1, 0, z }, 2, 0);

                // Front
                do_navigable_check(block_vertex, { x, 0, z }, { x, 0, z + 1 }, 2, 0);

                // Back
                do_navigable_check(block_vertex, { x, 0, z }, { x, 0, z - 1 }, 2, 0);
            }
        }
    }

    /**************************\
     * Navmesh vertical edges *
     *   except for corners.  *
    \**************************/

    for (BlockChunkPositionCoord y = 1; y < CHUNK_LENGTH - 2; ++y) {
        // Front-Left Edge
        {
            BlockIndex   block_index = hvox::block_index({ 0, y, CHUNK_LENGTH - 1 });
            const Block* block       = &chunk_blocks[block_index];

            // Only consider block if it is solid.
            if (!is_solid(block)) continue;

            BlockIndex block_above_index
                = hvox::block_index({ 0, y + 1, CHUNK_LENGTH - 1 });
            const Block* block_above = &chunk_blocks[block_above_index];

            // Only consider block if it is not covered above.
            if (is_solid(block_above)) continue;

            // Ensure node exists for this block.
            ChunkNavmeshNode block_coord = {
                {0, y, CHUNK_LENGTH - 1},
                chunk_pos
            };
            ChunkNavmeshVertexDescriptor block_vertex
                = impl::get_vertex(chunk_navmesh, block_coord);

            // Right
            do_navigable_check(
                block_vertex,
                { 0, y, CHUNK_LENGTH - 1 },
                { 1, y, CHUNK_LENGTH - 1 },
                2,
                -1
            );

            // Back
            do_navigable_check(
                block_vertex,
                { 0, y, CHUNK_LENGTH - 1 },
                { 0, y, CHUNK_LENGTH - 2 },
                2,
                -1
            );
        }

        // Front-Right Edge
        {
            BlockIndex block_index
                = hvox::block_index({ CHUNK_LENGTH - 1, y, CHUNK_LENGTH - 1 });
            const Block* block = &chunk_blocks[block_index];

            // Only consider block if it is solid.
            if (!is_solid(block)) continue;

            BlockIndex block_above_index
                = hvox::block_index({ CHUNK_LENGTH - 1, y + 1, CHUNK_LENGTH - 1 });
            const Block* block_above = &chunk_blocks[block_above_index];

            // Only consider block if it is not covered above.
            if (is_solid(block_above)) continue;

            // Ensure node exists for this block.
            ChunkNavmeshNode block_coord = {
                {CHUNK_LENGTH - 1, y, CHUNK_LENGTH - 1},
                chunk_pos
            };
            ChunkNavmeshVertexDescriptor block_vertex
                = impl::get_vertex(chunk_navmesh, block_coord);

            // Left
            do_navigable_check(
                block_vertex,
                { CHUNK_LENGTH - 1, y, CHUNK_LENGTH - 1 },
                { CHUNK_LENGTH - 2, y, CHUNK_LENGTH - 1 },
                2,
                -1
            );

            // Back
            do_navigable_check(
                block_vertex,
                { CHUNK_LENGTH - 1, y, CHUNK_LENGTH - 1 },
                { CHUNK_LENGTH - 1, y, CHUNK_LENGTH - 2 },
                2,
                -1
            );
        }

        // Back-Left Edge
        {
            BlockIndex   block_index = hvox::block_index({ 0, y, 0 });
            const Block* block       = &chunk_blocks[block_index];

            // Only consider block if it is solid.
            if (!is_solid(block)) continue;

            BlockIndex   block_above_index = hvox::block_index({ 0, y + 1, 0 });
            const Block* block_above       = &chunk_blocks[block_above_index];

            // Only consider block if it is not covered above.
            if (is_solid(block_above)) continue;

            // Ensure node exists for this block.
            ChunkNavmeshNode block_coord = {
                {0, y, 0},
                chunk_pos
            };
            ChunkNavmeshVertexDescriptor block_vertex
                = impl::get_vertex(chunk_navmesh, block_coord);

            // Right
            do_navigable_check(block_vertex, { 0, y, 0 }, { 1, y, 0 }, 2, -1);

            // Front
            do_navigable_check(block_vertex, { 0, y, 0 }, { 0, y, 1 }, 2, -1);
        }

        // Back-Right Edge
        {
            BlockIndex   block_index = hvox::block_index({ CHUNK_LENGTH - 1, y, 0 });
            const Block* block       = &chunk_blocks[block_index];

            // Only consider block if it is solid.
            if (!is_solid(block)) continue;

            BlockIndex block_above_index
                = hvox::block_index({ CHUNK_LENGTH - 1, y + 1, 0 });
            const Block* block_above = &chunk_blocks[block_above_index];

            // Only consider block if it is not covered above.
            if (is_solid(block_above)) continue;

            // Ensure node exists for this block.
            ChunkNavmeshNode block_coord = {
                {CHUNK_LENGTH - 1, y, 0},
                chunk_pos
            };
            ChunkNavmeshVertexDescriptor block_vertex
                = impl::get_vertex(chunk_navmesh, block_coord);

            // Left
            do_navigable_check(
                block_vertex,
                { CHUNK_LENGTH - 1, y, 0 },
                { CHUNK_LENGTH - 2, y, 0 },
                2,
                -1
            );

            // Front
            do_navigable_check(
                block_vertex,
                { CHUNK_LENGTH - 1, y, 0 },
                { CHUNK_LENGTH - 1, y, 1 },
                2,
                -1
            );
        }
    }

    // Second-to-top case
    // Front-Left Edge
    {
        BlockIndex block_index
            = hvox::block_index({ 0, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1 });
        const Block* block = &chunk_blocks[block_index];

        BlockIndex block_above_index
            = hvox::block_index({ 0, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1 });
        const Block* block_above = &chunk_blocks[block_above_index];

        // Only consider block if it is not covered above.
        if (is_solid(block) && !is_solid(block_above)) {
            // Ensure node exists for this block.
            ChunkNavmeshNode block_coord = {
                {0, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1},
                chunk_pos
            };
            ChunkNavmeshVertexDescriptor block_vertex
                = impl::get_vertex(chunk_navmesh, block_coord);

            // Right
            do_navigable_check(
                block_vertex,
                { 0, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1 },
                { 1, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1 },
                1,
                -1
            );

            // Back
            do_navigable_check(
                block_vertex,
                { 0, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1 },
                { 0, CHUNK_LENGTH - 2, CHUNK_LENGTH - 2 },
                1,
                -1
            );
        }
    }

    // Front-Right Edge
    {
        BlockIndex block_index
            = hvox::block_index({ CHUNK_LENGTH - 1, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1 }
            );
        const Block* block = &chunk_blocks[block_index];

        BlockIndex block_above_index
            = hvox::block_index({ CHUNK_LENGTH - 1, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1 }
            );
        const Block* block_above = &chunk_blocks[block_above_index];

        // Only consider block if it is not covered above.
        if (is_solid(block) && !is_solid(block_above)) {
            // Ensure node exists for this block.
            ChunkNavmeshNode block_coord = {
                {CHUNK_LENGTH - 1, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1},
                chunk_pos
            };
            ChunkNavmeshVertexDescriptor block_vertex
                = impl::get_vertex(chunk_navmesh, block_coord);

            // Left
            do_navigable_check(
                block_vertex,
                { CHUNK_LENGTH - 1, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1 },
                { CHUNK_LENGTH - 2, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1 },
                1,
                -1
            );

            // Back
            do_navigable_check(
                block_vertex,
                { CHUNK_LENGTH - 1, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1 },
                { CHUNK_LENGTH - 1, CHUNK_LENGTH - 2, CHUNK_LENGTH - 2 },
                1,
                -1
            );
        }
    }

    // Back-Left Edge
    {
        BlockIndex   block_index = hvox::block_index({ 0, CHUNK_LENGTH - 2, 0 });
        const Block* block       = &chunk_blocks[block_index];

        BlockIndex   block_above_index = hvox::block_index({ 0, CHUNK_LENGTH - 1, 0 });
        const Block* block_above       = &chunk_blocks[block_above_index];

        // Only consider block if it is not covered above.
        if (is_solid(block) && !is_solid(block_above)) {
            // Ensure node exists for this block.
            ChunkNavmeshNode block_coord = {
                {0, CHUNK_LENGTH - 2, 0},
                chunk_pos
            };
            ChunkNavmeshVertexDescriptor block_vertex
                = impl::get_vertex(chunk_navmesh, block_coord);

            // Right
            do_navigable_check(
                block_vertex,
                { 0, CHUNK_LENGTH - 2, 0 },
                { 1, CHUNK_LENGTH - 2, 0 },
                1,
                -1
            );

            // Front
            do_navigable_check(
                block_vertex,
                { 0, CHUNK_LENGTH - 2, 0 },
                { 0, CHUNK_LENGTH - 2, 1 },
                1,
                -1
            );
        }
    }

    // Back-Right Edge
    {
        BlockIndex block_index
            = hvox::block_index({ CHUNK_LENGTH - 1, CHUNK_LENGTH - 2, 0 });
        const Block* block = &chunk_blocks[block_index];

        BlockIndex block_above_index
            = hvox::block_index({ CHUNK_LENGTH - 1, CHUNK_LENGTH - 1, 0 });
        const Block* block_above = &chunk_blocks[block_above_index];

        // Only consider block if it is not covered above.
        if (is_solid(block) && !is_solid(block_above)) {
            // Ensure node exists for this block.
            ChunkNavmeshNode block_coord = {
                {CHUNK_LENGTH - 1, CHUNK_LENGTH - 2, 0},
                chunk_pos
            };
            ChunkNavmeshVertexDescriptor block_vertex
                = impl::get_vertex(chunk_navmesh, block_coord);

            // Left
            do_navigable_check(
                block_vertex,
                { CHUNK_LENGTH - 1, CHUNK_LENGTH - 2, 0 },
                { CHUNK_LENGTH - 2, CHUNK_LENGTH - 2, 0 },
                1,
                -1
            );

            // Front
            do_navigable_check(
                block_vertex,
                { CHUNK_LENGTH - 1, CHUNK_LENGTH - 2, 0 },
                { CHUNK_LENGTH - 1, CHUNK_LENGTH - 2, 1 },
                1,
                -1
            );
        }
    }

    /****************************\
     * Navmesh horizontal edges *
     *   except for corners.    *
    \****************************/

    // Left-Right Edges
    for (BlockChunkPositionCoord x = 1; x < CHUNK_LENGTH - 1; ++x) {
        // Front-Bottom Edge
        {
            BlockIndex   block_index = hvox::block_index({ x, 0, CHUNK_LENGTH - 1 });
            const Block* block       = &chunk_blocks[block_index];

            // Only consider block if it is solid.
            if (!is_solid(block)) continue;

            BlockIndex block_above_index
                = hvox::block_index({ x, 1, CHUNK_LENGTH - 1 });
            const Block* block_above = &chunk_blocks[block_above_index];

            // Only consider block if it is not covered above.
            if (is_solid(block_above)) continue;

            // Ensure node exists for this block.
            ChunkNavmeshNode block_coord = {
                {x, 0, CHUNK_LENGTH - 1},
                chunk_pos
            };
            ChunkNavmeshVertexDescriptor block_vertex
                = impl::get_vertex(chunk_navmesh, block_coord);

            // Left
            do_navigable_check(
                block_vertex,
                { x, 0, CHUNK_LENGTH - 1 },
                { x - 1, 0, CHUNK_LENGTH - 1 },
                2,
                0
            );

            // Right
            do_navigable_check(
                block_vertex,
                { x, 0, CHUNK_LENGTH - 1 },
                { x + 1, 0, CHUNK_LENGTH - 1 },
                2,
                0
            );

            // Back
            do_navigable_check(
                block_vertex,
                { x, 0, CHUNK_LENGTH - 1 },
                { x, 0, CHUNK_LENGTH - 2 },
                2,
                0
            );
        }

        // Back-Bottom Edge
        {
            BlockIndex   block_index = hvox::block_index({ x, 0, 0 });
            const Block* block       = &chunk_blocks[block_index];

            // Only consider block if it is solid.
            if (!is_solid(block)) continue;

            BlockIndex   block_above_index = hvox::block_index({ x, 1, 0 });
            const Block* block_above       = &chunk_blocks[block_above_index];

            // Only consider block if it is not covered above.
            if (is_solid(block_above)) continue;

            // Ensure node exists for this block.
            ChunkNavmeshNode block_coord = {
                {x, 0, 0},
                chunk_pos
            };
            ChunkNavmeshVertexDescriptor block_vertex
                = impl::get_vertex(chunk_navmesh, block_coord);

            // Left
            do_navigable_check(block_vertex, { x, 0, 0 }, { x - 1, 0, 0 }, 2, 0);

            // Right
            do_navigable_check(block_vertex, { x, 0, 0 }, { x + 1, 0, 0 }, 2, 0);

            // Front
            do_navigable_check(block_vertex, { x, 0, 0 }, { x, 0, 1 }, 2, 0);
        }
    }

    // Front-Back Edges
    for (BlockChunkPositionCoord z = 1; z < CHUNK_LENGTH - 1; ++z) {
        // Left-Bottom Edge
        {
            BlockIndex   block_index = hvox::block_index({ 0, 0, z });
            const Block* block       = &chunk_blocks[block_index];

            // Only consider block if it is solid.
            if (!is_solid(block)) continue;

            BlockIndex   block_above_index = hvox::block_index({ 0, 1, z });
            const Block* block_above       = &chunk_blocks[block_above_index];

            // Only consider block if it is not covered above.
            if (is_solid(block_above)) continue;

            // Ensure node exists for this block.
            ChunkNavmeshNode block_coord = {
                {0, 0, z},
                chunk_pos
            };
            ChunkNavmeshVertexDescriptor block_vertex
                = impl::get_vertex(chunk_navmesh, block_coord);

            // Right
            do_navigable_check(block_vertex, { 0, 0, z }, { 1, 0, z }, 2, 0);

            // Front
            do_navigable_check(block_vertex, { 0, 0, z }, { 0, 0, z + 1 }, 2, 0);

            // Back
            do_navigable_check(block_vertex, { 0, 0, z }, { 0, 0, z - 1 }, 2, 0);
        }

        // Right-Bottom Edge
        {
            BlockIndex   block_index = hvox::block_index({ CHUNK_LENGTH - 1, 0, z });
            const Block* block       = &chunk_blocks[block_index];

            // Only consider block if it is solid.
            if (!is_solid(block)) continue;

            BlockIndex block_above_index
                = hvox::block_index({ CHUNK_LENGTH - 1, 1, z });
            const Block* block_above = &chunk_blocks[block_above_index];

            // Only consider block if it is not covered above.
            if (is_solid(block_above)) continue;

            // Ensure node exists for this block.
            ChunkNavmeshNode block_coord = {
                {CHUNK_LENGTH - 1, 0, z},
                chunk_pos
            };
            ChunkNavmeshVertexDescriptor block_vertex
                = impl::get_vertex(chunk_navmesh, block_coord);

            // Left
            do_navigable_check(
                block_vertex,
                { CHUNK_LENGTH - 1, 0, z },
                { CHUNK_LENGTH - 2, 0, z },
                2,
                0
            );

            // Front
            do_navigable_check(
                block_vertex,
                { CHUNK_LENGTH - 1, 0, z },
                { CHUNK_LENGTH - 1, 0, z + 1 },
                2,
                0
            );

            // Back
            do_navigable_check(
                block_vertex,
                { CHUNK_LENGTH - 1, 0, z },
                { CHUNK_LENGTH - 1, 0, z - 1 },
                2,
                0
            );
        }
    }

    /********************\
     * Navmesh corners. *
    \********************/

    // As we do not navmesh the top face of the chunk, we consider here only the bottom
    // corners of the chunk.

    // Left-Bottom-Front
    {
        BlockIndex   block_index = hvox::block_index({ 0, 0, CHUNK_LENGTH - 1 });
        const Block* block       = &chunk_blocks[block_index];

        BlockIndex   block_above_index = hvox::block_index({ 0, 1, CHUNK_LENGTH - 1 });
        const Block* block_above       = &chunk_blocks[block_above_index];

        // Only consider block if it is not covered above.
        if (is_solid(block) && !is_solid(block_above)) {
            // Ensure node exists for this block.
            ChunkNavmeshNode block_coord = {
                {0, 0, CHUNK_LENGTH - 1},
                chunk_pos
            };
            ChunkNavmeshVertexDescriptor block_vertex
                = impl::get_vertex(chunk_navmesh, block_coord);

            // Right
            do_navigable_check(
                block_vertex,
                { 0, 0, CHUNK_LENGTH - 1 },
                { 1, 0, CHUNK_LENGTH - 1 },
                2,
                0
            );

            // Back
            do_navigable_check(
                block_vertex,
                { 0, 0, CHUNK_LENGTH - 1 },
                { 0, 0, CHUNK_LENGTH - 2 },
                2,
                0
            );
        }
    }

    // Left-Bottom-Back
    {
        BlockIndex   block_index = hvox::block_index({ 0, 0, 0 });
        const Block* block       = &chunk_blocks[block_index];

        BlockIndex   block_above_index = hvox::block_index({ 0, 1, 0 });
        const Block* block_above       = &chunk_blocks[block_above_index];

        // Only consider block if it is not covered above.
        if (is_solid(block) && !is_solid(block_above)) {
            // Ensure node exists for this block.
            ChunkNavmeshNode block_coord = {
                {0, 0, 0},
                chunk_pos
            };
            ChunkNavmeshVertexDescriptor block_vertex
                = impl::get_vertex(chunk_navmesh, block_coord);

            // Right
            do_navigable_check(block_vertex, { 0, 0, 0 }, { 1, 0, 0 }, 2, 0);

            // Front
            do_navigable_check(block_vertex, { 0, 0, 0 }, { 0, 0, 1 }, 2, 0);
        }
    }

    // Right-Bottom-Front
    {
        BlockIndex block_index
            = hvox::block_index({ CHUNK_LENGTH - 1, 0, CHUNK_LENGTH - 1 });
        const Block* block = &chunk_blocks[block_index];

        BlockIndex block_above_index
            = hvox::block_index({ CHUNK_LENGTH - 1, 1, CHUNK_LENGTH - 1 });
        const Block* block_above = &chunk_blocks[block_above_index];

        // Only consider block if it is not covered above.
        if (is_solid(block) && !is_solid(block_above)) {
            // Ensure node exists for this block.
            ChunkNavmeshNode block_coord = {
                {CHUNK_LENGTH - 1, 0, CHUNK_LENGTH - 1},
                chunk_pos
            };
            ChunkNavmeshVertexDescriptor block_vertex
                = impl::get_vertex(chunk_navmesh, block_coord);

            // Left
            do_navigable_check(
                block_vertex,
                { CHUNK_LENGTH - 1, 0, CHUNK_LENGTH - 1 },
                { CHUNK_LENGTH - 2, 0, CHUNK_LENGTH - 1 },
                2,
                0
            );

            // Back
            do_navigable_check(
                block_vertex,
                { CHUNK_LENGTH - 1, 0, CHUNK_LENGTH - 1 },
                { CHUNK_LENGTH - 1, 0, CHUNK_LENGTH - 2 },
                2,
                0
            );
        }
    }

    // Right-Bottom-Back
    {
        BlockIndex   block_index = hvox::block_index({ CHUNK_LENGTH - 1, 0, 0 });
        const Block* block       = &chunk_blocks[block_index];

        BlockIndex   block_above_index = hvox::block_index({ CHUNK_LENGTH - 1, 1, 0 });
        const Block* block_above       = &chunk_blocks[block_above_index];

        // Only consider block if it is not covered above.
        if (is_solid(block) && !is_solid(block_above)) {
            // Ensure node exists for this block.
            ChunkNavmeshNode block_coord = {
                {CHUNK_LENGTH - 1, 0, 0},
                chunk_pos
            };
            ChunkNavmeshVertexDescriptor block_vertex
                = impl::get_vertex(chunk_navmesh, block_coord);

            // Left
            do_navigable_check(
                block_vertex,
                { CHUNK_LENGTH - 1, 0, 0 },
                { CHUNK_LENGTH - 2, 0, 0 },
                2,
                0
            );

            // Front
            do_navigable_check(
                block_vertex,
                { CHUNK_LENGTH - 1, 0, 0 },
                { CHUNK_LENGTH - 1, 0, 1 },
                2,
                0
            );
        }
    }
}

template <hvox::IdealBlockConstraint IsSolid>
void hvox::ai::NaiveNavmeshStrategy<IsSolid>::do_stitch(
    hmem::Handle<ChunkGrid>, hmem::Handle<Chunk> chunk
) const {
    auto chunk_pos = chunk->position;

    std::shared_lock<std::shared_mutex> block_lock;
    const Block*                        chunk_blocks = chunk->blocks.get(block_lock);

    std::unique_lock<std::shared_mutex> navmesh_lock;
    hvox::ai::ChunkNavmesh chunk_navmesh = chunk->navmesh.get(navmesh_lock);

    const IsSolid is_solid{};

    //----------------------------------------------------------------------------------
    //
    // Navmesh between this chunk neighbours (stitch).
    //----------------------------------------------------------------------------------

    // Recall that the top face of the below neighbour is part of the stitching process.

    /*****************************************\
     * Betwee-chunk off-edge navigable check. *
    \******************************************/

    // TODO(Matthew): Is this optimised well by compiler?
    auto do_side_stitch_navigable_check = [&](const Block*            neighbour_blocks,
                                              hvox::ai::ChunkNavmesh& neighbour_navmesh,
                                              ChunkGridPosition  neighbour_position,
                                              BlockChunkPosition this_offset,
                                              BlockChunkPosition neighbour_offset,
                                              i64                start,
                                              i64                end) {
        BlockIndex   this_block_index = hvox::block_index(this_offset);
        const Block* this_block       = &chunk_blocks[this_block_index];

        BlockIndex above_this_block_index
            = hvox::block_index(this_offset + BlockChunkPosition{ 0, 1, 0 });
        const Block* above_this_block = &chunk_blocks[above_this_block_index];

        if (!is_solid(this_block) || is_solid(above_this_block)) return;

        ChunkNavmeshNode this_block_coord = { this_offset, chunk_pos };
        struct {
            ChunkNavmeshVertexDescriptor here, in_neighbour;
        } this_block_vertex = { impl::get_vertex(chunk_navmesh, this_block_coord),
                                impl::get_vertex(neighbour_navmesh, this_block_coord) };

        for (i64 y_off = start; y_off > end; --y_off) {
            BlockIndex above_candidate_index = hvox::block_index(
                static_cast<i64v3>(neighbour_offset) + i64v3{ 0, y_off, 0 }
            );
            const Block* above_candidate_block
                = &neighbour_blocks[above_candidate_index];

            BlockIndex candidate_index = hvox::block_index(
                static_cast<i64v3>(neighbour_offset) + i64v3{ 0, y_off - 1, 0 }
            );
            const Block* candidate_block = &neighbour_blocks[candidate_index];

            if (is_solid(candidate_block) && !is_solid(above_candidate_block)) {
                ChunkNavmeshNode candidate_block_coord = {
                    static_cast<i64v3>(neighbour_offset) + i64v3{0, y_off - 1, 0},
                    neighbour_position
                };
                struct {
                    ChunkNavmeshVertexDescriptor here, in_neighbour;
                } candidate_block_vertex
                    = { impl::get_vertex(chunk_navmesh, candidate_block_coord),
                        impl::get_vertex(neighbour_navmesh, candidate_block_coord) };

                boost::add_edge(
                    this_block_vertex.here,
                    candidate_block_vertex.here,
                    chunk_navmesh.graph
                );
                boost::add_edge(
                    candidate_block_vertex.here,
                    this_block_vertex.here,
                    chunk_navmesh.graph
                );

                boost::add_edge(
                    this_block_vertex.in_neighbour,
                    candidate_block_vertex.in_neighbour,
                    neighbour_navmesh.graph
                );
                boost::add_edge(
                    candidate_block_vertex.in_neighbour,
                    this_block_vertex.in_neighbour,
                    neighbour_navmesh.graph
                );
            }
        }
    };

    /*********************************\
     * Try to stitch left neighbour. *
     *   except along top edge.      *
    \*********************************/

    {
        auto       neighbour    = chunk->neighbours.one.left.lock();
        ChunkState stitch_state = ChunkState::NONE;
        if (neighbour != nullptr
            && neighbour->bulk_navmeshing.load() == ChunkState::COMPLETE
            && neighbour->navmesh_stitch.right.compare_exchange_strong(
                stitch_state, ChunkState::ACTIVE
            ))
        {
            std::shared_lock<std::shared_mutex> neighbour_block_lock;
            auto neighbour_blocks = neighbour->blocks.get(neighbour_block_lock);
            std::unique_lock<std::shared_mutex> neighbour_navmesh_lock;
            auto neighbour_navmesh = neighbour->navmesh.get(neighbour_navmesh_lock);

            for (BlockChunkPositionCoord z = 0; z < CHUNK_LENGTH; ++z) {
                for (BlockChunkPositionCoord y = 1; y < CHUNK_LENGTH - 2; ++y) {
                    do_side_stitch_navigable_check(
                        neighbour_blocks,
                        neighbour_navmesh,
                        neighbour->position,
                        { 0, y, z },
                        { CHUNK_LENGTH - 1, y, z },
                        2,
                        -1
                    );
                }
            }

            // Special handling for
            //      y == 0, CHUNK_LENGTH - 2
            // where chunks above need to be loaded too.

            // y == 0, CHUNK_LENGTH - 2
            for (BlockChunkPositionCoord z = 0; z < CHUNK_LENGTH; ++z) {
                // y == 0 - step across or up
                do_side_stitch_navigable_check(
                    neighbour_blocks,
                    neighbour_navmesh,
                    neighbour->position,
                    { 0, 0, z },
                    { CHUNK_LENGTH - 1, 0, z },
                    2,
                    0
                );

                // y == CHUNK_LENGTH - 2 - step down or across
                do_side_stitch_navigable_check(
                    neighbour_blocks,
                    neighbour_navmesh,
                    neighbour->position,
                    { 0, CHUNK_LENGTH - 2, z },
                    { CHUNK_LENGTH - 1, CHUNK_LENGTH - 2, z },
                    1,
                    -1
                );
            }

            // y == 0 - step down
            auto       below_neighbour    = neighbour->neighbours.one.bottom.lock();
            ChunkState below_stitch_state = ChunkState::NONE;
            if (below_neighbour != nullptr
                && below_neighbour->bulk_navmeshing.load() == ChunkState::COMPLETE
                && below_neighbour->navmesh_stitch.above_right.compare_exchange_strong(
                    below_stitch_state, ChunkState::ACTIVE
                ))
            {
                std::shared_lock<std::shared_mutex> below_neighbour_block_lock;
                auto                                below_neighbour_blocks
                    = below_neighbour->blocks.get(below_neighbour_block_lock);
                std::unique_lock<std::shared_mutex> below_neighbour_navmesh_lock;
                auto                                below_neighbour_navmesh
                    = below_neighbour->navmesh.get(below_neighbour_navmesh_lock);

                for (BlockChunkPositionCoord z = 0; z < CHUNK_LENGTH; ++z) {
                    BlockIndex   this_block_index = hvox::block_index({ 0, 0, z });
                    const Block* this_block       = &chunk_blocks[this_block_index];

                    BlockIndex above_this_block_index = hvox::block_index({ 0, 1, z });
                    const Block* above_this_block
                        = &chunk_blocks[above_this_block_index];

                    if (!is_solid(this_block) || is_solid(above_this_block)) continue;

                    ChunkNavmeshNode this_block_coord = {
                        {0, 0, z},
                        chunk_pos
                    };

                    struct {
                        ChunkNavmeshVertexDescriptor here, in_below_neighbour;
                    } this_block_vertex = {
                        impl::get_vertex(chunk_navmesh, this_block_coord),
                        impl::get_vertex(below_neighbour_navmesh, this_block_coord)
                    };

                    BlockIndex twice_above_candidate_index
                        = hvox::block_index({ CHUNK_LENGTH - 1, 1, z });
                    const Block* twice_above_candidate_block
                        = &neighbour_blocks[twice_above_candidate_index];

                    BlockIndex above_candidate_index
                        = hvox::block_index({ CHUNK_LENGTH - 1, 0, z });
                    const Block* above_candidate_block
                        = &neighbour_blocks[above_candidate_index];

                    BlockIndex candidate_index
                        = hvox::block_index({ CHUNK_LENGTH - 1, CHUNK_LENGTH - 1, z });
                    const Block* candidate_block
                        = &below_neighbour_blocks[candidate_index];

                    if (is_solid(candidate_block) && !is_solid(above_candidate_block)
                        && !is_solid(twice_above_candidate_block))
                    {
                        ChunkNavmeshNode candidate_block_coord = {
                            {CHUNK_LENGTH - 1, CHUNK_LENGTH - 1, z},
                            below_neighbour->position
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor here, in_below_neighbour;
                        } candidate_block_vertex
                            = { impl::get_vertex(chunk_navmesh, candidate_block_coord),
                                impl::get_vertex(
                                    below_neighbour_navmesh, candidate_block_coord
                                ) };

                        boost::add_edge(
                            this_block_vertex.here,
                            candidate_block_vertex.here,
                            chunk_navmesh.graph
                        );
                        boost::add_edge(
                            candidate_block_vertex.here,
                            this_block_vertex.here,
                            chunk_navmesh.graph
                        );

                        boost::add_edge(
                            this_block_vertex.in_below_neighbour,
                            candidate_block_vertex.in_below_neighbour,
                            below_neighbour_navmesh.graph
                        );
                        boost::add_edge(
                            candidate_block_vertex.in_below_neighbour,
                            this_block_vertex.in_below_neighbour,
                            below_neighbour_navmesh.graph
                        );
                    }
                }

                below_neighbour->navmesh_stitch.above_right.store(ChunkState::COMPLETE);
            }

            neighbour->navmesh_stitch.right.store(ChunkState::COMPLETE);
        }
    }

    /**********************************\
     * Try to stitch right neighbour. *
     *   except along top edge.       *
    \**********************************/

    {
        auto       neighbour    = chunk->neighbours.one.right.lock();
        ChunkState stitch_state = ChunkState::NONE;
        if (neighbour != nullptr
            && neighbour->bulk_navmeshing.load() == ChunkState::COMPLETE
            && chunk->navmesh_stitch.right.compare_exchange_strong(
                stitch_state, ChunkState::ACTIVE
            ))
        {
            std::shared_lock<std::shared_mutex> neighbour_block_lock;
            auto neighbour_blocks = neighbour->blocks.get(neighbour_block_lock);
            std::unique_lock<std::shared_mutex> neighbour_navmesh_lock;
            auto neighbour_navmesh = neighbour->navmesh.get(neighbour_navmesh_lock);

            for (BlockChunkPositionCoord z = 0; z < CHUNK_LENGTH; ++z) {
                for (BlockChunkPositionCoord y = 1; y < CHUNK_LENGTH - 2; ++y) {
                    do_side_stitch_navigable_check(
                        neighbour_blocks,
                        neighbour_navmesh,
                        neighbour->position,
                        { CHUNK_LENGTH - 1, y, z },
                        { 0, y, z },
                        2,
                        -1
                    );
                }
            }

            // Special handling for
            //      y == 0, CHUNK_LENGTH - 2
            // where chunks above need to be loaded too.

            // y == 0, CHUNK_LENGTH - 2
            for (BlockChunkPositionCoord z = 0; z < CHUNK_LENGTH; ++z) {
                // y == 0 - step across or up
                do_side_stitch_navigable_check(
                    neighbour_blocks,
                    neighbour_navmesh,
                    neighbour->position,
                    { CHUNK_LENGTH - 1, 0, z },
                    { 0, 0, z },
                    2,
                    0
                );

                // y == CHUNK_LENGTH - 2 - step down or across
                do_side_stitch_navigable_check(
                    neighbour_blocks,
                    neighbour_navmesh,
                    neighbour->position,
                    { CHUNK_LENGTH - 1, CHUNK_LENGTH - 2, z },
                    { 0, CHUNK_LENGTH - 2, z },
                    1,
                    -1
                );
            }

            // y == 0 - step down
            auto       below_neighbour    = neighbour->neighbours.one.bottom.lock();
            ChunkState below_stitch_state = ChunkState::NONE;
            if (below_neighbour != nullptr
                && below_neighbour->bulk_navmeshing.load() == ChunkState::COMPLETE
                && below_neighbour->navmesh_stitch.above_left.compare_exchange_strong(
                    below_stitch_state, ChunkState::ACTIVE
                ))
            {
                std::shared_lock<std::shared_mutex> below_neighbour_block_lock;
                auto                                below_neighbour_blocks
                    = below_neighbour->blocks.get(below_neighbour_block_lock);
                std::unique_lock<std::shared_mutex> below_neighbour_navmesh_lock;
                auto                                below_neighbour_navmesh
                    = below_neighbour->navmesh.get(below_neighbour_navmesh_lock);

                for (BlockChunkPositionCoord z = 0; z < CHUNK_LENGTH; ++z) {
                    BlockIndex this_block_index
                        = hvox::block_index({ CHUNK_LENGTH - 1, 0, z });
                    const Block* this_block = &chunk_blocks[this_block_index];

                    BlockIndex above_this_block_index
                        = hvox::block_index({ CHUNK_LENGTH - 1, 1, z });
                    const Block* above_this_block
                        = &chunk_blocks[above_this_block_index];

                    if (!is_solid(this_block) || is_solid(above_this_block)) continue;

                    ChunkNavmeshNode this_block_coord = {
                        {CHUNK_LENGTH - 1, 0, z},
                        chunk_pos
                    };

                    struct {
                        ChunkNavmeshVertexDescriptor here, in_below_neighbour;
                    } this_block_vertex = {
                        impl::get_vertex(chunk_navmesh, this_block_coord),
                        impl::get_vertex(below_neighbour_navmesh, this_block_coord)
                    };

                    BlockIndex twice_above_candidate_index
                        = hvox::block_index({ 0, 1, z });
                    const Block* twice_above_candidate_block
                        = &neighbour_blocks[twice_above_candidate_index];

                    BlockIndex   above_candidate_index = hvox::block_index({ 0, 0, z });
                    const Block* above_candidate_block
                        = &neighbour_blocks[above_candidate_index];

                    BlockIndex candidate_index
                        = hvox::block_index({ 0, CHUNK_LENGTH - 1, z });
                    const Block* candidate_block
                        = &below_neighbour_blocks[candidate_index];

                    if (is_solid(candidate_block) && !is_solid(above_candidate_block)
                        && !is_solid(twice_above_candidate_block))
                    {
                        ChunkNavmeshNode candidate_block_coord = {
                            {0, CHUNK_LENGTH - 1, z},
                            below_neighbour->position
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor here, in_below_neighbour;
                        } candidate_block_vertex
                            = { impl::get_vertex(chunk_navmesh, candidate_block_coord),
                                impl::get_vertex(
                                    below_neighbour_navmesh, candidate_block_coord
                                ) };

                        boost::add_edge(
                            this_block_vertex.here,
                            candidate_block_vertex.here,
                            chunk_navmesh.graph
                        );
                        boost::add_edge(
                            candidate_block_vertex.here,
                            this_block_vertex.here,
                            chunk_navmesh.graph
                        );

                        boost::add_edge(
                            this_block_vertex.in_below_neighbour,
                            candidate_block_vertex.in_below_neighbour,
                            below_neighbour_navmesh.graph
                        );
                        boost::add_edge(
                            candidate_block_vertex.in_below_neighbour,
                            this_block_vertex.in_below_neighbour,
                            below_neighbour_navmesh.graph
                        );
                    }
                }

                below_neighbour->navmesh_stitch.above_left.store(ChunkState::COMPLETE);
            }

            chunk->navmesh_stitch.right.store(ChunkState::COMPLETE);
        }
    }

    /**********************************\
     * Try to stitch front neighbour. *
    \**********************************/

    {
        auto       neighbour    = chunk->neighbours.one.front.lock();
        ChunkState stitch_state = ChunkState::NONE;
        if (neighbour != nullptr
            && neighbour->bulk_navmeshing.load() == ChunkState::COMPLETE
            && chunk->navmesh_stitch.front.compare_exchange_strong(
                stitch_state, ChunkState::ACTIVE
            ))
        {
            std::shared_lock<std::shared_mutex> neighbour_block_lock;
            auto neighbour_blocks = neighbour->blocks.get(neighbour_block_lock);
            std::unique_lock<std::shared_mutex> neighbour_navmesh_lock;
            auto neighbour_navmesh = neighbour->navmesh.get(neighbour_navmesh_lock);

            for (BlockChunkPositionCoord x = 0; x < CHUNK_LENGTH; ++x) {
                for (BlockChunkPositionCoord y = 1; y < CHUNK_LENGTH - 2; ++y) {
                    do_side_stitch_navigable_check(
                        neighbour_blocks,
                        neighbour_navmesh,
                        neighbour->position,
                        { x, y, CHUNK_LENGTH - 1 },
                        { x, y, 0 },
                        2,
                        -1
                    );
                }
            }

            // Special handling for
            //      y == 0, CHUNK_LENGTH - 2
            // where chunks above need to be loaded too.

            // y == 0, CHUNK_LENGTH - 2
            for (BlockChunkPositionCoord x = 0; x < CHUNK_LENGTH; ++x) {
                // y == 0 - step across or up
                do_side_stitch_navigable_check(
                    neighbour_blocks,
                    neighbour_navmesh,
                    neighbour->position,
                    { x, 0, CHUNK_LENGTH - 1 },
                    { x, 0, 0 },
                    2,
                    0
                );

                // y == CHUNK_LENGTH - 2 - step down or across
                do_side_stitch_navigable_check(
                    neighbour_blocks,
                    neighbour_navmesh,
                    neighbour->position,
                    { x, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1 },
                    { x, CHUNK_LENGTH - 2, 0 },
                    1,
                    -1
                );
            }

            // y == 0 - step down
            auto       below_neighbour    = neighbour->neighbours.one.bottom.lock();
            ChunkState below_stitch_state = ChunkState::NONE;
            if (below_neighbour != nullptr
                && below_neighbour->bulk_navmeshing.load() == ChunkState::COMPLETE
                && below_neighbour->navmesh_stitch.above_back.compare_exchange_strong(
                    below_stitch_state, ChunkState::ACTIVE
                ))
            {
                std::shared_lock<std::shared_mutex> below_neighbour_block_lock;
                auto                                below_neighbour_blocks
                    = below_neighbour->blocks.get(below_neighbour_block_lock);
                std::unique_lock<std::shared_mutex> below_neighbour_navmesh_lock;
                auto                                below_neighbour_navmesh
                    = below_neighbour->navmesh.get(below_neighbour_navmesh_lock);

                for (BlockChunkPositionCoord x = 0; x < CHUNK_LENGTH; ++x) {
                    BlockIndex this_block_index
                        = hvox::block_index({ x, 0, CHUNK_LENGTH - 1 });
                    const Block* this_block = &chunk_blocks[this_block_index];

                    BlockIndex above_this_block_index
                        = hvox::block_index({ x, 1, CHUNK_LENGTH - 1 });
                    const Block* above_this_block
                        = &chunk_blocks[above_this_block_index];

                    if (!is_solid(this_block) || is_solid(above_this_block)) continue;

                    ChunkNavmeshNode this_block_coord = {
                        {x, 0, CHUNK_LENGTH - 1},
                        chunk_pos
                    };

                    struct {
                        ChunkNavmeshVertexDescriptor here, in_below_neighbour;
                    } this_block_vertex = {
                        impl::get_vertex(chunk_navmesh, this_block_coord),
                        impl::get_vertex(below_neighbour_navmesh, this_block_coord)
                    };

                    BlockIndex twice_above_candidate_index
                        = hvox::block_index({ x, 1, 0 });
                    const Block* twice_above_candidate_block
                        = &neighbour_blocks[twice_above_candidate_index];

                    BlockIndex   above_candidate_index = hvox::block_index({ x, 0, 0 });
                    const Block* above_candidate_block
                        = &neighbour_blocks[above_candidate_index];

                    BlockIndex candidate_index
                        = hvox::block_index({ x, CHUNK_LENGTH - 1, 0 });
                    const Block* candidate_block
                        = &below_neighbour_blocks[candidate_index];

                    if (is_solid(candidate_block) && !is_solid(above_candidate_block)
                        && !is_solid(twice_above_candidate_block))
                    {
                        ChunkNavmeshNode candidate_block_coord = {
                            {x, CHUNK_LENGTH - 1, 0},
                            below_neighbour->position
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor here, in_below_neighbour;
                        } candidate_block_vertex
                            = { impl::get_vertex(chunk_navmesh, candidate_block_coord),
                                impl::get_vertex(
                                    below_neighbour_navmesh, candidate_block_coord
                                ) };

                        boost::add_edge(
                            this_block_vertex.here,
                            candidate_block_vertex.here,
                            chunk_navmesh.graph
                        );
                        boost::add_edge(
                            candidate_block_vertex.here,
                            this_block_vertex.here,
                            chunk_navmesh.graph
                        );

                        boost::add_edge(
                            this_block_vertex.in_below_neighbour,
                            candidate_block_vertex.in_below_neighbour,
                            below_neighbour_navmesh.graph
                        );
                        boost::add_edge(
                            candidate_block_vertex.in_below_neighbour,
                            this_block_vertex.in_below_neighbour,
                            below_neighbour_navmesh.graph
                        );
                    }
                }

                below_neighbour->navmesh_stitch.above_back.store(ChunkState::COMPLETE);
            }

            chunk->navmesh_stitch.front.store(ChunkState::COMPLETE);
        }
    }

    /*********************************\
     * Try to stitch back neighbour. *
     *   except along top edge.      *
    \*********************************/

    {
        auto       neighbour    = chunk->neighbours.one.back.lock();
        ChunkState stitch_state = ChunkState::NONE;
        if (neighbour != nullptr
            && neighbour->bulk_navmeshing.load() == ChunkState::COMPLETE
            && neighbour->navmesh_stitch.front.compare_exchange_strong(
                stitch_state, ChunkState::ACTIVE
            ))
        {
            std::shared_lock<std::shared_mutex> neighbour_block_lock;
            auto neighbour_blocks = neighbour->blocks.get(neighbour_block_lock);
            std::unique_lock<std::shared_mutex> neighbour_navmesh_lock;
            auto neighbour_navmesh = neighbour->navmesh.get(neighbour_navmesh_lock);

            for (BlockChunkPositionCoord x = 0; x < CHUNK_LENGTH; ++x) {
                for (BlockChunkPositionCoord y = 1; y < CHUNK_LENGTH - 2; ++y) {
                    do_side_stitch_navigable_check(
                        neighbour_blocks,
                        neighbour_navmesh,
                        neighbour->position,
                        { x, y, 0 },
                        { x, y, CHUNK_LENGTH - 1 },
                        2,
                        -1
                    );
                }
            }

            // Special handling for
            //      y == 0, CHUNK_LENGTH - 2
            // where chunks above need to be loaded too.

            // y == 0, CHUNK_LENGTH - 2
            for (BlockChunkPositionCoord x = 0; x < CHUNK_LENGTH; ++x) {
                // y == 0 - step across or up
                do_side_stitch_navigable_check(
                    neighbour_blocks,
                    neighbour_navmesh,
                    neighbour->position,
                    { x, 0, 0 },
                    { x, 0, CHUNK_LENGTH - 1 },
                    2,
                    0
                );

                // y == CHUNK_LENGTH - 2 - step down or across
                do_side_stitch_navigable_check(
                    neighbour_blocks,
                    neighbour_navmesh,
                    neighbour->position,
                    { x, CHUNK_LENGTH - 2, 0 },
                    { x, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1 },
                    1,
                    -1
                );
            }

            // y == 0 - step down
            auto       below_neighbour    = neighbour->neighbours.one.bottom.lock();
            ChunkState below_stitch_state = ChunkState::NONE;
            if (below_neighbour != nullptr
                && below_neighbour->bulk_navmeshing.load() == ChunkState::COMPLETE
                && below_neighbour->navmesh_stitch.above_front.compare_exchange_strong(
                    below_stitch_state, ChunkState::ACTIVE
                ))
            {
                std::shared_lock<std::shared_mutex> below_neighbour_block_lock;
                auto                                below_neighbour_blocks
                    = below_neighbour->blocks.get(below_neighbour_block_lock);
                std::unique_lock<std::shared_mutex> below_neighbour_navmesh_lock;
                auto                                below_neighbour_navmesh
                    = below_neighbour->navmesh.get(below_neighbour_navmesh_lock);

                for (BlockChunkPositionCoord x = 0; x < CHUNK_LENGTH; ++x) {
                    BlockIndex   this_block_index = hvox::block_index({ x, 0, 0 });
                    const Block* this_block       = &chunk_blocks[this_block_index];

                    BlockIndex above_this_block_index = hvox::block_index({ x, 1, 0 });
                    const Block* above_this_block
                        = &chunk_blocks[above_this_block_index];

                    if (!is_solid(this_block) || is_solid(above_this_block)) continue;

                    ChunkNavmeshNode this_block_coord = {
                        {x, 0, 0},
                        chunk_pos
                    };

                    struct {
                        ChunkNavmeshVertexDescriptor here, in_below_neighbour;
                    } this_block_vertex = {
                        impl::get_vertex(chunk_navmesh, this_block_coord),
                        impl::get_vertex(below_neighbour_navmesh, this_block_coord)
                    };

                    BlockIndex twice_above_candidate_index
                        = hvox::block_index({ x, 1, CHUNK_LENGTH - 1 });
                    const Block* twice_above_candidate_block
                        = &neighbour_blocks[twice_above_candidate_index];

                    BlockIndex above_candidate_index
                        = hvox::block_index({ x, 0, CHUNK_LENGTH - 1 });
                    const Block* above_candidate_block
                        = &neighbour_blocks[above_candidate_index];

                    BlockIndex candidate_index
                        = hvox::block_index({ x, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1 });
                    const Block* candidate_block
                        = &below_neighbour_blocks[candidate_index];

                    if (is_solid(candidate_block) && !is_solid(above_candidate_block)
                        && !is_solid(twice_above_candidate_block))
                    {
                        ChunkNavmeshNode candidate_block_coord = {
                            {x, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1},
                            below_neighbour->position
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor here, in_below_neighbour;
                        } candidate_block_vertex
                            = { impl::get_vertex(chunk_navmesh, candidate_block_coord),
                                impl::get_vertex(
                                    below_neighbour_navmesh, candidate_block_coord
                                ) };

                        boost::add_edge(
                            this_block_vertex.here,
                            candidate_block_vertex.here,
                            chunk_navmesh.graph
                        );
                        boost::add_edge(
                            candidate_block_vertex.here,
                            this_block_vertex.here,
                            chunk_navmesh.graph
                        );

                        boost::add_edge(
                            this_block_vertex.in_below_neighbour,
                            candidate_block_vertex.in_below_neighbour,
                            below_neighbour_navmesh.graph
                        );
                        boost::add_edge(
                            candidate_block_vertex.in_below_neighbour,
                            this_block_vertex.in_below_neighbour,
                            below_neighbour_navmesh.graph
                        );
                    }
                }

                below_neighbour->navmesh_stitch.above_front.store(ChunkState::COMPLETE);
            }

            neighbour->navmesh_stitch.front.store(ChunkState::COMPLETE);
        }
    }

    /********************************\
     * Try to stitch top neighbour. *
    \********************************/

    {
        auto       neighbour    = chunk->neighbours.one.top.lock();
        ChunkState stitch_state = ChunkState::NONE;
        if (neighbour != nullptr
            && neighbour->bulk_navmeshing.load() == ChunkState::COMPLETE
            && chunk->navmesh_stitch.top.compare_exchange_strong(
                stitch_state, ChunkState::ACTIVE
            ))
        {
            std::shared_lock<std::shared_mutex> neighbour_block_lock;
            auto neighbour_blocks = neighbour->blocks.get(neighbour_block_lock);
            std::unique_lock<std::shared_mutex> neighbour_navmesh_lock;
            auto neighbour_navmesh = neighbour->navmesh.get(neighbour_navmesh_lock);

            for (BlockChunkPositionCoord x = 1; x < CHUNK_LENGTH - 1; ++x) {
                for (BlockChunkPositionCoord z = 1; z < CHUNK_LENGTH - 1; ++z) {
                    BlockIndex this_block_index
                        = hvox::block_index({ x, CHUNK_LENGTH - 1, z });
                    const Block* this_block = &chunk_blocks[this_block_index];

                    BlockIndex   neighbour_block_index = hvox::block_index({ x, 0, z });
                    const Block* neighbour_block
                        = &neighbour_blocks[neighbour_block_index];

                    BlockIndex above_neighbour_block_index
                        = hvox::block_index({ x, 1, z });
                    const Block* above_neighbour_block
                        = &neighbour_blocks[above_neighbour_block_index];

                    if (is_solid(this_block) && !is_solid(neighbour_block)) {
                        // Ensure node exists for this block.
                        ChunkNavmeshNode this_block_coord = {
                            {x, CHUNK_LENGTH - 1, z},
                            chunk_pos
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor here, in_neighbour;
                        } this_block_vertex
                            = { impl::get_vertex(chunk_navmesh, this_block_coord),
                                impl::get_vertex(neighbour_navmesh, this_block_coord) };

                        // Up
                        if (!is_solid(above_neighbour_block)) {
                            // Left
                            BlockIndex left_of_neighbour_block_index
                                = hvox::block_index({ x - 1, 0, z });
                            const Block* left_of_neighbour_block
                                = &neighbour_blocks[left_of_neighbour_block_index];

                            BlockIndex above_and_left_of_neighbour_block_index
                                = hvox::block_index({ x - 1, 1, z });
                            const Block* above_and_left_of_neighbour_block
                                = &neighbour_blocks
                                      [above_and_left_of_neighbour_block_index];

                            if (is_solid(left_of_neighbour_block)
                                && !is_solid(above_and_left_of_neighbour_block))
                            {
                                ChunkNavmeshNode left_of_neighbour_block_coord = {
                                    {x - 1, 0, z},
                                    neighbour->position
                                };

                                // Ensure node exists for this block.
                                struct {
                                    ChunkNavmeshVertexDescriptor here, in_neighbour;
                                } left_of_neighbour_block_vertex
                                    = { impl::get_vertex(
                                            chunk_navmesh, left_of_neighbour_block_coord
                                        ),
                                        impl::get_vertex(
                                            neighbour_navmesh,
                                            left_of_neighbour_block_coord
                                        ) };

                                boost::add_edge(
                                    this_block_vertex.here,
                                    left_of_neighbour_block_vertex.here,
                                    chunk_navmesh.graph
                                );
                                boost::add_edge(
                                    left_of_neighbour_block_vertex.here,
                                    this_block_vertex.here,
                                    chunk_navmesh.graph
                                );

                                boost::add_edge(
                                    this_block_vertex.in_neighbour,
                                    left_of_neighbour_block_vertex.in_neighbour,
                                    neighbour_navmesh.graph
                                );
                                boost::add_edge(
                                    left_of_neighbour_block_vertex.in_neighbour,
                                    this_block_vertex.in_neighbour,
                                    neighbour_navmesh.graph
                                );
                            }

                            // Right
                            BlockIndex right_of_neighbour_block_index
                                = hvox::block_index({ x + 1, 0, z });
                            const Block* right_of_neighbour_block
                                = &neighbour_blocks[right_of_neighbour_block_index];

                            BlockIndex above_and_right_of_neighbour_block_index
                                = hvox::block_index({ x + 1, 1, z });
                            const Block* above_and_right_of_neighbour_block
                                = &neighbour_blocks
                                      [above_and_right_of_neighbour_block_index];

                            if (is_solid(right_of_neighbour_block)
                                && !is_solid(above_and_right_of_neighbour_block))
                            {
                                ChunkNavmeshNode right_of_neighbour_block_coord = {
                                    {x + 1, 0, z},
                                    neighbour->position
                                };

                                // Ensure node exists for this block.
                                struct {
                                    ChunkNavmeshVertexDescriptor here, in_neighbour;
                                } right_of_neighbour_block_vertex
                                    = { impl::get_vertex(
                                            chunk_navmesh,
                                            right_of_neighbour_block_coord
                                        ),
                                        impl::get_vertex(
                                            neighbour_navmesh,
                                            right_of_neighbour_block_coord
                                        ) };

                                boost::add_edge(
                                    this_block_vertex.here,
                                    right_of_neighbour_block_vertex.here,
                                    chunk_navmesh.graph
                                );
                                boost::add_edge(
                                    right_of_neighbour_block_vertex.here,
                                    this_block_vertex.here,
                                    chunk_navmesh.graph
                                );

                                boost::add_edge(
                                    this_block_vertex.in_neighbour,
                                    right_of_neighbour_block_vertex.in_neighbour,
                                    neighbour_navmesh.graph
                                );
                                boost::add_edge(
                                    right_of_neighbour_block_vertex.in_neighbour,
                                    this_block_vertex.in_neighbour,
                                    neighbour_navmesh.graph
                                );
                            }

                            // Front
                            BlockIndex front_of_neighbour_block_index
                                = hvox::block_index({ x, 0, z + 1 });
                            const Block* front_of_neighbour_block
                                = &neighbour_blocks[front_of_neighbour_block_index];

                            BlockIndex above_and_front_of_neighbour_block_index
                                = hvox::block_index({ x, 1, z + 1 });
                            const Block* above_and_front_of_neighbour_block
                                = &neighbour_blocks
                                      [above_and_front_of_neighbour_block_index];

                            if (is_solid(front_of_neighbour_block)
                                && !is_solid(above_and_front_of_neighbour_block))
                            {
                                ChunkNavmeshNode front_of_neighbour_block_coord = {
                                    {x, 0, z + 1},
                                    neighbour->position
                                };

                                // Ensure node exists for this block.
                                struct {
                                    ChunkNavmeshVertexDescriptor here, in_neighbour;
                                } front_of_neighbour_block_vertex
                                    = { impl::get_vertex(
                                            chunk_navmesh,
                                            front_of_neighbour_block_coord
                                        ),
                                        impl::get_vertex(
                                            neighbour_navmesh,
                                            front_of_neighbour_block_coord
                                        ) };

                                boost::add_edge(
                                    this_block_vertex.here,
                                    front_of_neighbour_block_vertex.here,
                                    chunk_navmesh.graph
                                );
                                boost::add_edge(
                                    front_of_neighbour_block_vertex.here,
                                    this_block_vertex.here,
                                    chunk_navmesh.graph
                                );

                                boost::add_edge(
                                    this_block_vertex.in_neighbour,
                                    front_of_neighbour_block_vertex.in_neighbour,
                                    neighbour_navmesh.graph
                                );
                                boost::add_edge(
                                    front_of_neighbour_block_vertex.in_neighbour,
                                    this_block_vertex.in_neighbour,
                                    neighbour_navmesh.graph
                                );
                            }

                            // Back
                            BlockIndex back_of_neighbour_block_index
                                = hvox::block_index({ x, 0, z - 1 });
                            const Block* back_of_neighbour_block
                                = &neighbour_blocks[back_of_neighbour_block_index];

                            BlockIndex above_and_back_of_neighbour_block_index
                                = hvox::block_index({ x, 1, z - 1 });
                            const Block* above_and_back_of_neighbour_block
                                = &neighbour_blocks
                                      [above_and_back_of_neighbour_block_index];

                            if (is_solid(back_of_neighbour_block)
                                && !is_solid(above_and_back_of_neighbour_block))
                            {
                                ChunkNavmeshNode back_of_neighbour_block_coord = {
                                    {x, 0, z - 1},
                                    neighbour->position
                                };

                                // Ensure node exists for this block.
                                struct {
                                    ChunkNavmeshVertexDescriptor here, in_neighbour;
                                } back_of_neighbour_block_vertex
                                    = { impl::get_vertex(
                                            chunk_navmesh, back_of_neighbour_block_coord
                                        ),
                                        impl::get_vertex(
                                            neighbour_navmesh,
                                            back_of_neighbour_block_coord
                                        ) };

                                boost::add_edge(
                                    this_block_vertex.here,
                                    back_of_neighbour_block_vertex.here,
                                    chunk_navmesh.graph
                                );
                                boost::add_edge(
                                    back_of_neighbour_block_vertex.here,
                                    this_block_vertex.here,
                                    chunk_navmesh.graph
                                );

                                boost::add_edge(
                                    this_block_vertex.in_neighbour,
                                    back_of_neighbour_block_vertex.in_neighbour,
                                    neighbour_navmesh.graph
                                );
                                boost::add_edge(
                                    back_of_neighbour_block_vertex.in_neighbour,
                                    this_block_vertex.in_neighbour,
                                    neighbour_navmesh.graph
                                );
                            }
                        }
                        // Across and Down
                        else
                        {
                            // Left
                            BlockIndex left_of_this_block_index
                                = hvox::block_index({ x - 1, CHUNK_LENGTH - 1, z });
                            const Block* left_of_this_block
                                = &chunk_blocks[left_of_this_block_index];

                            BlockIndex left_of_and_below_this_block_index
                                = hvox::block_index({ x - 1, CHUNK_LENGTH - 2, z });
                            const Block* left_of_and_below_this_block
                                = &chunk_blocks[left_of_and_below_this_block_index];

                            BlockIndex left_of_neighbour_block_index
                                = hvox::block_index({ x - 1, 0, z });
                            const Block* left_of_neighbour_block
                                = &neighbour_blocks[left_of_neighbour_block_index];

                            // Across
                            if (is_solid(left_of_this_block)
                                && !is_solid(left_of_neighbour_block))
                            {
                                ChunkNavmeshNode left_of_this_block_coord = {
                                    {x - 1, CHUNK_LENGTH - 1, z},
                                    chunk_pos
                                };

                                // Ensure node exists for this block.
                                ChunkNavmeshVertexDescriptor left_of_this_block_vertex
                                    = impl::get_vertex(
                                        chunk_navmesh, left_of_this_block_coord
                                    );

                                boost::add_edge(
                                    this_block_vertex.here,
                                    left_of_this_block_vertex,
                                    chunk_navmesh.graph
                                );
                                boost::add_edge(
                                    left_of_this_block_vertex,
                                    this_block_vertex.here,
                                    chunk_navmesh.graph
                                );
                                // Down
                            } else if (is_solid(left_of_and_below_this_block) && !is_solid(left_of_this_block) && !is_solid(left_of_neighbour_block))
                            {
                                ChunkNavmeshNode left_of_and_below_this_block_coord = {
                                    {x - 1, CHUNK_LENGTH - 2, z},
                                    chunk_pos
                                };

                                // Ensure node exists for this block.
                                ChunkNavmeshVertexDescriptor
                                    left_of_and_below_this_block_vertex
                                    = impl::get_vertex(
                                        chunk_navmesh,
                                        left_of_and_below_this_block_coord
                                    );

                                boost::add_edge(
                                    this_block_vertex.here,
                                    left_of_and_below_this_block_vertex,
                                    chunk_navmesh.graph
                                );
                                boost::add_edge(
                                    left_of_and_below_this_block_vertex,
                                    this_block_vertex.here,
                                    chunk_navmesh.graph
                                );
                            }

                            // Right
                            BlockIndex right_of_this_block_index
                                = hvox::block_index({ x + 1, CHUNK_LENGTH - 1, z });
                            const Block* right_of_this_block
                                = &chunk_blocks[right_of_this_block_index];

                            BlockIndex right_of_and_below_this_block_index
                                = hvox::block_index({ x + 1, CHUNK_LENGTH - 2, z });
                            const Block* right_of_and_below_this_block
                                = &chunk_blocks[right_of_and_below_this_block_index];

                            BlockIndex right_of_neighbour_block_index
                                = hvox::block_index({ x + 1, 0, z });
                            const Block* right_of_neighbour_block
                                = &neighbour_blocks[right_of_neighbour_block_index];

                            // Across
                            if (is_solid(right_of_this_block)
                                && !is_solid(right_of_neighbour_block))
                            {
                                ChunkNavmeshNode right_of_this_block_coord = {
                                    {x + 1, CHUNK_LENGTH - 1, z},
                                    chunk_pos
                                };

                                // Ensure node exists for this block.
                                ChunkNavmeshVertexDescriptor right_of_this_block_vertex
                                    = impl::get_vertex(
                                        chunk_navmesh, right_of_this_block_coord
                                    );

                                boost::add_edge(
                                    this_block_vertex.here,
                                    right_of_this_block_vertex,
                                    chunk_navmesh.graph
                                );
                                boost::add_edge(
                                    right_of_this_block_vertex,
                                    this_block_vertex.here,
                                    chunk_navmesh.graph
                                );
                                // Down
                            } else if (is_solid(right_of_and_below_this_block) && !is_solid(right_of_this_block) && !is_solid(right_of_neighbour_block))
                            {
                                ChunkNavmeshNode right_of_and_below_this_block_coord = {
                                    {x + 1, CHUNK_LENGTH - 2, z},
                                    chunk_pos
                                };

                                // Ensure node exists for this block.
                                ChunkNavmeshVertexDescriptor
                                    right_of_and_below_this_block_vertex
                                    = impl::get_vertex(
                                        chunk_navmesh,
                                        right_of_and_below_this_block_coord
                                    );

                                boost::add_edge(
                                    this_block_vertex.here,
                                    right_of_and_below_this_block_vertex,
                                    chunk_navmesh.graph
                                );
                                boost::add_edge(
                                    right_of_and_below_this_block_vertex,
                                    this_block_vertex.here,
                                    chunk_navmesh.graph
                                );
                            }

                            // Front
                            BlockIndex front_of_this_block_index
                                = hvox::block_index({ x, CHUNK_LENGTH - 1, z + 1 });
                            const Block* front_of_this_block
                                = &chunk_blocks[front_of_this_block_index];

                            BlockIndex front_of_and_below_this_block_index
                                = hvox::block_index({ x, CHUNK_LENGTH - 2, z + 1 });
                            const Block* front_of_and_below_this_block
                                = &chunk_blocks[front_of_and_below_this_block_index];

                            BlockIndex front_of_neighbour_block_index
                                = hvox::block_index({ x, 0, z + 1 });
                            const Block* front_of_neighbour_block
                                = &neighbour_blocks[front_of_neighbour_block_index];

                            // Across
                            if (is_solid(front_of_this_block)
                                && !is_solid(front_of_neighbour_block))
                            {
                                ChunkNavmeshNode front_of_this_block_coord = {
                                    {x, CHUNK_LENGTH - 1, z + 1},
                                    chunk_pos
                                };

                                // Ensure node exists for this block.
                                ChunkNavmeshVertexDescriptor front_of_this_block_vertex
                                    = impl::get_vertex(
                                        chunk_navmesh, front_of_this_block_coord
                                    );

                                boost::add_edge(
                                    this_block_vertex.here,
                                    front_of_this_block_vertex,
                                    chunk_navmesh.graph
                                );
                                boost::add_edge(
                                    front_of_this_block_vertex,
                                    this_block_vertex.here,
                                    chunk_navmesh.graph
                                );
                                // Down
                            } else if (is_solid(front_of_and_below_this_block) && !is_solid(front_of_this_block) && !is_solid(front_of_neighbour_block))
                            {
                                ChunkNavmeshNode front_of_and_below_this_block_coord = {
                                    {x, CHUNK_LENGTH - 2, z + 1},
                                    chunk_pos
                                };

                                // Ensure node exists for this block.
                                ChunkNavmeshVertexDescriptor
                                    front_of_and_below_this_block_vertex
                                    = impl::get_vertex(
                                        chunk_navmesh,
                                        front_of_and_below_this_block_coord
                                    );

                                boost::add_edge(
                                    this_block_vertex.here,
                                    front_of_and_below_this_block_vertex,
                                    chunk_navmesh.graph
                                );
                                boost::add_edge(
                                    front_of_and_below_this_block_vertex,
                                    this_block_vertex.here,
                                    chunk_navmesh.graph
                                );
                            }

                            // Back
                            BlockIndex back_of_this_block_index
                                = hvox::block_index({ x, CHUNK_LENGTH - 1, z - 1 });
                            const Block* back_of_this_block
                                = &chunk_blocks[back_of_this_block_index];

                            BlockIndex back_of_and_below_this_block_index
                                = hvox::block_index({ x, CHUNK_LENGTH - 2, z - 1 });
                            const Block* back_of_and_below_this_block
                                = &chunk_blocks[back_of_and_below_this_block_index];

                            BlockIndex back_of_neighbour_block_index
                                = hvox::block_index({ x, 0, z - 1 });
                            const Block* back_of_neighbour_block
                                = &neighbour_blocks[back_of_neighbour_block_index];

                            // Across
                            if (is_solid(back_of_this_block)
                                && !is_solid(back_of_neighbour_block))
                            {
                                ChunkNavmeshNode back_of_this_block_coord = {
                                    {x, CHUNK_LENGTH - 1, z - 1},
                                    chunk_pos
                                };

                                // Ensure node exists for this block.
                                ChunkNavmeshVertexDescriptor back_of_this_block_vertex
                                    = impl::get_vertex(
                                        chunk_navmesh, back_of_this_block_coord
                                    );

                                boost::add_edge(
                                    this_block_vertex.here,
                                    back_of_this_block_vertex,
                                    chunk_navmesh.graph
                                );
                                boost::add_edge(
                                    back_of_this_block_vertex,
                                    this_block_vertex.here,
                                    chunk_navmesh.graph
                                );
                                // Down
                            } else if (is_solid(back_of_and_below_this_block) && !is_solid(back_of_this_block) && !is_solid(back_of_neighbour_block))
                            {
                                ChunkNavmeshNode back_of_and_below_this_block_coord = {
                                    {x, CHUNK_LENGTH - 2, z - 1},
                                    chunk_pos
                                };

                                // Ensure node exists for this block.
                                ChunkNavmeshVertexDescriptor
                                    back_of_and_below_this_block_vertex
                                    = impl::get_vertex(
                                        chunk_navmesh,
                                        back_of_and_below_this_block_coord
                                    );

                                boost::add_edge(
                                    this_block_vertex.here,
                                    back_of_and_below_this_block_vertex,
                                    chunk_navmesh.graph
                                );
                                boost::add_edge(
                                    back_of_and_below_this_block_vertex,
                                    this_block_vertex.here,
                                    chunk_navmesh.graph
                                );
                            }
                        }
                    }
                }
            }

            // Note only one of top or bottom stitch will ever be done for any pair of
            // chunks, so we must do this next part in both cases.

            // Check step up from top layer of chunk to each adjacent neighbour of above
            // chunk.

            // Left
            {
                auto       left_of_neighbour = neighbour->neighbours.one.left.lock();
                ChunkState left_of_neighbour_stitch_state = ChunkState::NONE;
                if (left_of_neighbour != nullptr
                    && left_of_neighbour->bulk_navmeshing.load() == ChunkState::COMPLETE
                    && chunk->navmesh_stitch.above_left.compare_exchange_strong(
                        left_of_neighbour_stitch_state, ChunkState::ACTIVE
                    ))
                {
                    std::shared_lock<std::shared_mutex> left_of_neighbour_block_lock;
                    auto                                left_of_neighbour_blocks
                        = left_of_neighbour->blocks.get(left_of_neighbour_block_lock);
                    std::unique_lock<std::shared_mutex> left_of_neighbour_navmesh_lock;
                    auto                                left_of_neighbour_navmesh
                        = left_of_neighbour->navmesh.get(left_of_neighbour_navmesh_lock
                        );

                    for (BlockChunkPositionCoord z = 0; z < CHUNK_LENGTH; ++z) {
                        BlockIndex left_of_neighbour_block_index
                            = hvox::block_index({ CHUNK_LENGTH - 1, 0, z });
                        const Block* left_of_neighbour_block
                            = &left_of_neighbour_blocks[left_of_neighbour_block_index];

                        BlockIndex above_left_of_neighbour_block_index
                            = hvox::block_index({ CHUNK_LENGTH - 1, 1, z });
                        const Block* above_left_of_neighbour_block
                            = &left_of_neighbour_blocks
                                  [above_left_of_neighbour_block_index];

                        if (!is_solid(left_of_neighbour_block)
                            || is_solid(above_left_of_neighbour_block))
                            continue;

                        ChunkNavmeshNode left_of_neighbour_block_coord = {
                            {CHUNK_LENGTH - 1, 0, z},
                            left_of_neighbour->position
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor here, in_left_of_neighbour;
                        } left_of_neighbour_block_vertex
                            = { impl::get_vertex(
                                    chunk_navmesh, left_of_neighbour_block_coord
                                ),
                                impl::get_vertex(
                                    left_of_neighbour_navmesh,
                                    left_of_neighbour_block_coord
                                ) };

                        BlockIndex twice_above_candidate_index
                            = hvox::block_index({ 0, 1, z });
                        const Block* twice_above_candidate_block
                            = &neighbour_blocks[twice_above_candidate_index];

                        BlockIndex above_candidate_index
                            = hvox::block_index({ 0, 0, z });
                        const Block* above_candidate_block
                            = &neighbour_blocks[above_candidate_index];

                        BlockIndex candidate_index
                            = hvox::block_index({ 0, CHUNK_LENGTH - 1, z });
                        const Block* candidate_block = &chunk_blocks[candidate_index];

                        if (is_solid(candidate_block)
                            && !is_solid(above_candidate_block)
                            && !is_solid(twice_above_candidate_block))
                        {
                            ChunkNavmeshNode candidate_block_coord = {
                                {0, CHUNK_LENGTH - 1, z},
                                chunk_pos
                            };

                            struct {
                                ChunkNavmeshVertexDescriptor here, in_left_of_neighbour;
                            } candidate_block_vertex = {
                                impl::get_vertex(chunk_navmesh, candidate_block_coord),
                                impl::get_vertex(
                                    left_of_neighbour_navmesh, candidate_block_coord
                                )
                            };

                            boost::add_edge(
                                left_of_neighbour_block_vertex.here,
                                candidate_block_vertex.here,
                                chunk_navmesh.graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.here,
                                left_of_neighbour_block_vertex.here,
                                chunk_navmesh.graph
                            );

                            boost::add_edge(
                                left_of_neighbour_block_vertex.in_left_of_neighbour,
                                candidate_block_vertex.in_left_of_neighbour,
                                left_of_neighbour_navmesh.graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.in_left_of_neighbour,
                                left_of_neighbour_block_vertex.in_left_of_neighbour,
                                left_of_neighbour_navmesh.graph
                            );
                        }
                    }

                    chunk->navmesh_stitch.above_left.store(ChunkState::COMPLETE);
                }
            }

            // Right
            {
                auto       right_of_neighbour = neighbour->neighbours.one.right.lock();
                ChunkState right_of_neighbour_stitch_state = ChunkState::NONE;
                if (right_of_neighbour != nullptr
                    && right_of_neighbour->bulk_navmeshing.load()
                           == ChunkState::COMPLETE
                    && chunk->navmesh_stitch.above_right.compare_exchange_strong(
                        right_of_neighbour_stitch_state, ChunkState::ACTIVE
                    ))
                {
                    std::shared_lock<std::shared_mutex> right_of_neighbour_block_lock;
                    auto                                right_of_neighbour_blocks
                        = right_of_neighbour->blocks.get(right_of_neighbour_block_lock);
                    std::unique_lock<std::shared_mutex> right_of_neighbour_navmesh_lock;
                    auto right_of_neighbour_navmesh = right_of_neighbour->navmesh.get(
                        right_of_neighbour_navmesh_lock
                    );

                    for (BlockChunkPositionCoord z = 0; z < CHUNK_LENGTH; ++z) {
                        BlockIndex right_of_neighbour_block_index
                            = hvox::block_index({ 0, 0, z });
                        const Block* right_of_neighbour_block
                            = &right_of_neighbour_blocks
                                  [right_of_neighbour_block_index];

                        BlockIndex above_right_of_neighbour_block_index
                            = hvox::block_index({ 0, 1, z });
                        const Block* above_right_of_neighbour_block
                            = &right_of_neighbour_blocks
                                  [above_right_of_neighbour_block_index];

                        if (!is_solid(right_of_neighbour_block)
                            || is_solid(above_right_of_neighbour_block))
                            continue;

                        ChunkNavmeshNode right_of_neighbour_block_coord = {
                            {0, 0, z},
                            right_of_neighbour->position
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor here, in_right_of_neighbour;
                        } right_of_neighbour_block_vertex
                            = { impl::get_vertex(
                                    chunk_navmesh, right_of_neighbour_block_coord
                                ),
                                impl::get_vertex(
                                    right_of_neighbour_navmesh,
                                    right_of_neighbour_block_coord
                                ) };

                        BlockIndex twice_above_candidate_index
                            = hvox::block_index({ CHUNK_LENGTH - 1, 1, z });
                        const Block* twice_above_candidate_block
                            = &neighbour_blocks[twice_above_candidate_index];

                        BlockIndex above_candidate_index
                            = hvox::block_index({ CHUNK_LENGTH - 1, 0, z });
                        const Block* above_candidate_block
                            = &neighbour_blocks[above_candidate_index];

                        BlockIndex candidate_index = hvox::block_index(
                            { CHUNK_LENGTH - 1, CHUNK_LENGTH - 1, z }
                        );
                        const Block* candidate_block = &chunk_blocks[candidate_index];

                        if (is_solid(candidate_block)
                            && !is_solid(above_candidate_block)
                            && !is_solid(twice_above_candidate_block))
                        {
                            ChunkNavmeshNode candidate_block_coord = {
                                {CHUNK_LENGTH - 1, CHUNK_LENGTH - 1, z},
                                chunk_pos
                            };

                            struct {
                                ChunkNavmeshVertexDescriptor here,
                                    in_right_of_neighbour;
                            } candidate_block_vertex = {
                                impl::get_vertex(chunk_navmesh, candidate_block_coord),
                                impl::get_vertex(
                                    right_of_neighbour_navmesh, candidate_block_coord
                                )
                            };

                            boost::add_edge(
                                right_of_neighbour_block_vertex.here,
                                candidate_block_vertex.here,
                                chunk_navmesh.graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.here,
                                right_of_neighbour_block_vertex.here,
                                chunk_navmesh.graph
                            );

                            boost::add_edge(
                                right_of_neighbour_block_vertex.in_right_of_neighbour,
                                candidate_block_vertex.in_right_of_neighbour,
                                right_of_neighbour_navmesh.graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.in_right_of_neighbour,
                                right_of_neighbour_block_vertex.in_right_of_neighbour,
                                right_of_neighbour_navmesh.graph
                            );
                        }
                    }

                    chunk->navmesh_stitch.above_right.store(ChunkState::COMPLETE);
                }
            }

            // Front
            {
                auto       front_of_neighbour = neighbour->neighbours.one.front.lock();
                ChunkState front_of_neighbour_stitch_state = ChunkState::NONE;
                if (front_of_neighbour != nullptr
                    && front_of_neighbour->bulk_navmeshing.load()
                           == ChunkState::COMPLETE
                    && chunk->navmesh_stitch.above_front.compare_exchange_strong(
                        front_of_neighbour_stitch_state, ChunkState::ACTIVE
                    ))
                {
                    std::shared_lock<std::shared_mutex> front_of_neighbour_block_lock;
                    auto                                front_of_neighbour_blocks
                        = front_of_neighbour->blocks.get(front_of_neighbour_block_lock);
                    std::unique_lock<std::shared_mutex> front_of_neighbour_navmesh_lock;
                    auto front_of_neighbour_navmesh = front_of_neighbour->navmesh.get(
                        front_of_neighbour_navmesh_lock
                    );

                    for (BlockChunkPositionCoord x = 0; x < CHUNK_LENGTH; ++x) {
                        BlockIndex front_of_neighbour_block_index
                            = hvox::block_index({ x, 0, 0 });
                        const Block* front_of_neighbour_block
                            = &front_of_neighbour_blocks
                                  [front_of_neighbour_block_index];

                        BlockIndex above_front_of_neighbour_block_index
                            = hvox::block_index({ x, 1, 0 });
                        const Block* above_front_of_neighbour_block
                            = &front_of_neighbour_blocks
                                  [above_front_of_neighbour_block_index];

                        if (!is_solid(front_of_neighbour_block)
                            || is_solid(above_front_of_neighbour_block))
                            continue;

                        ChunkNavmeshNode front_of_neighbour_block_coord = {
                            {x, 0, 0},
                            front_of_neighbour->position
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor here, in_front_of_neighbour;
                        } front_of_neighbour_block_vertex
                            = { impl::get_vertex(
                                    chunk_navmesh, front_of_neighbour_block_coord
                                ),
                                impl::get_vertex(
                                    front_of_neighbour_navmesh,
                                    front_of_neighbour_block_coord
                                ) };

                        BlockIndex twice_above_candidate_index
                            = hvox::block_index({ x, 1, CHUNK_LENGTH - 1 });
                        const Block* twice_above_candidate_block
                            = &neighbour_blocks[twice_above_candidate_index];

                        BlockIndex above_candidate_index
                            = hvox::block_index({ x, 0, CHUNK_LENGTH - 1 });
                        const Block* above_candidate_block
                            = &neighbour_blocks[above_candidate_index];

                        BlockIndex candidate_index = hvox::block_index(
                            { x, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1 }
                        );
                        const Block* candidate_block = &chunk_blocks[candidate_index];

                        if (is_solid(candidate_block)
                            && !is_solid(above_candidate_block)
                            && !is_solid(twice_above_candidate_block))
                        {
                            ChunkNavmeshNode candidate_block_coord = {
                                {x, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1},
                                chunk_pos
                            };

                            struct {
                                ChunkNavmeshVertexDescriptor here,
                                    in_front_of_neighbour;
                            } candidate_block_vertex = {
                                impl::get_vertex(chunk_navmesh, candidate_block_coord),
                                impl::get_vertex(
                                    front_of_neighbour_navmesh, candidate_block_coord
                                )
                            };

                            boost::add_edge(
                                front_of_neighbour_block_vertex.here,
                                candidate_block_vertex.here,
                                chunk_navmesh.graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.here,
                                front_of_neighbour_block_vertex.here,
                                chunk_navmesh.graph
                            );

                            boost::add_edge(
                                front_of_neighbour_block_vertex.in_front_of_neighbour,
                                candidate_block_vertex.in_front_of_neighbour,
                                front_of_neighbour_navmesh.graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.in_front_of_neighbour,
                                front_of_neighbour_block_vertex.in_front_of_neighbour,
                                front_of_neighbour_navmesh.graph
                            );
                        }
                    }

                    chunk->navmesh_stitch.above_front.store(ChunkState::COMPLETE);
                }
            }

            // Back
            {
                auto       back_of_neighbour = neighbour->neighbours.one.back.lock();
                ChunkState back_of_neighbour_stitch_state = ChunkState::NONE;
                if (back_of_neighbour != nullptr
                    && back_of_neighbour->bulk_navmeshing.load() == ChunkState::COMPLETE
                    && chunk->navmesh_stitch.above_back.compare_exchange_strong(
                        back_of_neighbour_stitch_state, ChunkState::ACTIVE
                    ))
                {
                    std::shared_lock<std::shared_mutex> back_of_neighbour_block_lock;
                    auto                                back_of_neighbour_blocks
                        = back_of_neighbour->blocks.get(back_of_neighbour_block_lock);
                    std::unique_lock<std::shared_mutex> back_of_neighbour_navmesh_lock;
                    auto                                back_of_neighbour_navmesh
                        = back_of_neighbour->navmesh.get(back_of_neighbour_navmesh_lock
                        );

                    for (BlockChunkPositionCoord x = 0; x < CHUNK_LENGTH; ++x) {
                        BlockIndex back_of_neighbour_block_index
                            = hvox::block_index({ x, 0, CHUNK_LENGTH - 1 });
                        const Block* back_of_neighbour_block
                            = &back_of_neighbour_blocks[back_of_neighbour_block_index];

                        BlockIndex above_back_of_neighbour_block_index
                            = hvox::block_index({ x, 1, CHUNK_LENGTH - 1 });
                        const Block* above_back_of_neighbour_block
                            = &back_of_neighbour_blocks
                                  [above_back_of_neighbour_block_index];

                        if (!is_solid(back_of_neighbour_block)
                            || is_solid(above_back_of_neighbour_block))
                            continue;

                        ChunkNavmeshNode back_of_neighbour_block_coord = {
                            {x, 0, CHUNK_LENGTH - 1},
                            back_of_neighbour->position
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor here, in_back_of_neighbour;
                        } back_of_neighbour_block_vertex
                            = { impl::get_vertex(
                                    chunk_navmesh, back_of_neighbour_block_coord
                                ),
                                impl::get_vertex(
                                    back_of_neighbour_navmesh,
                                    back_of_neighbour_block_coord
                                ) };

                        BlockIndex twice_above_candidate_index
                            = hvox::block_index({ x, 1, 0 });
                        const Block* twice_above_candidate_block
                            = &neighbour_blocks[twice_above_candidate_index];

                        BlockIndex above_candidate_index
                            = hvox::block_index({ x, 0, 0 });
                        const Block* above_candidate_block
                            = &neighbour_blocks[above_candidate_index];

                        BlockIndex candidate_index
                            = hvox::block_index({ x, CHUNK_LENGTH - 1, 0 });
                        const Block* candidate_block = &chunk_blocks[candidate_index];

                        if (is_solid(candidate_block)
                            && !is_solid(above_candidate_block)
                            && !is_solid(twice_above_candidate_block))
                        {
                            ChunkNavmeshNode candidate_block_coord = {
                                {x, CHUNK_LENGTH - 1, 0},
                                chunk_pos
                            };

                            struct {
                                ChunkNavmeshVertexDescriptor here, in_back_of_neighbour;
                            } candidate_block_vertex = {
                                impl::get_vertex(chunk_navmesh, candidate_block_coord),
                                impl::get_vertex(
                                    back_of_neighbour_navmesh, candidate_block_coord
                                )
                            };

                            boost::add_edge(
                                back_of_neighbour_block_vertex.here,
                                candidate_block_vertex.here,
                                chunk_navmesh.graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.here,
                                back_of_neighbour_block_vertex.here,
                                chunk_navmesh.graph
                            );

                            boost::add_edge(
                                back_of_neighbour_block_vertex.in_back_of_neighbour,
                                candidate_block_vertex.in_back_of_neighbour,
                                back_of_neighbour_navmesh.graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.in_back_of_neighbour,
                                back_of_neighbour_block_vertex.in_back_of_neighbour,
                                back_of_neighbour_navmesh.graph
                            );
                        }
                    }

                    chunk->navmesh_stitch.above_back.store(ChunkState::COMPLETE);
                }
            }

            // Check step up from second-to-top layer of chunk to each adjacent
            // neighbour of above chunk, and check step across and down from top layer
            // of chunk to each adjacent neighbour of this and above chunk.

            // Left
            {
                auto       left_neighbour       = chunk->neighbours.one.left.lock();
                auto       above_left_neighbour = neighbour->neighbours.one.left.lock();
                ChunkState diagonal_stitch_state = ChunkState::NONE;
                if (left_neighbour != nullptr && above_left_neighbour != nullptr
                    && left_neighbour->bulk_navmeshing.load() == ChunkState::COMPLETE
                    && above_left_neighbour->bulk_navmeshing.load()
                           == ChunkState::COMPLETE
                    && chunk->navmesh_stitch.above_and_across_left
                           .compare_exchange_strong(
                               diagonal_stitch_state, ChunkState::ACTIVE
                           ))
                {
                    std::shared_lock<std::shared_mutex> left_neighbour_block_lock;
                    auto                                left_neighbour_blocks
                        = left_neighbour->blocks.get(left_neighbour_block_lock);
                    std::unique_lock<std::shared_mutex> left_neighbour_navmesh_lock;
                    auto                                left_neighbour_navmesh
                        = left_neighbour->navmesh.get(left_neighbour_navmesh_lock);

                    std::shared_lock<std::shared_mutex> above_left_neighbour_block_lock;
                    auto above_left_neighbour_blocks = above_left_neighbour->blocks.get(
                        above_left_neighbour_block_lock
                    );
                    std::unique_lock<std::shared_mutex>
                         above_left_neighbour_navmesh_lock;
                    auto above_left_neighbour_navmesh
                        = above_left_neighbour->navmesh.get(
                            above_left_neighbour_navmesh_lock
                        );

                    // Step up from y == CHUNK_LENGTH - 2
                    for (BlockChunkPositionCoord z = 0; z < CHUNK_LENGTH; ++z) {
                        BlockIndex this_block_index
                            = hvox::block_index({ 0, CHUNK_LENGTH - 2, z });
                        const Block* this_block = &chunk_blocks[this_block_index];

                        BlockIndex above_this_block_index
                            = hvox::block_index({ 0, CHUNK_LENGTH - 1, z });
                        const Block* above_this_block
                            = &chunk_blocks[above_this_block_index];

                        BlockIndex twice_above_this_block_index
                            = hvox::block_index({ 0, 0, z });
                        const Block* twice_above_this_block
                            = &neighbour_blocks[twice_above_this_block_index];

                        if (!is_solid(this_block) || is_solid(above_this_block)
                            || is_solid(twice_above_this_block))
                            continue;

                        ChunkNavmeshNode this_block_coord = {
                            {0, CHUNK_LENGTH - 2, z},
                            chunk_pos
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor here, in_left_neighbour;
                        } this_block_vertex = {
                            impl::get_vertex(chunk_navmesh, this_block_coord),
                            impl::get_vertex(left_neighbour_navmesh, this_block_coord)
                        };

                        BlockIndex above_candidate_index
                            = hvox::block_index({ CHUNK_LENGTH - 1, 0, z });
                        const Block* above_candidate_block
                            = &above_left_neighbour_blocks[above_candidate_index];

                        BlockIndex candidate_index = hvox::block_index(
                            { CHUNK_LENGTH - 1, CHUNK_LENGTH - 1, z }
                        );
                        const Block* candidate_block
                            = &left_neighbour_blocks[candidate_index];

                        if (!is_solid(candidate_block)
                            || is_solid(above_candidate_block))
                            continue;

                        ChunkNavmeshNode candidate_block_coord = {
                            {CHUNK_LENGTH - 1, CHUNK_LENGTH - 1, z},
                            left_neighbour->position
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor here, in_left_neighbour;
                        } candidate_block_vertex
                            = { impl::get_vertex(chunk_navmesh, candidate_block_coord),
                                impl::get_vertex(
                                    left_neighbour_navmesh, candidate_block_coord
                                ) };

                        boost::add_edge(
                            this_block_vertex.here,
                            candidate_block_vertex.here,
                            chunk_navmesh.graph
                        );
                        boost::add_edge(
                            candidate_block_vertex.here,
                            this_block_vertex.here,
                            chunk_navmesh.graph
                        );

                        boost::add_edge(
                            this_block_vertex.in_left_neighbour,
                            candidate_block_vertex.in_left_neighbour,
                            left_neighbour_navmesh.graph
                        );
                        boost::add_edge(
                            candidate_block_vertex.in_left_neighbour,
                            this_block_vertex.in_left_neighbour,
                            left_neighbour_navmesh.graph
                        );
                    }

                    // Step across and down from y == CHUNK_LENGTH - 1
                    for (BlockChunkPositionCoord z = 0; z < CHUNK_LENGTH; ++z) {
                        BlockIndex this_block_index
                            = hvox::block_index({ 0, CHUNK_LENGTH - 1, z });
                        const Block* this_block = &chunk_blocks[this_block_index];

                        BlockIndex above_this_block_index
                            = hvox::block_index({ 0, 0, z });
                        const Block* above_this_block
                            = &neighbour_blocks[above_this_block_index];

                        // Necessary condition for step across and down.
                        if (!is_solid(this_block) || is_solid(above_this_block))
                            continue;

                        ChunkNavmeshNode this_block_coord = {
                            {0, CHUNK_LENGTH - 1, z},
                            chunk_pos
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor here, in_left_neighbour;
                        } this_block_vertex = {
                            impl::get_vertex(chunk_navmesh, this_block_coord),
                            impl::get_vertex(left_neighbour_navmesh, this_block_coord)
                        };

                        BlockIndex step_down_candidate_index = hvox::block_index(
                            { CHUNK_LENGTH - 1, CHUNK_LENGTH - 2, z }
                        );
                        const Block* step_down_candidate_block
                            = &left_neighbour_blocks[step_down_candidate_index];

                        BlockIndex step_across_candidate_index = hvox::block_index(
                            { CHUNK_LENGTH - 1, CHUNK_LENGTH - 1, z }
                        );
                        const Block* step_across_candidate_block
                            = &left_neighbour_blocks[step_across_candidate_index];

                        BlockIndex above_candidates_index
                            = hvox::block_index({ CHUNK_LENGTH - 1, 0, z });
                        const Block* above_candidates_block
                            = &above_left_neighbour_blocks[above_candidates_index];

                        // Step across
                        if (is_solid(step_across_candidate_block)
                            && !is_solid(above_candidates_block))
                        {
                            ChunkNavmeshNode candidate_block_coord = {
                                {CHUNK_LENGTH - 1, CHUNK_LENGTH - 1, z},
                                left_neighbour->position
                            };

                            struct {
                                ChunkNavmeshVertexDescriptor here, in_left_neighbour;
                            } candidate_block_vertex = {
                                impl::get_vertex(chunk_navmesh, candidate_block_coord),
                                impl::get_vertex(
                                    left_neighbour_navmesh, candidate_block_coord
                                )
                            };

                            boost::add_edge(
                                this_block_vertex.here,
                                candidate_block_vertex.here,
                                chunk_navmesh.graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.here,
                                this_block_vertex.here,
                                chunk_navmesh.graph
                            );

                            boost::add_edge(
                                this_block_vertex.in_left_neighbour,
                                candidate_block_vertex.in_left_neighbour,
                                left_neighbour_navmesh.graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.in_left_neighbour,
                                this_block_vertex.in_left_neighbour,
                                left_neighbour_navmesh.graph
                            );
                        }
                        // Step down
                        else if (is_solid(step_down_candidate_block) && !is_solid(step_across_candidate_block) && !is_solid(above_candidates_block))
                        {
                            ChunkNavmeshNode candidate_block_coord = {
                                {CHUNK_LENGTH - 1, CHUNK_LENGTH - 2, z},
                                left_neighbour->position
                            };

                            struct {
                                ChunkNavmeshVertexDescriptor here, in_left_neighbour;
                            } candidate_block_vertex = {
                                impl::get_vertex(chunk_navmesh, candidate_block_coord),
                                impl::get_vertex(
                                    left_neighbour_navmesh, candidate_block_coord
                                )
                            };

                            boost::add_edge(
                                this_block_vertex.here,
                                candidate_block_vertex.here,
                                chunk_navmesh.graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.here,
                                this_block_vertex.here,
                                chunk_navmesh.graph
                            );

                            boost::add_edge(
                                this_block_vertex.in_left_neighbour,
                                candidate_block_vertex.in_left_neighbour,
                                left_neighbour_navmesh.graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.in_left_neighbour,
                                this_block_vertex.in_left_neighbour,
                                left_neighbour_navmesh.graph
                            );
                        }
                    }

                    chunk->navmesh_stitch.above_and_across_left.store(
                        ChunkState::COMPLETE
                    );
                }
            }

            // Right
            {
                auto right_neighbour       = chunk->neighbours.one.right.lock();
                auto above_right_neighbour = neighbour->neighbours.one.right.lock();
                ChunkState diagonal_stitch_state = ChunkState::NONE;
                if (right_neighbour != nullptr && above_right_neighbour != nullptr
                    && right_neighbour->bulk_navmeshing.load() == ChunkState::COMPLETE
                    && above_right_neighbour->bulk_navmeshing.load()
                           == ChunkState::COMPLETE
                    && chunk->navmesh_stitch.above_and_across_right
                           .compare_exchange_strong(
                               diagonal_stitch_state, ChunkState::ACTIVE
                           ))
                {
                    std::shared_lock<std::shared_mutex> right_neighbour_block_lock;
                    auto                                right_neighbour_blocks
                        = right_neighbour->blocks.get(right_neighbour_block_lock);
                    std::unique_lock<std::shared_mutex> right_neighbour_navmesh_lock;
                    auto                                right_neighbour_navmesh
                        = right_neighbour->navmesh.get(right_neighbour_navmesh_lock);

                    std::shared_lock<std::shared_mutex>
                         above_right_neighbour_block_lock;
                    auto above_right_neighbour_blocks
                        = above_right_neighbour->blocks.get(
                            above_right_neighbour_block_lock
                        );
                    std::unique_lock<std::shared_mutex>
                         above_right_neighbour_navmesh_lock;
                    auto above_right_neighbour_navmesh
                        = above_right_neighbour->navmesh.get(
                            above_right_neighbour_navmesh_lock
                        );

                    // Step up from y == CHUNK_LENGTH - 2
                    for (BlockChunkPositionCoord z = 0; z < CHUNK_LENGTH; ++z) {
                        BlockIndex this_block_index = hvox::block_index(
                            { CHUNK_LENGTH - 1, CHUNK_LENGTH - 2, z }
                        );
                        const Block* this_block = &chunk_blocks[this_block_index];

                        BlockIndex above_this_block_index = hvox::block_index(
                            { CHUNK_LENGTH - 1, CHUNK_LENGTH - 1, z }
                        );
                        const Block* above_this_block
                            = &chunk_blocks[above_this_block_index];

                        BlockIndex twice_above_this_block_index
                            = hvox::block_index({ CHUNK_LENGTH - 1, 0, z });
                        const Block* twice_above_this_block
                            = &neighbour_blocks[twice_above_this_block_index];

                        if (!is_solid(this_block) || is_solid(above_this_block)
                            || is_solid(twice_above_this_block))
                            continue;

                        ChunkNavmeshNode this_block_coord = {
                            {CHUNK_LENGTH - 1, CHUNK_LENGTH - 2, z},
                            chunk_pos
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor here, in_right_neighbour;
                        } this_block_vertex = {
                            impl::get_vertex(chunk_navmesh, this_block_coord),
                            impl::get_vertex(right_neighbour_navmesh, this_block_coord)
                        };

                        BlockIndex above_candidate_index
                            = hvox::block_index({ 0, 0, z });
                        const Block* above_candidate_block
                            = &above_right_neighbour_blocks[above_candidate_index];

                        BlockIndex candidate_index
                            = hvox::block_index({ 0, CHUNK_LENGTH - 1, z });
                        const Block* candidate_block
                            = &right_neighbour_blocks[candidate_index];

                        if (!is_solid(candidate_block)
                            || is_solid(above_candidate_block))
                            continue;

                        ChunkNavmeshNode candidate_block_coord = {
                            {0, CHUNK_LENGTH - 1, z},
                            right_neighbour->position
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor here, in_right_neighbour;
                        } candidate_block_vertex
                            = { impl::get_vertex(chunk_navmesh, candidate_block_coord),
                                impl::get_vertex(
                                    right_neighbour_navmesh, candidate_block_coord
                                ) };

                        boost::add_edge(
                            this_block_vertex.here,
                            candidate_block_vertex.here,
                            chunk_navmesh.graph
                        );
                        boost::add_edge(
                            candidate_block_vertex.here,
                            this_block_vertex.here,
                            chunk_navmesh.graph
                        );

                        boost::add_edge(
                            this_block_vertex.in_right_neighbour,
                            candidate_block_vertex.in_right_neighbour,
                            right_neighbour_navmesh.graph
                        );
                        boost::add_edge(
                            candidate_block_vertex.in_right_neighbour,
                            this_block_vertex.in_right_neighbour,
                            right_neighbour_navmesh.graph
                        );
                    }

                    // Step across and down from y == CHUNK_LENGTH - 1
                    for (BlockChunkPositionCoord z = 0; z < CHUNK_LENGTH; ++z) {
                        BlockIndex this_block_index = hvox::block_index(
                            { CHUNK_LENGTH - 1, CHUNK_LENGTH - 1, z }
                        );
                        const Block* this_block = &chunk_blocks[this_block_index];

                        BlockIndex above_this_block_index
                            = hvox::block_index({ CHUNK_LENGTH - 1, 0, z });
                        const Block* above_this_block
                            = &neighbour_blocks[above_this_block_index];

                        // Necessary condition for step across and down.
                        if (!is_solid(this_block) || is_solid(above_this_block))
                            continue;

                        ChunkNavmeshNode this_block_coord = {
                            {CHUNK_LENGTH - 1, CHUNK_LENGTH - 1, z},
                            chunk_pos
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor here, in_right_neighbour;
                        } this_block_vertex = {
                            impl::get_vertex(chunk_navmesh, this_block_coord),
                            impl::get_vertex(right_neighbour_navmesh, this_block_coord)
                        };

                        BlockIndex step_down_candidate_index
                            = hvox::block_index({ 0, CHUNK_LENGTH - 2, z });
                        const Block* step_down_candidate_block
                            = &right_neighbour_blocks[step_down_candidate_index];

                        BlockIndex step_across_candidate_index
                            = hvox::block_index({ 0, CHUNK_LENGTH - 1, z });
                        const Block* step_across_candidate_block
                            = &right_neighbour_blocks[step_across_candidate_index];

                        BlockIndex above_candidates_index
                            = hvox::block_index({ 0, 0, z });
                        const Block* above_candidates_block
                            = &above_right_neighbour_blocks[above_candidates_index];

                        // Step across
                        if (is_solid(step_across_candidate_block)
                            && !is_solid(above_candidates_block))
                        {
                            ChunkNavmeshNode candidate_block_coord = {
                                {0, CHUNK_LENGTH - 1, z},
                                right_neighbour->position
                            };

                            struct {
                                ChunkNavmeshVertexDescriptor here, in_right_neighbour;
                            } candidate_block_vertex = {
                                impl::get_vertex(chunk_navmesh, candidate_block_coord),
                                impl::get_vertex(
                                    right_neighbour_navmesh, candidate_block_coord
                                )
                            };

                            boost::add_edge(
                                this_block_vertex.here,
                                candidate_block_vertex.here,
                                chunk_navmesh.graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.here,
                                this_block_vertex.here,
                                chunk_navmesh.graph
                            );

                            boost::add_edge(
                                this_block_vertex.in_right_neighbour,
                                candidate_block_vertex.in_right_neighbour,
                                right_neighbour_navmesh.graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.in_right_neighbour,
                                this_block_vertex.in_right_neighbour,
                                right_neighbour_navmesh.graph
                            );
                        }
                        // Step down
                        else if (is_solid(step_down_candidate_block) && !is_solid(step_across_candidate_block) && !is_solid(above_candidates_block))
                        {
                            ChunkNavmeshNode candidate_block_coord = {
                                {0, CHUNK_LENGTH - 2, z},
                                right_neighbour->position
                            };

                            struct {
                                ChunkNavmeshVertexDescriptor here, in_right_neighbour;
                            } candidate_block_vertex = {
                                impl::get_vertex(chunk_navmesh, candidate_block_coord),
                                impl::get_vertex(
                                    right_neighbour_navmesh, candidate_block_coord
                                )
                            };

                            boost::add_edge(
                                this_block_vertex.here,
                                candidate_block_vertex.here,
                                chunk_navmesh.graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.here,
                                this_block_vertex.here,
                                chunk_navmesh.graph
                            );

                            boost::add_edge(
                                this_block_vertex.in_right_neighbour,
                                candidate_block_vertex.in_right_neighbour,
                                right_neighbour_navmesh.graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.in_right_neighbour,
                                this_block_vertex.in_right_neighbour,
                                right_neighbour_navmesh.graph
                            );
                        }
                    }

                    chunk->navmesh_stitch.above_and_across_right.store(
                        ChunkState::COMPLETE
                    );
                }
            }

            // Front
            {
                auto front_neighbour       = chunk->neighbours.one.front.lock();
                auto above_front_neighbour = neighbour->neighbours.one.front.lock();
                ChunkState diagonal_stitch_state = ChunkState::NONE;
                if (front_neighbour != nullptr && above_front_neighbour != nullptr
                    && front_neighbour->bulk_navmeshing.load() == ChunkState::COMPLETE
                    && above_front_neighbour->bulk_navmeshing.load()
                           == ChunkState::COMPLETE
                    && chunk->navmesh_stitch.above_and_across_front
                           .compare_exchange_strong(
                               diagonal_stitch_state, ChunkState::ACTIVE
                           ))
                {
                    std::shared_lock<std::shared_mutex> front_neighbour_block_lock;
                    auto                                front_neighbour_blocks
                        = front_neighbour->blocks.get(front_neighbour_block_lock);
                    std::unique_lock<std::shared_mutex> front_neighbour_navmesh_lock;
                    auto                                front_neighbour_navmesh
                        = front_neighbour->navmesh.get(front_neighbour_navmesh_lock);

                    std::shared_lock<std::shared_mutex>
                         above_front_neighbour_block_lock;
                    auto above_front_neighbour_blocks
                        = above_front_neighbour->blocks.get(
                            above_front_neighbour_block_lock
                        );
                    std::unique_lock<std::shared_mutex>
                         above_front_neighbour_navmesh_lock;
                    auto above_front_neighbour_navmesh
                        = above_front_neighbour->navmesh.get(
                            above_front_neighbour_navmesh_lock
                        );

                    // Step up from y == CHUNK_LENGTH - 2
                    for (BlockChunkPositionCoord x = 0; x < CHUNK_LENGTH; ++x) {
                        BlockIndex this_block_index = hvox::block_index(
                            { x, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1 }
                        );
                        const Block* this_block = &chunk_blocks[this_block_index];

                        BlockIndex above_this_block_index = hvox::block_index(
                            { x, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1 }
                        );
                        const Block* above_this_block
                            = &chunk_blocks[above_this_block_index];

                        BlockIndex twice_above_this_block_index
                            = hvox::block_index({ x, 0, CHUNK_LENGTH - 1 });
                        const Block* twice_above_this_block
                            = &neighbour_blocks[twice_above_this_block_index];

                        if (!is_solid(this_block) || is_solid(above_this_block)
                            || is_solid(twice_above_this_block))
                            continue;

                        ChunkNavmeshNode this_block_coord = {
                            {x, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1},
                            chunk_pos
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor here, in_front_neighbour;
                        } this_block_vertex = {
                            impl::get_vertex(chunk_navmesh, this_block_coord),
                            impl::get_vertex(front_neighbour_navmesh, this_block_coord)
                        };

                        BlockIndex above_candidate_index
                            = hvox::block_index({ x, 0, 0 });
                        const Block* above_candidate_block
                            = &above_front_neighbour_blocks[above_candidate_index];

                        BlockIndex candidate_index
                            = hvox::block_index({ x, CHUNK_LENGTH - 1, 0 });
                        const Block* candidate_block
                            = &front_neighbour_blocks[candidate_index];

                        if (!is_solid(candidate_block)
                            || is_solid(above_candidate_block))
                            continue;

                        ChunkNavmeshNode candidate_block_coord = {
                            {x, CHUNK_LENGTH - 1, 0},
                            front_neighbour->position
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor here, in_front_neighbour;
                        } candidate_block_vertex
                            = { impl::get_vertex(chunk_navmesh, candidate_block_coord),
                                impl::get_vertex(
                                    front_neighbour_navmesh, candidate_block_coord
                                ) };

                        boost::add_edge(
                            this_block_vertex.here,
                            candidate_block_vertex.here,
                            chunk_navmesh.graph
                        );
                        boost::add_edge(
                            candidate_block_vertex.here,
                            this_block_vertex.here,
                            chunk_navmesh.graph
                        );

                        boost::add_edge(
                            this_block_vertex.in_front_neighbour,
                            candidate_block_vertex.in_front_neighbour,
                            front_neighbour_navmesh.graph
                        );
                        boost::add_edge(
                            candidate_block_vertex.in_front_neighbour,
                            this_block_vertex.in_front_neighbour,
                            front_neighbour_navmesh.graph
                        );
                    }

                    // Step across and down from y == CHUNK_LENGTH - 1
                    for (BlockChunkPositionCoord x = 0; x < CHUNK_LENGTH; ++x) {
                        BlockIndex this_block_index = hvox::block_index(
                            { x, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1 }
                        );
                        const Block* this_block = &chunk_blocks[this_block_index];

                        BlockIndex above_this_block_index
                            = hvox::block_index({ x, 0, CHUNK_LENGTH - 1 });
                        const Block* above_this_block
                            = &neighbour_blocks[above_this_block_index];

                        // Necessary condition for step across and down.
                        if (!is_solid(this_block) || is_solid(above_this_block))
                            continue;

                        ChunkNavmeshNode this_block_coord = {
                            {x, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1},
                            chunk_pos
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor here, in_front_neighbour;
                        } this_block_vertex = {
                            impl::get_vertex(chunk_navmesh, this_block_coord),
                            impl::get_vertex(front_neighbour_navmesh, this_block_coord)
                        };

                        BlockIndex step_down_candidate_index
                            = hvox::block_index({ x, CHUNK_LENGTH - 2, 0 });
                        const Block* step_down_candidate_block
                            = &front_neighbour_blocks[step_down_candidate_index];

                        BlockIndex step_across_candidate_index
                            = hvox::block_index({ x, CHUNK_LENGTH - 1, 0 });
                        const Block* step_across_candidate_block
                            = &front_neighbour_blocks[step_across_candidate_index];

                        BlockIndex above_candidates_index
                            = hvox::block_index({ x, 0, 0 });
                        const Block* above_candidates_block
                            = &above_front_neighbour_blocks[above_candidates_index];

                        // Step across
                        if (is_solid(step_across_candidate_block)
                            && !is_solid(above_candidates_block))
                        {
                            ChunkNavmeshNode candidate_block_coord = {
                                {x, CHUNK_LENGTH - 1, 0},
                                front_neighbour->position
                            };

                            struct {
                                ChunkNavmeshVertexDescriptor here, in_front_neighbour;
                            } candidate_block_vertex = {
                                impl::get_vertex(chunk_navmesh, candidate_block_coord),
                                impl::get_vertex(
                                    front_neighbour_navmesh, candidate_block_coord
                                )
                            };

                            boost::add_edge(
                                this_block_vertex.here,
                                candidate_block_vertex.here,
                                chunk_navmesh.graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.here,
                                this_block_vertex.here,
                                chunk_navmesh.graph
                            );

                            boost::add_edge(
                                this_block_vertex.in_front_neighbour,
                                candidate_block_vertex.in_front_neighbour,
                                front_neighbour_navmesh.graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.in_front_neighbour,
                                this_block_vertex.in_front_neighbour,
                                front_neighbour_navmesh.graph
                            );
                        }
                        // Step down
                        else if (is_solid(step_down_candidate_block) && !is_solid(step_across_candidate_block) && !is_solid(above_candidates_block))
                        {
                            ChunkNavmeshNode candidate_block_coord = {
                                {x, CHUNK_LENGTH - 2, 0},
                                front_neighbour->position
                            };

                            struct {
                                ChunkNavmeshVertexDescriptor here, in_front_neighbour;
                            } candidate_block_vertex = {
                                impl::get_vertex(chunk_navmesh, candidate_block_coord),
                                impl::get_vertex(
                                    front_neighbour_navmesh, candidate_block_coord
                                )
                            };

                            boost::add_edge(
                                this_block_vertex.here,
                                candidate_block_vertex.here,
                                chunk_navmesh.graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.here,
                                this_block_vertex.here,
                                chunk_navmesh.graph
                            );

                            boost::add_edge(
                                this_block_vertex.in_front_neighbour,
                                candidate_block_vertex.in_front_neighbour,
                                front_neighbour_navmesh.graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.in_front_neighbour,
                                this_block_vertex.in_front_neighbour,
                                front_neighbour_navmesh.graph
                            );
                        }
                    }

                    chunk->navmesh_stitch.above_and_across_front.store(
                        ChunkState::COMPLETE
                    );
                }
            }

            // Back
            {
                auto       back_neighbour       = chunk->neighbours.one.back.lock();
                auto       above_back_neighbour = neighbour->neighbours.one.back.lock();
                ChunkState diagonal_stitch_state = ChunkState::NONE;
                if (back_neighbour != nullptr && above_back_neighbour != nullptr
                    && back_neighbour->bulk_navmeshing.load() == ChunkState::COMPLETE
                    && above_back_neighbour->bulk_navmeshing.load()
                           == ChunkState::COMPLETE
                    && chunk->navmesh_stitch.above_and_across_back
                           .compare_exchange_strong(
                               diagonal_stitch_state, ChunkState::ACTIVE
                           ))
                {
                    std::shared_lock<std::shared_mutex> back_neighbour_block_lock;
                    auto                                back_neighbour_blocks
                        = back_neighbour->blocks.get(back_neighbour_block_lock);
                    std::unique_lock<std::shared_mutex> back_neighbour_navmesh_lock;
                    auto                                back_neighbour_navmesh
                        = back_neighbour->navmesh.get(back_neighbour_navmesh_lock);

                    std::shared_lock<std::shared_mutex> above_back_neighbour_block_lock;
                    auto above_back_neighbour_blocks = above_back_neighbour->blocks.get(
                        above_back_neighbour_block_lock
                    );
                    std::unique_lock<std::shared_mutex>
                         above_back_neighbour_navmesh_lock;
                    auto above_back_neighbour_navmesh
                        = above_back_neighbour->navmesh.get(
                            above_back_neighbour_navmesh_lock
                        );

                    // Step up from y == CHUNK_LENGTH - 2
                    for (BlockChunkPositionCoord x = 0; x < CHUNK_LENGTH; ++x) {
                        BlockIndex this_block_index
                            = hvox::block_index({ x, CHUNK_LENGTH - 2, 0 });
                        const Block* this_block = &chunk_blocks[this_block_index];

                        BlockIndex above_this_block_index
                            = hvox::block_index({ x, CHUNK_LENGTH - 1, 0 });
                        const Block* above_this_block
                            = &chunk_blocks[above_this_block_index];

                        BlockIndex twice_above_this_block_index
                            = hvox::block_index({ CHUNK_LENGTH - 1, 0, 0 });
                        const Block* twice_above_this_block
                            = &neighbour_blocks[twice_above_this_block_index];

                        if (!is_solid(this_block) || is_solid(above_this_block)
                            || is_solid(twice_above_this_block))
                            continue;

                        ChunkNavmeshNode this_block_coord = {
                            {x, CHUNK_LENGTH - 2, 0},
                            chunk_pos
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor here, in_back_neighbour;
                        } this_block_vertex = {
                            impl::get_vertex(chunk_navmesh, this_block_coord),
                            impl::get_vertex(back_neighbour_navmesh, this_block_coord)
                        };

                        BlockIndex above_candidate_index
                            = hvox::block_index({ x, 0, CHUNK_LENGTH - 1 });
                        const Block* above_candidate_block
                            = &above_back_neighbour_blocks[above_candidate_index];

                        BlockIndex candidate_index = hvox::block_index(
                            { x, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1 }
                        );
                        const Block* candidate_block
                            = &back_neighbour_blocks[candidate_index];

                        if (!is_solid(candidate_block)
                            || is_solid(above_candidate_block))
                            continue;

                        ChunkNavmeshNode candidate_block_coord = {
                            {x, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1},
                            back_neighbour->position
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor here, in_back_neighbour;
                        } candidate_block_vertex
                            = { impl::get_vertex(chunk_navmesh, candidate_block_coord),
                                impl::get_vertex(
                                    back_neighbour_navmesh, candidate_block_coord
                                ) };

                        boost::add_edge(
                            this_block_vertex.here,
                            candidate_block_vertex.here,
                            chunk_navmesh.graph
                        );
                        boost::add_edge(
                            candidate_block_vertex.here,
                            this_block_vertex.here,
                            chunk_navmesh.graph
                        );

                        boost::add_edge(
                            this_block_vertex.in_back_neighbour,
                            candidate_block_vertex.in_back_neighbour,
                            back_neighbour_navmesh.graph
                        );
                        boost::add_edge(
                            candidate_block_vertex.in_back_neighbour,
                            this_block_vertex.in_back_neighbour,
                            back_neighbour_navmesh.graph
                        );
                    }

                    // Step across and down from y == CHUNK_LENGTH - 1
                    for (BlockChunkPositionCoord x = 0; x < CHUNK_LENGTH; ++x) {
                        BlockIndex this_block_index
                            = hvox::block_index({ x, CHUNK_LENGTH - 1, 0 });
                        const Block* this_block = &chunk_blocks[this_block_index];

                        BlockIndex above_this_block_index
                            = hvox::block_index({ x, 0, 0 });
                        const Block* above_this_block
                            = &neighbour_blocks[above_this_block_index];

                        // Necessary condition for step across and down.
                        if (!is_solid(this_block) || is_solid(above_this_block))
                            continue;

                        ChunkNavmeshNode this_block_coord = {
                            {x, CHUNK_LENGTH - 1, 0},
                            chunk_pos
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor here, in_back_neighbour;
                        } this_block_vertex = {
                            impl::get_vertex(chunk_navmesh, this_block_coord),
                            impl::get_vertex(back_neighbour_navmesh, this_block_coord)
                        };

                        BlockIndex step_down_candidate_index = hvox::block_index(
                            { x, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1 }
                        );
                        const Block* step_down_candidate_block
                            = &back_neighbour_blocks[step_down_candidate_index];

                        BlockIndex step_across_candidate_index = hvox::block_index(
                            { x, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1 }
                        );
                        const Block* step_across_candidate_block
                            = &back_neighbour_blocks[step_across_candidate_index];

                        BlockIndex above_candidates_index
                            = hvox::block_index({ x, 0, CHUNK_LENGTH - 1 });
                        const Block* above_candidates_block
                            = &above_back_neighbour_blocks[above_candidates_index];

                        // Step across
                        if (is_solid(step_across_candidate_block)
                            && !is_solid(above_candidates_block))
                        {
                            ChunkNavmeshNode candidate_block_coord = {
                                {x, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1},
                                back_neighbour->position
                            };

                            struct {
                                ChunkNavmeshVertexDescriptor here, in_back_neighbour;
                            } candidate_block_vertex = {
                                impl::get_vertex(chunk_navmesh, candidate_block_coord),
                                impl::get_vertex(
                                    back_neighbour_navmesh, candidate_block_coord
                                )
                            };

                            boost::add_edge(
                                this_block_vertex.here,
                                candidate_block_vertex.here,
                                chunk_navmesh.graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.here,
                                this_block_vertex.here,
                                chunk_navmesh.graph
                            );

                            boost::add_edge(
                                this_block_vertex.in_back_neighbour,
                                candidate_block_vertex.in_back_neighbour,
                                back_neighbour_navmesh.graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.in_back_neighbour,
                                this_block_vertex.in_back_neighbour,
                                back_neighbour_navmesh.graph
                            );
                        }
                        // Step down
                        else if (is_solid(step_down_candidate_block) && !is_solid(step_across_candidate_block) && !is_solid(above_candidates_block))
                        {
                            ChunkNavmeshNode candidate_block_coord = {
                                {x, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1},
                                back_neighbour->position
                            };

                            struct {
                                ChunkNavmeshVertexDescriptor here, in_back_neighbour;
                            } candidate_block_vertex = {
                                impl::get_vertex(chunk_navmesh, candidate_block_coord),
                                impl::get_vertex(
                                    back_neighbour_navmesh, candidate_block_coord
                                )
                            };

                            boost::add_edge(
                                this_block_vertex.here,
                                candidate_block_vertex.here,
                                chunk_navmesh.graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.here,
                                this_block_vertex.here,
                                chunk_navmesh.graph
                            );

                            boost::add_edge(
                                this_block_vertex.in_back_neighbour,
                                candidate_block_vertex.in_back_neighbour,
                                back_neighbour_navmesh.graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.in_back_neighbour,
                                this_block_vertex.in_back_neighbour,
                                back_neighbour_navmesh.graph
                            );
                        }
                    }

                    chunk->navmesh_stitch.above_and_across_left.store(
                        ChunkState::COMPLETE
                    );
                }
            }
        }
    }

    /***********************************\
     * Try to stitch bottom neighbour. *
    \***********************************/

    {
        auto       neighbour    = chunk->neighbours.one.bottom.lock();
        ChunkState stitch_state = ChunkState::NONE;
        if (neighbour != nullptr
            && neighbour->bulk_navmeshing.load() == ChunkState::COMPLETE
            && neighbour->navmesh_stitch.top.compare_exchange_strong(
                stitch_state, ChunkState::ACTIVE
            ))
        {
            std::shared_lock<std::shared_mutex> neighbour_block_lock;
            auto neighbour_blocks = neighbour->blocks.get(neighbour_block_lock);
            std::unique_lock<std::shared_mutex> neighbour_navmesh_lock;
            auto neighbour_navmesh = neighbour->navmesh.get(neighbour_navmesh_lock);

            for (BlockChunkPositionCoord x = 1; x < CHUNK_LENGTH - 1; ++x) {
                for (BlockChunkPositionCoord z = 1; z < CHUNK_LENGTH - 1; ++z) {
                    BlockIndex   this_block_index = hvox::block_index({ x, 0, z });
                    const Block* this_block       = &chunk_blocks[this_block_index];

                    BlockIndex   above_this_index = hvox::block_index({ x, 1, z });
                    const Block* above_this_block = &chunk_blocks[above_this_index];

                    BlockIndex neighbour_block_index
                        = hvox::block_index({ x, CHUNK_LENGTH - 1, z });
                    const Block* neighbour_block
                        = &neighbour_blocks[neighbour_block_index];

                    if (is_solid(neighbour_block) && !is_solid(this_block)) {
                        // Ensure node exists for this block.
                        ChunkNavmeshNode neighbour_block_coord = {
                            {x, CHUNK_LENGTH - 1, z},
                            neighbour->position
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor here, in_neighbour;
                        } neighbour_block_vertex = {
                            impl::get_vertex(chunk_navmesh, neighbour_block_coord),
                            impl::get_vertex(neighbour_navmesh, neighbour_block_coord)
                        };

                        // Up
                        if (!is_solid(above_this_block)) {
                            // Left
                            BlockIndex left_of_this_block_index
                                = hvox::block_index({ x - 1, 0, z });
                            const Block* left_of_this_block
                                = &chunk_blocks[left_of_this_block_index];

                            BlockIndex above_and_left_of_this_block_index
                                = hvox::block_index({ x - 1, 1, z });
                            const Block* above_and_left_of_this_block
                                = &chunk_blocks[above_and_left_of_this_block_index];

                            if (is_solid(left_of_this_block)
                                && !is_solid(above_and_left_of_this_block))
                            {
                                ChunkNavmeshNode left_of_this_block_coord = {
                                    {x - 1, 0, z},
                                    chunk_pos
                                };

                                // Ensure node exists for this block.
                                struct {
                                    ChunkNavmeshVertexDescriptor here, in_neighbour;
                                } left_of_this_block_vertex
                                    = { impl::get_vertex(
                                            chunk_navmesh, left_of_this_block_coord
                                        ),
                                        impl::get_vertex(
                                            neighbour_navmesh, left_of_this_block_coord
                                        ) };

                                boost::add_edge(
                                    neighbour_block_vertex.here,
                                    left_of_this_block_vertex.here,
                                    chunk_navmesh.graph
                                );
                                boost::add_edge(
                                    left_of_this_block_vertex.here,
                                    neighbour_block_vertex.here,
                                    chunk_navmesh.graph
                                );

                                boost::add_edge(
                                    neighbour_block_vertex.in_neighbour,
                                    left_of_this_block_vertex.in_neighbour,
                                    neighbour_navmesh.graph
                                );
                                boost::add_edge(
                                    left_of_this_block_vertex.in_neighbour,
                                    neighbour_block_vertex.in_neighbour,
                                    neighbour_navmesh.graph
                                );
                            }

                            // Right
                            BlockIndex right_of_this_block_index
                                = hvox::block_index({ x + 1, 0, z });
                            const Block* right_of_this_block
                                = &chunk_blocks[right_of_this_block_index];

                            BlockIndex above_and_right_of_this_block_index
                                = hvox::block_index({ x + 1, 1, z });
                            const Block* above_and_right_of_this_block
                                = &chunk_blocks[above_and_right_of_this_block_index];

                            if (is_solid(right_of_this_block)
                                && !is_solid(above_and_right_of_this_block))
                            {
                                ChunkNavmeshNode right_of_this_block_coord = {
                                    {x + 1, 0, z},
                                    chunk_pos
                                };

                                // Ensure node exists for this block.
                                struct {
                                    ChunkNavmeshVertexDescriptor here, in_neighbour;
                                } right_of_this_block_vertex
                                    = { impl::get_vertex(
                                            chunk_navmesh, right_of_this_block_coord
                                        ),
                                        impl::get_vertex(
                                            neighbour_navmesh, right_of_this_block_coord
                                        ) };

                                boost::add_edge(
                                    neighbour_block_vertex.here,
                                    right_of_this_block_vertex.here,
                                    chunk_navmesh.graph
                                );
                                boost::add_edge(
                                    right_of_this_block_vertex.here,
                                    neighbour_block_vertex.here,
                                    chunk_navmesh.graph
                                );

                                boost::add_edge(
                                    neighbour_block_vertex.in_neighbour,
                                    right_of_this_block_vertex.in_neighbour,
                                    neighbour_navmesh.graph
                                );
                                boost::add_edge(
                                    right_of_this_block_vertex.in_neighbour,
                                    neighbour_block_vertex.in_neighbour,
                                    neighbour_navmesh.graph
                                );
                            }

                            // Front
                            BlockIndex front_of_this_block_index
                                = hvox::block_index({ x, 0, z + 1 });
                            const Block* front_of_this_block
                                = &chunk_blocks[front_of_this_block_index];

                            BlockIndex above_and_front_of_this_block_index
                                = hvox::block_index({ x, 1, z + 1 });
                            const Block* above_and_front_of_this_block
                                = &chunk_blocks[above_and_front_of_this_block_index];

                            if (is_solid(front_of_this_block)
                                && !is_solid(above_and_front_of_this_block))
                            {
                                ChunkNavmeshNode front_of_this_block_coord = {
                                    {x, 0, z + 1},
                                    chunk_pos
                                };

                                // Ensure node exists for this block.
                                struct {
                                    ChunkNavmeshVertexDescriptor here, in_neighbour;
                                } front_of_this_block_vertex
                                    = { impl::get_vertex(
                                            chunk_navmesh, front_of_this_block_coord
                                        ),
                                        impl::get_vertex(
                                            neighbour_navmesh, front_of_this_block_coord
                                        ) };

                                boost::add_edge(
                                    neighbour_block_vertex.here,
                                    front_of_this_block_vertex.here,
                                    chunk_navmesh.graph
                                );
                                boost::add_edge(
                                    front_of_this_block_vertex.here,
                                    neighbour_block_vertex.here,
                                    chunk_navmesh.graph
                                );

                                boost::add_edge(
                                    neighbour_block_vertex.in_neighbour,
                                    front_of_this_block_vertex.in_neighbour,
                                    neighbour_navmesh.graph
                                );
                                boost::add_edge(
                                    front_of_this_block_vertex.in_neighbour,
                                    neighbour_block_vertex.in_neighbour,
                                    neighbour_navmesh.graph
                                );
                            }

                            // Back
                            BlockIndex back_of_this_block_index
                                = hvox::block_index({ x, 0, z - 1 });
                            const Block* back_of_this_block
                                = &chunk_blocks[back_of_this_block_index];

                            BlockIndex above_and_back_of_this_block_index
                                = hvox::block_index({ x, 1, z - 1 });
                            const Block* above_and_back_of_this_block
                                = &chunk_blocks[above_and_back_of_this_block_index];

                            if (is_solid(back_of_this_block)
                                && !is_solid(above_and_back_of_this_block))
                            {
                                ChunkNavmeshNode back_of_this_block_coord = {
                                    {x, 0, z - 1},
                                    chunk_pos
                                };

                                // Ensure node exists for this block.
                                struct {
                                    ChunkNavmeshVertexDescriptor here, in_neighbour;
                                } back_of_this_block_vertex
                                    = { impl::get_vertex(
                                            chunk_navmesh, back_of_this_block_coord
                                        ),
                                        impl::get_vertex(
                                            neighbour_navmesh, back_of_this_block_coord
                                        ) };

                                boost::add_edge(
                                    neighbour_block_vertex.here,
                                    back_of_this_block_vertex.here,
                                    chunk_navmesh.graph
                                );
                                boost::add_edge(
                                    back_of_this_block_vertex.here,
                                    neighbour_block_vertex.here,
                                    chunk_navmesh.graph
                                );

                                boost::add_edge(
                                    neighbour_block_vertex.in_neighbour,
                                    back_of_this_block_vertex.in_neighbour,
                                    neighbour_navmesh.graph
                                );
                                boost::add_edge(
                                    back_of_this_block_vertex.in_neighbour,
                                    neighbour_block_vertex.in_neighbour,
                                    neighbour_navmesh.graph
                                );
                            }
                        }
                        // Across and Down
                        else
                        {
                            // Left
                            BlockIndex left_of_neighbour_block_index
                                = hvox::block_index({ x - 1, CHUNK_LENGTH - 1, z });
                            const Block* left_of_neighbour_block
                                = &neighbour_blocks[left_of_neighbour_block_index];

                            BlockIndex left_of_and_below_neighbour_block_index
                                = hvox::block_index({ x - 1, CHUNK_LENGTH - 2, z });
                            const Block* left_of_and_below_neighbour_block
                                = &neighbour_blocks
                                      [left_of_and_below_neighbour_block_index];

                            BlockIndex left_of_this_block_index
                                = hvox::block_index({ x - 1, 0, z });
                            const Block* left_of_this_block
                                = &chunk_blocks[left_of_this_block_index];

                            // Across
                            if (is_solid(left_of_neighbour_block)
                                && !is_solid(left_of_this_block))
                            {
                                ChunkNavmeshNode left_of_neighbour_block_coord = {
                                    {x - 1, CHUNK_LENGTH - 1, z},
                                    neighbour->position
                                };

                                // Ensure node exists for this block.
                                ChunkNavmeshVertexDescriptor
                                    left_of_neighbour_block_vertex
                                    = impl::get_vertex(
                                        neighbour_navmesh, left_of_neighbour_block_coord
                                    );

                                boost::add_edge(
                                    neighbour_block_vertex.here,
                                    left_of_neighbour_block_vertex,
                                    neighbour_navmesh.graph
                                );
                                boost::add_edge(
                                    left_of_neighbour_block_vertex,
                                    neighbour_block_vertex.here,
                                    neighbour_navmesh.graph
                                );
                                // Down
                            } else if (is_solid(left_of_and_below_neighbour_block) && !is_solid(left_of_neighbour_block) && !is_solid(left_of_this_block))
                            {
                                ChunkNavmeshNode left_of_and_below_neighbour_block_coord
                                    = {
                                          {x - 1, CHUNK_LENGTH - 2, z},
                                          neighbour->position
                                };

                                // Ensure node exists for this block.
                                ChunkNavmeshVertexDescriptor
                                    left_of_and_below_neighbour_block_vertex
                                    = impl::get_vertex(
                                        neighbour_navmesh,
                                        left_of_and_below_neighbour_block_coord
                                    );

                                boost::add_edge(
                                    neighbour_block_vertex.in_neighbour,
                                    left_of_and_below_neighbour_block_vertex,
                                    neighbour_navmesh.graph
                                );
                                boost::add_edge(
                                    left_of_and_below_neighbour_block_vertex,
                                    neighbour_block_vertex.in_neighbour,
                                    neighbour_navmesh.graph
                                );
                            }

                            // Right
                            BlockIndex right_of_neighbour_block_index
                                = hvox::block_index({ x + 1, CHUNK_LENGTH - 1, z });
                            const Block* right_of_neighbour_block
                                = &neighbour_blocks[right_of_neighbour_block_index];

                            BlockIndex right_of_and_below_neighbour_block_index
                                = hvox::block_index({ x + 1, CHUNK_LENGTH - 2, z });
                            const Block* right_of_and_below_neighbour_block
                                = &neighbour_blocks
                                      [right_of_and_below_neighbour_block_index];

                            BlockIndex right_of_this_block_index
                                = hvox::block_index({ x + 1, 0, z });
                            const Block* right_of_this_block
                                = &chunk_blocks[right_of_this_block_index];

                            // Across
                            if (is_solid(right_of_neighbour_block)
                                && !is_solid(right_of_this_block))
                            {
                                ChunkNavmeshNode right_of_neighbour_block_coord = {
                                    {x + 1, CHUNK_LENGTH - 1, z},
                                    neighbour->position
                                };

                                // Ensure node exists for this block.
                                ChunkNavmeshVertexDescriptor
                                    right_of_neighbour_block_vertex
                                    = impl::get_vertex(
                                        neighbour_navmesh,
                                        right_of_neighbour_block_coord
                                    );

                                boost::add_edge(
                                    neighbour_block_vertex.in_neighbour,
                                    right_of_neighbour_block_vertex,
                                    neighbour_navmesh.graph
                                );
                                boost::add_edge(
                                    right_of_neighbour_block_vertex,
                                    neighbour_block_vertex.in_neighbour,
                                    neighbour_navmesh.graph
                                );
                                // Down
                            } else if (is_solid(right_of_and_below_neighbour_block) && !is_solid(right_of_neighbour_block) && !is_solid(right_of_this_block))
                            {
                                ChunkNavmeshNode
                                    right_of_and_below_neighbour_block_coord
                                    = {
                                          {x + 1, CHUNK_LENGTH - 2, z},
                                          neighbour->position
                                };

                                // Ensure node exists for this block.
                                ChunkNavmeshVertexDescriptor
                                    right_of_and_below_neighbour_block_vertex
                                    = impl::get_vertex(
                                        neighbour_navmesh,
                                        right_of_and_below_neighbour_block_coord
                                    );

                                boost::add_edge(
                                    neighbour_block_vertex.in_neighbour,
                                    right_of_and_below_neighbour_block_vertex,
                                    neighbour_navmesh.graph
                                );
                                boost::add_edge(
                                    right_of_and_below_neighbour_block_vertex,
                                    neighbour_block_vertex.in_neighbour,
                                    neighbour_navmesh.graph
                                );
                            }

                            // Front
                            BlockIndex front_of_neighbour_block_index
                                = hvox::block_index({ x, CHUNK_LENGTH - 1, z + 1 });
                            const Block* front_of_neighbour_block
                                = &neighbour_blocks[front_of_neighbour_block_index];

                            BlockIndex front_of_and_below_neighbour_block_index
                                = hvox::block_index({ x, CHUNK_LENGTH - 2, z + 1 });
                            const Block* front_of_and_below_neighbour_block
                                = &neighbour_blocks
                                      [front_of_and_below_neighbour_block_index];

                            BlockIndex front_of_this_block_index
                                = hvox::block_index({ x, 0, z + 1 });
                            const Block* front_of_this_block
                                = &chunk_blocks[front_of_this_block_index];

                            // Across
                            if (is_solid(front_of_neighbour_block)
                                && !is_solid(front_of_this_block))
                            {
                                ChunkNavmeshNode front_of_neighbour_block_coord = {
                                    {x, CHUNK_LENGTH - 1, z + 1},
                                    neighbour->position
                                };

                                // Ensure node exists for this block.
                                ChunkNavmeshVertexDescriptor
                                    front_of_neighbour_block_vertex
                                    = impl::get_vertex(
                                        neighbour_navmesh,
                                        front_of_neighbour_block_coord
                                    );

                                boost::add_edge(
                                    neighbour_block_vertex.in_neighbour,
                                    front_of_neighbour_block_vertex,
                                    neighbour_navmesh.graph
                                );
                                boost::add_edge(
                                    front_of_neighbour_block_vertex,
                                    neighbour_block_vertex.in_neighbour,
                                    neighbour_navmesh.graph
                                );
                                // Down
                            } else if (is_solid(front_of_and_below_neighbour_block) && !is_solid(front_of_neighbour_block) && !is_solid(front_of_this_block))
                            {
                                ChunkNavmeshNode
                                    front_of_and_below_neighbour_block_coord
                                    = {
                                          {x, CHUNK_LENGTH - 2, z + 1},
                                          neighbour->position
                                };

                                // Ensure node exists for this block.
                                ChunkNavmeshVertexDescriptor
                                    front_of_and_below_neighbour_block_vertex
                                    = impl::get_vertex(
                                        neighbour_navmesh,
                                        front_of_and_below_neighbour_block_coord
                                    );

                                boost::add_edge(
                                    neighbour_block_vertex.in_neighbour,
                                    front_of_and_below_neighbour_block_vertex,
                                    neighbour_navmesh.graph
                                );
                                boost::add_edge(
                                    front_of_and_below_neighbour_block_vertex,
                                    neighbour_block_vertex.in_neighbour,
                                    neighbour_navmesh.graph
                                );
                            }

                            // Back
                            BlockIndex back_of_neighbour_block_index
                                = hvox::block_index({ x, CHUNK_LENGTH - 1, z - 1 });
                            const Block* back_of_neighbour_block
                                = &neighbour_blocks[back_of_neighbour_block_index];

                            BlockIndex back_of_and_below_neighbour_block_index
                                = hvox::block_index({ x, CHUNK_LENGTH - 2, z - 1 });
                            const Block* back_of_and_below_neighbour_block
                                = &neighbour_blocks
                                      [back_of_and_below_neighbour_block_index];

                            BlockIndex back_of_this_block_index
                                = hvox::block_index({ x, 0, z - 1 });
                            const Block* back_of_this_block
                                = &chunk_blocks[back_of_this_block_index];

                            // Across
                            if (is_solid(back_of_neighbour_block)
                                && !is_solid(back_of_this_block))
                            {
                                ChunkNavmeshNode back_of_neighbour_block_coord = {
                                    {x, CHUNK_LENGTH - 1, z - 1},
                                    neighbour->position
                                };

                                // Ensure node exists for this block.
                                ChunkNavmeshVertexDescriptor
                                    back_of_neighbour_block_vertex
                                    = impl::get_vertex(
                                        neighbour_navmesh, back_of_neighbour_block_coord
                                    );

                                boost::add_edge(
                                    neighbour_block_vertex.in_neighbour,
                                    back_of_neighbour_block_vertex,
                                    neighbour_navmesh.graph
                                );
                                boost::add_edge(
                                    back_of_neighbour_block_vertex,
                                    neighbour_block_vertex.in_neighbour,
                                    neighbour_navmesh.graph
                                );
                                // Down
                            } else if (is_solid(back_of_and_below_neighbour_block) && !is_solid(back_of_neighbour_block) && !is_solid(back_of_this_block))
                            {
                                ChunkNavmeshNode back_of_and_below_neighbour_block_coord
                                    = {
                                          {x, CHUNK_LENGTH - 2, z - 1},
                                          neighbour->position
                                };

                                // Ensure node exists for this block.
                                ChunkNavmeshVertexDescriptor
                                    back_of_and_below_neighbour_block_vertex
                                    = impl::get_vertex(
                                        neighbour_navmesh,
                                        back_of_and_below_neighbour_block_coord
                                    );

                                boost::add_edge(
                                    neighbour_block_vertex.in_neighbour,
                                    back_of_and_below_neighbour_block_vertex,
                                    neighbour_navmesh.graph
                                );
                                boost::add_edge(
                                    back_of_and_below_neighbour_block_vertex,
                                    neighbour_block_vertex.in_neighbour,
                                    neighbour_navmesh.graph
                                );
                            }
                        }
                    }
                }
            }

            // Check step up from second-to-top layer of chunk to each adjacent
            // neighbour of above chunk, and check step across and down from top layer
            // of chunk to each adjacent neighbour of this and above chunk.

            // Left
            {
                auto       left_neighbour       = chunk->neighbours.one.left.lock();
                auto       below_left_neighbour = neighbour->neighbours.one.left.lock();
                ChunkState neighbour_stitch_state = ChunkState::NONE;
                if (left_neighbour != nullptr && below_left_neighbour != nullptr
                    && left_neighbour->bulk_navmeshing.load() == ChunkState::COMPLETE
                    && below_left_neighbour->bulk_navmeshing.load()
                           == ChunkState::COMPLETE
                    && neighbour->navmesh_stitch.above_and_across_left
                           .compare_exchange_strong(
                               neighbour_stitch_state, ChunkState::ACTIVE
                           ))
                {
                    std::shared_lock<std::shared_mutex> left_neighbour_block_lock;
                    auto                                left_neighbour_blocks
                        = left_neighbour->blocks.get(left_neighbour_block_lock);
                    std::unique_lock<std::shared_mutex> left_neighbour_navmesh_lock;
                    auto                                left_neighbour_navmesh
                        = left_neighbour->navmesh.get(left_neighbour_navmesh_lock);

                    std::shared_lock<std::shared_mutex> below_left_neighbour_block_lock;
                    auto below_left_neighbour_blocks = below_left_neighbour->blocks.get(
                        below_left_neighbour_block_lock
                    );
                    std::unique_lock<std::shared_mutex>
                         below_left_neighbour_navmesh_lock;
                    auto below_left_neighbour_navmesh
                        = below_left_neighbour->navmesh.get(
                            below_left_neighbour_navmesh_lock
                        );

                    // Step up from y == CHUNK_LENGTH - 2
                    for (BlockChunkPositionCoord z = 0; z < CHUNK_LENGTH; ++z) {
                        BlockIndex neighbour_block_index
                            = hvox::block_index({ 0, CHUNK_LENGTH - 2, z });
                        const Block* neighbour_block
                            = &neighbour_blocks[neighbour_block_index];

                        BlockIndex above_neighbour_block_index
                            = hvox::block_index({ 0, CHUNK_LENGTH - 1, z });
                        const Block* above_neighbour_block
                            = &neighbour_blocks[above_neighbour_block_index];

                        BlockIndex twice_above_neighbour_block_index
                            = hvox::block_index({ 0, 0, z });
                        const Block* twice_above_neighbour_block
                            = &chunk_blocks[twice_above_neighbour_block_index];

                        if (!is_solid(neighbour_block)
                            || is_solid(above_neighbour_block)
                            || is_solid(twice_above_neighbour_block))
                            continue;

                        ChunkNavmeshNode neighbour_block_coord = {
                            {0, CHUNK_LENGTH - 2, z},
                            neighbour->position
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor in_neighbour,
                                in_below_left_neighbour;
                        } neighbour_block_vertex = {
                            impl::get_vertex(neighbour_navmesh, neighbour_block_coord),
                            impl::get_vertex(
                                below_left_neighbour_navmesh, neighbour_block_coord
                            )
                        };

                        BlockIndex above_candidate_index
                            = hvox::block_index({ CHUNK_LENGTH - 1, 0, z });
                        const Block* above_candidate_block
                            = &left_neighbour_blocks[above_candidate_index];

                        BlockIndex candidate_index = hvox::block_index(
                            { CHUNK_LENGTH - 1, CHUNK_LENGTH - 1, z }
                        );
                        const Block* candidate_block
                            = &below_left_neighbour_blocks[candidate_index];

                        if (!is_solid(candidate_block)
                            || is_solid(above_candidate_block))
                            continue;

                        ChunkNavmeshNode candidate_block_coord = {
                            {CHUNK_LENGTH - 1, CHUNK_LENGTH - 1, z},
                            below_left_neighbour->position
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor in_neighbour,
                                in_below_left_neighbour;
                        } candidate_block_vertex = {
                            impl::get_vertex(neighbour_navmesh, candidate_block_coord),
                            impl::get_vertex(
                                below_left_neighbour_navmesh, candidate_block_coord
                            )
                        };

                        boost::add_edge(
                            neighbour_block_vertex.in_neighbour,
                            candidate_block_vertex.in_neighbour,
                            neighbour_navmesh.graph
                        );
                        boost::add_edge(
                            candidate_block_vertex.in_neighbour,
                            neighbour_block_vertex.in_neighbour,
                            neighbour_navmesh.graph
                        );

                        boost::add_edge(
                            neighbour_block_vertex.in_below_left_neighbour,
                            candidate_block_vertex.in_below_left_neighbour,
                            below_left_neighbour_navmesh.graph
                        );
                        boost::add_edge(
                            candidate_block_vertex.in_below_left_neighbour,
                            neighbour_block_vertex.in_below_left_neighbour,
                            below_left_neighbour_navmesh.graph
                        );
                    }

                    // Step across and down from y == CHUNK_LENGTH - 1
                    for (BlockChunkPositionCoord z = 0; z < CHUNK_LENGTH; ++z) {
                        BlockIndex neighbour_block_index
                            = hvox::block_index({ 0, CHUNK_LENGTH - 1, z });
                        const Block* neighbour_block
                            = &neighbour_blocks[neighbour_block_index];

                        BlockIndex above_neighbour_block_index
                            = hvox::block_index({ 0, 0, z });
                        const Block* above_neighbour_block
                            = &chunk_blocks[above_neighbour_block_index];

                        // Necessary condition for step across and down.
                        if (!is_solid(neighbour_block)
                            || is_solid(above_neighbour_block))
                            continue;

                        ChunkNavmeshNode neighbour_block_coord = {
                            {0, CHUNK_LENGTH - 1, z},
                            neighbour->position
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor in_neighbour,
                                in_below_left_neighbour;
                        } neighbour_block_vertex = {
                            impl::get_vertex(neighbour_navmesh, neighbour_block_coord),
                            impl::get_vertex(
                                below_left_neighbour_navmesh, neighbour_block_coord
                            )
                        };

                        BlockIndex step_down_candidate_index = hvox::block_index(
                            { CHUNK_LENGTH - 1, CHUNK_LENGTH - 2, z }
                        );
                        const Block* step_down_candidate_block
                            = &below_left_neighbour_blocks[step_down_candidate_index];

                        BlockIndex step_across_candidate_index = hvox::block_index(
                            { CHUNK_LENGTH - 1, CHUNK_LENGTH - 1, z }
                        );
                        const Block* step_across_candidate_block
                            = &below_left_neighbour_blocks[step_across_candidate_index];

                        BlockIndex above_candidates_index
                            = hvox::block_index({ CHUNK_LENGTH - 1, 0, z });
                        const Block* above_candidates_block
                            = &left_neighbour_blocks[above_candidates_index];

                        // Step across
                        if (is_solid(step_across_candidate_block)
                            && !is_solid(above_candidates_block))
                        {
                            ChunkNavmeshNode candidate_block_coord = {
                                {CHUNK_LENGTH - 1, CHUNK_LENGTH - 1, z},
                                below_left_neighbour->position
                            };

                            struct {
                                ChunkNavmeshVertexDescriptor in_neighbour,
                                    in_below_left_neighbour;
                            } candidate_block_vertex
                                = { impl::get_vertex(
                                        neighbour_navmesh, candidate_block_coord
                                    ),
                                    impl::get_vertex(
                                        below_left_neighbour_navmesh,
                                        candidate_block_coord
                                    ) };

                            boost::add_edge(
                                neighbour_block_vertex.in_neighbour,
                                candidate_block_vertex.in_neighbour,
                                neighbour_navmesh.graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.in_neighbour,
                                neighbour_block_vertex.in_neighbour,
                                neighbour_navmesh.graph
                            );

                            boost::add_edge(
                                neighbour_block_vertex.in_below_left_neighbour,
                                candidate_block_vertex.in_below_left_neighbour,
                                below_left_neighbour_navmesh.graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.in_below_left_neighbour,
                                neighbour_block_vertex.in_below_left_neighbour,
                                below_left_neighbour_navmesh.graph
                            );
                        }
                        // Step down
                        else if (is_solid(step_down_candidate_block) && !is_solid(step_across_candidate_block) && !is_solid(above_candidates_block))
                        {
                            ChunkNavmeshNode candidate_block_coord = {
                                {CHUNK_LENGTH - 1, CHUNK_LENGTH - 2, z},
                                below_left_neighbour->position
                            };

                            struct {
                                ChunkNavmeshVertexDescriptor in_neighbour,
                                    in_below_left_neighbour;
                            } candidate_block_vertex
                                = { impl::get_vertex(
                                        neighbour_navmesh, candidate_block_coord
                                    ),
                                    impl::get_vertex(
                                        below_left_neighbour_navmesh,
                                        candidate_block_coord
                                    ) };

                            boost::add_edge(
                                neighbour_block_vertex.in_neighbour,
                                candidate_block_vertex.in_neighbour,
                                chunk_navmesh.graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.in_neighbour,
                                neighbour_block_vertex.in_neighbour,
                                chunk_navmesh.graph
                            );

                            boost::add_edge(
                                neighbour_block_vertex.in_below_left_neighbour,
                                candidate_block_vertex.in_below_left_neighbour,
                                left_neighbour_navmesh.graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.in_below_left_neighbour,
                                neighbour_block_vertex.in_below_left_neighbour,
                                left_neighbour_navmesh.graph
                            );
                        }
                    }

                    neighbour->navmesh_stitch.above_and_across_left.store(
                        ChunkState::COMPLETE
                    );
                }
            }

            // Right
            {
                auto right_neighbour       = chunk->neighbours.one.right.lock();
                auto below_right_neighbour = neighbour->neighbours.one.right.lock();
                ChunkState neighbour_stitch_state = ChunkState::NONE;
                if (right_neighbour != nullptr && below_right_neighbour != nullptr
                    && right_neighbour->bulk_navmeshing.load() == ChunkState::COMPLETE
                    && below_right_neighbour->bulk_navmeshing.load()
                           == ChunkState::COMPLETE
                    && neighbour->navmesh_stitch.above_and_across_right
                           .compare_exchange_strong(
                               neighbour_stitch_state, ChunkState::ACTIVE
                           ))
                {
                    std::shared_lock<std::shared_mutex> right_neighbour_block_lock;
                    auto                                right_neighbour_blocks
                        = right_neighbour->blocks.get(right_neighbour_block_lock);
                    std::unique_lock<std::shared_mutex> right_neighbour_navmesh_lock;
                    auto                                right_neighbour_navmesh
                        = right_neighbour->navmesh.get(right_neighbour_navmesh_lock);

                    std::shared_lock<std::shared_mutex>
                         below_right_neighbour_block_lock;
                    auto below_right_neighbour_blocks
                        = below_right_neighbour->blocks.get(
                            below_right_neighbour_block_lock
                        );
                    std::unique_lock<std::shared_mutex>
                         below_right_neighbour_navmesh_lock;
                    auto below_right_neighbour_navmesh
                        = below_right_neighbour->navmesh.get(
                            below_right_neighbour_navmesh_lock
                        );

                    // Step up from y == CHUNK_LENGTH - 2
                    for (BlockChunkPositionCoord z = 0; z < CHUNK_LENGTH; ++z) {
                        BlockIndex neighbour_block_index = hvox::block_index(
                            { CHUNK_LENGTH - 1, CHUNK_LENGTH - 2, z }
                        );
                        const Block* neighbour_block
                            = &neighbour_blocks[neighbour_block_index];

                        BlockIndex above_neighbour_block_index = hvox::block_index(
                            { CHUNK_LENGTH - 1, CHUNK_LENGTH - 1, z }
                        );
                        const Block* above_neighbour_block
                            = &neighbour_blocks[above_neighbour_block_index];

                        BlockIndex twice_above_neighbour_block_index
                            = hvox::block_index({ CHUNK_LENGTH - 1, 0, z });
                        const Block* twice_above_neighbour_block
                            = &chunk_blocks[twice_above_neighbour_block_index];

                        if (!is_solid(neighbour_block)
                            || is_solid(above_neighbour_block)
                            || is_solid(twice_above_neighbour_block))
                            continue;

                        ChunkNavmeshNode neighbour_block_coord = {
                            {CHUNK_LENGTH - 1, CHUNK_LENGTH - 2, z},
                            neighbour->position
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor in_neighbour,
                                in_below_right_neighbour;
                        } neighbour_block_vertex = {
                            impl::get_vertex(neighbour_navmesh, neighbour_block_coord),
                            impl::get_vertex(
                                right_neighbour_navmesh, neighbour_block_coord
                            )
                        };

                        BlockIndex above_candidate_index
                            = hvox::block_index({ 0, 0, z });
                        const Block* above_candidate_block
                            = &right_neighbour_blocks[above_candidate_index];

                        BlockIndex candidate_index
                            = hvox::block_index({ 0, CHUNK_LENGTH - 1, z });
                        const Block* candidate_block
                            = &below_right_neighbour_blocks[candidate_index];

                        if (!is_solid(candidate_block)
                            || is_solid(above_candidate_block))
                            continue;

                        ChunkNavmeshNode candidate_block_coord = {
                            {0, CHUNK_LENGTH - 1, z},
                            below_right_neighbour->position
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor in_neighbour,
                                in_below_right_neighbour;
                        } candidate_block_vertex = {
                            impl::get_vertex(neighbour_navmesh, candidate_block_coord),
                            impl::get_vertex(
                                below_right_neighbour_navmesh, candidate_block_coord
                            )
                        };

                        boost::add_edge(
                            neighbour_block_vertex.in_neighbour,
                            candidate_block_vertex.in_neighbour,
                            neighbour_navmesh.graph
                        );
                        boost::add_edge(
                            candidate_block_vertex.in_neighbour,
                            neighbour_block_vertex.in_neighbour,
                            neighbour_navmesh.graph
                        );

                        boost::add_edge(
                            neighbour_block_vertex.in_below_right_neighbour,
                            candidate_block_vertex.in_below_right_neighbour,
                            below_right_neighbour_navmesh.graph
                        );
                        boost::add_edge(
                            candidate_block_vertex.in_below_right_neighbour,
                            neighbour_block_vertex.in_below_right_neighbour,
                            below_right_neighbour_navmesh.graph
                        );
                    }

                    // Step across and down from y == CHUNK_LENGTH - 1
                    for (BlockChunkPositionCoord z = 0; z < CHUNK_LENGTH; ++z) {
                        BlockIndex neighbour_block_index = hvox::block_index(
                            { CHUNK_LENGTH - 1, CHUNK_LENGTH - 1, z }
                        );
                        const Block* neighbour_block
                            = &neighbour_blocks[neighbour_block_index];

                        BlockIndex above_neighbour_block_index
                            = hvox::block_index({ CHUNK_LENGTH - 1, 0, z });
                        const Block* above_neighbour_block
                            = &chunk_blocks[above_neighbour_block_index];

                        // Necessary condition for step across and down.
                        if (!is_solid(neighbour_block)
                            || is_solid(above_neighbour_block))
                            continue;

                        ChunkNavmeshNode neighbour_block_coord = {
                            {CHUNK_LENGTH - 1, CHUNK_LENGTH - 1, z},
                            neighbour->position
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor in_neighbour,
                                in_below_right_neighbour;
                        } neighbour_block_vertex = {
                            impl::get_vertex(neighbour_navmesh, neighbour_block_coord),
                            impl::get_vertex(
                                below_right_neighbour_navmesh, neighbour_block_coord
                            )
                        };

                        BlockIndex step_down_candidate_index
                            = hvox::block_index({ 0, CHUNK_LENGTH - 2, z });
                        const Block* step_down_candidate_block
                            = &below_right_neighbour_blocks[step_down_candidate_index];

                        BlockIndex step_across_candidate_index
                            = hvox::block_index({ 0, CHUNK_LENGTH - 1, z });
                        const Block* step_across_candidate_block
                            = &below_right_neighbour_blocks
                                  [step_across_candidate_index];

                        BlockIndex above_candidates_index
                            = hvox::block_index({ 0, 0, z });
                        const Block* above_candidates_block
                            = &right_neighbour_blocks[above_candidates_index];

                        // Step across
                        if (is_solid(step_across_candidate_block)
                            && !is_solid(above_candidates_block))
                        {
                            ChunkNavmeshNode candidate_block_coord = {
                                {0, CHUNK_LENGTH - 1, z},
                                below_right_neighbour->position
                            };

                            struct {
                                ChunkNavmeshVertexDescriptor in_neighbour,
                                    in_below_right_neighbour;
                            } candidate_block_vertex
                                = { impl::get_vertex(
                                        neighbour_navmesh, candidate_block_coord
                                    ),
                                    impl::get_vertex(
                                        below_right_neighbour_navmesh,
                                        candidate_block_coord
                                    ) };

                            boost::add_edge(
                                neighbour_block_vertex.in_neighbour,
                                candidate_block_vertex.in_neighbour,
                                neighbour_navmesh.graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.in_neighbour,
                                neighbour_block_vertex.in_neighbour,
                                neighbour_navmesh.graph
                            );

                            boost::add_edge(
                                neighbour_block_vertex.in_below_right_neighbour,
                                candidate_block_vertex.in_below_right_neighbour,
                                below_right_neighbour_navmesh.graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.in_below_right_neighbour,
                                neighbour_block_vertex.in_below_right_neighbour,
                                below_right_neighbour_navmesh.graph
                            );
                        }
                        // Step down
                        else if (is_solid(step_down_candidate_block) && !is_solid(step_across_candidate_block) && !is_solid(above_candidates_block))
                        {
                            ChunkNavmeshNode candidate_block_coord = {
                                {0, CHUNK_LENGTH - 2, z},
                                below_right_neighbour->position
                            };

                            struct {
                                ChunkNavmeshVertexDescriptor in_neighbour,
                                    in_below_right_neighbour;
                            } candidate_block_vertex
                                = { impl::get_vertex(
                                        neighbour_navmesh, candidate_block_coord
                                    ),
                                    impl::get_vertex(
                                        below_right_neighbour_navmesh,
                                        candidate_block_coord
                                    ) };

                            boost::add_edge(
                                neighbour_block_vertex.in_neighbour,
                                candidate_block_vertex.in_neighbour,
                                neighbour_navmesh.graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.in_neighbour,
                                neighbour_block_vertex.in_neighbour,
                                neighbour_navmesh.graph
                            );

                            boost::add_edge(
                                neighbour_block_vertex.in_below_right_neighbour,
                                candidate_block_vertex.in_below_right_neighbour,
                                below_right_neighbour_navmesh.graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.in_below_right_neighbour,
                                neighbour_block_vertex.in_below_right_neighbour,
                                below_right_neighbour_navmesh.graph
                            );
                        }
                    }

                    neighbour->navmesh_stitch.above_and_across_right.store(
                        ChunkState::COMPLETE
                    );
                }
            }

            // Front
            {
                auto front_neighbour       = chunk->neighbours.one.front.lock();
                auto below_front_neighbour = neighbour->neighbours.one.front.lock();
                ChunkState neighbour_stitch_state = ChunkState::NONE;
                if (front_neighbour != nullptr && below_front_neighbour != nullptr
                    && front_neighbour->bulk_navmeshing.load() == ChunkState::COMPLETE
                    && below_front_neighbour->bulk_navmeshing.load()
                           == ChunkState::COMPLETE
                    && neighbour->navmesh_stitch.above_and_across_front
                           .compare_exchange_strong(
                               neighbour_stitch_state, ChunkState::ACTIVE
                           ))
                {
                    std::shared_lock<std::shared_mutex> front_neighbour_block_lock;
                    auto                                front_neighbour_blocks
                        = front_neighbour->blocks.get(front_neighbour_block_lock);
                    std::unique_lock<std::shared_mutex> front_neighbour_navmesh_lock;
                    auto                                front_neighbour_navmesh
                        = front_neighbour->navmesh.get(front_neighbour_navmesh_lock);

                    std::shared_lock<std::shared_mutex>
                         below_front_neighbour_block_lock;
                    auto below_front_neighbour_blocks
                        = below_front_neighbour->blocks.get(
                            below_front_neighbour_block_lock
                        );
                    std::unique_lock<std::shared_mutex>
                         below_front_neighbour_navmesh_lock;
                    auto below_front_neighbour_navmesh
                        = below_front_neighbour->navmesh.get(
                            below_front_neighbour_navmesh_lock
                        );

                    // Step up from y == CHUNK_LENGTH - 2
                    for (BlockChunkPositionCoord x = 0; x < CHUNK_LENGTH; ++x) {
                        BlockIndex neighbour_block_index = hvox::block_index(
                            { x, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1 }
                        );
                        const Block* neighbour_block
                            = &neighbour_blocks[neighbour_block_index];

                        BlockIndex above_neighbour_block_index = hvox::block_index(
                            { x, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1 }
                        );
                        const Block* above_neighbour_block
                            = &neighbour_blocks[above_neighbour_block_index];

                        BlockIndex twice_above_neighbour_block_index
                            = hvox::block_index({ x, 0, CHUNK_LENGTH - 1 });
                        const Block* twice_above_neighbour_block
                            = &chunk_blocks[twice_above_neighbour_block_index];

                        if (!is_solid(neighbour_block)
                            || is_solid(above_neighbour_block)
                            || is_solid(twice_above_neighbour_block))
                            continue;

                        ChunkNavmeshNode neighbour_block_coord = {
                            {x, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1},
                            neighbour->position
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor in_neighbour,
                                in_below_front_neighbour;
                        } neighbour_block_vertex = {
                            impl::get_vertex(neighbour_navmesh, neighbour_block_coord),
                            impl::get_vertex(
                                below_front_neighbour_navmesh, neighbour_block_coord
                            )
                        };

                        BlockIndex above_candidate_index
                            = hvox::block_index({ x, 0, 0 });
                        const Block* above_candidate_block
                            = &front_neighbour_blocks[above_candidate_index];

                        BlockIndex candidate_index
                            = hvox::block_index({ x, CHUNK_LENGTH - 1, 0 });
                        const Block* candidate_block
                            = &below_front_neighbour_blocks[candidate_index];

                        if (!is_solid(candidate_block)
                            || is_solid(above_candidate_block))
                            continue;

                        ChunkNavmeshNode candidate_block_coord = {
                            {x, CHUNK_LENGTH - 1, 0},
                            below_front_neighbour->position
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor in_neighbour,
                                in_below_front_neighbour;
                        } candidate_block_vertex = {
                            impl::get_vertex(neighbour_navmesh, candidate_block_coord),
                            impl::get_vertex(
                                below_front_neighbour_navmesh, candidate_block_coord
                            )
                        };

                        boost::add_edge(
                            neighbour_block_vertex.in_neighbour,
                            candidate_block_vertex.in_neighbour,
                            neighbour_navmesh.graph
                        );
                        boost::add_edge(
                            candidate_block_vertex.in_neighbour,
                            neighbour_block_vertex.in_neighbour,
                            neighbour_navmesh.graph
                        );

                        boost::add_edge(
                            neighbour_block_vertex.in_below_front_neighbour,
                            candidate_block_vertex.in_below_front_neighbour,
                            below_front_neighbour_navmesh.graph
                        );
                        boost::add_edge(
                            candidate_block_vertex.in_below_front_neighbour,
                            neighbour_block_vertex.in_below_front_neighbour,
                            below_front_neighbour_navmesh.graph
                        );
                    }

                    // Step across and down from y == CHUNK_LENGTH - 1
                    for (BlockChunkPositionCoord x = 0; x < CHUNK_LENGTH; ++x) {
                        BlockIndex neighbour_block_index = hvox::block_index(
                            { x, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1 }
                        );
                        const Block* neighbour_block
                            = &neighbour_blocks[neighbour_block_index];

                        BlockIndex above_neighbour_block_index
                            = hvox::block_index({ x, 0, CHUNK_LENGTH - 1 });
                        const Block* above_neighbour_block
                            = &chunk_blocks[above_neighbour_block_index];

                        // Necessary condition for step across and down.
                        if (!is_solid(neighbour_block)
                            || is_solid(above_neighbour_block))
                            continue;

                        ChunkNavmeshNode neighbour_block_coord = {
                            {x, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1},
                            neighbour->position
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor in_neighbour,
                                in_below_front_neighbour;
                        } neighbour_block_vertex = {
                            impl::get_vertex(neighbour_navmesh, neighbour_block_coord),
                            impl::get_vertex(
                                below_front_neighbour_navmesh, neighbour_block_coord
                            )
                        };

                        BlockIndex step_down_candidate_index
                            = hvox::block_index({ x, CHUNK_LENGTH - 2, 0 });
                        const Block* step_down_candidate_block
                            = &below_front_neighbour_blocks[step_down_candidate_index];

                        BlockIndex step_across_candidate_index
                            = hvox::block_index({ x, CHUNK_LENGTH - 1, 0 });
                        const Block* step_across_candidate_block
                            = &below_front_neighbour_blocks
                                  [step_across_candidate_index];

                        BlockIndex above_candidates_index
                            = hvox::block_index({ x, 0, 0 });
                        const Block* above_candidates_block
                            = &front_neighbour_blocks[above_candidates_index];

                        // Step across
                        if (is_solid(step_across_candidate_block)
                            && !is_solid(above_candidates_block))
                        {
                            ChunkNavmeshNode candidate_block_coord = {
                                {x, CHUNK_LENGTH - 1, 0},
                                below_front_neighbour->position
                            };

                            struct {
                                ChunkNavmeshVertexDescriptor in_neighbour,
                                    in_below_front_neighbour;
                            } candidate_block_vertex
                                = { impl::get_vertex(
                                        neighbour_navmesh, candidate_block_coord
                                    ),
                                    impl::get_vertex(
                                        below_front_neighbour_navmesh,
                                        candidate_block_coord
                                    ) };

                            boost::add_edge(
                                neighbour_block_vertex.in_neighbour,
                                candidate_block_vertex.in_neighbour,
                                neighbour_navmesh.graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.in_neighbour,
                                neighbour_block_vertex.in_neighbour,
                                neighbour_navmesh.graph
                            );

                            boost::add_edge(
                                neighbour_block_vertex.in_below_front_neighbour,
                                candidate_block_vertex.in_below_front_neighbour,
                                below_front_neighbour_navmesh.graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.in_below_front_neighbour,
                                neighbour_block_vertex.in_below_front_neighbour,
                                below_front_neighbour_navmesh.graph
                            );
                        }
                        // Step down
                        else if (is_solid(step_down_candidate_block) && !is_solid(step_across_candidate_block) && !is_solid(above_candidates_block))
                        {
                            ChunkNavmeshNode candidate_block_coord = {
                                {x, CHUNK_LENGTH - 2, 0},
                                below_front_neighbour->position
                            };

                            struct {
                                ChunkNavmeshVertexDescriptor in_neighbour,
                                    in_below_front_neighbour;
                            } candidate_block_vertex
                                = { impl::get_vertex(
                                        neighbour_navmesh, candidate_block_coord
                                    ),
                                    impl::get_vertex(
                                        below_front_neighbour_navmesh,
                                        candidate_block_coord
                                    ) };

                            boost::add_edge(
                                neighbour_block_vertex.in_neighbour,
                                candidate_block_vertex.in_neighbour,
                                neighbour_navmesh.graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.in_neighbour,
                                neighbour_block_vertex.in_neighbour,
                                neighbour_navmesh.graph
                            );

                            boost::add_edge(
                                neighbour_block_vertex.in_below_front_neighbour,
                                candidate_block_vertex.in_below_front_neighbour,
                                below_front_neighbour_navmesh.graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.in_below_front_neighbour,
                                neighbour_block_vertex.in_below_front_neighbour,
                                below_front_neighbour_navmesh.graph
                            );
                        }
                    }

                    chunk->navmesh_stitch.above_and_across_front.store(
                        ChunkState::COMPLETE
                    );
                }
            }

            // Back
            {
                auto       back_neighbour       = chunk->neighbours.one.back.lock();
                auto       below_back_neighbour = neighbour->neighbours.one.back.lock();
                ChunkState neighbour_stitch_state = ChunkState::NONE;
                if (back_neighbour != nullptr && below_back_neighbour != nullptr
                    && back_neighbour->bulk_navmeshing.load() == ChunkState::COMPLETE
                    && below_back_neighbour->bulk_navmeshing.load()
                           == ChunkState::COMPLETE
                    && chunk->navmesh_stitch.above_and_across_back
                           .compare_exchange_strong(
                               neighbour_stitch_state, ChunkState::ACTIVE
                           ))
                {
                    std::shared_lock<std::shared_mutex> back_neighbour_block_lock;
                    auto                                back_neighbour_blocks
                        = back_neighbour->blocks.get(back_neighbour_block_lock);
                    std::unique_lock<std::shared_mutex> back_neighbour_navmesh_lock;
                    auto                                back_neighbour_navmesh
                        = back_neighbour->navmesh.get(back_neighbour_navmesh_lock);

                    std::shared_lock<std::shared_mutex> below_back_neighbour_block_lock;
                    auto below_back_neighbour_blocks = below_back_neighbour->blocks.get(
                        below_back_neighbour_block_lock
                    );
                    std::unique_lock<std::shared_mutex>
                         below_back_neighbour_navmesh_lock;
                    auto below_back_neighbour_navmesh
                        = below_back_neighbour->navmesh.get(
                            below_back_neighbour_navmesh_lock
                        );

                    // Step up from y == CHUNK_LENGTH - 2
                    for (BlockChunkPositionCoord x = 0; x < CHUNK_LENGTH; ++x) {
                        BlockIndex neighbour_block_index
                            = hvox::block_index({ x, CHUNK_LENGTH - 2, 0 });
                        const Block* neighbour_block
                            = &neighbour_blocks[neighbour_block_index];

                        BlockIndex above_neighbour_block_index
                            = hvox::block_index({ x, CHUNK_LENGTH - 1, 0 });
                        const Block* above_neighbour_block
                            = &neighbour_blocks[above_neighbour_block_index];

                        BlockIndex twice_above_neighbour_block_index
                            = hvox::block_index({ CHUNK_LENGTH - 1, 0, 0 });
                        const Block* twice_above_neighbour_block
                            = &chunk_blocks[twice_above_neighbour_block_index];

                        if (!is_solid(neighbour_block)
                            || is_solid(above_neighbour_block)
                            || is_solid(twice_above_neighbour_block))
                            continue;

                        ChunkNavmeshNode neighbour_block_coord = {
                            {x, CHUNK_LENGTH - 2, 0},
                            neighbour->position
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor in_neighbour,
                                in_below_back_neighbour;
                        } neighbour_block_vertex = {
                            impl::get_vertex(neighbour_navmesh, neighbour_block_coord),
                            impl::get_vertex(
                                below_back_neighbour_navmesh, neighbour_block_coord
                            )
                        };

                        BlockIndex above_candidate_index
                            = hvox::block_index({ x, 0, CHUNK_LENGTH - 1 });
                        const Block* above_candidate_block
                            = &back_neighbour_blocks[above_candidate_index];

                        BlockIndex candidate_index = hvox::block_index(
                            { x, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1 }
                        );
                        const Block* candidate_block
                            = &below_back_neighbour_blocks[candidate_index];

                        if (!is_solid(candidate_block)
                            || is_solid(above_candidate_block))
                            continue;

                        ChunkNavmeshNode candidate_block_coord = {
                            {x, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1},
                            below_back_neighbour->position
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor in_neighbour,
                                in_below_back_neighbour;
                        } candidate_block_vertex = {
                            impl::get_vertex(neighbour_navmesh, candidate_block_coord),
                            impl::get_vertex(
                                below_back_neighbour_navmesh, candidate_block_coord
                            )
                        };

                        boost::add_edge(
                            neighbour_block_vertex.in_neighbour,
                            candidate_block_vertex.in_neighbour,
                            neighbour_navmesh.graph
                        );
                        boost::add_edge(
                            candidate_block_vertex.in_neighbour,
                            neighbour_block_vertex.in_neighbour,
                            neighbour_navmesh.graph
                        );

                        boost::add_edge(
                            neighbour_block_vertex.in_below_back_neighbour,
                            candidate_block_vertex.in_below_back_neighbour,
                            below_back_neighbour_navmesh.graph
                        );
                        boost::add_edge(
                            candidate_block_vertex.in_below_back_neighbour,
                            neighbour_block_vertex.in_below_back_neighbour,
                            below_back_neighbour_navmesh.graph
                        );
                    }

                    // Step across and down from y == CHUNK_LENGTH - 1
                    for (BlockChunkPositionCoord x = 0; x < CHUNK_LENGTH; ++x) {
                        BlockIndex neighbour_block_index
                            = hvox::block_index({ x, CHUNK_LENGTH - 1, 0 });
                        const Block* neighbour_block
                            = &neighbour_blocks[neighbour_block_index];

                        BlockIndex above_neighbour_block_index
                            = hvox::block_index({ x, 0, 0 });
                        const Block* above_neighbour_block
                            = &chunk_blocks[above_neighbour_block_index];

                        // Necessary condition for step across and down.
                        if (!is_solid(neighbour_block)
                            || is_solid(above_neighbour_block))
                            continue;

                        ChunkNavmeshNode neighbour_block_coord = {
                            {x, CHUNK_LENGTH - 1, 0},
                            neighbour->position
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor in_neighbour,
                                in_below_back_neighbour;
                        } neighbour_block_vertex = {
                            impl::get_vertex(neighbour_navmesh, neighbour_block_coord),
                            impl::get_vertex(
                                below_back_neighbour_navmesh, neighbour_block_coord
                            )
                        };

                        BlockIndex step_down_candidate_index = hvox::block_index(
                            { x, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1 }
                        );
                        const Block* step_down_candidate_block
                            = &below_back_neighbour_blocks[step_down_candidate_index];

                        BlockIndex step_across_candidate_index = hvox::block_index(
                            { x, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1 }
                        );
                        const Block* step_across_candidate_block
                            = &below_back_neighbour_blocks[step_across_candidate_index];

                        BlockIndex above_candidates_index
                            = hvox::block_index({ x, 0, CHUNK_LENGTH - 1 });
                        const Block* above_candidates_block
                            = &back_neighbour_blocks[above_candidates_index];

                        // Step across
                        if (is_solid(step_across_candidate_block)
                            && !is_solid(above_candidates_block))
                        {
                            ChunkNavmeshNode candidate_block_coord = {
                                {x, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1},
                                below_back_neighbour->position
                            };

                            struct {
                                ChunkNavmeshVertexDescriptor in_neighbour,
                                    in_below_back_neighbour;
                            } candidate_block_vertex
                                = { impl::get_vertex(
                                        neighbour_navmesh, candidate_block_coord
                                    ),
                                    impl::get_vertex(
                                        below_back_neighbour_navmesh,
                                        candidate_block_coord
                                    ) };

                            boost::add_edge(
                                neighbour_block_vertex.in_neighbour,
                                candidate_block_vertex.in_neighbour,
                                neighbour_navmesh.graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.in_neighbour,
                                neighbour_block_vertex.in_neighbour,
                                neighbour_navmesh.graph
                            );

                            boost::add_edge(
                                neighbour_block_vertex.in_below_back_neighbour,
                                candidate_block_vertex.in_below_back_neighbour,
                                below_back_neighbour_navmesh.graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.in_below_back_neighbour,
                                neighbour_block_vertex.in_below_back_neighbour,
                                below_back_neighbour_navmesh.graph
                            );
                        }
                        // Step down
                        else if (is_solid(step_down_candidate_block) && !is_solid(step_across_candidate_block) && !is_solid(above_candidates_block))
                        {
                            ChunkNavmeshNode candidate_block_coord = {
                                {x, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1},
                                below_back_neighbour->position
                            };

                            struct {
                                ChunkNavmeshVertexDescriptor in_neighbour,
                                    in_below_back_neighbour;
                            } candidate_block_vertex
                                = { impl::get_vertex(
                                        neighbour_navmesh, candidate_block_coord
                                    ),
                                    impl::get_vertex(
                                        below_back_neighbour_navmesh,
                                        candidate_block_coord
                                    ) };

                            boost::add_edge(
                                neighbour_block_vertex.in_neighbour,
                                candidate_block_vertex.in_neighbour,
                                neighbour_navmesh.graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.in_neighbour,
                                neighbour_block_vertex.in_neighbour,
                                neighbour_navmesh.graph
                            );

                            boost::add_edge(
                                neighbour_block_vertex.in_below_back_neighbour,
                                candidate_block_vertex.in_below_back_neighbour,
                                below_back_neighbour_navmesh.graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.in_below_back_neighbour,
                                neighbour_block_vertex.in_below_back_neighbour,
                                below_back_neighbour_navmesh.graph
                            );
                        }
                    }
                }
            }

            neighbour->navmesh_stitch.top.store(ChunkState::COMPLETE);
        }
    }

    chunk->navmeshing.store(ChunkState::COMPLETE, std::memory_order_release);

    chunk->on_navmesh_change();
}
