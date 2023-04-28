#include "memory/handle.hpp"
#include "voxel/chunk.h"
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

namespace hemlock::voxel::ai::impl {
    inline ChunkNavmeshVertexDescriptor
    get_vertex(const hmem::Handle<Chunk>& chunk, const ChunkNavmeshNode& coord) {
        try {
            hmem::SharedResourceLock lock;
            auto                     navmesh = chunk->navmesh.get(lock);

            return navmesh.data->coord_vertex_map.at(coord);
        } catch (std::out_of_range&) {
            hmem::UniqueResourceLock lock;
            auto                     navmesh = chunk->navmesh.get(lock);

            auto vertex = boost::add_vertex(navmesh.data->graph);
            navmesh.data->coord_vertex_map[coord]  = vertex;
            navmesh.data->vertex_coord_map[vertex] = coord;
            return vertex;
        }
    }
}  // namespace hemlock::voxel::ai::impl

template <hvox::IdealBlockConstraint IsSolid>
void hvox::ai::NaiveNavmeshStrategy<IsSolid>::do_bulk(
    hmem::Handle<ChunkGrid>, hmem::Handle<Chunk> chunk
) const {
    auto chunk_pos = chunk->position;

    hmem::SharedResourceLock block_lock;
    auto                     blocks = chunk->blocks.get(block_lock);

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
            Block* above_candidate_block = &blocks.data[above_candidate_index];

            BlockIndex candidate_index = hvox::block_index(
                static_cast<i64v3>(offset) + i64v3{ 0, y_off - 1, 0 }
            );
            Block* candidate_block = &blocks.data[candidate_index];

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
                    Block* twice_above_start_block
                        = &blocks.data[twice_above_start_index];

                    if (is_solid(twice_above_start_block)) continue;
                } else if (y_off - 1 == -1) {
                    BlockIndex twice_above_candidate_index
                        = hvox::block_index(offset + BlockChunkPosition{ 0, 1, 0 });
                    Block* twice_above_candidate_block
                        = &blocks.data[twice_above_candidate_index];

                    if (is_solid(twice_above_candidate_block)) continue;
                }

                ChunkNavmeshNode candidate_block_coord = {
                    static_cast<i64v3>(offset) + i64v3{0, y_off - 1, 0},
                      chunk_pos
                };

                // Ensure node exists for this block.
                ChunkNavmeshVertexDescriptor candidate_block_vertex
                    = impl::get_vertex(chunk, candidate_block_coord);

                {
                    hmem::UniqueResourceLock lock;
                    auto                     navmesh = chunk->navmesh.get(lock);

                    boost::add_edge(
                        block_vertex, candidate_block_vertex, navmesh.data->graph
                    );
                    boost::add_edge(
                        candidate_block_vertex, block_vertex, navmesh.data->graph
                    );
                }
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
                BlockIndex block_index = hvox::block_index({ x, y, z });
                Block*     block       = &blocks.data[block_index];

                // Only consider block if it is solid.
                if (!is_solid(block)) continue;

                BlockIndex block_above_index = hvox::block_index({ x, y + 1, z });
                Block*     block_above       = &blocks.data[block_above_index];

                // Only consider block if it is not covered above.
                if (is_solid(block_above)) continue;

                // Ensure node exists for this block.
                ChunkNavmeshNode block_coord = {
                    {x, y, z},
                    chunk_pos
                };
                ChunkNavmeshVertexDescriptor block_vertex
                    = impl::get_vertex(chunk, block_coord);

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
                Block*     block       = &blocks.data[block_index];

                // Only consider block if it is solid.
                if (!is_solid(block)) continue;

                BlockIndex block_above_index
                    = hvox::block_index({ x, CHUNK_LENGTH - 1, z });
                Block* block_above = &blocks.data[block_above_index];

                // Only consider block if it is not covered above.
                if (is_solid(block_above)) continue;

                // Ensure node exists for this block.
                ChunkNavmeshNode block_coord = {
                    {x, CHUNK_LENGTH - 2, z},
                    chunk_pos
                };
                ChunkNavmeshVertexDescriptor block_vertex
                    = impl::get_vertex(chunk, block_coord);

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

    // Left Face (except second-to-top and top layers)
    for (BlockChunkPositionCoord z = 1; z < CHUNK_LENGTH - 1; ++z) {
        for (BlockChunkPositionCoord y = 1; y < CHUNK_LENGTH - 2; ++y) {
            BlockIndex block_index = hvox::block_index({ 0, y, z });
            Block*     block       = &blocks.data[block_index];

            // Only consider block if it is solid.
            if (!is_solid(block)) continue;

            BlockIndex block_above_index = hvox::block_index({ 0, y + 1, z });
            Block*     block_above       = &blocks.data[block_above_index];

            // Only consider block if it is not covered above.
            if (is_solid(block_above)) continue;

            // Ensure node exists for this block.
            ChunkNavmeshNode block_coord = {
                {0, y, z},
                chunk_pos
            };
            ChunkNavmeshVertexDescriptor block_vertex
                = impl::get_vertex(chunk, block_coord);

            // Right
            do_navigable_check(block_vertex, { 0, y, z }, { 1, y, z }, 2, -1);

            // Front
            do_navigable_check(block_vertex, { 0, y, z }, { 0, y, z + 1 }, 2, -1);

            // Back
            do_navigable_check(block_vertex, { 0, y, z }, { 0, y, z - 1 }, 2, -1);
        }
    }

    // Right Face (except second-to-top and top layers)
    for (BlockChunkPositionCoord z = 1; z < CHUNK_LENGTH - 1; ++z) {
        for (BlockChunkPositionCoord y = 1; y < CHUNK_LENGTH - 2; ++y) {
            BlockIndex block_index = hvox::block_index({ CHUNK_LENGTH - 1, y, z });
            Block*     block       = &blocks.data[block_index];

            // Only consider block if it is solid.
            if (!is_solid(block)) continue;

            BlockIndex block_above_index
                = hvox::block_index({ CHUNK_LENGTH - 1, y + 1, z });
            Block* block_above = &blocks.data[block_above_index];

            // Only consider block if it is not covered above.
            if (is_solid(block_above)) continue;

            // Ensure node exists for this block.
            ChunkNavmeshNode block_coord = {
                {CHUNK_LENGTH - 1, y, z},
                chunk_pos
            };
            ChunkNavmeshVertexDescriptor block_vertex
                = impl::get_vertex(chunk, block_coord);

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

    // Left Face (second-to-top case)
    for (BlockChunkPositionCoord z = 1; z < CHUNK_LENGTH - 1; ++z) {
        for (BlockChunkPositionCoord y = 1; y < CHUNK_LENGTH - 2; ++y) {
            BlockIndex block_index = hvox::block_index({ 0, CHUNK_LENGTH - 2, z });
            Block*     block       = &blocks.data[block_index];

            // Only consider block if it is solid.
            if (!is_solid(block)) continue;

            BlockIndex block_above_index
                = hvox::block_index({ 0, CHUNK_LENGTH - 1, z });
            Block* block_above = &blocks.data[block_above_index];

            // Only consider block if it is not covered above.
            if (is_solid(block_above)) continue;

            // Ensure node exists for this block.
            ChunkNavmeshNode block_coord = {
                {0, CHUNK_LENGTH - 2, z},
                chunk_pos
            };
            ChunkNavmeshVertexDescriptor block_vertex
                = impl::get_vertex(chunk, block_coord);

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
    }

    // Right Face (second-to-top case)
    for (BlockChunkPositionCoord z = 1; z < CHUNK_LENGTH - 1; ++z) {
        for (BlockChunkPositionCoord y = 1; y < CHUNK_LENGTH - 2; ++y) {
            BlockIndex block_index
                = hvox::block_index({ CHUNK_LENGTH - 1, CHUNK_LENGTH - 2, z });
            Block* block = &blocks.data[block_index];

            // Only consider block if it is solid.
            if (!is_solid(block)) continue;

            BlockIndex block_above_index
                = hvox::block_index({ CHUNK_LENGTH - 1, CHUNK_LENGTH - 1, z });
            Block* block_above = &blocks.data[block_above_index];

            // Only consider block if it is not covered above.
            if (is_solid(block_above)) continue;

            // Ensure node exists for this block.
            ChunkNavmeshNode block_coord = {
                {CHUNK_LENGTH - 1, CHUNK_LENGTH - 2, z},
                chunk_pos
            };
            ChunkNavmeshVertexDescriptor block_vertex
                = impl::get_vertex(chunk, block_coord);

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

    /********************************\
     * Navmesh front and back faces *
     *   except for edges.          *
    \********************************/

    // Front Face (except second-to-top and top layers)
    for (BlockChunkPositionCoord x = 1; x < CHUNK_LENGTH - 1; ++x) {
        for (BlockChunkPositionCoord y = 1; y < CHUNK_LENGTH - 2; ++y) {
            BlockIndex block_index = hvox::block_index({ x, y, CHUNK_LENGTH - 1 });
            Block*     block       = &blocks.data[block_index];

            // Only consider block if it is solid.
            if (!is_solid(block)) continue;

            BlockIndex block_above_index
                = hvox::block_index({ x, y + 1, CHUNK_LENGTH - 1 });
            Block* block_above = &blocks.data[block_above_index];

            // Only consider block if it is not covered above.
            if (is_solid(block_above)) continue;

            // Ensure node exists for this block.
            ChunkNavmeshNode block_coord = {
                {x, y, CHUNK_LENGTH - 1},
                chunk_pos
            };
            ChunkNavmeshVertexDescriptor block_vertex
                = impl::get_vertex(chunk, block_coord);

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
    }

    // Back Face (except second-to-top and top layers)
    for (BlockChunkPositionCoord x = 1; x < CHUNK_LENGTH - 1; ++x) {
        for (BlockChunkPositionCoord y = 1; y < CHUNK_LENGTH - 2; ++y) {
            BlockIndex block_index = hvox::block_index({ x, y, 0 });
            Block*     block       = &blocks.data[block_index];

            // Only consider block if it is solid.
            if (!is_solid(block)) continue;

            BlockIndex block_above_index = hvox::block_index({ x, y + 1, 0 });
            Block*     block_above       = &blocks.data[block_above_index];

            // Only consider block if it is not covered above.
            if (is_solid(block_above)) continue;

            // Ensure node exists for this block.
            ChunkNavmeshNode block_coord = {
                {x, y, 0},
                chunk_pos
            };
            ChunkNavmeshVertexDescriptor block_vertex
                = impl::get_vertex(chunk, block_coord);

            // Left
            do_navigable_check(block_vertex, { x, y, 0 }, { x - 1, y, 0 }, 2, -1);

            // Right
            do_navigable_check(block_vertex, { x, y, 0 }, { x + 1, y, 0 }, 2, -1);

            // Front
            do_navigable_check(block_vertex, { x, y, 0 }, { x, y, 1 }, 2, -1);
        }
    }

    // Front Face (second-to-top case)
    for (BlockChunkPositionCoord x = 1; x < CHUNK_LENGTH - 1; ++x) {
        for (BlockChunkPositionCoord y = 1; y < CHUNK_LENGTH - 2; ++y) {
            BlockIndex block_index
                = hvox::block_index({ x, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1 });
            Block* block = &blocks.data[block_index];

            // Only consider block if it is solid.
            if (!is_solid(block)) continue;

            BlockIndex block_above_index
                = hvox::block_index({ x, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1 });
            Block* block_above = &blocks.data[block_above_index];

            // Only consider block if it is not covered above.
            if (is_solid(block_above)) continue;

            // Ensure node exists for this block.
            ChunkNavmeshNode block_coord = {
                {x, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1},
                chunk_pos
            };
            ChunkNavmeshVertexDescriptor block_vertex
                = impl::get_vertex(chunk, block_coord);

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
    }

    // Back Face (second-to-top case)
    for (BlockChunkPositionCoord x = 1; x < CHUNK_LENGTH - 1; ++x) {
        for (BlockChunkPositionCoord y = 1; y < CHUNK_LENGTH - 2; ++y) {
            BlockIndex block_index = hvox::block_index({ x, CHUNK_LENGTH - 2, 0 });
            Block*     block       = &blocks.data[block_index];

            // Only consider block if it is solid.
            if (!is_solid(block)) continue;

            BlockIndex block_above_index
                = hvox::block_index({ x, CHUNK_LENGTH - 1, 0 });
            Block* block_above = &blocks.data[block_above_index];

            // Only consider block if it is not covered above.
            if (is_solid(block_above)) continue;

            // Ensure node exists for this block.
            ChunkNavmeshNode block_coord = {
                {x, CHUNK_LENGTH - 2, 0},
                chunk_pos
            };
            ChunkNavmeshVertexDescriptor block_vertex
                = impl::get_vertex(chunk, block_coord);

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
                BlockIndex block_index = hvox::block_index({ x, 0, z });
                Block*     block       = &blocks.data[block_index];

                // Only consider block if it is solid.
                if (!is_solid(block)) continue;

                BlockIndex block_above_index = hvox::block_index({ x, 1, z });
                Block*     block_above       = &blocks.data[block_above_index];

                // Only consider block if it is not covered above.
                if (is_solid(block_above)) continue;

                // Ensure node exists for this block.
                ChunkNavmeshNode block_coord = {
                    {x, 0, z},
                    chunk_pos
                };
                ChunkNavmeshVertexDescriptor block_vertex
                    = impl::get_vertex(chunk, block_coord);

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

    // Front-Left Edge
    for (BlockChunkPositionCoord y = 1; y < CHUNK_LENGTH - 2; ++y) {
        BlockIndex block_index = hvox::block_index({ 0, y, CHUNK_LENGTH - 1 });
        Block*     block       = &blocks.data[block_index];

        // Only consider block if it is solid.
        if (!is_solid(block)) continue;

        BlockIndex block_above_index
            = hvox::block_index({ 0, y + 1, CHUNK_LENGTH - 1 });
        Block* block_above = &blocks.data[block_above_index];

        // Only consider block if it is not covered above.
        if (is_solid(block_above)) continue;

        // Ensure node exists for this block.
        ChunkNavmeshNode block_coord = {
            {0, y, CHUNK_LENGTH - 1},
            chunk_pos
        };
        ChunkNavmeshVertexDescriptor block_vertex
            = impl::get_vertex(chunk, block_coord);

        // Right
        do_navigable_check(
            block_vertex, { 0, y, CHUNK_LENGTH - 1 }, { 1, y, CHUNK_LENGTH - 1 }, 2, -1
        );

        // Back
        do_navigable_check(
            block_vertex, { 0, y, CHUNK_LENGTH - 1 }, { 0, y, CHUNK_LENGTH - 2 }, 2, -1
        );
    }

    // Front-Right Edge
    for (BlockChunkPositionCoord y = 1; y < CHUNK_LENGTH - 2; ++y) {
        BlockIndex block_index
            = hvox::block_index({ CHUNK_LENGTH - 1, y, CHUNK_LENGTH - 1 });
        Block* block = &blocks.data[block_index];

        // Only consider block if it is solid.
        if (!is_solid(block)) continue;

        BlockIndex block_above_index
            = hvox::block_index({ CHUNK_LENGTH - 1, y + 1, CHUNK_LENGTH - 1 });
        Block* block_above = &blocks.data[block_above_index];

        // Only consider block if it is not covered above.
        if (is_solid(block_above)) continue;

        // Ensure node exists for this block.
        ChunkNavmeshNode block_coord = {
            {CHUNK_LENGTH - 1, y, CHUNK_LENGTH - 1},
            chunk_pos
        };
        ChunkNavmeshVertexDescriptor block_vertex
            = impl::get_vertex(chunk, block_coord);

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
    for (BlockChunkPositionCoord y = 1; y < CHUNK_LENGTH - 2; ++y) {
        BlockIndex block_index = hvox::block_index({ 0, y, 0 });
        Block*     block       = &blocks.data[block_index];

        // Only consider block if it is solid.
        if (!is_solid(block)) continue;

        BlockIndex block_above_index = hvox::block_index({ 0, y + 1, 0 });
        Block*     block_above       = &blocks.data[block_above_index];

        // Only consider block if it is not covered above.
        if (is_solid(block_above)) continue;

        // Ensure node exists for this block.
        ChunkNavmeshNode block_coord = {
            {0, y, 0},
            chunk_pos
        };
        ChunkNavmeshVertexDescriptor block_vertex
            = impl::get_vertex(chunk, block_coord);

        // Right
        do_navigable_check(block_vertex, { 0, y, 0 }, { 1, y, 0 }, 2, -1);

        // Front
        do_navigable_check(block_vertex, { 0, y, 0 }, { 0, y, 1 }, 2, -1);
    }

    // Back-Right Edge
    for (BlockChunkPositionCoord y = 1; y < CHUNK_LENGTH - 2; ++y) {
        BlockIndex block_index = hvox::block_index({ CHUNK_LENGTH - 1, y, 0 });
        Block*     block       = &blocks.data[block_index];

        // Only consider block if it is solid.
        if (!is_solid(block)) continue;

        BlockIndex block_above_index
            = hvox::block_index({ CHUNK_LENGTH - 1, y + 1, 0 });
        Block* block_above = &blocks.data[block_above_index];

        // Only consider block if it is not covered above.
        if (is_solid(block_above)) continue;

        // Ensure node exists for this block.
        ChunkNavmeshNode block_coord = {
            {CHUNK_LENGTH - 1, y, 0},
            chunk_pos
        };
        ChunkNavmeshVertexDescriptor block_vertex
            = impl::get_vertex(chunk, block_coord);

        // Left
        do_navigable_check(
            block_vertex, { CHUNK_LENGTH - 1, y, 0 }, { CHUNK_LENGTH - 2, y, 0 }, 2, -1
        );

        // Front
        do_navigable_check(
            block_vertex, { CHUNK_LENGTH - 1, y, 0 }, { CHUNK_LENGTH - 1, y, 1 }, 2, -1
        );
    }

    // Second-to-top case
    // Front-Left Edge
    {
        BlockIndex block_index
            = hvox::block_index({ 0, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1 });
        Block* block = &blocks.data[block_index];

        BlockIndex block_above_index
            = hvox::block_index({ 0, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1 });
        Block* block_above = &blocks.data[block_above_index];

        // Only consider block if it is not covered above.
        if (is_solid(block) && !is_solid(block_above)) {
            // Ensure node exists for this block.
            ChunkNavmeshNode block_coord = {
                {0, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1},
                chunk_pos
            };
            ChunkNavmeshVertexDescriptor block_vertex
                = impl::get_vertex(chunk, block_coord);

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
        Block* block = &blocks.data[block_index];

        BlockIndex block_above_index
            = hvox::block_index({ CHUNK_LENGTH - 1, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1 }
            );
        Block* block_above = &blocks.data[block_above_index];

        // Only consider block if it is not covered above.
        if (is_solid(block) && !is_solid(block_above)) {
            // Ensure node exists for this block.
            ChunkNavmeshNode block_coord = {
                {CHUNK_LENGTH - 1, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1},
                chunk_pos
            };
            ChunkNavmeshVertexDescriptor block_vertex
                = impl::get_vertex(chunk, block_coord);

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
        BlockIndex block_index = hvox::block_index({ 0, CHUNK_LENGTH - 2, 0 });
        Block*     block       = &blocks.data[block_index];

        BlockIndex block_above_index = hvox::block_index({ 0, CHUNK_LENGTH - 1, 0 });
        Block*     block_above       = &blocks.data[block_above_index];

        // Only consider block if it is not covered above.
        if (is_solid(block) && !is_solid(block_above)) {
            // Ensure node exists for this block.
            ChunkNavmeshNode block_coord = {
                {0, CHUNK_LENGTH - 2, 0},
                chunk_pos
            };
            ChunkNavmeshVertexDescriptor block_vertex
                = impl::get_vertex(chunk, block_coord);

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
        Block* block = &blocks.data[block_index];

        BlockIndex block_above_index
            = hvox::block_index({ CHUNK_LENGTH - 1, CHUNK_LENGTH - 1, 0 });
        Block* block_above = &blocks.data[block_above_index];

        // Only consider block if it is not covered above.
        if (is_solid(block) && !is_solid(block_above)) {
            // Ensure node exists for this block.
            ChunkNavmeshNode block_coord = {
                {CHUNK_LENGTH - 1, CHUNK_LENGTH - 2, 0},
                chunk_pos
            };
            ChunkNavmeshVertexDescriptor block_vertex
                = impl::get_vertex(chunk, block_coord);

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

    // Front-Bottom Edge
    for (BlockChunkPositionCoord x = 1; x < CHUNK_LENGTH - 1; ++x) {
        BlockIndex block_index = hvox::block_index({ x, 0, CHUNK_LENGTH - 1 });
        Block*     block       = &blocks.data[block_index];

        // Only consider block if it is solid.
        if (!is_solid(block)) continue;

        BlockIndex block_above_index = hvox::block_index({ x, 1, CHUNK_LENGTH - 1 });
        Block*     block_above       = &blocks.data[block_above_index];

        // Only consider block if it is not covered above.
        if (is_solid(block_above)) continue;

        // Ensure node exists for this block.
        ChunkNavmeshNode block_coord = {
            {x, 0, CHUNK_LENGTH - 1},
            chunk_pos
        };
        ChunkNavmeshVertexDescriptor block_vertex
            = impl::get_vertex(chunk, block_coord);

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
            block_vertex, { x, 0, CHUNK_LENGTH - 1 }, { x, 0, CHUNK_LENGTH - 2 }, 2, 0
        );
    }

    // Back-Bottom Edge
    for (BlockChunkPositionCoord x = 1; x < CHUNK_LENGTH - 1; ++x) {
        BlockIndex block_index = hvox::block_index({ x, 0, 0 });
        Block*     block       = &blocks.data[block_index];

        // Only consider block if it is solid.
        if (!is_solid(block)) continue;

        BlockIndex block_above_index = hvox::block_index({ x, 1, 0 });
        Block*     block_above       = &blocks.data[block_above_index];

        // Only consider block if it is not covered above.
        if (is_solid(block_above)) continue;

        // Ensure node exists for this block.
        ChunkNavmeshNode block_coord = {
            {x, 0, 0},
            chunk_pos
        };
        ChunkNavmeshVertexDescriptor block_vertex
            = impl::get_vertex(chunk, block_coord);

        // Left
        do_navigable_check(block_vertex, { x, 0, 0 }, { x - 1, 0, 0 }, 2, 0);

        // Right
        do_navigable_check(block_vertex, { x, 0, 0 }, { x + 1, 0, 0 }, 2, 0);

        // Front
        do_navigable_check(block_vertex, { x, 0, 0 }, { x, 0, 1 }, 2, 0);
    }

    // Front-Back Edges

    // Left-Bottom Edge
    for (BlockChunkPositionCoord z = 1; z < CHUNK_LENGTH - 1; ++z) {
        BlockIndex block_index = hvox::block_index({ 0, 0, z });
        Block*     block       = &blocks.data[block_index];

        // Only consider block if it is solid.
        if (!is_solid(block)) continue;

        BlockIndex block_above_index = hvox::block_index({ 0, 1, z });
        Block*     block_above       = &blocks.data[block_above_index];

        // Only consider block if it is not covered above.
        if (is_solid(block_above)) continue;

        // Ensure node exists for this block.
        ChunkNavmeshNode block_coord = {
            {0, 0, z},
            chunk_pos
        };
        ChunkNavmeshVertexDescriptor block_vertex
            = impl::get_vertex(chunk, block_coord);

        // Right
        do_navigable_check(block_vertex, { 0, 0, z }, { 1, 0, z }, 2, 0);

        // Front
        do_navigable_check(block_vertex, { 0, 0, z }, { 0, 0, z + 1 }, 2, 0);

        // Back
        do_navigable_check(block_vertex, { 0, 0, z }, { 0, 0, z - 1 }, 2, 0);
    }

    // Right-Bottom Edge
    for (BlockChunkPositionCoord z = 1; z < CHUNK_LENGTH - 1; ++z) {
        BlockIndex block_index = hvox::block_index({ CHUNK_LENGTH - 1, 0, z });
        Block*     block       = &blocks.data[block_index];

        // Only consider block if it is solid.
        if (!is_solid(block)) continue;

        BlockIndex block_above_index = hvox::block_index({ CHUNK_LENGTH - 1, 1, z });
        Block*     block_above       = &blocks.data[block_above_index];

        // Only consider block if it is not covered above.
        if (is_solid(block_above)) continue;

        // Ensure node exists for this block.
        ChunkNavmeshNode block_coord = {
            {CHUNK_LENGTH - 1, 0, z},
            chunk_pos
        };
        ChunkNavmeshVertexDescriptor block_vertex
            = impl::get_vertex(chunk, block_coord);

        // Left
        do_navigable_check(
            block_vertex, { CHUNK_LENGTH - 1, 0, z }, { CHUNK_LENGTH - 2, 0, z }, 2, 0
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

    /********************\
     * Navmesh corners. *
    \********************/

    // As we do not navmesh the top face of the chunk, we consider here only the bottom
    // corners of the chunk.

    // Left-Bottom-Front
    {
        BlockIndex block_index = hvox::block_index({ 0, 0, CHUNK_LENGTH - 1 });
        Block*     block       = &blocks.data[block_index];

        BlockIndex block_above_index = hvox::block_index({ 0, 1, CHUNK_LENGTH - 1 });
        Block*     block_above       = &blocks.data[block_above_index];

        // Only consider block if it is not covered above.
        if (is_solid(block) && !is_solid(block_above)) {
            // Ensure node exists for this block.
            ChunkNavmeshNode block_coord = {
                {0, 0, CHUNK_LENGTH - 1},
                chunk_pos
            };
            ChunkNavmeshVertexDescriptor block_vertex
                = impl::get_vertex(chunk, block_coord);

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
        BlockIndex block_index = hvox::block_index({ 0, 0, 0 });
        Block*     block       = &blocks.data[block_index];

        BlockIndex block_above_index = hvox::block_index({ 0, 1, 0 });
        Block*     block_above       = &blocks.data[block_above_index];

        // Only consider block if it is not covered above.
        if (is_solid(block) && !is_solid(block_above)) {
            // Ensure node exists for this block.
            ChunkNavmeshNode block_coord = {
                {0, 0, 0},
                chunk_pos
            };
            ChunkNavmeshVertexDescriptor block_vertex
                = impl::get_vertex(chunk, block_coord);

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
        Block* block = &blocks.data[block_index];

        BlockIndex block_above_index
            = hvox::block_index({ CHUNK_LENGTH - 1, 1, CHUNK_LENGTH - 1 });
        Block* block_above = &blocks.data[block_above_index];

        // Only consider block if it is not covered above.
        if (is_solid(block) && !is_solid(block_above)) {
            // Ensure node exists for this block.
            ChunkNavmeshNode block_coord = {
                {CHUNK_LENGTH - 1, 0, CHUNK_LENGTH - 1},
                chunk_pos
            };
            ChunkNavmeshVertexDescriptor block_vertex
                = impl::get_vertex(chunk, block_coord);

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
        BlockIndex block_index = hvox::block_index({ CHUNK_LENGTH - 1, 0, 0 });
        Block*     block       = &blocks.data[block_index];

        BlockIndex block_above_index = hvox::block_index({ CHUNK_LENGTH - 1, 1, 0 });
        Block*     block_above       = &blocks.data[block_above_index];

        // Only consider block if it is not covered above.
        if (is_solid(block) && !is_solid(block_above)) {
            // Ensure node exists for this block.
            ChunkNavmeshNode block_coord = {
                {CHUNK_LENGTH - 1, 0, 0},
                chunk_pos
            };
            ChunkNavmeshVertexDescriptor block_vertex
                = impl::get_vertex(chunk, block_coord);

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

    hmem::SharedResourceLock block_lock;
    auto                     blocks = chunk->blocks.get(block_lock);

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
    auto do_side_stitch_navigable_check = [&](memory::Handle<Chunk>& neighbour,
                                              BlockChunkPosition     this_offset,
                                              BlockChunkPosition     neighbour_offset,
                                              i64                    start,
                                              i64                    end) {
        hmem::SharedResourceLock neighbour_block_lock;
        auto neighbour_blocks = neighbour->blocks.get(neighbour_block_lock);

        BlockIndex this_block_index = hvox::block_index(this_offset);
        Block*     this_block       = &blocks.data[this_block_index];

        BlockIndex above_this_block_index
            = hvox::block_index(this_offset + BlockChunkPosition{ 0, 1, 0 });
        Block* above_this_block = &blocks.data[above_this_block_index];

        if (!is_solid(this_block) || is_solid(above_this_block)) return;

        ChunkNavmeshNode this_block_coord = { this_offset, chunk_pos };
        struct {
            ChunkNavmeshVertexDescriptor here, in_neighbour;
        } this_block_vertex = { impl::get_vertex(chunk, this_block_coord),
                                impl::get_vertex(neighbour, this_block_coord) };

        for (i64 y_off = start; y_off > end; --y_off) {
            BlockIndex above_candidate_index = hvox::block_index(
                static_cast<i64v3>(neighbour_offset) + i64v3{ 0, y_off, 0 }
            );
            Block* above_candidate_block
                = &neighbour_blocks.data[above_candidate_index];

            BlockIndex candidate_index = hvox::block_index(
                static_cast<i64v3>(neighbour_offset) + i64v3{ 0, y_off - 1, 0 }
            );
            Block* candidate_block = &neighbour_blocks.data[candidate_index];

            if (is_solid(candidate_block) && !is_solid(above_candidate_block)) {
                ChunkNavmeshNode candidate_block_coord = {
                    static_cast<i64v3>(neighbour_offset) + i64v3{0, y_off - 1, 0},
                    neighbour->position
                };
                struct {
                    ChunkNavmeshVertexDescriptor here, in_neighbour;
                } candidate_block_vertex
                    = { impl::get_vertex(chunk, candidate_block_coord),
                        impl::get_vertex(neighbour, candidate_block_coord) };

                {
                    hmem::UniqueResourceLock lock;
                    auto                     navmesh = chunk->navmesh.get(lock);

                    boost::add_edge(
                        this_block_vertex.here,
                        candidate_block_vertex.here,
                        navmesh.data->graph
                    );
                    boost::add_edge(
                        candidate_block_vertex.here,
                        this_block_vertex.here,
                        navmesh.data->graph
                    );
                }

                {
                    hmem::UniqueResourceLock lock;
                    auto                     navmesh = neighbour->navmesh.get(lock);

                    boost::add_edge(
                        this_block_vertex.in_neighbour,
                        candidate_block_vertex.in_neighbour,
                        navmesh.data->graph
                    );
                    boost::add_edge(
                        candidate_block_vertex.in_neighbour,
                        this_block_vertex.in_neighbour,
                        navmesh.data->graph
                    );
                }
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
            hmem::SharedResourceLock neighbour_block_lock;
            auto neighbour_blocks = neighbour->blocks.get(neighbour_block_lock);

            for (BlockChunkPositionCoord z = 0; z < CHUNK_LENGTH; ++z) {
                for (BlockChunkPositionCoord y = 1; y < CHUNK_LENGTH - 2; ++y) {
                    do_side_stitch_navigable_check(
                        neighbour, { 0, y, z }, { CHUNK_LENGTH - 1, y, z }, 2, -1
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
                    neighbour, { 0, 0, z }, { CHUNK_LENGTH - 1, 0, z }, 2, 0
                );

                // y == CHUNK_LENGTH - 2 - step down or across
                do_side_stitch_navigable_check(
                    neighbour,
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
                hmem::SharedResourceLock below_neighbour_block_lock;
                auto                     below_neighbour_blocks
                    = below_neighbour->blocks.get(below_neighbour_block_lock);

                for (BlockChunkPositionCoord z = 0; z < CHUNK_LENGTH; ++z) {
                    BlockIndex this_block_index = hvox::block_index({ 0, 0, z });
                    Block*     this_block       = &blocks.data[this_block_index];

                    BlockIndex above_this_block_index = hvox::block_index({ 0, 1, z });
                    Block*     above_this_block = &blocks.data[above_this_block_index];

                    if (!is_solid(this_block) || is_solid(above_this_block)) continue;

                    ChunkNavmeshNode this_block_coord = {
                        {0, 0, z},
                        chunk_pos
                    };

                    struct {
                        ChunkNavmeshVertexDescriptor here, in_below_neighbour;
                    } this_block_vertex
                        = { impl::get_vertex(chunk, this_block_coord),
                            impl::get_vertex(below_neighbour, this_block_coord) };

                    BlockIndex twice_above_candidate_index
                        = hvox::block_index({ CHUNK_LENGTH - 1, 1, z });
                    Block* twice_above_candidate_block
                        = &neighbour_blocks.data[twice_above_candidate_index];

                    BlockIndex above_candidate_index
                        = hvox::block_index({ CHUNK_LENGTH - 1, 0, z });
                    Block* above_candidate_block
                        = &neighbour_blocks.data[above_candidate_index];

                    BlockIndex candidate_index
                        = hvox::block_index({ CHUNK_LENGTH - 1, CHUNK_LENGTH - 1, z });
                    Block* candidate_block
                        = &below_neighbour_blocks.data[candidate_index];

                    if (is_solid(candidate_block) && !is_solid(above_candidate_block)
                        && !is_solid(twice_above_candidate_block))
                    {
                        ChunkNavmeshNode candidate_block_coord = {
                            {CHUNK_LENGTH - 1, CHUNK_LENGTH - 1, z},
                            below_neighbour->position
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor here, in_below_neighbour;
                        } candidate_block_vertex = {
                            impl::get_vertex(chunk, candidate_block_coord),
                            impl::get_vertex(below_neighbour, candidate_block_coord)
                        };

                        {
                            hmem::UniqueResourceLock lock;
                            auto                     navmesh = chunk->navmesh.get(lock);

                            boost::add_edge(
                                this_block_vertex.here,
                                candidate_block_vertex.here,
                                navmesh.data->graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.here,
                                this_block_vertex.here,
                                navmesh.data->graph
                            );
                        }

                        {
                            hmem::UniqueResourceLock lock;
                            auto navmesh = below_neighbour->navmesh.get(lock);

                            boost::add_edge(
                                this_block_vertex.in_below_neighbour,
                                candidate_block_vertex.in_below_neighbour,
                                navmesh.data->graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.in_below_neighbour,
                                this_block_vertex.in_below_neighbour,
                                navmesh.data->graph
                            );
                        }
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
            hmem::SharedResourceLock neighbour_block_lock;
            auto neighbour_blocks = neighbour->blocks.get(neighbour_block_lock);
            for (BlockChunkPositionCoord z = 0; z < CHUNK_LENGTH; ++z) {
                for (BlockChunkPositionCoord y = 1; y < CHUNK_LENGTH - 2; ++y) {
                    do_side_stitch_navigable_check(
                        neighbour, { CHUNK_LENGTH - 1, y, z }, { 0, y, z }, 2, -1
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
                    neighbour, { CHUNK_LENGTH - 1, 0, z }, { 0, 0, z }, 2, 0
                );

                // y == CHUNK_LENGTH - 2 - step down or across
                do_side_stitch_navigable_check(
                    neighbour,
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
                hmem::SharedResourceLock below_neighbour_block_lock;
                auto                     below_neighbour_blocks
                    = below_neighbour->blocks.get(below_neighbour_block_lock);

                for (BlockChunkPositionCoord z = 0; z < CHUNK_LENGTH; ++z) {
                    BlockIndex this_block_index
                        = hvox::block_index({ CHUNK_LENGTH - 1, 0, z });
                    Block* this_block = &blocks.data[this_block_index];

                    BlockIndex above_this_block_index
                        = hvox::block_index({ CHUNK_LENGTH - 1, 1, z });
                    Block* above_this_block = &blocks.data[above_this_block_index];

                    if (!is_solid(this_block) || is_solid(above_this_block)) continue;

                    ChunkNavmeshNode this_block_coord = {
                        {CHUNK_LENGTH - 1, 0, z},
                        chunk_pos
                    };

                    struct {
                        ChunkNavmeshVertexDescriptor here, in_below_neighbour;
                    } this_block_vertex
                        = { impl::get_vertex(chunk, this_block_coord),
                            impl::get_vertex(below_neighbour, this_block_coord) };

                    BlockIndex twice_above_candidate_index
                        = hvox::block_index({ 0, 1, z });
                    Block* twice_above_candidate_block
                        = &neighbour_blocks.data[twice_above_candidate_index];

                    BlockIndex above_candidate_index = hvox::block_index({ 0, 0, z });
                    Block*     above_candidate_block
                        = &neighbour_blocks.data[above_candidate_index];

                    BlockIndex candidate_index
                        = hvox::block_index({ 0, CHUNK_LENGTH - 1, z });
                    Block* candidate_block
                        = &below_neighbour_blocks.data[candidate_index];

                    if (is_solid(candidate_block) && !is_solid(above_candidate_block)
                        && !is_solid(twice_above_candidate_block))
                    {
                        ChunkNavmeshNode candidate_block_coord = {
                            {0, CHUNK_LENGTH - 1, z},
                            below_neighbour->position
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor here, in_below_neighbour;
                        } candidate_block_vertex = {
                            impl::get_vertex(chunk, candidate_block_coord),
                            impl::get_vertex(below_neighbour, candidate_block_coord)
                        };

                        {
                            hmem::UniqueResourceLock lock;
                            auto                     navmesh = chunk->navmesh.get(lock);

                            boost::add_edge(
                                this_block_vertex.here,
                                candidate_block_vertex.here,
                                navmesh.data->graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.here,
                                this_block_vertex.here,
                                navmesh.data->graph
                            );
                        }

                        {
                            hmem::UniqueResourceLock lock;
                            auto navmesh = below_neighbour->navmesh.get(lock);

                            boost::add_edge(
                                this_block_vertex.in_below_neighbour,
                                candidate_block_vertex.in_below_neighbour,
                                navmesh.data->graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.in_below_neighbour,
                                this_block_vertex.in_below_neighbour,
                                navmesh.data->graph
                            );
                        }
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
            hmem::SharedResourceLock neighbour_block_lock;
            auto neighbour_blocks = neighbour->blocks.get(neighbour_block_lock);
            for (BlockChunkPositionCoord x = 0; x < CHUNK_LENGTH; ++x) {
                for (BlockChunkPositionCoord y = 1; y < CHUNK_LENGTH - 2; ++y) {
                    do_side_stitch_navigable_check(
                        neighbour, { x, y, CHUNK_LENGTH - 1 }, { x, y, 0 }, 2, -1
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
                    neighbour, { x, 0, CHUNK_LENGTH - 1 }, { x, 0, 0 }, 2, 0
                );

                // y == CHUNK_LENGTH - 2 - step down or across
                do_side_stitch_navigable_check(
                    neighbour,
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
                hmem::SharedResourceLock below_neighbour_block_lock;
                auto                     below_neighbour_blocks
                    = below_neighbour->blocks.get(below_neighbour_block_lock);

                for (BlockChunkPositionCoord x = 0; x < CHUNK_LENGTH; ++x) {
                    BlockIndex this_block_index
                        = hvox::block_index({ x, 0, CHUNK_LENGTH - 1 });
                    Block* this_block = &blocks.data[this_block_index];

                    BlockIndex above_this_block_index
                        = hvox::block_index({ x, 1, CHUNK_LENGTH - 1 });
                    Block* above_this_block = &blocks.data[above_this_block_index];

                    if (!is_solid(this_block) || is_solid(above_this_block)) continue;

                    ChunkNavmeshNode this_block_coord = {
                        {x, 0, CHUNK_LENGTH - 1},
                        chunk_pos
                    };

                    struct {
                        ChunkNavmeshVertexDescriptor here, in_below_neighbour;
                    } this_block_vertex
                        = { impl::get_vertex(chunk, this_block_coord),
                            impl::get_vertex(below_neighbour, this_block_coord) };

                    BlockIndex twice_above_candidate_index
                        = hvox::block_index({ x, 1, 0 });
                    Block* twice_above_candidate_block
                        = &neighbour_blocks.data[twice_above_candidate_index];

                    BlockIndex above_candidate_index = hvox::block_index({ x, 0, 0 });
                    Block*     above_candidate_block
                        = &neighbour_blocks.data[above_candidate_index];

                    BlockIndex candidate_index
                        = hvox::block_index({ x, CHUNK_LENGTH - 1, 0 });
                    Block* candidate_block
                        = &below_neighbour_blocks.data[candidate_index];

                    if (is_solid(candidate_block) && !is_solid(above_candidate_block)
                        && !is_solid(twice_above_candidate_block))
                    {
                        ChunkNavmeshNode candidate_block_coord = {
                            {x, CHUNK_LENGTH - 1, 0},
                            below_neighbour->position
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor here, in_below_neighbour;
                        } candidate_block_vertex = {
                            impl::get_vertex(chunk, candidate_block_coord),
                            impl::get_vertex(below_neighbour, candidate_block_coord)
                        };

                        {
                            hmem::UniqueResourceLock lock;
                            auto                     navmesh = chunk->navmesh.get(lock);

                            boost::add_edge(
                                this_block_vertex.here,
                                candidate_block_vertex.here,
                                navmesh.data->graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.here,
                                this_block_vertex.here,
                                navmesh.data->graph
                            );
                        }

                        {
                            hmem::UniqueResourceLock lock;
                            auto navmesh = below_neighbour->navmesh.get(lock);

                            boost::add_edge(
                                this_block_vertex.in_below_neighbour,
                                candidate_block_vertex.in_below_neighbour,
                                navmesh.data->graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.in_below_neighbour,
                                this_block_vertex.in_below_neighbour,
                                navmesh.data->graph
                            );
                        }
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
            hmem::SharedResourceLock neighbour_block_lock;
            auto neighbour_blocks = neighbour->blocks.get(neighbour_block_lock);
            for (BlockChunkPositionCoord x = 0; x < CHUNK_LENGTH; ++x) {
                for (BlockChunkPositionCoord y = 1; y < CHUNK_LENGTH - 2; ++y) {
                    do_side_stitch_navigable_check(
                        neighbour, { x, y, 0 }, { x, y, CHUNK_LENGTH - 1 }, 2, -1
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
                    neighbour, { x, 0, 0 }, { x, 0, CHUNK_LENGTH - 1 }, 2, 0
                );

                // y == CHUNK_LENGTH - 2 - step down or across
                do_side_stitch_navigable_check(
                    neighbour,
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
                hmem::SharedResourceLock below_neighbour_block_lock;
                auto                     below_neighbour_blocks
                    = below_neighbour->blocks.get(below_neighbour_block_lock);

                for (BlockChunkPositionCoord x = 0; x < CHUNK_LENGTH; ++x) {
                    BlockIndex this_block_index = hvox::block_index({ x, 0, 0 });
                    Block*     this_block       = &blocks.data[this_block_index];

                    BlockIndex above_this_block_index = hvox::block_index({ x, 1, 0 });
                    Block*     above_this_block = &blocks.data[above_this_block_index];

                    if (!is_solid(this_block) || is_solid(above_this_block)) continue;

                    ChunkNavmeshNode this_block_coord = {
                        {x, 0, 0},
                        chunk_pos
                    };

                    struct {
                        ChunkNavmeshVertexDescriptor here, in_below_neighbour;
                    } this_block_vertex
                        = { impl::get_vertex(chunk, this_block_coord),
                            impl::get_vertex(below_neighbour, this_block_coord) };

                    BlockIndex twice_above_candidate_index
                        = hvox::block_index({ x, 1, CHUNK_LENGTH - 1 });
                    Block* twice_above_candidate_block
                        = &neighbour_blocks.data[twice_above_candidate_index];

                    BlockIndex above_candidate_index
                        = hvox::block_index({ x, 0, CHUNK_LENGTH - 1 });
                    Block* above_candidate_block
                        = &neighbour_blocks.data[above_candidate_index];

                    BlockIndex candidate_index
                        = hvox::block_index({ x, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1 });
                    Block* candidate_block
                        = &below_neighbour_blocks.data[candidate_index];

                    if (is_solid(candidate_block) && !is_solid(above_candidate_block)
                        && !is_solid(twice_above_candidate_block))
                    {
                        ChunkNavmeshNode candidate_block_coord = {
                            {x, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1},
                            below_neighbour->position
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor here, in_below_neighbour;
                        } candidate_block_vertex = {
                            impl::get_vertex(chunk, candidate_block_coord),
                            impl::get_vertex(below_neighbour, candidate_block_coord)
                        };

                        {
                            hmem::UniqueResourceLock lock;
                            auto                     navmesh = chunk->navmesh.get(lock);

                            boost::add_edge(
                                this_block_vertex.here,
                                candidate_block_vertex.here,
                                navmesh.data->graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.here,
                                this_block_vertex.here,
                                navmesh.data->graph
                            );
                        }

                        {
                            hmem::UniqueResourceLock lock;
                            auto navmesh = below_neighbour->navmesh.get(lock);

                            boost::add_edge(
                                this_block_vertex.in_below_neighbour,
                                candidate_block_vertex.in_below_neighbour,
                                navmesh.data->graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.in_below_neighbour,
                                this_block_vertex.in_below_neighbour,
                                navmesh.data->graph
                            );
                        }
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
            hmem::SharedResourceLock neighbour_block_lock;
            auto neighbour_blocks = neighbour->blocks.get(neighbour_block_lock);
            for (BlockChunkPositionCoord x = 1; x < CHUNK_LENGTH - 1; ++x) {
                for (BlockChunkPositionCoord z = 1; z < CHUNK_LENGTH - 1; ++z) {
                    BlockIndex this_block_index
                        = hvox::block_index({ x, CHUNK_LENGTH - 1, z });
                    Block* this_block = &blocks.data[this_block_index];

                    BlockIndex neighbour_block_index = hvox::block_index({ x, 0, z });
                    Block*     neighbour_block
                        = &neighbour_blocks.data[neighbour_block_index];

                    BlockIndex above_neighbour_block_index
                        = hvox::block_index({ x, 1, z });
                    Block* above_neighbour_block
                        = &neighbour_blocks.data[above_neighbour_block_index];

                    if (!is_solid(this_block) || is_solid(neighbour_block)) continue;

                    // Ensure node exists for this block.
                    ChunkNavmeshNode this_block_coord = {
                        {x, CHUNK_LENGTH - 1, z},
                        chunk_pos
                    };

                    struct {
                        ChunkNavmeshVertexDescriptor here, in_neighbour;
                    } this_block_vertex
                        = { impl::get_vertex(chunk, this_block_coord),
                            impl::get_vertex(neighbour, this_block_coord) };

                    // Up
                    if (!is_solid(above_neighbour_block)) {
                        // Left
                        BlockIndex left_of_neighbour_block_index
                            = hvox::block_index({ x - 1, 0, z });
                        Block* left_of_neighbour_block
                            = &neighbour_blocks.data[left_of_neighbour_block_index];

                        BlockIndex above_and_left_of_neighbour_block_index
                            = hvox::block_index({ x - 1, 1, z });
                        Block* above_and_left_of_neighbour_block
                            = &neighbour_blocks
                                   .data[above_and_left_of_neighbour_block_index];

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
                            } left_of_neighbour_block_vertex = {
                                impl::get_vertex(chunk, left_of_neighbour_block_coord),
                                impl::get_vertex(
                                    neighbour, left_of_neighbour_block_coord
                                )
                            };

                            {
                                hmem::UniqueResourceLock lock;
                                auto navmesh = chunk->navmesh.get(lock);

                                boost::add_edge(
                                    this_block_vertex.here,
                                    left_of_neighbour_block_vertex.here,
                                    navmesh.data->graph
                                );
                                boost::add_edge(
                                    left_of_neighbour_block_vertex.here,
                                    this_block_vertex.here,
                                    navmesh.data->graph
                                );
                            }

                            {
                                hmem::UniqueResourceLock lock;
                                auto navmesh = neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    this_block_vertex.in_neighbour,
                                    left_of_neighbour_block_vertex.in_neighbour,
                                    navmesh.data->graph
                                );
                                boost::add_edge(
                                    left_of_neighbour_block_vertex.in_neighbour,
                                    this_block_vertex.in_neighbour,
                                    navmesh.data->graph
                                );
                            }
                        }

                        // Right
                        BlockIndex right_of_neighbour_block_index
                            = hvox::block_index({ x + 1, 0, z });
                        Block* right_of_neighbour_block
                            = &neighbour_blocks.data[right_of_neighbour_block_index];

                        BlockIndex above_and_right_of_neighbour_block_index
                            = hvox::block_index({ x + 1, 1, z });
                        Block* above_and_right_of_neighbour_block
                            = &neighbour_blocks
                                   .data[above_and_right_of_neighbour_block_index];

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
                            } right_of_neighbour_block_vertex = {
                                impl::get_vertex(chunk, right_of_neighbour_block_coord),
                                impl::get_vertex(
                                    neighbour, right_of_neighbour_block_coord
                                )
                            };

                            {
                                hmem::UniqueResourceLock lock;
                                auto navmesh = chunk->navmesh.get(lock);

                                boost::add_edge(
                                    this_block_vertex.here,
                                    right_of_neighbour_block_vertex.here,
                                    navmesh.data->graph
                                );
                                boost::add_edge(
                                    right_of_neighbour_block_vertex.here,
                                    this_block_vertex.here,
                                    navmesh.data->graph
                                );
                            }

                            {
                                hmem::UniqueResourceLock lock;
                                auto navmesh = neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    this_block_vertex.in_neighbour,
                                    right_of_neighbour_block_vertex.in_neighbour,
                                    navmesh.data->graph
                                );
                                boost::add_edge(
                                    right_of_neighbour_block_vertex.in_neighbour,
                                    this_block_vertex.in_neighbour,
                                    navmesh.data->graph
                                );
                            }
                        }

                        // Front
                        BlockIndex front_of_neighbour_block_index
                            = hvox::block_index({ x, 0, z + 1 });
                        Block* front_of_neighbour_block
                            = &neighbour_blocks.data[front_of_neighbour_block_index];

                        BlockIndex above_and_front_of_neighbour_block_index
                            = hvox::block_index({ x, 1, z + 1 });
                        Block* above_and_front_of_neighbour_block
                            = &neighbour_blocks
                                   .data[above_and_front_of_neighbour_block_index];

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
                            } front_of_neighbour_block_vertex = {
                                impl::get_vertex(chunk, front_of_neighbour_block_coord),
                                impl::get_vertex(
                                    neighbour, front_of_neighbour_block_coord
                                )
                            };

                            {
                                hmem::UniqueResourceLock lock;
                                auto navmesh = chunk->navmesh.get(lock);

                                boost::add_edge(
                                    this_block_vertex.here,
                                    front_of_neighbour_block_vertex.here,
                                    navmesh.data->graph
                                );
                                boost::add_edge(
                                    front_of_neighbour_block_vertex.here,
                                    this_block_vertex.here,
                                    navmesh.data->graph
                                );
                            }

                            {
                                hmem::UniqueResourceLock lock;
                                auto navmesh = neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    this_block_vertex.in_neighbour,
                                    front_of_neighbour_block_vertex.in_neighbour,
                                    navmesh.data->graph
                                );
                                boost::add_edge(
                                    front_of_neighbour_block_vertex.in_neighbour,
                                    this_block_vertex.in_neighbour,
                                    navmesh.data->graph
                                );
                            }
                        }

                        // Back
                        BlockIndex back_of_neighbour_block_index
                            = hvox::block_index({ x, 0, z - 1 });
                        Block* back_of_neighbour_block
                            = &neighbour_blocks.data[back_of_neighbour_block_index];

                        BlockIndex above_and_back_of_neighbour_block_index
                            = hvox::block_index({ x, 1, z - 1 });
                        Block* above_and_back_of_neighbour_block
                            = &neighbour_blocks
                                   .data[above_and_back_of_neighbour_block_index];

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
                            } back_of_neighbour_block_vertex = {
                                impl::get_vertex(chunk, back_of_neighbour_block_coord),
                                impl::get_vertex(
                                    neighbour, back_of_neighbour_block_coord
                                )
                            };

                            {
                                hmem::UniqueResourceLock lock;
                                auto navmesh = chunk->navmesh.get(lock);

                                boost::add_edge(
                                    this_block_vertex.here,
                                    back_of_neighbour_block_vertex.here,
                                    navmesh.data->graph
                                );
                                boost::add_edge(
                                    back_of_neighbour_block_vertex.here,
                                    this_block_vertex.here,
                                    navmesh.data->graph
                                );
                            }

                            {
                                hmem::UniqueResourceLock lock;
                                auto navmesh = neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    this_block_vertex.in_neighbour,
                                    back_of_neighbour_block_vertex.in_neighbour,
                                    navmesh.data->graph
                                );
                                boost::add_edge(
                                    back_of_neighbour_block_vertex.in_neighbour,
                                    this_block_vertex.in_neighbour,
                                    navmesh.data->graph
                                );
                            }
                        }
                    }

                    ///// Across and Down
                    // Left
                    BlockIndex left_of_this_block_index
                        = hvox::block_index({ x - 1, CHUNK_LENGTH - 1, z });
                    Block* left_of_this_block = &blocks.data[left_of_this_block_index];

                    BlockIndex left_of_and_below_this_block_index
                        = hvox::block_index({ x - 1, CHUNK_LENGTH - 2, z });
                    Block* left_of_and_below_this_block
                        = &blocks.data[left_of_and_below_this_block_index];

                    BlockIndex left_of_neighbour_block_index
                        = hvox::block_index({ x - 1, 0, z });
                    Block* left_of_neighbour_block
                        = &neighbour_blocks.data[left_of_neighbour_block_index];

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
                            = impl::get_vertex(chunk, left_of_this_block_coord);

                        {
                            hmem::UniqueResourceLock lock;
                            auto                     navmesh = chunk->navmesh.get(lock);

                            boost::add_edge(
                                this_block_vertex.here,
                                left_of_this_block_vertex,
                                navmesh.data->graph
                            );
                            boost::add_edge(
                                left_of_this_block_vertex,
                                this_block_vertex.here,
                                navmesh.data->graph
                            );
                        }
                        // Down
                    } else if (is_solid(left_of_and_below_this_block) && !is_solid(left_of_this_block) && !is_solid(left_of_neighbour_block))
                    {
                        ChunkNavmeshNode left_of_and_below_this_block_coord = {
                            {x - 1, CHUNK_LENGTH - 2, z},
                            chunk_pos
                        };

                        // Ensure node exists for this block.
                        ChunkNavmeshVertexDescriptor left_of_and_below_this_block_vertex
                            = impl::get_vertex(
                                chunk, left_of_and_below_this_block_coord
                            );

                        {
                            hmem::UniqueResourceLock lock;
                            auto                     navmesh = chunk->navmesh.get(lock);

                            boost::add_edge(
                                this_block_vertex.here,
                                left_of_and_below_this_block_vertex,
                                navmesh.data->graph
                            );
                            boost::add_edge(
                                left_of_and_below_this_block_vertex,
                                this_block_vertex.here,
                                navmesh.data->graph
                            );
                        }
                    }

                    // Right
                    BlockIndex right_of_this_block_index
                        = hvox::block_index({ x + 1, CHUNK_LENGTH - 1, z });
                    Block* right_of_this_block
                        = &blocks.data[right_of_this_block_index];

                    BlockIndex right_of_and_below_this_block_index
                        = hvox::block_index({ x + 1, CHUNK_LENGTH - 2, z });
                    Block* right_of_and_below_this_block
                        = &blocks.data[right_of_and_below_this_block_index];

                    BlockIndex right_of_neighbour_block_index
                        = hvox::block_index({ x + 1, 0, z });
                    Block* right_of_neighbour_block
                        = &neighbour_blocks.data[right_of_neighbour_block_index];

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
                            = impl::get_vertex(chunk, right_of_this_block_coord);

                        {
                            hmem::UniqueResourceLock lock;
                            auto                     navmesh = chunk->navmesh.get(lock);

                            boost::add_edge(
                                this_block_vertex.here,
                                right_of_this_block_vertex,
                                navmesh.data->graph
                            );
                            boost::add_edge(
                                right_of_this_block_vertex,
                                this_block_vertex.here,
                                navmesh.data->graph
                            );
                        }
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
                                chunk, right_of_and_below_this_block_coord
                            );

                        {
                            hmem::UniqueResourceLock lock;
                            auto                     navmesh = chunk->navmesh.get(lock);

                            boost::add_edge(
                                this_block_vertex.here,
                                right_of_and_below_this_block_vertex,
                                navmesh.data->graph
                            );
                            boost::add_edge(
                                right_of_and_below_this_block_vertex,
                                this_block_vertex.here,
                                navmesh.data->graph
                            );
                        }
                    }

                    // Front
                    BlockIndex front_of_this_block_index
                        = hvox::block_index({ x, CHUNK_LENGTH - 1, z + 1 });
                    Block* front_of_this_block
                        = &blocks.data[front_of_this_block_index];

                    BlockIndex front_of_and_below_this_block_index
                        = hvox::block_index({ x, CHUNK_LENGTH - 2, z + 1 });
                    Block* front_of_and_below_this_block
                        = &blocks.data[front_of_and_below_this_block_index];

                    BlockIndex front_of_neighbour_block_index
                        = hvox::block_index({ x, 0, z + 1 });
                    Block* front_of_neighbour_block
                        = &neighbour_blocks.data[front_of_neighbour_block_index];

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
                            = impl::get_vertex(chunk, front_of_this_block_coord);

                        {
                            hmem::UniqueResourceLock lock;
                            auto                     navmesh = chunk->navmesh.get(lock);

                            boost::add_edge(
                                this_block_vertex.here,
                                front_of_this_block_vertex,
                                navmesh.data->graph
                            );
                            boost::add_edge(
                                front_of_this_block_vertex,
                                this_block_vertex.here,
                                navmesh.data->graph
                            );
                        }
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
                                chunk, front_of_and_below_this_block_coord
                            );

                        {
                            hmem::UniqueResourceLock lock;
                            auto                     navmesh = chunk->navmesh.get(lock);

                            boost::add_edge(
                                this_block_vertex.here,
                                front_of_and_below_this_block_vertex,
                                navmesh.data->graph
                            );
                            boost::add_edge(
                                front_of_and_below_this_block_vertex,
                                this_block_vertex.here,
                                navmesh.data->graph
                            );
                        }
                    }

                    // Back
                    BlockIndex back_of_this_block_index
                        = hvox::block_index({ x, CHUNK_LENGTH - 1, z - 1 });
                    Block* back_of_this_block = &blocks.data[back_of_this_block_index];

                    BlockIndex back_of_and_below_this_block_index
                        = hvox::block_index({ x, CHUNK_LENGTH - 2, z - 1 });
                    Block* back_of_and_below_this_block
                        = &blocks.data[back_of_and_below_this_block_index];

                    BlockIndex back_of_neighbour_block_index
                        = hvox::block_index({ x, 0, z - 1 });
                    Block* back_of_neighbour_block
                        = &neighbour_blocks.data[back_of_neighbour_block_index];

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
                            = impl::get_vertex(chunk, back_of_this_block_coord);

                        {
                            hmem::UniqueResourceLock lock;
                            auto                     navmesh = chunk->navmesh.get(lock);

                            boost::add_edge(
                                this_block_vertex.here,
                                back_of_this_block_vertex,
                                navmesh.data->graph
                            );
                            boost::add_edge(
                                back_of_this_block_vertex,
                                this_block_vertex.here,
                                navmesh.data->graph
                            );
                        }
                        // Down
                    } else if (is_solid(back_of_and_below_this_block) && !is_solid(back_of_this_block) && !is_solid(back_of_neighbour_block))
                    {
                        ChunkNavmeshNode back_of_and_below_this_block_coord = {
                            {x, CHUNK_LENGTH - 2, z - 1},
                            chunk_pos
                        };

                        // Ensure node exists for this block.
                        ChunkNavmeshVertexDescriptor back_of_and_below_this_block_vertex
                            = impl::get_vertex(
                                chunk, back_of_and_below_this_block_coord
                            );

                        {
                            hmem::UniqueResourceLock lock;
                            auto                     navmesh = chunk->navmesh.get(lock);

                            boost::add_edge(
                                this_block_vertex.here,
                                back_of_and_below_this_block_vertex,
                                navmesh.data->graph
                            );
                            boost::add_edge(
                                back_of_and_below_this_block_vertex,
                                this_block_vertex.here,
                                navmesh.data->graph
                            );
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
                    hmem::SharedResourceLock left_of_neighbour_block_lock;
                    auto                     left_of_neighbour_blocks
                        = left_of_neighbour->blocks.get(left_of_neighbour_block_lock);

                    for (BlockChunkPositionCoord z = 0; z < CHUNK_LENGTH; ++z) {
                        BlockIndex left_of_neighbour_block_index
                            = hvox::block_index({ CHUNK_LENGTH - 1, 0, z });
                        Block* left_of_neighbour_block
                            = &left_of_neighbour_blocks
                                   .data[left_of_neighbour_block_index];

                        BlockIndex above_left_of_neighbour_block_index
                            = hvox::block_index({ CHUNK_LENGTH - 1, 1, z });
                        Block* above_left_of_neighbour_block
                            = &left_of_neighbour_blocks
                                   .data[above_left_of_neighbour_block_index];

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
                            = { impl::get_vertex(chunk, left_of_neighbour_block_coord),
                                impl::get_vertex(
                                    left_of_neighbour, left_of_neighbour_block_coord
                                ) };

                        BlockIndex twice_above_candidate_index
                            = hvox::block_index({ 0, 1, z });
                        Block* twice_above_candidate_block
                            = &neighbour_blocks.data[twice_above_candidate_index];

                        BlockIndex above_candidate_index
                            = hvox::block_index({ 0, 0, z });
                        Block* above_candidate_block
                            = &neighbour_blocks.data[above_candidate_index];

                        BlockIndex candidate_index
                            = hvox::block_index({ 0, CHUNK_LENGTH - 1, z });
                        Block* candidate_block = &blocks.data[candidate_index];

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
                            } candidate_block_vertex
                                = { impl::get_vertex(chunk, candidate_block_coord),
                                    impl::get_vertex(
                                        left_of_neighbour, candidate_block_coord
                                    ) };

                            {
                                hmem::UniqueResourceLock lock;
                                auto navmesh = chunk->navmesh.get(lock);

                                boost::add_edge(
                                    left_of_neighbour_block_vertex.here,
                                    candidate_block_vertex.here,
                                    navmesh.data->graph
                                );
                                boost::add_edge(
                                    candidate_block_vertex.here,
                                    left_of_neighbour_block_vertex.here,
                                    navmesh.data->graph
                                );
                            }

                            {
                                hmem::UniqueResourceLock lock;
                                auto navmesh = left_of_neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    left_of_neighbour_block_vertex.in_left_of_neighbour,
                                    candidate_block_vertex.in_left_of_neighbour,
                                    navmesh.data->graph
                                );
                                boost::add_edge(
                                    candidate_block_vertex.in_left_of_neighbour,
                                    left_of_neighbour_block_vertex.in_left_of_neighbour,
                                    navmesh.data->graph
                                );
                            }
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
                    hmem::SharedResourceLock right_of_neighbour_block_lock;
                    auto                     right_of_neighbour_blocks
                        = right_of_neighbour->blocks.get(right_of_neighbour_block_lock);

                    for (BlockChunkPositionCoord z = 0; z < CHUNK_LENGTH; ++z) {
                        BlockIndex right_of_neighbour_block_index
                            = hvox::block_index({ 0, 0, z });
                        Block* right_of_neighbour_block
                            = &right_of_neighbour_blocks
                                   .data[right_of_neighbour_block_index];

                        BlockIndex above_right_of_neighbour_block_index
                            = hvox::block_index({ 0, 1, z });
                        Block* above_right_of_neighbour_block
                            = &right_of_neighbour_blocks
                                   .data[above_right_of_neighbour_block_index];

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
                            = { impl::get_vertex(chunk, right_of_neighbour_block_coord),
                                impl::get_vertex(
                                    right_of_neighbour, right_of_neighbour_block_coord
                                ) };

                        BlockIndex twice_above_candidate_index
                            = hvox::block_index({ CHUNK_LENGTH - 1, 1, z });
                        Block* twice_above_candidate_block
                            = &neighbour_blocks.data[twice_above_candidate_index];

                        BlockIndex above_candidate_index
                            = hvox::block_index({ CHUNK_LENGTH - 1, 0, z });
                        Block* above_candidate_block
                            = &neighbour_blocks.data[above_candidate_index];

                        BlockIndex candidate_index = hvox::block_index(
                            { CHUNK_LENGTH - 1, CHUNK_LENGTH - 1, z }
                        );
                        Block* candidate_block = &blocks.data[candidate_index];

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
                            } candidate_block_vertex
                                = { impl::get_vertex(chunk, candidate_block_coord),
                                    impl::get_vertex(
                                        right_of_neighbour, candidate_block_coord
                                    ) };

                            {
                                hmem::UniqueResourceLock lock;
                                auto navmesh = chunk->navmesh.get(lock);

                                boost::add_edge(
                                    right_of_neighbour_block_vertex.here,
                                    candidate_block_vertex.here,
                                    navmesh.data->graph
                                );
                                boost::add_edge(
                                    candidate_block_vertex.here,
                                    right_of_neighbour_block_vertex.here,
                                    navmesh.data->graph
                                );
                            }

                            {
                                hmem::UniqueResourceLock lock;
                                auto navmesh = right_of_neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    right_of_neighbour_block_vertex
                                        .in_right_of_neighbour,
                                    candidate_block_vertex.in_right_of_neighbour,
                                    navmesh.data->graph
                                );
                                boost::add_edge(
                                    candidate_block_vertex.in_right_of_neighbour,
                                    right_of_neighbour_block_vertex
                                        .in_right_of_neighbour,
                                    navmesh.data->graph
                                );
                            }
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
                    hmem::SharedResourceLock front_of_neighbour_block_lock;
                    auto                     front_of_neighbour_blocks
                        = front_of_neighbour->blocks.get(front_of_neighbour_block_lock);

                    for (BlockChunkPositionCoord x = 0; x < CHUNK_LENGTH; ++x) {
                        BlockIndex front_of_neighbour_block_index
                            = hvox::block_index({ x, 0, 0 });
                        Block* front_of_neighbour_block
                            = &front_of_neighbour_blocks
                                   .data[front_of_neighbour_block_index];

                        BlockIndex above_front_of_neighbour_block_index
                            = hvox::block_index({ x, 1, 0 });
                        Block* above_front_of_neighbour_block
                            = &front_of_neighbour_blocks
                                   .data[above_front_of_neighbour_block_index];

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
                            = { impl::get_vertex(chunk, front_of_neighbour_block_coord),
                                impl::get_vertex(
                                    front_of_neighbour, front_of_neighbour_block_coord
                                ) };

                        BlockIndex twice_above_candidate_index
                            = hvox::block_index({ x, 1, CHUNK_LENGTH - 1 });
                        Block* twice_above_candidate_block
                            = &neighbour_blocks.data[twice_above_candidate_index];

                        BlockIndex above_candidate_index
                            = hvox::block_index({ x, 0, CHUNK_LENGTH - 1 });
                        Block* above_candidate_block
                            = &neighbour_blocks.data[above_candidate_index];

                        BlockIndex candidate_index = hvox::block_index(
                            { x, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1 }
                        );
                        Block* candidate_block = &blocks.data[candidate_index];

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
                            } candidate_block_vertex
                                = { impl::get_vertex(chunk, candidate_block_coord),
                                    impl::get_vertex(
                                        front_of_neighbour, candidate_block_coord
                                    ) };

                            {
                                hmem::UniqueResourceLock lock;
                                auto navmesh = chunk->navmesh.get(lock);

                                boost::add_edge(
                                    front_of_neighbour_block_vertex.here,
                                    candidate_block_vertex.here,
                                    navmesh.data->graph
                                );
                                boost::add_edge(
                                    candidate_block_vertex.here,
                                    front_of_neighbour_block_vertex.here,
                                    navmesh.data->graph
                                );
                            }

                            {
                                hmem::UniqueResourceLock lock;
                                auto navmesh = front_of_neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    front_of_neighbour_block_vertex
                                        .in_front_of_neighbour,
                                    candidate_block_vertex.in_front_of_neighbour,
                                    navmesh.data->graph
                                );
                                boost::add_edge(
                                    candidate_block_vertex.in_front_of_neighbour,
                                    front_of_neighbour_block_vertex
                                        .in_front_of_neighbour,
                                    navmesh.data->graph
                                );
                            }
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
                    hmem::SharedResourceLock back_of_neighbour_block_lock;
                    auto                     back_of_neighbour_blocks
                        = back_of_neighbour->blocks.get(back_of_neighbour_block_lock);

                    for (BlockChunkPositionCoord x = 0; x < CHUNK_LENGTH; ++x) {
                        BlockIndex back_of_neighbour_block_index
                            = hvox::block_index({ x, 0, CHUNK_LENGTH - 1 });
                        Block* back_of_neighbour_block
                            = &back_of_neighbour_blocks
                                   .data[back_of_neighbour_block_index];

                        BlockIndex above_back_of_neighbour_block_index
                            = hvox::block_index({ x, 1, CHUNK_LENGTH - 1 });
                        Block* above_back_of_neighbour_block
                            = &back_of_neighbour_blocks
                                   .data[above_back_of_neighbour_block_index];

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
                            = { impl::get_vertex(chunk, back_of_neighbour_block_coord),
                                impl::get_vertex(
                                    back_of_neighbour, back_of_neighbour_block_coord
                                ) };

                        BlockIndex twice_above_candidate_index
                            = hvox::block_index({ x, 1, 0 });
                        Block* twice_above_candidate_block
                            = &neighbour_blocks.data[twice_above_candidate_index];

                        BlockIndex above_candidate_index
                            = hvox::block_index({ x, 0, 0 });
                        Block* above_candidate_block
                            = &neighbour_blocks.data[above_candidate_index];

                        BlockIndex candidate_index
                            = hvox::block_index({ x, CHUNK_LENGTH - 1, 0 });
                        Block* candidate_block = &blocks.data[candidate_index];

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
                            } candidate_block_vertex
                                = { impl::get_vertex(chunk, candidate_block_coord),
                                    impl::get_vertex(
                                        back_of_neighbour, candidate_block_coord
                                    ) };

                            {
                                hmem::UniqueResourceLock lock;
                                auto navmesh = chunk->navmesh.get(lock);

                                boost::add_edge(
                                    back_of_neighbour_block_vertex.here,
                                    candidate_block_vertex.here,
                                    navmesh.data->graph
                                );
                                boost::add_edge(
                                    candidate_block_vertex.here,
                                    back_of_neighbour_block_vertex.here,
                                    navmesh.data->graph
                                );
                            }

                            {
                                hmem::UniqueResourceLock lock;
                                auto navmesh = back_of_neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    back_of_neighbour_block_vertex.in_back_of_neighbour,
                                    candidate_block_vertex.in_back_of_neighbour,
                                    navmesh.data->graph
                                );
                                boost::add_edge(
                                    candidate_block_vertex.in_back_of_neighbour,
                                    back_of_neighbour_block_vertex.in_back_of_neighbour,
                                    navmesh.data->graph
                                );
                            }
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
                    hmem::SharedResourceLock left_neighbour_block_lock;
                    auto                     left_neighbour_blocks
                        = left_neighbour->blocks.get(left_neighbour_block_lock);
                    hmem::SharedResourceLock above_left_neighbour_block_lock;
                    auto above_left_neighbour_blocks = above_left_neighbour->blocks.get(
                        above_left_neighbour_block_lock
                    );

                    // Step up from y == CHUNK_LENGTH - 2
                    for (BlockChunkPositionCoord z = 0; z < CHUNK_LENGTH; ++z) {
                        BlockIndex this_block_index
                            = hvox::block_index({ 0, CHUNK_LENGTH - 2, z });
                        Block* this_block = &blocks.data[this_block_index];

                        BlockIndex above_this_block_index
                            = hvox::block_index({ 0, CHUNK_LENGTH - 1, z });
                        Block* above_this_block = &blocks.data[above_this_block_index];

                        BlockIndex twice_above_this_block_index
                            = hvox::block_index({ 0, 0, z });
                        Block* twice_above_this_block
                            = &neighbour_blocks.data[twice_above_this_block_index];

                        if (!is_solid(this_block) || is_solid(above_this_block)
                            || is_solid(twice_above_this_block))
                            continue;

                        ChunkNavmeshNode this_block_coord = {
                            {0, CHUNK_LENGTH - 2, z},
                            chunk_pos
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor here, in_left_neighbour;
                        } this_block_vertex
                            = { impl::get_vertex(chunk, this_block_coord),
                                impl::get_vertex(left_neighbour, this_block_coord) };

                        BlockIndex above_candidate_index
                            = hvox::block_index({ CHUNK_LENGTH - 1, 0, z });
                        Block* above_candidate_block
                            = &above_left_neighbour_blocks.data[above_candidate_index];

                        BlockIndex candidate_index = hvox::block_index(
                            { CHUNK_LENGTH - 1, CHUNK_LENGTH - 1, z }
                        );
                        Block* candidate_block
                            = &left_neighbour_blocks.data[candidate_index];

                        if (!is_solid(candidate_block)
                            || is_solid(above_candidate_block))
                            continue;

                        ChunkNavmeshNode candidate_block_coord = {
                            {CHUNK_LENGTH - 1, CHUNK_LENGTH - 1, z},
                            left_neighbour->position
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor here, in_left_neighbour;
                        } candidate_block_vertex = {
                            impl::get_vertex(chunk, candidate_block_coord),
                            impl::get_vertex(left_neighbour, candidate_block_coord)
                        };

                        {
                            hmem::UniqueResourceLock lock;
                            auto                     navmesh = chunk->navmesh.get(lock);

                            boost::add_edge(
                                this_block_vertex.here,
                                candidate_block_vertex.here,
                                navmesh.data->graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.here,
                                this_block_vertex.here,
                                navmesh.data->graph
                            );
                        }

                        {
                            hmem::UniqueResourceLock lock;
                            auto navmesh = left_neighbour->navmesh.get(lock);

                            boost::add_edge(
                                this_block_vertex.in_left_neighbour,
                                candidate_block_vertex.in_left_neighbour,
                                navmesh.data->graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.in_left_neighbour,
                                this_block_vertex.in_left_neighbour,
                                navmesh.data->graph
                            );
                        }
                    }

                    // Step across and down from y == CHUNK_LENGTH - 1
                    for (BlockChunkPositionCoord z = 0; z < CHUNK_LENGTH; ++z) {
                        BlockIndex this_block_index
                            = hvox::block_index({ 0, CHUNK_LENGTH - 1, z });
                        Block* this_block = &blocks.data[this_block_index];

                        BlockIndex above_this_block_index
                            = hvox::block_index({ 0, 0, z });
                        Block* above_this_block
                            = &neighbour_blocks.data[above_this_block_index];

                        // Necessary condition for step across and down.
                        if (!is_solid(this_block) || is_solid(above_this_block))
                            continue;

                        ChunkNavmeshNode this_block_coord = {
                            {0, CHUNK_LENGTH - 1, z},
                            chunk_pos
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor here, in_left_neighbour;
                        } this_block_vertex
                            = { impl::get_vertex(chunk, this_block_coord),
                                impl::get_vertex(left_neighbour, this_block_coord) };

                        BlockIndex step_down_candidate_index = hvox::block_index(
                            { CHUNK_LENGTH - 1, CHUNK_LENGTH - 2, z }
                        );
                        Block* step_down_candidate_block
                            = &left_neighbour_blocks.data[step_down_candidate_index];

                        BlockIndex step_across_candidate_index = hvox::block_index(
                            { CHUNK_LENGTH - 1, CHUNK_LENGTH - 1, z }
                        );
                        Block* step_across_candidate_block
                            = &left_neighbour_blocks.data[step_across_candidate_index];

                        BlockIndex above_candidates_index
                            = hvox::block_index({ CHUNK_LENGTH - 1, 0, z });
                        Block* above_candidates_block
                            = &above_left_neighbour_blocks.data[above_candidates_index];

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
                                impl::get_vertex(chunk, candidate_block_coord),
                                impl::get_vertex(left_neighbour, candidate_block_coord)
                            };

                            {
                                hmem::UniqueResourceLock lock;
                                auto navmesh = chunk->navmesh.get(lock);

                                boost::add_edge(
                                    this_block_vertex.here,
                                    candidate_block_vertex.here,
                                    navmesh.data->graph
                                );
                                boost::add_edge(
                                    candidate_block_vertex.here,
                                    this_block_vertex.here,
                                    navmesh.data->graph
                                );
                            }

                            {
                                hmem::UniqueResourceLock lock;
                                auto navmesh = left_neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    this_block_vertex.in_left_neighbour,
                                    candidate_block_vertex.in_left_neighbour,
                                    navmesh.data->graph
                                );
                                boost::add_edge(
                                    candidate_block_vertex.in_left_neighbour,
                                    this_block_vertex.in_left_neighbour,
                                    navmesh.data->graph
                                );
                            }
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
                                impl::get_vertex(chunk, candidate_block_coord),
                                impl::get_vertex(left_neighbour, candidate_block_coord)
                            };

                            {
                                hmem::UniqueResourceLock lock;
                                auto navmesh = chunk->navmesh.get(lock);

                                boost::add_edge(
                                    this_block_vertex.here,
                                    candidate_block_vertex.here,
                                    navmesh.data->graph
                                );
                                boost::add_edge(
                                    candidate_block_vertex.here,
                                    this_block_vertex.here,
                                    navmesh.data->graph
                                );
                            }

                            {
                                hmem::UniqueResourceLock lock;
                                auto navmesh = left_neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    this_block_vertex.in_left_neighbour,
                                    candidate_block_vertex.in_left_neighbour,
                                    navmesh.data->graph
                                );
                                boost::add_edge(
                                    candidate_block_vertex.in_left_neighbour,
                                    this_block_vertex.in_left_neighbour,
                                    navmesh.data->graph
                                );
                            }
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
                    hmem::SharedResourceLock right_neighbour_block_lock;
                    auto                     right_neighbour_blocks
                        = right_neighbour->blocks.get(right_neighbour_block_lock);
                    hmem::SharedResourceLock above_right_neighbour_block_lock;
                    auto                     above_right_neighbour_blocks
                        = above_right_neighbour->blocks.get(
                            above_right_neighbour_block_lock
                        );

                    // Step up from y == CHUNK_LENGTH - 2
                    for (BlockChunkPositionCoord z = 0; z < CHUNK_LENGTH; ++z) {
                        BlockIndex this_block_index = hvox::block_index(
                            { CHUNK_LENGTH - 1, CHUNK_LENGTH - 2, z }
                        );
                        Block* this_block = &blocks.data[this_block_index];

                        BlockIndex above_this_block_index = hvox::block_index(
                            { CHUNK_LENGTH - 1, CHUNK_LENGTH - 1, z }
                        );
                        Block* above_this_block = &blocks.data[above_this_block_index];

                        BlockIndex twice_above_this_block_index
                            = hvox::block_index({ CHUNK_LENGTH - 1, 0, z });
                        Block* twice_above_this_block
                            = &neighbour_blocks.data[twice_above_this_block_index];

                        if (!is_solid(this_block) || is_solid(above_this_block)
                            || is_solid(twice_above_this_block))
                            continue;

                        ChunkNavmeshNode this_block_coord = {
                            {CHUNK_LENGTH - 1, CHUNK_LENGTH - 2, z},
                            chunk_pos
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor here, in_right_neighbour;
                        } this_block_vertex
                            = { impl::get_vertex(chunk, this_block_coord),
                                impl::get_vertex(right_neighbour, this_block_coord) };

                        BlockIndex above_candidate_index
                            = hvox::block_index({ 0, 0, z });
                        Block* above_candidate_block
                            = &above_right_neighbour_blocks.data[above_candidate_index];

                        BlockIndex candidate_index
                            = hvox::block_index({ 0, CHUNK_LENGTH - 1, z });
                        Block* candidate_block
                            = &right_neighbour_blocks.data[candidate_index];

                        if (!is_solid(candidate_block)
                            || is_solid(above_candidate_block))
                            continue;

                        ChunkNavmeshNode candidate_block_coord = {
                            {0, CHUNK_LENGTH - 1, z},
                            right_neighbour->position
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor here, in_right_neighbour;
                        } candidate_block_vertex = {
                            impl::get_vertex(chunk, candidate_block_coord),
                            impl::get_vertex(right_neighbour, candidate_block_coord)
                        };

                        {
                            hmem::UniqueResourceLock lock;
                            auto                     navmesh = chunk->navmesh.get(lock);

                            boost::add_edge(
                                this_block_vertex.here,
                                candidate_block_vertex.here,
                                navmesh.data->graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.here,
                                this_block_vertex.here,
                                navmesh.data->graph
                            );
                        }

                        {
                            hmem::UniqueResourceLock lock;
                            auto navmesh = right_neighbour->navmesh.get(lock);

                            boost::add_edge(
                                this_block_vertex.in_right_neighbour,
                                candidate_block_vertex.in_right_neighbour,
                                navmesh.data->graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.in_right_neighbour,
                                this_block_vertex.in_right_neighbour,
                                navmesh.data->graph
                            );
                        }
                    }

                    // Step across and down from y == CHUNK_LENGTH - 1
                    for (BlockChunkPositionCoord z = 0; z < CHUNK_LENGTH; ++z) {
                        BlockIndex this_block_index = hvox::block_index(
                            { CHUNK_LENGTH - 1, CHUNK_LENGTH - 1, z }
                        );
                        Block* this_block = &blocks.data[this_block_index];

                        BlockIndex above_this_block_index
                            = hvox::block_index({ CHUNK_LENGTH - 1, 0, z });
                        Block* above_this_block
                            = &neighbour_blocks.data[above_this_block_index];

                        // Necessary condition for step across and down.
                        if (!is_solid(this_block) || is_solid(above_this_block))
                            continue;

                        ChunkNavmeshNode this_block_coord = {
                            {CHUNK_LENGTH - 1, CHUNK_LENGTH - 1, z},
                            chunk_pos
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor here, in_right_neighbour;
                        } this_block_vertex
                            = { impl::get_vertex(chunk, this_block_coord),
                                impl::get_vertex(right_neighbour, this_block_coord) };

                        BlockIndex step_down_candidate_index
                            = hvox::block_index({ 0, CHUNK_LENGTH - 2, z });
                        Block* step_down_candidate_block
                            = &right_neighbour_blocks.data[step_down_candidate_index];

                        BlockIndex step_across_candidate_index
                            = hvox::block_index({ 0, CHUNK_LENGTH - 1, z });
                        Block* step_across_candidate_block
                            = &right_neighbour_blocks.data[step_across_candidate_index];

                        BlockIndex above_candidates_index
                            = hvox::block_index({ 0, 0, z });
                        Block* above_candidates_block
                            = &above_right_neighbour_blocks
                                   .data[above_candidates_index];

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
                                impl::get_vertex(chunk, candidate_block_coord),
                                impl::get_vertex(right_neighbour, candidate_block_coord)
                            };

                            {
                                hmem::UniqueResourceLock lock;
                                auto navmesh = chunk->navmesh.get(lock);

                                boost::add_edge(
                                    this_block_vertex.here,
                                    candidate_block_vertex.here,
                                    navmesh.data->graph
                                );
                                boost::add_edge(
                                    candidate_block_vertex.here,
                                    this_block_vertex.here,
                                    navmesh.data->graph
                                );
                            }

                            {
                                hmem::UniqueResourceLock lock;
                                auto navmesh = right_neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    this_block_vertex.in_right_neighbour,
                                    candidate_block_vertex.in_right_neighbour,
                                    navmesh.data->graph
                                );
                                boost::add_edge(
                                    candidate_block_vertex.in_right_neighbour,
                                    this_block_vertex.in_right_neighbour,
                                    navmesh.data->graph
                                );
                            }
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
                                impl::get_vertex(chunk, candidate_block_coord),
                                impl::get_vertex(right_neighbour, candidate_block_coord)
                            };

                            {
                                hmem::UniqueResourceLock lock;
                                auto navmesh = chunk->navmesh.get(lock);

                                boost::add_edge(
                                    this_block_vertex.here,
                                    candidate_block_vertex.here,
                                    navmesh.data->graph
                                );
                                boost::add_edge(
                                    candidate_block_vertex.here,
                                    this_block_vertex.here,
                                    navmesh.data->graph
                                );
                            }

                            {
                                hmem::UniqueResourceLock lock;
                                auto navmesh = right_neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    this_block_vertex.in_right_neighbour,
                                    candidate_block_vertex.in_right_neighbour,
                                    navmesh.data->graph
                                );
                                boost::add_edge(
                                    candidate_block_vertex.in_right_neighbour,
                                    this_block_vertex.in_right_neighbour,
                                    navmesh.data->graph
                                );
                            }
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
                    hmem::SharedResourceLock front_neighbour_block_lock;
                    auto                     front_neighbour_blocks
                        = front_neighbour->blocks.get(front_neighbour_block_lock);
                    hmem::SharedResourceLock above_front_neighbour_block_lock;
                    auto                     above_front_neighbour_blocks
                        = above_front_neighbour->blocks.get(
                            above_front_neighbour_block_lock
                        );

                    // Step up from y == CHUNK_LENGTH - 2
                    for (BlockChunkPositionCoord x = 0; x < CHUNK_LENGTH; ++x) {
                        BlockIndex this_block_index = hvox::block_index(
                            { x, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1 }
                        );
                        Block* this_block = &blocks.data[this_block_index];

                        BlockIndex above_this_block_index = hvox::block_index(
                            { x, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1 }
                        );
                        Block* above_this_block = &blocks.data[above_this_block_index];

                        BlockIndex twice_above_this_block_index
                            = hvox::block_index({ x, 0, CHUNK_LENGTH - 1 });
                        Block* twice_above_this_block
                            = &neighbour_blocks.data[twice_above_this_block_index];

                        if (!is_solid(this_block) || is_solid(above_this_block)
                            || is_solid(twice_above_this_block))
                            continue;

                        ChunkNavmeshNode this_block_coord = {
                            {x, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1},
                            chunk_pos
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor here, in_front_neighbour;
                        } this_block_vertex
                            = { impl::get_vertex(chunk, this_block_coord),
                                impl::get_vertex(front_neighbour, this_block_coord) };

                        BlockIndex above_candidate_index
                            = hvox::block_index({ x, 0, 0 });
                        Block* above_candidate_block
                            = &above_front_neighbour_blocks.data[above_candidate_index];

                        BlockIndex candidate_index
                            = hvox::block_index({ x, CHUNK_LENGTH - 1, 0 });
                        Block* candidate_block
                            = &front_neighbour_blocks.data[candidate_index];

                        if (!is_solid(candidate_block)
                            || is_solid(above_candidate_block))
                            continue;

                        ChunkNavmeshNode candidate_block_coord = {
                            {x, CHUNK_LENGTH - 1, 0},
                            front_neighbour->position
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor here, in_front_neighbour;
                        } candidate_block_vertex = {
                            impl::get_vertex(chunk, candidate_block_coord),
                            impl::get_vertex(front_neighbour, candidate_block_coord)
                        };

                        {
                            hmem::UniqueResourceLock lock;
                            auto                     navmesh = chunk->navmesh.get(lock);

                            boost::add_edge(
                                this_block_vertex.here,
                                candidate_block_vertex.here,
                                navmesh.data->graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.here,
                                this_block_vertex.here,
                                navmesh.data->graph
                            );
                        }

                        {
                            hmem::UniqueResourceLock lock;
                            auto navmesh = front_neighbour->navmesh.get(lock);

                            boost::add_edge(
                                this_block_vertex.in_front_neighbour,
                                candidate_block_vertex.in_front_neighbour,
                                navmesh.data->graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.in_front_neighbour,
                                this_block_vertex.in_front_neighbour,
                                navmesh.data->graph
                            );
                        }
                    }

                    // Step across and down from y == CHUNK_LENGTH - 1
                    for (BlockChunkPositionCoord x = 0; x < CHUNK_LENGTH; ++x) {
                        BlockIndex this_block_index = hvox::block_index(
                            { x, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1 }
                        );
                        Block* this_block = &blocks.data[this_block_index];

                        BlockIndex above_this_block_index
                            = hvox::block_index({ x, 0, CHUNK_LENGTH - 1 });
                        Block* above_this_block
                            = &neighbour_blocks.data[above_this_block_index];

                        // Necessary condition for step across and down.
                        if (!is_solid(this_block) || is_solid(above_this_block))
                            continue;

                        ChunkNavmeshNode this_block_coord = {
                            {x, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1},
                            chunk_pos
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor here, in_front_neighbour;
                        } this_block_vertex
                            = { impl::get_vertex(chunk, this_block_coord),
                                impl::get_vertex(front_neighbour, this_block_coord) };

                        BlockIndex step_down_candidate_index
                            = hvox::block_index({ x, CHUNK_LENGTH - 2, 0 });
                        Block* step_down_candidate_block
                            = &front_neighbour_blocks.data[step_down_candidate_index];

                        BlockIndex step_across_candidate_index
                            = hvox::block_index({ x, CHUNK_LENGTH - 1, 0 });
                        Block* step_across_candidate_block
                            = &front_neighbour_blocks.data[step_across_candidate_index];

                        BlockIndex above_candidates_index
                            = hvox::block_index({ x, 0, 0 });
                        Block* above_candidates_block
                            = &above_front_neighbour_blocks
                                   .data[above_candidates_index];

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
                                impl::get_vertex(chunk, candidate_block_coord),
                                impl::get_vertex(front_neighbour, candidate_block_coord)
                            };

                            {
                                hmem::UniqueResourceLock lock;
                                auto navmesh = chunk->navmesh.get(lock);

                                boost::add_edge(
                                    this_block_vertex.here,
                                    candidate_block_vertex.here,
                                    navmesh.data->graph
                                );
                                boost::add_edge(
                                    candidate_block_vertex.here,
                                    this_block_vertex.here,
                                    navmesh.data->graph
                                );
                            }

                            {
                                hmem::UniqueResourceLock lock;
                                auto navmesh = front_neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    this_block_vertex.in_front_neighbour,
                                    candidate_block_vertex.in_front_neighbour,
                                    navmesh.data->graph
                                );
                                boost::add_edge(
                                    candidate_block_vertex.in_front_neighbour,
                                    this_block_vertex.in_front_neighbour,
                                    navmesh.data->graph
                                );
                            }
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
                                impl::get_vertex(chunk, candidate_block_coord),
                                impl::get_vertex(front_neighbour, candidate_block_coord)
                            };

                            {
                                hmem::UniqueResourceLock lock;
                                auto navmesh = chunk->navmesh.get(lock);

                                boost::add_edge(
                                    this_block_vertex.here,
                                    candidate_block_vertex.here,
                                    navmesh.data->graph
                                );
                                boost::add_edge(
                                    candidate_block_vertex.here,
                                    this_block_vertex.here,
                                    navmesh.data->graph
                                );
                            }

                            {
                                hmem::UniqueResourceLock lock;
                                auto navmesh = front_neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    this_block_vertex.in_front_neighbour,
                                    candidate_block_vertex.in_front_neighbour,
                                    navmesh.data->graph
                                );
                                boost::add_edge(
                                    candidate_block_vertex.in_front_neighbour,
                                    this_block_vertex.in_front_neighbour,
                                    navmesh.data->graph
                                );
                            }
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
                    hmem::SharedResourceLock back_neighbour_block_lock;
                    auto                     back_neighbour_blocks
                        = back_neighbour->blocks.get(back_neighbour_block_lock);
                    hmem::SharedResourceLock above_back_neighbour_block_lock;
                    auto above_back_neighbour_blocks = above_back_neighbour->blocks.get(
                        above_back_neighbour_block_lock
                    );

                    // Step up from y == CHUNK_LENGTH - 2
                    for (BlockChunkPositionCoord x = 0; x < CHUNK_LENGTH; ++x) {
                        BlockIndex this_block_index
                            = hvox::block_index({ x, CHUNK_LENGTH - 2, 0 });
                        Block* this_block = &blocks.data[this_block_index];

                        BlockIndex above_this_block_index
                            = hvox::block_index({ x, CHUNK_LENGTH - 1, 0 });
                        Block* above_this_block = &blocks.data[above_this_block_index];

                        BlockIndex twice_above_this_block_index
                            = hvox::block_index({ CHUNK_LENGTH - 1, 0, 0 });
                        Block* twice_above_this_block
                            = &neighbour_blocks.data[twice_above_this_block_index];

                        if (!is_solid(this_block) || is_solid(above_this_block)
                            || is_solid(twice_above_this_block))
                            continue;

                        ChunkNavmeshNode this_block_coord = {
                            {x, CHUNK_LENGTH - 2, 0},
                            chunk_pos
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor here, in_back_neighbour;
                        } this_block_vertex
                            = { impl::get_vertex(chunk, this_block_coord),
                                impl::get_vertex(back_neighbour, this_block_coord) };

                        BlockIndex above_candidate_index
                            = hvox::block_index({ x, 0, CHUNK_LENGTH - 1 });
                        Block* above_candidate_block
                            = &above_back_neighbour_blocks.data[above_candidate_index];

                        BlockIndex candidate_index = hvox::block_index(
                            { x, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1 }
                        );
                        Block* candidate_block
                            = &back_neighbour_blocks.data[candidate_index];

                        if (!is_solid(candidate_block)
                            || is_solid(above_candidate_block))
                            continue;

                        ChunkNavmeshNode candidate_block_coord = {
                            {x, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1},
                            back_neighbour->position
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor here, in_back_neighbour;
                        } candidate_block_vertex = {
                            impl::get_vertex(chunk, candidate_block_coord),
                            impl::get_vertex(back_neighbour, candidate_block_coord)
                        };

                        {
                            hmem::UniqueResourceLock lock;
                            auto                     navmesh = chunk->navmesh.get(lock);

                            boost::add_edge(
                                this_block_vertex.here,
                                candidate_block_vertex.here,
                                navmesh.data->graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.here,
                                this_block_vertex.here,
                                navmesh.data->graph
                            );
                        }

                        {
                            hmem::UniqueResourceLock lock;
                            auto navmesh = back_neighbour->navmesh.get(lock);

                            boost::add_edge(
                                this_block_vertex.in_back_neighbour,
                                candidate_block_vertex.in_back_neighbour,
                                navmesh.data->graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.in_back_neighbour,
                                this_block_vertex.in_back_neighbour,
                                navmesh.data->graph
                            );
                        }
                    }

                    // Step across and down from y == CHUNK_LENGTH - 1
                    for (BlockChunkPositionCoord x = 0; x < CHUNK_LENGTH; ++x) {
                        BlockIndex this_block_index
                            = hvox::block_index({ x, CHUNK_LENGTH - 1, 0 });
                        Block* this_block = &blocks.data[this_block_index];

                        BlockIndex above_this_block_index
                            = hvox::block_index({ x, 0, 0 });
                        Block* above_this_block
                            = &neighbour_blocks.data[above_this_block_index];

                        // Necessary condition for step across and down.
                        if (!is_solid(this_block) || is_solid(above_this_block))
                            continue;

                        ChunkNavmeshNode this_block_coord = {
                            {x, CHUNK_LENGTH - 1, 0},
                            chunk_pos
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor here, in_back_neighbour;
                        } this_block_vertex
                            = { impl::get_vertex(chunk, this_block_coord),
                                impl::get_vertex(back_neighbour, this_block_coord) };

                        BlockIndex step_down_candidate_index = hvox::block_index(
                            { x, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1 }
                        );
                        Block* step_down_candidate_block
                            = &back_neighbour_blocks.data[step_down_candidate_index];

                        BlockIndex step_across_candidate_index = hvox::block_index(
                            { x, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1 }
                        );
                        Block* step_across_candidate_block
                            = &back_neighbour_blocks.data[step_across_candidate_index];

                        BlockIndex above_candidates_index
                            = hvox::block_index({ x, 0, CHUNK_LENGTH - 1 });
                        Block* above_candidates_block
                            = &above_back_neighbour_blocks.data[above_candidates_index];

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
                                impl::get_vertex(chunk, candidate_block_coord),
                                impl::get_vertex(back_neighbour, candidate_block_coord)
                            };

                            {
                                hmem::UniqueResourceLock lock;
                                auto navmesh = chunk->navmesh.get(lock);

                                boost::add_edge(
                                    this_block_vertex.here,
                                    candidate_block_vertex.here,
                                    navmesh.data->graph
                                );
                                boost::add_edge(
                                    candidate_block_vertex.here,
                                    this_block_vertex.here,
                                    navmesh.data->graph
                                );
                            }

                            {
                                hmem::UniqueResourceLock lock;
                                auto navmesh = back_neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    this_block_vertex.in_back_neighbour,
                                    candidate_block_vertex.in_back_neighbour,
                                    navmesh.data->graph
                                );
                                boost::add_edge(
                                    candidate_block_vertex.in_back_neighbour,
                                    this_block_vertex.in_back_neighbour,
                                    navmesh.data->graph
                                );
                            }
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
                                impl::get_vertex(chunk, candidate_block_coord),
                                impl::get_vertex(back_neighbour, candidate_block_coord)
                            };

                            {
                                hmem::UniqueResourceLock lock;
                                auto navmesh = chunk->navmesh.get(lock);

                                boost::add_edge(
                                    this_block_vertex.here,
                                    candidate_block_vertex.here,
                                    navmesh.data->graph
                                );
                                boost::add_edge(
                                    candidate_block_vertex.here,
                                    this_block_vertex.here,
                                    navmesh.data->graph
                                );
                            }

                            {
                                hmem::UniqueResourceLock lock;
                                auto navmesh = back_neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    this_block_vertex.in_back_neighbour,
                                    candidate_block_vertex.in_back_neighbour,
                                    navmesh.data->graph
                                );
                                boost::add_edge(
                                    candidate_block_vertex.in_back_neighbour,
                                    this_block_vertex.in_back_neighbour,
                                    navmesh.data->graph
                                );
                            }
                        }
                    }

                    chunk->navmesh_stitch.above_and_across_left.store(
                        ChunkState::COMPLETE
                    );
                }
            }

            neighbour->navmesh_stitch.top.store(ChunkState::COMPLETE);
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
            hmem::SharedResourceLock neighbour_block_lock;
            auto neighbour_blocks = neighbour->blocks.get(neighbour_block_lock);
            for (BlockChunkPositionCoord x = 1; x < CHUNK_LENGTH - 1; ++x) {
                for (BlockChunkPositionCoord z = 1; z < CHUNK_LENGTH - 1; ++z) {
                    BlockIndex this_block_index = hvox::block_index({ x, 0, z });
                    Block*     this_block       = &blocks.data[this_block_index];

                    BlockIndex above_this_index = hvox::block_index({ x, 1, z });
                    Block*     above_this_block = &blocks.data[above_this_index];

                    BlockIndex neighbour_block_index
                        = hvox::block_index({ x, CHUNK_LENGTH - 1, z });
                    Block* neighbour_block
                        = &neighbour_blocks.data[neighbour_block_index];

                    if (!is_solid(neighbour_block) || is_solid(this_block)) continue;

                    // Ensure node exists for this block.
                    ChunkNavmeshNode neighbour_block_coord = {
                        {x, CHUNK_LENGTH - 1, z},
                        neighbour->position
                    };

                    struct {
                        ChunkNavmeshVertexDescriptor here, in_neighbour;
                    } neighbour_block_vertex
                        = { impl::get_vertex(chunk, neighbour_block_coord),
                            impl::get_vertex(neighbour, neighbour_block_coord) };

                    // Up
                    if (!is_solid(above_this_block)) {
                        // Left
                        BlockIndex left_of_this_block_index
                            = hvox::block_index({ x - 1, 0, z });
                        Block* left_of_this_block
                            = &blocks.data[left_of_this_block_index];

                        BlockIndex above_and_left_of_this_block_index
                            = hvox::block_index({ x - 1, 1, z });
                        Block* above_and_left_of_this_block
                            = &blocks.data[above_and_left_of_this_block_index];

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
                            } left_of_this_block_vertex = {
                                impl::get_vertex(chunk, left_of_this_block_coord),
                                impl::get_vertex(neighbour, left_of_this_block_coord)
                            };

                            {
                                hmem::UniqueResourceLock lock;
                                auto navmesh = chunk->navmesh.get(lock);

                                boost::add_edge(
                                    neighbour_block_vertex.here,
                                    left_of_this_block_vertex.here,
                                    navmesh.data->graph
                                );
                                boost::add_edge(
                                    left_of_this_block_vertex.here,
                                    neighbour_block_vertex.here,
                                    navmesh.data->graph
                                );
                            }

                            {
                                hmem::UniqueResourceLock lock;
                                auto navmesh = neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    neighbour_block_vertex.in_neighbour,
                                    left_of_this_block_vertex.in_neighbour,
                                    navmesh.data->graph
                                );
                                boost::add_edge(
                                    left_of_this_block_vertex.in_neighbour,
                                    neighbour_block_vertex.in_neighbour,
                                    navmesh.data->graph
                                );
                            }
                        }

                        // Right
                        BlockIndex right_of_this_block_index
                            = hvox::block_index({ x + 1, 0, z });
                        Block* right_of_this_block
                            = &blocks.data[right_of_this_block_index];

                        BlockIndex above_and_right_of_this_block_index
                            = hvox::block_index({ x + 1, 1, z });
                        Block* above_and_right_of_this_block
                            = &blocks.data[above_and_right_of_this_block_index];

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
                            } right_of_this_block_vertex = {
                                impl::get_vertex(chunk, right_of_this_block_coord),
                                impl::get_vertex(neighbour, right_of_this_block_coord)
                            };

                            {
                                hmem::UniqueResourceLock lock;
                                auto navmesh = chunk->navmesh.get(lock);

                                boost::add_edge(
                                    neighbour_block_vertex.here,
                                    right_of_this_block_vertex.here,
                                    navmesh.data->graph
                                );
                                boost::add_edge(
                                    right_of_this_block_vertex.here,
                                    neighbour_block_vertex.here,
                                    navmesh.data->graph
                                );
                            }

                            {
                                hmem::UniqueResourceLock lock;
                                auto navmesh = neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    neighbour_block_vertex.in_neighbour,
                                    right_of_this_block_vertex.in_neighbour,
                                    navmesh.data->graph
                                );
                                boost::add_edge(
                                    right_of_this_block_vertex.in_neighbour,
                                    neighbour_block_vertex.in_neighbour,
                                    navmesh.data->graph
                                );
                            }
                        }

                        // Front
                        BlockIndex front_of_this_block_index
                            = hvox::block_index({ x, 0, z + 1 });
                        Block* front_of_this_block
                            = &blocks.data[front_of_this_block_index];

                        BlockIndex above_and_front_of_this_block_index
                            = hvox::block_index({ x, 1, z + 1 });
                        Block* above_and_front_of_this_block
                            = &blocks.data[above_and_front_of_this_block_index];

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
                            } front_of_this_block_vertex = {
                                impl::get_vertex(chunk, front_of_this_block_coord),
                                impl::get_vertex(neighbour, front_of_this_block_coord)
                            };

                            {
                                hmem::UniqueResourceLock lock;
                                auto navmesh = chunk->navmesh.get(lock);

                                boost::add_edge(
                                    neighbour_block_vertex.here,
                                    front_of_this_block_vertex.here,
                                    navmesh.data->graph
                                );
                                boost::add_edge(
                                    front_of_this_block_vertex.here,
                                    neighbour_block_vertex.here,
                                    navmesh.data->graph
                                );
                            }

                            {
                                hmem::UniqueResourceLock lock;
                                auto navmesh = neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    neighbour_block_vertex.in_neighbour,
                                    front_of_this_block_vertex.in_neighbour,
                                    navmesh.data->graph
                                );
                                boost::add_edge(
                                    front_of_this_block_vertex.in_neighbour,
                                    neighbour_block_vertex.in_neighbour,
                                    navmesh.data->graph
                                );
                            }
                        }

                        // Back
                        BlockIndex back_of_this_block_index
                            = hvox::block_index({ x, 0, z - 1 });
                        Block* back_of_this_block
                            = &blocks.data[back_of_this_block_index];

                        BlockIndex above_and_back_of_this_block_index
                            = hvox::block_index({ x, 1, z - 1 });
                        Block* above_and_back_of_this_block
                            = &blocks.data[above_and_back_of_this_block_index];

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
                            } back_of_this_block_vertex = {
                                impl::get_vertex(chunk, back_of_this_block_coord),
                                impl::get_vertex(neighbour, back_of_this_block_coord)
                            };

                            {
                                hmem::UniqueResourceLock lock;
                                auto navmesh = chunk->navmesh.get(lock);

                                boost::add_edge(
                                    neighbour_block_vertex.here,
                                    back_of_this_block_vertex.here,
                                    navmesh.data->graph
                                );
                                boost::add_edge(
                                    back_of_this_block_vertex.here,
                                    neighbour_block_vertex.here,
                                    navmesh.data->graph
                                );
                            }

                            {
                                hmem::UniqueResourceLock lock;
                                auto navmesh = neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    neighbour_block_vertex.in_neighbour,
                                    back_of_this_block_vertex.in_neighbour,
                                    navmesh.data->graph
                                );
                                boost::add_edge(
                                    back_of_this_block_vertex.in_neighbour,
                                    neighbour_block_vertex.in_neighbour,
                                    navmesh.data->graph
                                );
                            }
                        }
                    }

                    ///// Across and Down

                    // Left
                    BlockIndex left_of_neighbour_block_index
                        = hvox::block_index({ x - 1, CHUNK_LENGTH - 1, z });
                    Block* left_of_neighbour_block
                        = &neighbour_blocks.data[left_of_neighbour_block_index];

                    BlockIndex left_of_and_below_neighbour_block_index
                        = hvox::block_index({ x - 1, CHUNK_LENGTH - 2, z });
                    Block* left_of_and_below_neighbour_block
                        = &neighbour_blocks
                               .data[left_of_and_below_neighbour_block_index];

                    BlockIndex left_of_this_block_index
                        = hvox::block_index({ x - 1, 0, z });
                    Block* left_of_this_block = &blocks.data[left_of_this_block_index];

                    // Across
                    if (is_solid(left_of_neighbour_block)
                        && !is_solid(left_of_this_block))
                    {
                        ChunkNavmeshNode left_of_neighbour_block_coord = {
                            {x - 1, CHUNK_LENGTH - 1, z},
                            neighbour->position
                        };

                        // Ensure node exists for this block.
                        ChunkNavmeshVertexDescriptor left_of_neighbour_block_vertex
                            = impl::get_vertex(
                                neighbour, left_of_neighbour_block_coord
                            );

                        {
                            hmem::UniqueResourceLock lock;
                            auto navmesh = neighbour->navmesh.get(lock);

                            boost::add_edge(
                                neighbour_block_vertex.in_neighbour,
                                left_of_neighbour_block_vertex,
                                navmesh.data->graph
                            );
                            boost::add_edge(
                                left_of_neighbour_block_vertex,
                                neighbour_block_vertex.in_neighbour,
                                navmesh.data->graph
                            );
                        }
                        // Down
                    } else if (is_solid(left_of_and_below_neighbour_block) && !is_solid(left_of_neighbour_block) && !is_solid(left_of_this_block))
                    {
                        ChunkNavmeshNode left_of_and_below_neighbour_block_coord = {
                            {x - 1, CHUNK_LENGTH - 2, z},
                            neighbour->position
                        };

                        // Ensure node exists for this block.
                        ChunkNavmeshVertexDescriptor
                            left_of_and_below_neighbour_block_vertex
                            = impl::get_vertex(
                                neighbour, left_of_and_below_neighbour_block_coord
                            );

                        {
                            hmem::UniqueResourceLock lock;
                            auto navmesh = neighbour->navmesh.get(lock);

                            boost::add_edge(
                                neighbour_block_vertex.in_neighbour,
                                left_of_and_below_neighbour_block_vertex,
                                navmesh.data->graph
                            );
                            boost::add_edge(
                                left_of_and_below_neighbour_block_vertex,
                                neighbour_block_vertex.in_neighbour,
                                navmesh.data->graph
                            );
                        }
                    }

                    // Right
                    BlockIndex right_of_neighbour_block_index
                        = hvox::block_index({ x + 1, CHUNK_LENGTH - 1, z });
                    Block* right_of_neighbour_block
                        = &neighbour_blocks.data[right_of_neighbour_block_index];

                    BlockIndex right_of_and_below_neighbour_block_index
                        = hvox::block_index({ x + 1, CHUNK_LENGTH - 2, z });
                    Block* right_of_and_below_neighbour_block
                        = &neighbour_blocks
                               .data[right_of_and_below_neighbour_block_index];

                    BlockIndex right_of_this_block_index
                        = hvox::block_index({ x + 1, 0, z });
                    Block* right_of_this_block
                        = &blocks.data[right_of_this_block_index];

                    // Across
                    if (is_solid(right_of_neighbour_block)
                        && !is_solid(right_of_this_block))
                    {
                        ChunkNavmeshNode right_of_neighbour_block_coord = {
                            {x + 1, CHUNK_LENGTH - 1, z},
                            neighbour->position
                        };

                        // Ensure node exists for this block.
                        ChunkNavmeshVertexDescriptor right_of_neighbour_block_vertex
                            = impl::get_vertex(
                                neighbour, right_of_neighbour_block_coord
                            );

                        {
                            hmem::UniqueResourceLock lock;
                            auto navmesh = neighbour->navmesh.get(lock);

                            boost::add_edge(
                                neighbour_block_vertex.in_neighbour,
                                right_of_neighbour_block_vertex,
                                navmesh.data->graph
                            );
                            boost::add_edge(
                                right_of_neighbour_block_vertex,
                                neighbour_block_vertex.in_neighbour,
                                navmesh.data->graph
                            );
                        }
                        // Down
                    } else if (is_solid(right_of_and_below_neighbour_block) && !is_solid(right_of_neighbour_block) && !is_solid(right_of_this_block))
                    {
                        ChunkNavmeshNode right_of_and_below_neighbour_block_coord = {
                            {x + 1, CHUNK_LENGTH - 2, z},
                            neighbour->position
                        };

                        // Ensure node exists for this block.
                        ChunkNavmeshVertexDescriptor
                            right_of_and_below_neighbour_block_vertex
                            = impl::get_vertex(
                                neighbour, right_of_and_below_neighbour_block_coord
                            );

                        {
                            hmem::UniqueResourceLock lock;
                            auto navmesh = neighbour->navmesh.get(lock);

                            boost::add_edge(
                                neighbour_block_vertex.in_neighbour,
                                right_of_and_below_neighbour_block_vertex,
                                navmesh.data->graph
                            );
                            boost::add_edge(
                                right_of_and_below_neighbour_block_vertex,
                                neighbour_block_vertex.in_neighbour,
                                navmesh.data->graph
                            );
                        }
                    }

                    // Front
                    BlockIndex front_of_neighbour_block_index
                        = hvox::block_index({ x, CHUNK_LENGTH - 1, z + 1 });
                    Block* front_of_neighbour_block
                        = &neighbour_blocks.data[front_of_neighbour_block_index];

                    BlockIndex front_of_and_below_neighbour_block_index
                        = hvox::block_index({ x, CHUNK_LENGTH - 2, z + 1 });
                    Block* front_of_and_below_neighbour_block
                        = &neighbour_blocks
                               .data[front_of_and_below_neighbour_block_index];

                    BlockIndex front_of_this_block_index
                        = hvox::block_index({ x, 0, z + 1 });
                    Block* front_of_this_block
                        = &blocks.data[front_of_this_block_index];

                    // Across
                    if (is_solid(front_of_neighbour_block)
                        && !is_solid(front_of_this_block))
                    {
                        ChunkNavmeshNode front_of_neighbour_block_coord = {
                            {x, CHUNK_LENGTH - 1, z + 1},
                            neighbour->position
                        };

                        // Ensure node exists for this block.
                        ChunkNavmeshVertexDescriptor front_of_neighbour_block_vertex
                            = impl::get_vertex(
                                neighbour, front_of_neighbour_block_coord
                            );

                        {
                            hmem::UniqueResourceLock lock;
                            auto navmesh = neighbour->navmesh.get(lock);

                            boost::add_edge(
                                neighbour_block_vertex.in_neighbour,
                                front_of_neighbour_block_vertex,
                                navmesh.data->graph
                            );
                            boost::add_edge(
                                front_of_neighbour_block_vertex,
                                neighbour_block_vertex.in_neighbour,
                                navmesh.data->graph
                            );
                        }
                        // Down
                    } else if (is_solid(front_of_and_below_neighbour_block) && !is_solid(front_of_neighbour_block) && !is_solid(front_of_this_block))
                    {
                        ChunkNavmeshNode front_of_and_below_neighbour_block_coord = {
                            {x, CHUNK_LENGTH - 2, z + 1},
                            neighbour->position
                        };

                        // Ensure node exists for this block.
                        ChunkNavmeshVertexDescriptor
                            front_of_and_below_neighbour_block_vertex
                            = impl::get_vertex(
                                neighbour, front_of_and_below_neighbour_block_coord
                            );

                        {
                            hmem::UniqueResourceLock lock;
                            auto navmesh = neighbour->navmesh.get(lock);

                            boost::add_edge(
                                neighbour_block_vertex.in_neighbour,
                                front_of_and_below_neighbour_block_vertex,
                                navmesh.data->graph
                            );
                            boost::add_edge(
                                front_of_and_below_neighbour_block_vertex,
                                neighbour_block_vertex.in_neighbour,
                                navmesh.data->graph
                            );
                        }
                    }

                    // Back
                    BlockIndex back_of_neighbour_block_index
                        = hvox::block_index({ x, CHUNK_LENGTH - 1, z - 1 });
                    Block* back_of_neighbour_block
                        = &neighbour_blocks.data[back_of_neighbour_block_index];

                    BlockIndex back_of_and_below_neighbour_block_index
                        = hvox::block_index({ x, CHUNK_LENGTH - 2, z - 1 });
                    Block* back_of_and_below_neighbour_block
                        = &neighbour_blocks
                               .data[back_of_and_below_neighbour_block_index];

                    BlockIndex back_of_this_block_index
                        = hvox::block_index({ x, 0, z - 1 });
                    Block* back_of_this_block = &blocks.data[back_of_this_block_index];

                    // Across
                    if (is_solid(back_of_neighbour_block)
                        && !is_solid(back_of_this_block))
                    {
                        ChunkNavmeshNode back_of_neighbour_block_coord = {
                            {x, CHUNK_LENGTH - 1, z - 1},
                            neighbour->position
                        };

                        // Ensure node exists for this block.
                        ChunkNavmeshVertexDescriptor back_of_neighbour_block_vertex
                            = impl::get_vertex(
                                neighbour, back_of_neighbour_block_coord
                            );

                        {
                            hmem::UniqueResourceLock lock;
                            auto navmesh = neighbour->navmesh.get(lock);

                            boost::add_edge(
                                neighbour_block_vertex.in_neighbour,
                                back_of_neighbour_block_vertex,
                                navmesh.data->graph
                            );
                            boost::add_edge(
                                back_of_neighbour_block_vertex,
                                neighbour_block_vertex.in_neighbour,
                                navmesh.data->graph
                            );
                        }
                        // Down
                    } else if (is_solid(back_of_and_below_neighbour_block) && !is_solid(back_of_neighbour_block) && !is_solid(back_of_this_block))
                    {
                        ChunkNavmeshNode back_of_and_below_neighbour_block_coord = {
                            {x, CHUNK_LENGTH - 2, z - 1},
                            neighbour->position
                        };

                        // Ensure node exists for this block.
                        ChunkNavmeshVertexDescriptor
                            back_of_and_below_neighbour_block_vertex
                            = impl::get_vertex(
                                neighbour, back_of_and_below_neighbour_block_coord
                            );

                        {
                            hmem::UniqueResourceLock lock;
                            auto navmesh = neighbour->navmesh.get(lock);

                            boost::add_edge(
                                neighbour_block_vertex.in_neighbour,
                                back_of_and_below_neighbour_block_vertex,
                                navmesh.data->graph
                            );
                            boost::add_edge(
                                back_of_and_below_neighbour_block_vertex,
                                neighbour_block_vertex.in_neighbour,
                                navmesh.data->graph
                            );
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
                    hmem::SharedResourceLock left_neighbour_block_lock;
                    auto                     left_neighbour_blocks
                        = left_neighbour->blocks.get(left_neighbour_block_lock);
                    hmem::SharedResourceLock below_left_neighbour_block_lock;
                    auto below_left_neighbour_blocks = below_left_neighbour->blocks.get(
                        below_left_neighbour_block_lock
                    );

                    // Step up from y == CHUNK_LENGTH - 2
                    for (BlockChunkPositionCoord z = 0; z < CHUNK_LENGTH; ++z) {
                        BlockIndex neighbour_block_index
                            = hvox::block_index({ 0, CHUNK_LENGTH - 2, z });
                        Block* neighbour_block
                            = &neighbour_blocks.data[neighbour_block_index];

                        BlockIndex above_neighbour_block_index
                            = hvox::block_index({ 0, CHUNK_LENGTH - 1, z });
                        Block* above_neighbour_block
                            = &neighbour_blocks.data[above_neighbour_block_index];

                        BlockIndex twice_above_neighbour_block_index
                            = hvox::block_index({ 0, 0, z });
                        Block* twice_above_neighbour_block
                            = &blocks.data[twice_above_neighbour_block_index];

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
                        } neighbour_block_vertex
                            = { impl::get_vertex(neighbour, neighbour_block_coord),
                                impl::get_vertex(
                                    below_left_neighbour, neighbour_block_coord
                                ) };

                        BlockIndex above_candidate_index
                            = hvox::block_index({ CHUNK_LENGTH - 1, 0, z });
                        Block* above_candidate_block
                            = &left_neighbour_blocks.data[above_candidate_index];

                        BlockIndex candidate_index = hvox::block_index(
                            { CHUNK_LENGTH - 1, CHUNK_LENGTH - 1, z }
                        );
                        Block* candidate_block
                            = &below_left_neighbour_blocks.data[candidate_index];

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
                        } candidate_block_vertex
                            = { impl::get_vertex(neighbour, candidate_block_coord),
                                impl::get_vertex(
                                    below_left_neighbour, candidate_block_coord
                                ) };

                        {
                            hmem::UniqueResourceLock lock;
                            auto navmesh = neighbour->navmesh.get(lock);

                            boost::add_edge(
                                neighbour_block_vertex.in_neighbour,
                                candidate_block_vertex.in_neighbour,
                                navmesh.data->graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.in_neighbour,
                                neighbour_block_vertex.in_neighbour,
                                navmesh.data->graph
                            );
                        }

                        {
                            hmem::UniqueResourceLock lock;
                            auto navmesh = below_left_neighbour->navmesh.get(lock);

                            boost::add_edge(
                                neighbour_block_vertex.in_below_left_neighbour,
                                candidate_block_vertex.in_below_left_neighbour,
                                navmesh.data->graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.in_below_left_neighbour,
                                neighbour_block_vertex.in_below_left_neighbour,
                                navmesh.data->graph
                            );
                        }
                    }

                    // Step across and down from y == CHUNK_LENGTH - 1
                    for (BlockChunkPositionCoord z = 0; z < CHUNK_LENGTH; ++z) {
                        BlockIndex neighbour_block_index
                            = hvox::block_index({ 0, CHUNK_LENGTH - 1, z });
                        Block* neighbour_block
                            = &neighbour_blocks.data[neighbour_block_index];

                        BlockIndex above_neighbour_block_index
                            = hvox::block_index({ 0, 0, z });
                        Block* above_neighbour_block
                            = &blocks.data[above_neighbour_block_index];

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
                        } neighbour_block_vertex
                            = { impl::get_vertex(neighbour, neighbour_block_coord),
                                impl::get_vertex(
                                    below_left_neighbour, neighbour_block_coord
                                ) };

                        BlockIndex step_down_candidate_index = hvox::block_index(
                            { CHUNK_LENGTH - 1, CHUNK_LENGTH - 2, z }
                        );
                        Block* step_down_candidate_block
                            = &below_left_neighbour_blocks
                                   .data[step_down_candidate_index];

                        BlockIndex step_across_candidate_index = hvox::block_index(
                            { CHUNK_LENGTH - 1, CHUNK_LENGTH - 1, z }
                        );
                        Block* step_across_candidate_block
                            = &below_left_neighbour_blocks
                                   .data[step_across_candidate_index];

                        BlockIndex above_candidates_index
                            = hvox::block_index({ CHUNK_LENGTH - 1, 0, z });
                        Block* above_candidates_block
                            = &left_neighbour_blocks.data[above_candidates_index];

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
                                = { impl::get_vertex(neighbour, candidate_block_coord),
                                    impl::get_vertex(
                                        below_left_neighbour, candidate_block_coord
                                    ) };

                            {
                                hmem::UniqueResourceLock lock;
                                auto navmesh = neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    neighbour_block_vertex.in_neighbour,
                                    candidate_block_vertex.in_neighbour,
                                    navmesh.data->graph
                                );
                                boost::add_edge(
                                    candidate_block_vertex.in_neighbour,
                                    neighbour_block_vertex.in_neighbour,
                                    navmesh.data->graph
                                );
                            }

                            {
                                hmem::UniqueResourceLock lock;
                                auto navmesh = below_left_neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    neighbour_block_vertex.in_below_left_neighbour,
                                    candidate_block_vertex.in_below_left_neighbour,
                                    navmesh.data->graph
                                );
                                boost::add_edge(
                                    candidate_block_vertex.in_below_left_neighbour,
                                    neighbour_block_vertex.in_below_left_neighbour,
                                    navmesh.data->graph
                                );
                            }
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
                                = { impl::get_vertex(neighbour, candidate_block_coord),
                                    impl::get_vertex(
                                        below_left_neighbour, candidate_block_coord
                                    ) };

                            {
                                hmem::UniqueResourceLock lock;
                                auto navmesh = chunk->navmesh.get(lock);

                                boost::add_edge(
                                    neighbour_block_vertex.in_neighbour,
                                    candidate_block_vertex.in_neighbour,
                                    navmesh.data->graph
                                );
                                boost::add_edge(
                                    candidate_block_vertex.in_neighbour,
                                    neighbour_block_vertex.in_neighbour,
                                    navmesh.data->graph
                                );
                            }

                            {
                                hmem::UniqueResourceLock lock;
                                auto navmesh = left_neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    neighbour_block_vertex.in_below_left_neighbour,
                                    candidate_block_vertex.in_below_left_neighbour,
                                    navmesh.data->graph
                                );
                                boost::add_edge(
                                    candidate_block_vertex.in_below_left_neighbour,
                                    neighbour_block_vertex.in_below_left_neighbour,
                                    navmesh.data->graph
                                );
                            }
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
                    hmem::SharedResourceLock right_neighbour_block_lock;
                    auto                     right_neighbour_blocks
                        = right_neighbour->blocks.get(right_neighbour_block_lock);
                    hmem::SharedResourceLock below_right_neighbour_block_lock;
                    auto                     below_right_neighbour_blocks
                        = below_right_neighbour->blocks.get(
                            below_right_neighbour_block_lock
                        );

                    // Step up from y == CHUNK_LENGTH - 2
                    for (BlockChunkPositionCoord z = 0; z < CHUNK_LENGTH; ++z) {
                        BlockIndex neighbour_block_index = hvox::block_index(
                            { CHUNK_LENGTH - 1, CHUNK_LENGTH - 2, z }
                        );
                        Block* neighbour_block
                            = &neighbour_blocks.data[neighbour_block_index];

                        BlockIndex above_neighbour_block_index = hvox::block_index(
                            { CHUNK_LENGTH - 1, CHUNK_LENGTH - 1, z }
                        );
                        Block* above_neighbour_block
                            = &neighbour_blocks.data[above_neighbour_block_index];

                        BlockIndex twice_above_neighbour_block_index
                            = hvox::block_index({ CHUNK_LENGTH - 1, 0, z });
                        Block* twice_above_neighbour_block
                            = &blocks.data[twice_above_neighbour_block_index];

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
                            impl::get_vertex(neighbour, neighbour_block_coord),
                            impl::get_vertex(right_neighbour, neighbour_block_coord)
                        };

                        BlockIndex above_candidate_index
                            = hvox::block_index({ 0, 0, z });
                        Block* above_candidate_block
                            = &right_neighbour_blocks.data[above_candidate_index];

                        BlockIndex candidate_index
                            = hvox::block_index({ 0, CHUNK_LENGTH - 1, z });
                        Block* candidate_block
                            = &below_right_neighbour_blocks.data[candidate_index];

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
                        } candidate_block_vertex
                            = { impl::get_vertex(neighbour, candidate_block_coord),
                                impl::get_vertex(
                                    below_right_neighbour, candidate_block_coord
                                ) };

                        {
                            hmem::UniqueResourceLock lock;
                            auto navmesh = neighbour->navmesh.get(lock);

                            boost::add_edge(
                                neighbour_block_vertex.in_neighbour,
                                candidate_block_vertex.in_neighbour,
                                navmesh.data->graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.in_neighbour,
                                neighbour_block_vertex.in_neighbour,
                                navmesh.data->graph
                            );
                        }

                        {
                            hmem::UniqueResourceLock lock;
                            auto navmesh = below_right_neighbour->navmesh.get(lock);

                            boost::add_edge(
                                neighbour_block_vertex.in_below_right_neighbour,
                                candidate_block_vertex.in_below_right_neighbour,
                                navmesh.data->graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.in_below_right_neighbour,
                                neighbour_block_vertex.in_below_right_neighbour,
                                navmesh.data->graph
                            );
                        }
                    }

                    // Step across and down from y == CHUNK_LENGTH - 1
                    for (BlockChunkPositionCoord z = 0; z < CHUNK_LENGTH; ++z) {
                        BlockIndex neighbour_block_index = hvox::block_index(
                            { CHUNK_LENGTH - 1, CHUNK_LENGTH - 1, z }
                        );
                        Block* neighbour_block
                            = &neighbour_blocks.data[neighbour_block_index];

                        BlockIndex above_neighbour_block_index
                            = hvox::block_index({ CHUNK_LENGTH - 1, 0, z });
                        Block* above_neighbour_block
                            = &blocks.data[above_neighbour_block_index];

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
                        } neighbour_block_vertex
                            = { impl::get_vertex(neighbour, neighbour_block_coord),
                                impl::get_vertex(
                                    below_right_neighbour, neighbour_block_coord
                                ) };

                        BlockIndex step_down_candidate_index
                            = hvox::block_index({ 0, CHUNK_LENGTH - 2, z });
                        Block* step_down_candidate_block
                            = &below_right_neighbour_blocks
                                   .data[step_down_candidate_index];

                        BlockIndex step_across_candidate_index
                            = hvox::block_index({ 0, CHUNK_LENGTH - 1, z });
                        Block* step_across_candidate_block
                            = &below_right_neighbour_blocks
                                   .data[step_across_candidate_index];

                        BlockIndex above_candidates_index
                            = hvox::block_index({ 0, 0, z });
                        Block* above_candidates_block
                            = &right_neighbour_blocks.data[above_candidates_index];

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
                                = { impl::get_vertex(neighbour, candidate_block_coord),
                                    impl::get_vertex(
                                        below_right_neighbour, candidate_block_coord
                                    ) };

                            {
                                hmem::UniqueResourceLock lock;
                                auto navmesh = neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    neighbour_block_vertex.in_neighbour,
                                    candidate_block_vertex.in_neighbour,
                                    navmesh.data->graph
                                );
                                boost::add_edge(
                                    candidate_block_vertex.in_neighbour,
                                    neighbour_block_vertex.in_neighbour,
                                    navmesh.data->graph
                                );
                            }

                            {
                                hmem::UniqueResourceLock lock;
                                auto navmesh = below_right_neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    neighbour_block_vertex.in_below_right_neighbour,
                                    candidate_block_vertex.in_below_right_neighbour,
                                    navmesh.data->graph
                                );
                                boost::add_edge(
                                    candidate_block_vertex.in_below_right_neighbour,
                                    neighbour_block_vertex.in_below_right_neighbour,
                                    navmesh.data->graph
                                );
                            }
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
                                = { impl::get_vertex(neighbour, candidate_block_coord),
                                    impl::get_vertex(
                                        below_right_neighbour, candidate_block_coord
                                    ) };

                            {
                                hmem::UniqueResourceLock lock;
                                auto navmesh = neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    neighbour_block_vertex.in_neighbour,
                                    candidate_block_vertex.in_neighbour,
                                    navmesh.data->graph
                                );
                                boost::add_edge(
                                    candidate_block_vertex.in_neighbour,
                                    neighbour_block_vertex.in_neighbour,
                                    navmesh.data->graph
                                );
                            }

                            {
                                hmem::UniqueResourceLock lock;
                                auto navmesh = below_right_neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    neighbour_block_vertex.in_below_right_neighbour,
                                    candidate_block_vertex.in_below_right_neighbour,
                                    navmesh.data->graph
                                );
                                boost::add_edge(
                                    candidate_block_vertex.in_below_right_neighbour,
                                    neighbour_block_vertex.in_below_right_neighbour,
                                    navmesh.data->graph
                                );
                            }
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
                    hmem::SharedResourceLock front_neighbour_block_lock;
                    auto                     front_neighbour_blocks
                        = front_neighbour->blocks.get(front_neighbour_block_lock);
                    hmem::SharedResourceLock below_front_neighbour_block_lock;
                    auto                     below_front_neighbour_blocks
                        = below_front_neighbour->blocks.get(
                            below_front_neighbour_block_lock
                        );

                    // Step up from y == CHUNK_LENGTH - 2
                    for (BlockChunkPositionCoord x = 0; x < CHUNK_LENGTH; ++x) {
                        BlockIndex neighbour_block_index = hvox::block_index(
                            { x, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1 }
                        );
                        Block* neighbour_block
                            = &neighbour_blocks.data[neighbour_block_index];

                        BlockIndex above_neighbour_block_index = hvox::block_index(
                            { x, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1 }
                        );
                        Block* above_neighbour_block
                            = &neighbour_blocks.data[above_neighbour_block_index];

                        BlockIndex twice_above_neighbour_block_index
                            = hvox::block_index({ x, 0, CHUNK_LENGTH - 1 });
                        Block* twice_above_neighbour_block
                            = &blocks.data[twice_above_neighbour_block_index];

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
                        } neighbour_block_vertex
                            = { impl::get_vertex(neighbour, neighbour_block_coord),
                                impl::get_vertex(
                                    below_front_neighbour, neighbour_block_coord
                                ) };

                        BlockIndex above_candidate_index
                            = hvox::block_index({ x, 0, 0 });
                        Block* above_candidate_block
                            = &front_neighbour_blocks.data[above_candidate_index];

                        BlockIndex candidate_index
                            = hvox::block_index({ x, CHUNK_LENGTH - 1, 0 });
                        Block* candidate_block
                            = &below_front_neighbour_blocks.data[candidate_index];

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
                        } candidate_block_vertex
                            = { impl::get_vertex(neighbour, candidate_block_coord),
                                impl::get_vertex(
                                    below_front_neighbour, candidate_block_coord
                                ) };

                        {
                            hmem::UniqueResourceLock lock;
                            auto navmesh = neighbour->navmesh.get(lock);

                            boost::add_edge(
                                neighbour_block_vertex.in_neighbour,
                                candidate_block_vertex.in_neighbour,
                                navmesh.data->graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.in_neighbour,
                                neighbour_block_vertex.in_neighbour,
                                navmesh.data->graph
                            );
                        }

                        {
                            hmem::UniqueResourceLock lock;
                            auto navmesh = below_front_neighbour->navmesh.get(lock);

                            boost::add_edge(
                                neighbour_block_vertex.in_below_front_neighbour,
                                candidate_block_vertex.in_below_front_neighbour,
                                navmesh.data->graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.in_below_front_neighbour,
                                neighbour_block_vertex.in_below_front_neighbour,
                                navmesh.data->graph
                            );
                        }
                    }

                    // Step across and down from y == CHUNK_LENGTH - 1
                    for (BlockChunkPositionCoord x = 0; x < CHUNK_LENGTH; ++x) {
                        BlockIndex neighbour_block_index = hvox::block_index(
                            { x, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1 }
                        );
                        Block* neighbour_block
                            = &neighbour_blocks.data[neighbour_block_index];

                        BlockIndex above_neighbour_block_index
                            = hvox::block_index({ x, 0, CHUNK_LENGTH - 1 });
                        Block* above_neighbour_block
                            = &blocks.data[above_neighbour_block_index];

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
                        } neighbour_block_vertex
                            = { impl::get_vertex(neighbour, neighbour_block_coord),
                                impl::get_vertex(
                                    below_front_neighbour, neighbour_block_coord
                                ) };

                        BlockIndex step_down_candidate_index
                            = hvox::block_index({ x, CHUNK_LENGTH - 2, 0 });
                        Block* step_down_candidate_block
                            = &below_front_neighbour_blocks
                                   .data[step_down_candidate_index];

                        BlockIndex step_across_candidate_index
                            = hvox::block_index({ x, CHUNK_LENGTH - 1, 0 });
                        Block* step_across_candidate_block
                            = &below_front_neighbour_blocks
                                   .data[step_across_candidate_index];

                        BlockIndex above_candidates_index
                            = hvox::block_index({ x, 0, 0 });
                        Block* above_candidates_block
                            = &front_neighbour_blocks.data[above_candidates_index];

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
                                = { impl::get_vertex(neighbour, candidate_block_coord),
                                    impl::get_vertex(
                                        below_front_neighbour, candidate_block_coord
                                    ) };

                            {
                                hmem::UniqueResourceLock lock;
                                auto navmesh = neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    neighbour_block_vertex.in_neighbour,
                                    candidate_block_vertex.in_neighbour,
                                    navmesh.data->graph
                                );
                                boost::add_edge(
                                    candidate_block_vertex.in_neighbour,
                                    neighbour_block_vertex.in_neighbour,
                                    navmesh.data->graph
                                );
                            }

                            {
                                hmem::UniqueResourceLock lock;
                                auto navmesh = below_front_neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    neighbour_block_vertex.in_below_front_neighbour,
                                    candidate_block_vertex.in_below_front_neighbour,
                                    navmesh.data->graph
                                );
                                boost::add_edge(
                                    candidate_block_vertex.in_below_front_neighbour,
                                    neighbour_block_vertex.in_below_front_neighbour,
                                    navmesh.data->graph
                                );
                            }
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
                                = { impl::get_vertex(neighbour, candidate_block_coord),
                                    impl::get_vertex(
                                        below_front_neighbour, candidate_block_coord
                                    ) };

                            {
                                hmem::UniqueResourceLock lock;
                                auto navmesh = neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    neighbour_block_vertex.in_neighbour,
                                    candidate_block_vertex.in_neighbour,
                                    navmesh.data->graph
                                );
                                boost::add_edge(
                                    candidate_block_vertex.in_neighbour,
                                    neighbour_block_vertex.in_neighbour,
                                    navmesh.data->graph
                                );
                            }

                            {
                                hmem::UniqueResourceLock lock;
                                auto navmesh = below_front_neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    neighbour_block_vertex.in_below_front_neighbour,
                                    candidate_block_vertex.in_below_front_neighbour,
                                    navmesh.data->graph
                                );
                                boost::add_edge(
                                    candidate_block_vertex.in_below_front_neighbour,
                                    neighbour_block_vertex.in_below_front_neighbour,
                                    navmesh.data->graph
                                );
                            }
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
                    hmem::SharedResourceLock back_neighbour_block_lock;
                    auto                     back_neighbour_blocks
                        = back_neighbour->blocks.get(back_neighbour_block_lock);
                    hmem::SharedResourceLock below_back_neighbour_block_lock;
                    auto below_back_neighbour_blocks = below_back_neighbour->blocks.get(
                        below_back_neighbour_block_lock
                    );

                    // Step up from y == CHUNK_LENGTH - 2
                    for (BlockChunkPositionCoord x = 0; x < CHUNK_LENGTH; ++x) {
                        BlockIndex neighbour_block_index
                            = hvox::block_index({ x, CHUNK_LENGTH - 2, 0 });
                        Block* neighbour_block
                            = &neighbour_blocks.data[neighbour_block_index];

                        BlockIndex above_neighbour_block_index
                            = hvox::block_index({ x, CHUNK_LENGTH - 1, 0 });
                        Block* above_neighbour_block
                            = &neighbour_blocks.data[above_neighbour_block_index];

                        BlockIndex twice_above_neighbour_block_index
                            = hvox::block_index({ CHUNK_LENGTH - 1, 0, 0 });
                        Block* twice_above_neighbour_block
                            = &blocks.data[twice_above_neighbour_block_index];

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
                        } neighbour_block_vertex
                            = { impl::get_vertex(neighbour, neighbour_block_coord),
                                impl::get_vertex(
                                    below_back_neighbour, neighbour_block_coord
                                ) };

                        BlockIndex above_candidate_index
                            = hvox::block_index({ x, 0, CHUNK_LENGTH - 1 });
                        Block* above_candidate_block
                            = &back_neighbour_blocks.data[above_candidate_index];

                        BlockIndex candidate_index = hvox::block_index(
                            { x, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1 }
                        );
                        Block* candidate_block
                            = &below_back_neighbour_blocks.data[candidate_index];

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
                        } candidate_block_vertex
                            = { impl::get_vertex(neighbour, candidate_block_coord),
                                impl::get_vertex(
                                    below_back_neighbour, candidate_block_coord
                                ) };

                        {
                            hmem::UniqueResourceLock lock;
                            auto navmesh = neighbour->navmesh.get(lock);

                            boost::add_edge(
                                neighbour_block_vertex.in_neighbour,
                                candidate_block_vertex.in_neighbour,
                                navmesh.data->graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.in_neighbour,
                                neighbour_block_vertex.in_neighbour,
                                navmesh.data->graph
                            );
                        }

                        {
                            hmem::UniqueResourceLock lock;
                            auto navmesh = below_back_neighbour->navmesh.get(lock);

                            boost::add_edge(
                                neighbour_block_vertex.in_below_back_neighbour,
                                candidate_block_vertex.in_below_back_neighbour,
                                navmesh.data->graph
                            );
                            boost::add_edge(
                                candidate_block_vertex.in_below_back_neighbour,
                                neighbour_block_vertex.in_below_back_neighbour,
                                navmesh.data->graph
                            );
                        }
                    }

                    // Step across and down from y == CHUNK_LENGTH - 1
                    for (BlockChunkPositionCoord x = 0; x < CHUNK_LENGTH; ++x) {
                        BlockIndex neighbour_block_index
                            = hvox::block_index({ x, CHUNK_LENGTH - 1, 0 });
                        Block* neighbour_block
                            = &neighbour_blocks.data[neighbour_block_index];

                        BlockIndex above_neighbour_block_index
                            = hvox::block_index({ x, 0, 0 });
                        Block* above_neighbour_block
                            = &blocks.data[above_neighbour_block_index];

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
                        } neighbour_block_vertex
                            = { impl::get_vertex(neighbour, neighbour_block_coord),
                                impl::get_vertex(
                                    below_back_neighbour, neighbour_block_coord
                                ) };

                        BlockIndex step_down_candidate_index = hvox::block_index(
                            { x, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1 }
                        );
                        Block* step_down_candidate_block
                            = &below_back_neighbour_blocks
                                   .data[step_down_candidate_index];

                        BlockIndex step_across_candidate_index = hvox::block_index(
                            { x, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1 }
                        );
                        Block* step_across_candidate_block
                            = &below_back_neighbour_blocks
                                   .data[step_across_candidate_index];

                        BlockIndex above_candidates_index
                            = hvox::block_index({ x, 0, CHUNK_LENGTH - 1 });
                        Block* above_candidates_block
                            = &back_neighbour_blocks.data[above_candidates_index];

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
                                = { impl::get_vertex(neighbour, candidate_block_coord),
                                    impl::get_vertex(
                                        below_back_neighbour, candidate_block_coord
                                    ) };

                            {
                                hmem::UniqueResourceLock lock;
                                auto navmesh = neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    neighbour_block_vertex.in_neighbour,
                                    candidate_block_vertex.in_neighbour,
                                    navmesh.data->graph
                                );
                                boost::add_edge(
                                    candidate_block_vertex.in_neighbour,
                                    neighbour_block_vertex.in_neighbour,
                                    navmesh.data->graph
                                );
                            }

                            {
                                hmem::UniqueResourceLock lock;
                                auto navmesh = below_back_neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    neighbour_block_vertex.in_below_back_neighbour,
                                    candidate_block_vertex.in_below_back_neighbour,
                                    navmesh.data->graph
                                );
                                boost::add_edge(
                                    candidate_block_vertex.in_below_back_neighbour,
                                    neighbour_block_vertex.in_below_back_neighbour,
                                    navmesh.data->graph
                                );
                            }
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
                                = { impl::get_vertex(neighbour, candidate_block_coord),
                                    impl::get_vertex(
                                        below_back_neighbour, candidate_block_coord
                                    ) };

                            {
                                hmem::UniqueResourceLock lock;
                                auto navmesh = neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    neighbour_block_vertex.in_neighbour,
                                    candidate_block_vertex.in_neighbour,
                                    navmesh.data->graph
                                );
                                boost::add_edge(
                                    candidate_block_vertex.in_neighbour,
                                    neighbour_block_vertex.in_neighbour,
                                    navmesh.data->graph
                                );
                            }

                            {
                                hmem::UniqueResourceLock lock;
                                auto navmesh = below_back_neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    neighbour_block_vertex.in_below_back_neighbour,
                                    candidate_block_vertex.in_below_back_neighbour,
                                    navmesh.data->graph
                                );
                                boost::add_edge(
                                    candidate_block_vertex.in_below_back_neighbour,
                                    neighbour_block_vertex.in_below_back_neighbour,
                                    navmesh.data->graph
                                );
                            }
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
