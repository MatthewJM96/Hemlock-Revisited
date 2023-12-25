#include "memory/handle.hpp"
#include "voxel/ai/navmesh/state.hpp"
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

namespace hemlock::voxel::ai::impl {
    inline ChunkNavmeshVertexDescriptor
    get_vertex(hmem::Handle<Chunk>& chunk, const ChunkNavmeshNode& coord) {
        try {
            std::shared_lock<std::shared_mutex> navmesh_lock;
            const hvox::ai::ChunkNavmesh* navmesh = chunk->navmesh.get(navmesh_lock);

            return navmesh->coord_vertex_map.at(coord);
        } catch (std::out_of_range&) {
            std::unique_lock<std::shared_mutex> navmesh_lock;
            hvox::ai::ChunkNavmesh* navmesh = chunk->navmesh.get(navmesh_lock);

            auto vertex                       = boost::add_vertex(navmesh->graph);
            navmesh->coord_vertex_map[coord]  = vertex;
            navmesh->vertex_coord_map[vertex] = coord;
            return vertex;
        }
    }

    inline ChunkNavmeshVertexDescriptor
    get_vertex(hvox::ai::ChunkNavmesh& navmesh, const ChunkNavmeshNode& coord) {
        try {
            return navmesh.coord_vertex_map.at(coord);
        } catch (std::out_of_range&) {
            auto vertex                      = boost::add_vertex(navmesh.graph);
            navmesh.coord_vertex_map[coord]  = vertex;
            navmesh.vertex_coord_map[vertex] = coord;
            return vertex;
        }
    }
}  // namespace hemlock::voxel::ai::impl

template <hvox::IdealVoxelConstraint IsSolid>
void hvox::ai::NaiveNavmeshStrategy<IsSolid>::do_bulk(
    hmem::Handle<ChunkGrid>, hmem::Handle<Chunk> chunk
) const {
    auto chunk_pos = chunk->position;

    std::shared_lock<std::shared_mutex> voxel_lock;
    const Voxel*                        voxels = chunk->voxels.get(voxel_lock);

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
    auto do_navigable_check = [&](const ChunkNavmeshVertexDescriptor& voxel_vertex,
                                  VoxelChunkPosition                  start_offset,
                                  VoxelChunkPosition                  offset,
                                  i64                                 start,
                                  i64                                 end) {
        for (i64 y_off = start; y_off > end; --y_off) {
            VoxelIndex above_candidate_index
                = hvox::voxel_index(static_cast<i64v3>(offset) + i64v3{ 0, y_off, 0 });
            const Voxel* above_candidate_voxel = &voxels[above_candidate_index];

            VoxelIndex candidate_index = hvox::voxel_index(
                static_cast<i64v3>(offset) + i64v3{ 0, y_off - 1, 0 }
            );
            const Voxel* candidate_voxel = &voxels[candidate_index];

            if (is_solid(candidate_voxel) && !is_solid(above_candidate_voxel)) {
                // TODO(Matthew): Put this in to make up for forgetting to ask if air
                //                gap exists to allow step up or down.
                //                  / _   _ /
                //                  _|     |_
                //                I.e. the slashed voxels in these two examples.
                //                Hardcoded for one step as that is all we're doing for
                //                now.
                if (y_off - 1 == 1) {
                    VoxelIndex twice_above_start_index = hvox::voxel_index(
                        start_offset + VoxelChunkPosition{ 0, 2, 0 }
                    );
                    const Voxel* twice_above_start_voxel
                        = &voxels[twice_above_start_index];

                    if (is_solid(twice_above_start_voxel)) continue;
                } else if (y_off - 1 == -1) {
                    VoxelIndex twice_above_candidate_index
                        = hvox::voxel_index(offset + VoxelChunkPosition{ 0, 1, 0 });
                    const Voxel* twice_above_candidate_voxel
                        = &voxels[twice_above_candidate_index];

                    if (is_solid(twice_above_candidate_voxel)) continue;
                }

                ChunkNavmeshNode candidate_voxel_coord = {
                    static_cast<i64v3>(offset) + i64v3{0, y_off - 1, 0},
                      chunk_pos
                };

                // Ensure node exists for this voxel.
                ChunkNavmeshVertexDescriptor candidate_voxel_vertex
                    = impl::get_vertex(chunk, candidate_voxel_coord);

                {
                    std::unique_lock<std::shared_mutex> lock;
                    auto& navmesh = chunk->navmesh.get(lock);

                    boost::add_edge(
                        voxel_vertex, candidate_voxel_vertex, navmesh->graph
                    );
                    boost::add_edge(
                        candidate_voxel_vertex, voxel_vertex, navmesh->graph
                    );
                }
            }
        }
    };

    /*******************************\
     * Navmesh bulk of this chunk. *
     *   i.e. not faces of chunk.  *
    \*******************************/

    for (VoxelChunkPositionCoord x = 1; x < CHUNK_LENGTH - 1; ++x) {
        for (VoxelChunkPositionCoord z = 1; z < CHUNK_LENGTH - 1; ++z) {
            for (VoxelChunkPositionCoord y = 1; y < CHUNK_LENGTH - 2; ++y) {
                VoxelIndex   voxel_index = hvox::voxel_index({ x, y, z });
                const Voxel* voxel       = &voxels[voxel_index];

                // Only consider voxel if it is solid.
                if (!is_solid(voxel)) continue;

                VoxelIndex   voxel_above_index = hvox::voxel_index({ x, y + 1, z });
                const Voxel* voxel_above       = &voxels[voxel_above_index];

                // Only consider voxel if it is not covered above.
                if (is_solid(voxel_above)) continue;

                // Ensure node exists for this voxel.
                ChunkNavmeshNode voxel_coord = {
                    {x, y, z},
                    chunk_pos
                };
                ChunkNavmeshVertexDescriptor voxel_vertex
                    = impl::get_vertex(chunk, voxel_coord);

                // Left
                do_navigable_check(voxel_vertex, { x, y, z }, { x - 1, y, z }, 2, -1);

                // Right
                do_navigable_check(voxel_vertex, { x, y, z }, { x + 1, y, z }, 2, -1);

                // Front
                do_navigable_check(voxel_vertex, { x, y, z }, { x, y, z + 1 }, 2, -1);

                // Back
                do_navigable_check(voxel_vertex, { x, y, z }, { x, y, z - 1 }, 2, -1);
            }

            // Second-to-top case.
            {
                VoxelIndex voxel_index = hvox::voxel_index({ x, CHUNK_LENGTH - 2, z });
                const Voxel* voxel     = &voxels[voxel_index];

                // Only consider voxel if it is solid.
                if (!is_solid(voxel)) continue;

                VoxelIndex voxel_above_index
                    = hvox::voxel_index({ x, CHUNK_LENGTH - 1, z });
                const Voxel* voxel_above = &voxels[voxel_above_index];

                // Only consider voxel if it is not covered above.
                if (is_solid(voxel_above)) continue;

                // Ensure node exists for this voxel.
                ChunkNavmeshNode voxel_coord = {
                    {x, CHUNK_LENGTH - 2, z},
                    chunk_pos
                };
                ChunkNavmeshVertexDescriptor voxel_vertex
                    = impl::get_vertex(chunk, voxel_coord);

                // Left
                do_navigable_check(
                    voxel_vertex,
                    { x, CHUNK_LENGTH - 2, z },
                    { x - 1, CHUNK_LENGTH - 2, z },
                    1,
                    -1
                );

                // Right
                do_navigable_check(
                    voxel_vertex,
                    { x, CHUNK_LENGTH - 2, z },
                    { x + 1, CHUNK_LENGTH - 2, z },
                    1,
                    -1
                );

                // Front
                do_navigable_check(
                    voxel_vertex,
                    { x, CHUNK_LENGTH - 2, z },
                    { x, CHUNK_LENGTH - 2, z + 1 },
                    1,
                    -1
                );

                // Back
                do_navigable_check(
                    voxel_vertex,
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
    for (VoxelChunkPositionCoord z = 1; z < CHUNK_LENGTH - 1; ++z) {
        for (VoxelChunkPositionCoord y = 1; y < CHUNK_LENGTH - 2; ++y) {
            VoxelIndex   voxel_index = hvox::voxel_index({ 0, y, z });
            const Voxel* voxel       = &voxels[voxel_index];

            // Only consider voxel if it is solid.
            if (!is_solid(voxel)) continue;

            VoxelIndex   voxel_above_index = hvox::voxel_index({ 0, y + 1, z });
            const Voxel* voxel_above       = &voxels[voxel_above_index];

            // Only consider voxel if it is not covered above.
            if (is_solid(voxel_above)) continue;

            // Ensure node exists for this voxel.
            ChunkNavmeshNode voxel_coord = {
                {0, y, z},
                chunk_pos
            };
            ChunkNavmeshVertexDescriptor voxel_vertex
                = impl::get_vertex(chunk, voxel_coord);

            // Right
            do_navigable_check(voxel_vertex, { 0, y, z }, { 1, y, z }, 2, -1);

            // Front
            do_navigable_check(voxel_vertex, { 0, y, z }, { 0, y, z + 1 }, 2, -1);

            // Back
            do_navigable_check(voxel_vertex, { 0, y, z }, { 0, y, z - 1 }, 2, -1);
        }
    }

    // Right Face (except second-to-top and top layers)
    for (VoxelChunkPositionCoord z = 1; z < CHUNK_LENGTH - 1; ++z) {
        for (VoxelChunkPositionCoord y = 1; y < CHUNK_LENGTH - 2; ++y) {
            VoxelIndex   voxel_index = hvox::voxel_index({ CHUNK_LENGTH - 1, y, z });
            const Voxel* voxel       = &voxels[voxel_index];

            // Only consider voxel if it is solid.
            if (!is_solid(voxel)) continue;

            VoxelIndex voxel_above_index
                = hvox::voxel_index({ CHUNK_LENGTH - 1, y + 1, z });
            const Voxel* voxel_above = &voxels[voxel_above_index];

            // Only consider voxel if it is not covered above.
            if (is_solid(voxel_above)) continue;

            // Ensure node exists for this voxel.
            ChunkNavmeshNode voxel_coord = {
                {CHUNK_LENGTH - 1, y, z},
                chunk_pos
            };
            ChunkNavmeshVertexDescriptor voxel_vertex
                = impl::get_vertex(chunk, voxel_coord);

            // Left
            do_navigable_check(
                voxel_vertex,
                { CHUNK_LENGTH - 1, y, z },
                { CHUNK_LENGTH - 2, y, z },
                2,
                -1
            );

            // Front
            do_navigable_check(
                voxel_vertex,
                { CHUNK_LENGTH - 1, y, z },
                { CHUNK_LENGTH - 1, y, z + 1 },
                2,
                -1
            );

            // Back
            do_navigable_check(
                voxel_vertex,
                { CHUNK_LENGTH - 1, y, z },
                { CHUNK_LENGTH - 1, y, z - 1 },
                2,
                -1
            );
        }
    }

    // Left Face (second-to-top case)
    for (VoxelChunkPositionCoord z = 1; z < CHUNK_LENGTH - 1; ++z) {
        for (VoxelChunkPositionCoord y = 1; y < CHUNK_LENGTH - 2; ++y) {
            VoxelIndex   voxel_index = hvox::voxel_index({ 0, CHUNK_LENGTH - 2, z });
            const Voxel* voxel       = &voxels[voxel_index];

            // Only consider voxel if it is solid.
            if (!is_solid(voxel)) continue;

            VoxelIndex voxel_above_index
                = hvox::voxel_index({ 0, CHUNK_LENGTH - 1, z });
            const Voxel* voxel_above = &voxels[voxel_above_index];

            // Only consider voxel if it is not covered above.
            if (is_solid(voxel_above)) continue;

            // Ensure node exists for this voxel.
            ChunkNavmeshNode voxel_coord = {
                {0, CHUNK_LENGTH - 2, z},
                chunk_pos
            };
            ChunkNavmeshVertexDescriptor voxel_vertex
                = impl::get_vertex(chunk, voxel_coord);

            // Right
            do_navigable_check(
                voxel_vertex,
                { 0, CHUNK_LENGTH - 2, z },
                { 1, CHUNK_LENGTH - 2, z },
                1,
                -1
            );

            // Front
            do_navigable_check(
                voxel_vertex,
                { 0, CHUNK_LENGTH - 2, z },
                { 0, CHUNK_LENGTH - 2, z + 1 },
                1,
                -1
            );

            // Back
            do_navigable_check(
                voxel_vertex,
                { 0, CHUNK_LENGTH - 2, z },
                { 0, CHUNK_LENGTH - 2, z - 1 },
                1,
                -1
            );
        }
    }

    // Right Face (second-to-top case)
    for (VoxelChunkPositionCoord z = 1; z < CHUNK_LENGTH - 1; ++z) {
        for (VoxelChunkPositionCoord y = 1; y < CHUNK_LENGTH - 2; ++y) {
            VoxelIndex voxel_index
                = hvox::voxel_index({ CHUNK_LENGTH - 1, CHUNK_LENGTH - 2, z });
            const Voxel* voxel = &voxels[voxel_index];

            // Only consider voxel if it is solid.
            if (!is_solid(voxel)) continue;

            VoxelIndex voxel_above_index
                = hvox::voxel_index({ CHUNK_LENGTH - 1, CHUNK_LENGTH - 1, z });
            const Voxel* voxel_above = &voxels[voxel_above_index];

            // Only consider voxel if it is not covered above.
            if (is_solid(voxel_above)) continue;

            // Ensure node exists for this voxel.
            ChunkNavmeshNode voxel_coord = {
                {CHUNK_LENGTH - 1, CHUNK_LENGTH - 2, z},
                chunk_pos
            };
            ChunkNavmeshVertexDescriptor voxel_vertex
                = impl::get_vertex(chunk, voxel_coord);

            // Left
            do_navigable_check(
                voxel_vertex,
                { CHUNK_LENGTH - 1, CHUNK_LENGTH - 2, z },
                { CHUNK_LENGTH - 2, CHUNK_LENGTH - 2, z },
                1,
                -1
            );

            // Front
            do_navigable_check(
                voxel_vertex,
                { CHUNK_LENGTH - 1, CHUNK_LENGTH - 2, z },
                { CHUNK_LENGTH - 1, CHUNK_LENGTH - 2, z + 1 },
                1,
                -1
            );

            // Back
            do_navigable_check(
                voxel_vertex,
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
    for (VoxelChunkPositionCoord x = 1; x < CHUNK_LENGTH - 1; ++x) {
        for (VoxelChunkPositionCoord y = 1; y < CHUNK_LENGTH - 2; ++y) {
            VoxelIndex   voxel_index = hvox::voxel_index({ x, y, CHUNK_LENGTH - 1 });
            const Voxel* voxel       = &voxels[voxel_index];

            // Only consider voxel if it is solid.
            if (!is_solid(voxel)) continue;

            VoxelIndex voxel_above_index
                = hvox::voxel_index({ x, y + 1, CHUNK_LENGTH - 1 });
            const Voxel* voxel_above = &voxels[voxel_above_index];

            // Only consider voxel if it is not covered above.
            if (is_solid(voxel_above)) continue;

            // Ensure node exists for this voxel.
            ChunkNavmeshNode voxel_coord = {
                {x, y, CHUNK_LENGTH - 1},
                chunk_pos
            };
            ChunkNavmeshVertexDescriptor voxel_vertex
                = impl::get_vertex(chunk, voxel_coord);

            // Left
            do_navigable_check(
                voxel_vertex,
                { x, y, CHUNK_LENGTH - 1 },
                { x - 1, y, CHUNK_LENGTH - 1 },
                2,
                -1
            );

            // Right
            do_navigable_check(
                voxel_vertex,
                { x, y, CHUNK_LENGTH - 1 },
                { x + 1, y, CHUNK_LENGTH - 1 },
                2,
                -1
            );

            // Back
            do_navigable_check(
                voxel_vertex,
                { x, y, CHUNK_LENGTH - 1 },
                { x, y, CHUNK_LENGTH - 2 },
                2,
                -1
            );
        }
    }

    // Back Face (except second-to-top and top layers)
    for (VoxelChunkPositionCoord x = 1; x < CHUNK_LENGTH - 1; ++x) {
        for (VoxelChunkPositionCoord y = 1; y < CHUNK_LENGTH - 2; ++y) {
            VoxelIndex   voxel_index = hvox::voxel_index({ x, y, 0 });
            const Voxel* voxel       = &voxels[voxel_index];

            // Only consider voxel if it is solid.
            if (!is_solid(voxel)) continue;

            VoxelIndex   voxel_above_index = hvox::voxel_index({ x, y + 1, 0 });
            const Voxel* voxel_above       = &voxels[voxel_above_index];

            // Only consider voxel if it is not covered above.
            if (is_solid(voxel_above)) continue;

            // Ensure node exists for this voxel.
            ChunkNavmeshNode voxel_coord = {
                {x, y, 0},
                chunk_pos
            };
            ChunkNavmeshVertexDescriptor voxel_vertex
                = impl::get_vertex(chunk, voxel_coord);

            // Left
            do_navigable_check(voxel_vertex, { x, y, 0 }, { x - 1, y, 0 }, 2, -1);

            // Right
            do_navigable_check(voxel_vertex, { x, y, 0 }, { x + 1, y, 0 }, 2, -1);

            // Front
            do_navigable_check(voxel_vertex, { x, y, 0 }, { x, y, 1 }, 2, -1);
        }
    }

    // Front Face (second-to-top case)
    for (VoxelChunkPositionCoord x = 1; x < CHUNK_LENGTH - 1; ++x) {
        for (VoxelChunkPositionCoord y = 1; y < CHUNK_LENGTH - 2; ++y) {
            VoxelIndex voxel_index
                = hvox::voxel_index({ x, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1 });
            const Voxel* voxel = &voxels[voxel_index];

            // Only consider voxel if it is solid.
            if (!is_solid(voxel)) continue;

            VoxelIndex voxel_above_index
                = hvox::voxel_index({ x, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1 });
            const Voxel* voxel_above = &voxels[voxel_above_index];

            // Only consider voxel if it is not covered above.
            if (is_solid(voxel_above)) continue;

            // Ensure node exists for this voxel.
            ChunkNavmeshNode voxel_coord = {
                {x, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1},
                chunk_pos
            };
            ChunkNavmeshVertexDescriptor voxel_vertex
                = impl::get_vertex(chunk, voxel_coord);

            // Left
            do_navigable_check(
                voxel_vertex,
                { x, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1 },
                { x - 1, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1 },
                1,
                -1
            );

            // Right
            do_navigable_check(
                voxel_vertex,
                { x, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1 },
                { x + 1, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1 },
                1,
                -1
            );

            // Back
            do_navigable_check(
                voxel_vertex,
                { x, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1 },
                { x, CHUNK_LENGTH - 2, CHUNK_LENGTH - 2 },
                1,
                -1
            );
        }
    }

    // Back Face (second-to-top case)
    for (VoxelChunkPositionCoord x = 1; x < CHUNK_LENGTH - 1; ++x) {
        for (VoxelChunkPositionCoord y = 1; y < CHUNK_LENGTH - 2; ++y) {
            VoxelIndex   voxel_index = hvox::voxel_index({ x, CHUNK_LENGTH - 2, 0 });
            const Voxel* voxel       = &voxels[voxel_index];

            // Only consider voxel if it is solid.
            if (!is_solid(voxel)) continue;

            VoxelIndex voxel_above_index
                = hvox::voxel_index({ x, CHUNK_LENGTH - 1, 0 });
            const Voxel* voxel_above = &voxels[voxel_above_index];

            // Only consider voxel if it is not covered above.
            if (is_solid(voxel_above)) continue;

            // Ensure node exists for this voxel.
            ChunkNavmeshNode voxel_coord = {
                {x, CHUNK_LENGTH - 2, 0},
                chunk_pos
            };
            ChunkNavmeshVertexDescriptor voxel_vertex
                = impl::get_vertex(chunk, voxel_coord);

            // Left
            do_navigable_check(
                voxel_vertex,
                { x, CHUNK_LENGTH - 2, 0 },
                { x - 1, CHUNK_LENGTH - 2, 0 },
                1,
                -1
            );

            // Right
            do_navigable_check(
                voxel_vertex,
                { x, CHUNK_LENGTH - 2, 0 },
                { x + 1, CHUNK_LENGTH - 2, 0 },
                1,
                -1
            );

            // Front
            do_navigable_check(
                voxel_vertex,
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

    for (VoxelChunkPositionCoord x = 1; x < CHUNK_LENGTH - 1; ++x) {
        for (VoxelChunkPositionCoord z = 1; z < CHUNK_LENGTH - 1; ++z) {
            // Bottom Face
            {
                VoxelIndex   voxel_index = hvox::voxel_index({ x, 0, z });
                const Voxel* voxel       = &voxels[voxel_index];

                // Only consider voxel if it is solid.
                if (!is_solid(voxel)) continue;

                VoxelIndex   voxel_above_index = hvox::voxel_index({ x, 1, z });
                const Voxel* voxel_above       = &voxels[voxel_above_index];

                // Only consider voxel if it is not covered above.
                if (is_solid(voxel_above)) continue;

                // Ensure node exists for this voxel.
                ChunkNavmeshNode voxel_coord = {
                    {x, 0, z},
                    chunk_pos
                };
                ChunkNavmeshVertexDescriptor voxel_vertex
                    = impl::get_vertex(chunk, voxel_coord);

                // Left
                do_navigable_check(voxel_vertex, { x, 0, z }, { x - 1, 0, z }, 2, 0);

                // Right
                do_navigable_check(voxel_vertex, { x, 0, z }, { x + 1, 0, z }, 2, 0);

                // Front
                do_navigable_check(voxel_vertex, { x, 0, z }, { x, 0, z + 1 }, 2, 0);

                // Back
                do_navigable_check(voxel_vertex, { x, 0, z }, { x, 0, z - 1 }, 2, 0);
            }
        }
    }

    /**************************\
     * Navmesh vertical edges *
     *   except for corners.  *
    \**************************/

    // Front-Left Edge
    for (VoxelChunkPositionCoord y = 1; y < CHUNK_LENGTH - 2; ++y) {
        VoxelIndex   voxel_index = hvox::voxel_index({ 0, y, CHUNK_LENGTH - 1 });
        const Voxel* voxel       = &voxels[voxel_index];

        // Only consider voxel if it is solid.
        if (!is_solid(voxel)) continue;

        VoxelIndex voxel_above_index
            = hvox::voxel_index({ 0, y + 1, CHUNK_LENGTH - 1 });
        const Voxel* voxel_above = &voxels[voxel_above_index];

        // Only consider voxel if it is not covered above.
        if (is_solid(voxel_above)) continue;

        // Ensure node exists for this voxel.
        ChunkNavmeshNode voxel_coord = {
            {0, y, CHUNK_LENGTH - 1},
            chunk_pos
        };
        ChunkNavmeshVertexDescriptor voxel_vertex
            = impl::get_vertex(chunk, voxel_coord);

        // Right
        do_navigable_check(
            voxel_vertex, { 0, y, CHUNK_LENGTH - 1 }, { 1, y, CHUNK_LENGTH - 1 }, 2, -1
        );

        // Back
        do_navigable_check(
            voxel_vertex, { 0, y, CHUNK_LENGTH - 1 }, { 0, y, CHUNK_LENGTH - 2 }, 2, -1
        );
    }

    // Front-Right Edge
    for (VoxelChunkPositionCoord y = 1; y < CHUNK_LENGTH - 2; ++y) {
        VoxelIndex voxel_index
            = hvox::voxel_index({ CHUNK_LENGTH - 1, y, CHUNK_LENGTH - 1 });
        const Voxel* voxel = &voxels[voxel_index];

        // Only consider voxel if it is solid.
        if (!is_solid(voxel)) continue;

        VoxelIndex voxel_above_index
            = hvox::voxel_index({ CHUNK_LENGTH - 1, y + 1, CHUNK_LENGTH - 1 });
        const Voxel* voxel_above = &voxels[voxel_above_index];

        // Only consider voxel if it is not covered above.
        if (is_solid(voxel_above)) continue;

        // Ensure node exists for this voxel.
        ChunkNavmeshNode voxel_coord = {
            {CHUNK_LENGTH - 1, y, CHUNK_LENGTH - 1},
            chunk_pos
        };
        ChunkNavmeshVertexDescriptor voxel_vertex
            = impl::get_vertex(chunk, voxel_coord);

        // Left
        do_navigable_check(
            voxel_vertex,
            { CHUNK_LENGTH - 1, y, CHUNK_LENGTH - 1 },
            { CHUNK_LENGTH - 2, y, CHUNK_LENGTH - 1 },
            2,
            -1
        );

        // Back
        do_navigable_check(
            voxel_vertex,
            { CHUNK_LENGTH - 1, y, CHUNK_LENGTH - 1 },
            { CHUNK_LENGTH - 1, y, CHUNK_LENGTH - 2 },
            2,
            -1
        );
    }

    // Back-Left Edge
    for (VoxelChunkPositionCoord y = 1; y < CHUNK_LENGTH - 2; ++y) {
        VoxelIndex   voxel_index = hvox::voxel_index({ 0, y, 0 });
        const Voxel* voxel       = &voxels[voxel_index];

        // Only consider voxel if it is solid.
        if (!is_solid(voxel)) continue;

        VoxelIndex   voxel_above_index = hvox::voxel_index({ 0, y + 1, 0 });
        const Voxel* voxel_above       = &voxels[voxel_above_index];

        // Only consider voxel if it is not covered above.
        if (is_solid(voxel_above)) continue;

        // Ensure node exists for this voxel.
        ChunkNavmeshNode voxel_coord = {
            {0, y, 0},
            chunk_pos
        };
        ChunkNavmeshVertexDescriptor voxel_vertex
            = impl::get_vertex(chunk, voxel_coord);

        // Right
        do_navigable_check(voxel_vertex, { 0, y, 0 }, { 1, y, 0 }, 2, -1);

        // Front
        do_navigable_check(voxel_vertex, { 0, y, 0 }, { 0, y, 1 }, 2, -1);
    }

    // Back-Right Edge
    for (VoxelChunkPositionCoord y = 1; y < CHUNK_LENGTH - 2; ++y) {
        VoxelIndex   voxel_index = hvox::voxel_index({ CHUNK_LENGTH - 1, y, 0 });
        const Voxel* voxel       = &voxels[voxel_index];

        // Only consider voxel if it is solid.
        if (!is_solid(voxel)) continue;

        VoxelIndex voxel_above_index
            = hvox::voxel_index({ CHUNK_LENGTH - 1, y + 1, 0 });
        const Voxel* voxel_above = &voxels[voxel_above_index];

        // Only consider voxel if it is not covered above.
        if (is_solid(voxel_above)) continue;

        // Ensure node exists for this voxel.
        ChunkNavmeshNode voxel_coord = {
            {CHUNK_LENGTH - 1, y, 0},
            chunk_pos
        };
        ChunkNavmeshVertexDescriptor voxel_vertex
            = impl::get_vertex(chunk, voxel_coord);

        // Left
        do_navigable_check(
            voxel_vertex, { CHUNK_LENGTH - 1, y, 0 }, { CHUNK_LENGTH - 2, y, 0 }, 2, -1
        );

        // Front
        do_navigable_check(
            voxel_vertex, { CHUNK_LENGTH - 1, y, 0 }, { CHUNK_LENGTH - 1, y, 1 }, 2, -1
        );
    }

    // Second-to-top case
    // Front-Left Edge
    {
        VoxelIndex voxel_index
            = hvox::voxel_index({ 0, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1 });
        const Voxel* voxel = &voxels[voxel_index];

        VoxelIndex voxel_above_index
            = hvox::voxel_index({ 0, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1 });
        const Voxel* voxel_above = &voxels[voxel_above_index];

        // Only consider voxel if it is not covered above.
        if (is_solid(voxel) && !is_solid(voxel_above)) {
            // Ensure node exists for this voxel.
            ChunkNavmeshNode voxel_coord = {
                {0, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1},
                chunk_pos
            };
            ChunkNavmeshVertexDescriptor voxel_vertex
                = impl::get_vertex(chunk, voxel_coord);

            // Right
            do_navigable_check(
                voxel_vertex,
                { 0, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1 },
                { 1, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1 },
                1,
                -1
            );

            // Back
            do_navigable_check(
                voxel_vertex,
                { 0, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1 },
                { 0, CHUNK_LENGTH - 2, CHUNK_LENGTH - 2 },
                1,
                -1
            );
        }
    }

    // Front-Right Edge
    {
        VoxelIndex voxel_index
            = hvox::voxel_index({ CHUNK_LENGTH - 1, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1 }
            );
        const Voxel* voxel = &voxels[voxel_index];

        VoxelIndex voxel_above_index
            = hvox::voxel_index({ CHUNK_LENGTH - 1, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1 }
            );
        const Voxel* voxel_above = &voxels[voxel_above_index];

        // Only consider voxel if it is not covered above.
        if (is_solid(voxel) && !is_solid(voxel_above)) {
            // Ensure node exists for this voxel.
            ChunkNavmeshNode voxel_coord = {
                {CHUNK_LENGTH - 1, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1},
                chunk_pos
            };
            ChunkNavmeshVertexDescriptor voxel_vertex
                = impl::get_vertex(chunk, voxel_coord);

            // Left
            do_navigable_check(
                voxel_vertex,
                { CHUNK_LENGTH - 1, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1 },
                { CHUNK_LENGTH - 2, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1 },
                1,
                -1
            );

            // Back
            do_navigable_check(
                voxel_vertex,
                { CHUNK_LENGTH - 1, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1 },
                { CHUNK_LENGTH - 1, CHUNK_LENGTH - 2, CHUNK_LENGTH - 2 },
                1,
                -1
            );
        }
    }

    // Back-Left Edge
    {
        VoxelIndex   voxel_index = hvox::voxel_index({ 0, CHUNK_LENGTH - 2, 0 });
        const Voxel* voxel       = &voxels[voxel_index];

        VoxelIndex   voxel_above_index = hvox::voxel_index({ 0, CHUNK_LENGTH - 1, 0 });
        const Voxel* voxel_above       = &voxels[voxel_above_index];

        // Only consider voxel if it is not covered above.
        if (is_solid(voxel) && !is_solid(voxel_above)) {
            // Ensure node exists for this voxel.
            ChunkNavmeshNode voxel_coord = {
                {0, CHUNK_LENGTH - 2, 0},
                chunk_pos
            };
            ChunkNavmeshVertexDescriptor voxel_vertex
                = impl::get_vertex(chunk, voxel_coord);

            // Right
            do_navigable_check(
                voxel_vertex,
                { 0, CHUNK_LENGTH - 2, 0 },
                { 1, CHUNK_LENGTH - 2, 0 },
                1,
                -1
            );

            // Front
            do_navigable_check(
                voxel_vertex,
                { 0, CHUNK_LENGTH - 2, 0 },
                { 0, CHUNK_LENGTH - 2, 1 },
                1,
                -1
            );
        }
    }

    // Back-Right Edge
    {
        VoxelIndex voxel_index
            = hvox::voxel_index({ CHUNK_LENGTH - 1, CHUNK_LENGTH - 2, 0 });
        const Voxel* voxel = &voxels[voxel_index];

        VoxelIndex voxel_above_index
            = hvox::voxel_index({ CHUNK_LENGTH - 1, CHUNK_LENGTH - 1, 0 });
        const Voxel* voxel_above = &voxels[voxel_above_index];

        // Only consider voxel if it is not covered above.
        if (is_solid(voxel) && !is_solid(voxel_above)) {
            // Ensure node exists for this voxel.
            ChunkNavmeshNode voxel_coord = {
                {CHUNK_LENGTH - 1, CHUNK_LENGTH - 2, 0},
                chunk_pos
            };
            ChunkNavmeshVertexDescriptor voxel_vertex
                = impl::get_vertex(chunk, voxel_coord);

            // Left
            do_navigable_check(
                voxel_vertex,
                { CHUNK_LENGTH - 1, CHUNK_LENGTH - 2, 0 },
                { CHUNK_LENGTH - 2, CHUNK_LENGTH - 2, 0 },
                1,
                -1
            );

            // Front
            do_navigable_check(
                voxel_vertex,
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
    for (VoxelChunkPositionCoord x = 1; x < CHUNK_LENGTH - 1; ++x) {
        VoxelIndex   voxel_index = hvox::voxel_index({ x, 0, CHUNK_LENGTH - 1 });
        const Voxel* voxel       = &voxels[voxel_index];

        // Only consider voxel if it is solid.
        if (!is_solid(voxel)) continue;

        VoxelIndex   voxel_above_index = hvox::voxel_index({ x, 1, CHUNK_LENGTH - 1 });
        const Voxel* voxel_above       = &voxels[voxel_above_index];

        // Only consider voxel if it is not covered above.
        if (is_solid(voxel_above)) continue;

        // Ensure node exists for this voxel.
        ChunkNavmeshNode voxel_coord = {
            {x, 0, CHUNK_LENGTH - 1},
            chunk_pos
        };
        ChunkNavmeshVertexDescriptor voxel_vertex
            = impl::get_vertex(chunk, voxel_coord);

        // Left
        do_navigable_check(
            voxel_vertex,
            { x, 0, CHUNK_LENGTH - 1 },
            { x - 1, 0, CHUNK_LENGTH - 1 },
            2,
            0
        );

        // Right
        do_navigable_check(
            voxel_vertex,
            { x, 0, CHUNK_LENGTH - 1 },
            { x + 1, 0, CHUNK_LENGTH - 1 },
            2,
            0
        );

        // Back
        do_navigable_check(
            voxel_vertex, { x, 0, CHUNK_LENGTH - 1 }, { x, 0, CHUNK_LENGTH - 2 }, 2, 0
        );
    }

    // Back-Bottom Edge
    for (VoxelChunkPositionCoord x = 1; x < CHUNK_LENGTH - 1; ++x) {
        VoxelIndex   voxel_index = hvox::voxel_index({ x, 0, 0 });
        const Voxel* voxel       = &voxels[voxel_index];

        // Only consider voxel if it is solid.
        if (!is_solid(voxel)) continue;

        VoxelIndex   voxel_above_index = hvox::voxel_index({ x, 1, 0 });
        const Voxel* voxel_above       = &voxels[voxel_above_index];

        // Only consider voxel if it is not covered above.
        if (is_solid(voxel_above)) continue;

        // Ensure node exists for this voxel.
        ChunkNavmeshNode voxel_coord = {
            {x, 0, 0},
            chunk_pos
        };
        ChunkNavmeshVertexDescriptor voxel_vertex
            = impl::get_vertex(chunk, voxel_coord);

        // Left
        do_navigable_check(voxel_vertex, { x, 0, 0 }, { x - 1, 0, 0 }, 2, 0);

        // Right
        do_navigable_check(voxel_vertex, { x, 0, 0 }, { x + 1, 0, 0 }, 2, 0);

        // Front
        do_navigable_check(voxel_vertex, { x, 0, 0 }, { x, 0, 1 }, 2, 0);
    }

    // Front-Back Edges

    // Left-Bottom Edge
    for (VoxelChunkPositionCoord z = 1; z < CHUNK_LENGTH - 1; ++z) {
        VoxelIndex   voxel_index = hvox::voxel_index({ 0, 0, z });
        const Voxel* voxel       = &voxels[voxel_index];

        // Only consider voxel if it is solid.
        if (!is_solid(voxel)) continue;

        VoxelIndex   voxel_above_index = hvox::voxel_index({ 0, 1, z });
        const Voxel* voxel_above       = &voxels[voxel_above_index];

        // Only consider voxel if it is not covered above.
        if (is_solid(voxel_above)) continue;

        // Ensure node exists for this voxel.
        ChunkNavmeshNode voxel_coord = {
            {0, 0, z},
            chunk_pos
        };
        ChunkNavmeshVertexDescriptor voxel_vertex
            = impl::get_vertex(chunk, voxel_coord);

        // Right
        do_navigable_check(voxel_vertex, { 0, 0, z }, { 1, 0, z }, 2, 0);

        // Front
        do_navigable_check(voxel_vertex, { 0, 0, z }, { 0, 0, z + 1 }, 2, 0);

        // Back
        do_navigable_check(voxel_vertex, { 0, 0, z }, { 0, 0, z - 1 }, 2, 0);
    }

    // Right-Bottom Edge
    for (VoxelChunkPositionCoord z = 1; z < CHUNK_LENGTH - 1; ++z) {
        VoxelIndex   voxel_index = hvox::voxel_index({ CHUNK_LENGTH - 1, 0, z });
        const Voxel* voxel       = &voxels[voxel_index];

        // Only consider voxel if it is solid.
        if (!is_solid(voxel)) continue;

        VoxelIndex   voxel_above_index = hvox::voxel_index({ CHUNK_LENGTH - 1, 1, z });
        const Voxel* voxel_above       = &voxels[voxel_above_index];

        // Only consider voxel if it is not covered above.
        if (is_solid(voxel_above)) continue;

        // Ensure node exists for this voxel.
        ChunkNavmeshNode voxel_coord = {
            {CHUNK_LENGTH - 1, 0, z},
            chunk_pos
        };
        ChunkNavmeshVertexDescriptor voxel_vertex
            = impl::get_vertex(chunk, voxel_coord);

        // Left
        do_navigable_check(
            voxel_vertex, { CHUNK_LENGTH - 1, 0, z }, { CHUNK_LENGTH - 2, 0, z }, 2, 0
        );

        // Front
        do_navigable_check(
            voxel_vertex,
            { CHUNK_LENGTH - 1, 0, z },
            { CHUNK_LENGTH - 1, 0, z + 1 },
            2,
            0
        );

        // Back
        do_navigable_check(
            voxel_vertex,
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
        VoxelIndex   voxel_index = hvox::voxel_index({ 0, 0, CHUNK_LENGTH - 1 });
        const Voxel* voxel       = &voxels[voxel_index];

        VoxelIndex   voxel_above_index = hvox::voxel_index({ 0, 1, CHUNK_LENGTH - 1 });
        const Voxel* voxel_above       = &voxels[voxel_above_index];

        // Only consider voxel if it is not covered above.
        if (is_solid(voxel) && !is_solid(voxel_above)) {
            // Ensure node exists for this voxel.
            ChunkNavmeshNode voxel_coord = {
                {0, 0, CHUNK_LENGTH - 1},
                chunk_pos
            };
            ChunkNavmeshVertexDescriptor voxel_vertex
                = impl::get_vertex(chunk, voxel_coord);

            // Right
            do_navigable_check(
                voxel_vertex,
                { 0, 0, CHUNK_LENGTH - 1 },
                { 1, 0, CHUNK_LENGTH - 1 },
                2,
                0
            );

            // Back
            do_navigable_check(
                voxel_vertex,
                { 0, 0, CHUNK_LENGTH - 1 },
                { 0, 0, CHUNK_LENGTH - 2 },
                2,
                0
            );
        }
    }

    // Left-Bottom-Back
    {
        VoxelIndex   voxel_index = hvox::voxel_index({ 0, 0, 0 });
        const Voxel* voxel       = &voxels[voxel_index];

        VoxelIndex   voxel_above_index = hvox::voxel_index({ 0, 1, 0 });
        const Voxel* voxel_above       = &voxels[voxel_above_index];

        // Only consider voxel if it is not covered above.
        if (is_solid(voxel) && !is_solid(voxel_above)) {
            // Ensure node exists for this voxel.
            ChunkNavmeshNode voxel_coord = {
                {0, 0, 0},
                chunk_pos
            };
            ChunkNavmeshVertexDescriptor voxel_vertex
                = impl::get_vertex(chunk, voxel_coord);

            // Right
            do_navigable_check(voxel_vertex, { 0, 0, 0 }, { 1, 0, 0 }, 2, 0);

            // Front
            do_navigable_check(voxel_vertex, { 0, 0, 0 }, { 0, 0, 1 }, 2, 0);
        }
    }

    // Right-Bottom-Front
    {
        VoxelIndex voxel_index
            = hvox::voxel_index({ CHUNK_LENGTH - 1, 0, CHUNK_LENGTH - 1 });
        const Voxel* voxel = &voxels[voxel_index];

        VoxelIndex voxel_above_index
            = hvox::voxel_index({ CHUNK_LENGTH - 1, 1, CHUNK_LENGTH - 1 });
        const Voxel* voxel_above = &voxels[voxel_above_index];

        // Only consider voxel if it is not covered above.
        if (is_solid(voxel) && !is_solid(voxel_above)) {
            // Ensure node exists for this voxel.
            ChunkNavmeshNode voxel_coord = {
                {CHUNK_LENGTH - 1, 0, CHUNK_LENGTH - 1},
                chunk_pos
            };
            ChunkNavmeshVertexDescriptor voxel_vertex
                = impl::get_vertex(chunk, voxel_coord);

            // Left
            do_navigable_check(
                voxel_vertex,
                { CHUNK_LENGTH - 1, 0, CHUNK_LENGTH - 1 },
                { CHUNK_LENGTH - 2, 0, CHUNK_LENGTH - 1 },
                2,
                0
            );

            // Back
            do_navigable_check(
                voxel_vertex,
                { CHUNK_LENGTH - 1, 0, CHUNK_LENGTH - 1 },
                { CHUNK_LENGTH - 1, 0, CHUNK_LENGTH - 2 },
                2,
                0
            );
        }
    }

    // Right-Bottom-Back
    {
        VoxelIndex   voxel_index = hvox::voxel_index({ CHUNK_LENGTH - 1, 0, 0 });
        const Voxel* voxel       = &voxels[voxel_index];

        VoxelIndex   voxel_above_index = hvox::voxel_index({ CHUNK_LENGTH - 1, 1, 0 });
        const Voxel* voxel_above       = &voxels[voxel_above_index];

        // Only consider voxel if it is not covered above.
        if (is_solid(voxel) && !is_solid(voxel_above)) {
            // Ensure node exists for this voxel.
            ChunkNavmeshNode voxel_coord = {
                {CHUNK_LENGTH - 1, 0, 0},
                chunk_pos
            };
            ChunkNavmeshVertexDescriptor voxel_vertex
                = impl::get_vertex(chunk, voxel_coord);

            // Left
            do_navigable_check(
                voxel_vertex,
                { CHUNK_LENGTH - 1, 0, 0 },
                { CHUNK_LENGTH - 2, 0, 0 },
                2,
                0
            );

            // Front
            do_navigable_check(
                voxel_vertex,
                { CHUNK_LENGTH - 1, 0, 0 },
                { CHUNK_LENGTH - 1, 0, 1 },
                2,
                0
            );
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
//                                                                                    //
//                                   Stitch Method                                    //
//                                                                                    //
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

template <hvox::IdealVoxelConstraint IsSolid>
void hvox::ai::NaiveNavmeshStrategy<IsSolid>::do_stitch(
    hmem::Handle<ChunkGrid>, hmem::Handle<Chunk> chunk
) const {
    auto chunk_pos = chunk->position;

    std::shared_lock<std::shared_mutex> voxel_lock;
    const Voxel*                        voxels = chunk->voxels.get(voxel_lock);

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
                                              VoxelChunkPosition     this_offset,
                                              VoxelChunkPosition     neighbour_offset,
                                              i64                    start,
                                              i64                    end) {
        std::shared_lock<std::shared_mutex> neighbour_voxel_lock;
        auto& neighbour_voxels = neighbour->voxels.get(neighbour_voxel_lock);

        VoxelIndex   this_voxel_index = hvox::voxel_index(this_offset);
        const Voxel* this_voxel       = &voxels[this_voxel_index];

        VoxelIndex above_this_voxel_index
            = hvox::voxel_index(this_offset + VoxelChunkPosition{ 0, 1, 0 });
        const Voxel* above_this_voxel = &voxels[above_this_voxel_index];

        if (!is_solid(this_voxel) || is_solid(above_this_voxel)) return;

        ChunkNavmeshNode this_voxel_coord = { this_offset, chunk_pos };
        struct {
            ChunkNavmeshVertexDescriptor here, in_neighbour;
        } this_voxel_vertex = { impl::get_vertex(chunk, this_voxel_coord),
                                impl::get_vertex(neighbour, this_voxel_coord) };

        for (i64 y_off = start; y_off > end; --y_off) {
            VoxelIndex above_candidate_index = hvox::voxel_index(
                static_cast<i64v3>(neighbour_offset) + i64v3{ 0, y_off, 0 }
            );
            const Voxel* above_candidate_voxel
                = &neighbour_voxels[above_candidate_index];

            VoxelIndex candidate_index = hvox::voxel_index(
                static_cast<i64v3>(neighbour_offset) + i64v3{ 0, y_off - 1, 0 }
            );
            const Voxel* candidate_voxel = &neighbour_voxels[candidate_index];

            if (is_solid(candidate_voxel) && !is_solid(above_candidate_voxel)) {
                ChunkNavmeshNode candidate_voxel_coord = {
                    static_cast<i64v3>(neighbour_offset) + i64v3{0, y_off - 1, 0},
                    neighbour->position
                };
                struct {
                    ChunkNavmeshVertexDescriptor here, in_neighbour;
                } candidate_voxel_vertex
                    = { impl::get_vertex(chunk, candidate_voxel_coord),
                        impl::get_vertex(neighbour, candidate_voxel_coord) };

                {
                    std::unique_lock<std::shared_mutex> lock;
                    auto& navmesh = chunk->navmesh.get(lock);

                    boost::add_edge(
                        this_voxel_vertex.here,
                        candidate_voxel_vertex.here,
                        navmesh->graph
                    );
                    boost::add_edge(
                        candidate_voxel_vertex.here,
                        this_voxel_vertex.here,
                        navmesh->graph
                    );
                }

                {
                    std::unique_lock<std::shared_mutex> lock;
                    auto& navmesh = neighbour->navmesh.get(lock);

                    boost::add_edge(
                        this_voxel_vertex.in_neighbour,
                        candidate_voxel_vertex.in_neighbour,
                        navmesh->graph
                    );
                    boost::add_edge(
                        candidate_voxel_vertex.in_neighbour,
                        this_voxel_vertex.in_neighbour,
                        navmesh->graph
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
            std::shared_lock<std::shared_mutex> neighbour_voxel_lock;
            auto& neighbour_voxels = neighbour->voxels.get(neighbour_voxel_lock);

            for (VoxelChunkPositionCoord z = 0; z < CHUNK_LENGTH; ++z) {
                for (VoxelChunkPositionCoord y = 1; y < CHUNK_LENGTH - 2; ++y) {
                    do_side_stitch_navigable_check(
                        neighbour, { 0, y, z }, { CHUNK_LENGTH - 1, y, z }, 2, -1
                    );
                }
            }

            // Special handling for
            //      y == 0, CHUNK_LENGTH - 2
            // where chunks above need to be loaded too.

            // y == 0, CHUNK_LENGTH - 2
            for (VoxelChunkPositionCoord z = 0; z < CHUNK_LENGTH; ++z) {
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
                std::shared_lock<std::shared_mutex> below_neighbour_voxel_lock;
                auto                                below_neighbour_voxels
                    = below_neighbour->voxels.get(below_neighbour_voxel_lock);

                for (VoxelChunkPositionCoord z = 0; z < CHUNK_LENGTH; ++z) {
                    VoxelIndex   this_voxel_index = hvox::voxel_index({ 0, 0, z });
                    const Voxel* this_voxel       = &voxels[this_voxel_index];

                    VoxelIndex above_this_voxel_index = hvox::voxel_index({ 0, 1, z });
                    const Voxel* above_this_voxel     = &voxels[above_this_voxel_index];

                    if (!is_solid(this_voxel) || is_solid(above_this_voxel)) continue;

                    ChunkNavmeshNode this_voxel_coord = {
                        {0, 0, z},
                        chunk_pos
                    };

                    struct {
                        ChunkNavmeshVertexDescriptor here, in_below_neighbour;
                    } this_voxel_vertex
                        = { impl::get_vertex(chunk, this_voxel_coord),
                            impl::get_vertex(below_neighbour, this_voxel_coord) };

                    VoxelIndex twice_above_candidate_index
                        = hvox::voxel_index({ CHUNK_LENGTH - 1, 1, z });
                    const Voxel* twice_above_candidate_voxel
                        = &neighbour_voxels[twice_above_candidate_index];

                    VoxelIndex above_candidate_index
                        = hvox::voxel_index({ CHUNK_LENGTH - 1, 0, z });
                    const Voxel* above_candidate_voxel
                        = &neighbour_voxels[above_candidate_index];

                    VoxelIndex candidate_index
                        = hvox::voxel_index({ CHUNK_LENGTH - 1, CHUNK_LENGTH - 1, z });
                    const Voxel* candidate_voxel
                        = &below_neighbour_voxels[candidate_index];

                    if (is_solid(candidate_voxel) && !is_solid(above_candidate_voxel)
                        && !is_solid(twice_above_candidate_voxel))
                    {
                        ChunkNavmeshNode candidate_voxel_coord = {
                            {CHUNK_LENGTH - 1, CHUNK_LENGTH - 1, z},
                            below_neighbour->position
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor here, in_below_neighbour;
                        } candidate_voxel_vertex = {
                            impl::get_vertex(chunk, candidate_voxel_coord),
                            impl::get_vertex(below_neighbour, candidate_voxel_coord)
                        };

                        {
                            std::unique_lock<std::shared_mutex> lock;
                            auto& navmesh = chunk->navmesh.get(lock);

                            boost::add_edge(
                                this_voxel_vertex.here,
                                candidate_voxel_vertex.here,
                                navmesh->graph
                            );
                            boost::add_edge(
                                candidate_voxel_vertex.here,
                                this_voxel_vertex.here,
                                navmesh->graph
                            );
                        }

                        {
                            std::unique_lock<std::shared_mutex> lock;
                            auto& navmesh = below_neighbour->navmesh.get(lock);

                            boost::add_edge(
                                this_voxel_vertex.in_below_neighbour,
                                candidate_voxel_vertex.in_below_neighbour,
                                navmesh->graph
                            );
                            boost::add_edge(
                                candidate_voxel_vertex.in_below_neighbour,
                                this_voxel_vertex.in_below_neighbour,
                                navmesh->graph
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
            std::shared_lock<std::shared_mutex> neighbour_voxel_lock;
            auto& neighbour_voxels = neighbour->voxels.get(neighbour_voxel_lock);
            for (VoxelChunkPositionCoord z = 0; z < CHUNK_LENGTH; ++z) {
                for (VoxelChunkPositionCoord y = 1; y < CHUNK_LENGTH - 2; ++y) {
                    do_side_stitch_navigable_check(
                        neighbour, { CHUNK_LENGTH - 1, y, z }, { 0, y, z }, 2, -1
                    );
                }
            }

            // Special handling for
            //      y == 0, CHUNK_LENGTH - 2
            // where chunks above need to be loaded too.

            // y == 0, CHUNK_LENGTH - 2
            for (VoxelChunkPositionCoord z = 0; z < CHUNK_LENGTH; ++z) {
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
                std::shared_lock<std::shared_mutex> below_neighbour_voxel_lock;
                auto                                below_neighbour_voxels
                    = below_neighbour->voxels.get(below_neighbour_voxel_lock);

                for (VoxelChunkPositionCoord z = 0; z < CHUNK_LENGTH; ++z) {
                    VoxelIndex this_voxel_index
                        = hvox::voxel_index({ CHUNK_LENGTH - 1, 0, z });
                    const Voxel* this_voxel = &voxels[this_voxel_index];

                    VoxelIndex above_this_voxel_index
                        = hvox::voxel_index({ CHUNK_LENGTH - 1, 1, z });
                    const Voxel* above_this_voxel = &voxels[above_this_voxel_index];

                    if (!is_solid(this_voxel) || is_solid(above_this_voxel)) continue;

                    ChunkNavmeshNode this_voxel_coord = {
                        {CHUNK_LENGTH - 1, 0, z},
                        chunk_pos
                    };

                    struct {
                        ChunkNavmeshVertexDescriptor here, in_below_neighbour;
                    } this_voxel_vertex
                        = { impl::get_vertex(chunk, this_voxel_coord),
                            impl::get_vertex(below_neighbour, this_voxel_coord) };

                    VoxelIndex twice_above_candidate_index
                        = hvox::voxel_index({ 0, 1, z });
                    const Voxel* twice_above_candidate_voxel
                        = &neighbour_voxels[twice_above_candidate_index];

                    VoxelIndex   above_candidate_index = hvox::voxel_index({ 0, 0, z });
                    const Voxel* above_candidate_voxel
                        = &neighbour_voxels[above_candidate_index];

                    VoxelIndex candidate_index
                        = hvox::voxel_index({ 0, CHUNK_LENGTH - 1, z });
                    const Voxel* candidate_voxel
                        = &below_neighbour_voxels[candidate_index];

                    if (is_solid(candidate_voxel) && !is_solid(above_candidate_voxel)
                        && !is_solid(twice_above_candidate_voxel))
                    {
                        ChunkNavmeshNode candidate_voxel_coord = {
                            {0, CHUNK_LENGTH - 1, z},
                            below_neighbour->position
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor here, in_below_neighbour;
                        } candidate_voxel_vertex = {
                            impl::get_vertex(chunk, candidate_voxel_coord),
                            impl::get_vertex(below_neighbour, candidate_voxel_coord)
                        };

                        {
                            std::unique_lock<std::shared_mutex> lock;
                            auto& navmesh = chunk->navmesh.get(lock);

                            boost::add_edge(
                                this_voxel_vertex.here,
                                candidate_voxel_vertex.here,
                                navmesh->graph
                            );
                            boost::add_edge(
                                candidate_voxel_vertex.here,
                                this_voxel_vertex.here,
                                navmesh->graph
                            );
                        }

                        {
                            std::unique_lock<std::shared_mutex> lock;
                            auto& navmesh = below_neighbour->navmesh.get(lock);

                            boost::add_edge(
                                this_voxel_vertex.in_below_neighbour,
                                candidate_voxel_vertex.in_below_neighbour,
                                navmesh->graph
                            );
                            boost::add_edge(
                                candidate_voxel_vertex.in_below_neighbour,
                                this_voxel_vertex.in_below_neighbour,
                                navmesh->graph
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
            std::shared_lock<std::shared_mutex> neighbour_voxel_lock;
            auto& neighbour_voxels = neighbour->voxels.get(neighbour_voxel_lock);
            for (VoxelChunkPositionCoord x = 0; x < CHUNK_LENGTH; ++x) {
                for (VoxelChunkPositionCoord y = 1; y < CHUNK_LENGTH - 2; ++y) {
                    do_side_stitch_navigable_check(
                        neighbour, { x, y, CHUNK_LENGTH - 1 }, { x, y, 0 }, 2, -1
                    );
                }
            }

            // Special handling for
            //      y == 0, CHUNK_LENGTH - 2
            // where chunks above need to be loaded too.

            // y == 0, CHUNK_LENGTH - 2
            for (VoxelChunkPositionCoord x = 0; x < CHUNK_LENGTH; ++x) {
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
                std::shared_lock<std::shared_mutex> below_neighbour_voxel_lock;
                auto                                below_neighbour_voxels
                    = below_neighbour->voxels.get(below_neighbour_voxel_lock);

                for (VoxelChunkPositionCoord x = 0; x < CHUNK_LENGTH; ++x) {
                    VoxelIndex this_voxel_index
                        = hvox::voxel_index({ x, 0, CHUNK_LENGTH - 1 });
                    const Voxel* this_voxel = &voxels[this_voxel_index];

                    VoxelIndex above_this_voxel_index
                        = hvox::voxel_index({ x, 1, CHUNK_LENGTH - 1 });
                    const Voxel* above_this_voxel = &voxels[above_this_voxel_index];

                    if (!is_solid(this_voxel) || is_solid(above_this_voxel)) continue;

                    ChunkNavmeshNode this_voxel_coord = {
                        {x, 0, CHUNK_LENGTH - 1},
                        chunk_pos
                    };

                    struct {
                        ChunkNavmeshVertexDescriptor here, in_below_neighbour;
                    } this_voxel_vertex
                        = { impl::get_vertex(chunk, this_voxel_coord),
                            impl::get_vertex(below_neighbour, this_voxel_coord) };

                    VoxelIndex twice_above_candidate_index
                        = hvox::voxel_index({ x, 1, 0 });
                    const Voxel* twice_above_candidate_voxel
                        = &neighbour_voxels[twice_above_candidate_index];

                    VoxelIndex   above_candidate_index = hvox::voxel_index({ x, 0, 0 });
                    const Voxel* above_candidate_voxel
                        = &neighbour_voxels[above_candidate_index];

                    VoxelIndex candidate_index
                        = hvox::voxel_index({ x, CHUNK_LENGTH - 1, 0 });
                    const Voxel* candidate_voxel
                        = &below_neighbour_voxels[candidate_index];

                    if (is_solid(candidate_voxel) && !is_solid(above_candidate_voxel)
                        && !is_solid(twice_above_candidate_voxel))
                    {
                        ChunkNavmeshNode candidate_voxel_coord = {
                            {x, CHUNK_LENGTH - 1, 0},
                            below_neighbour->position
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor here, in_below_neighbour;
                        } candidate_voxel_vertex = {
                            impl::get_vertex(chunk, candidate_voxel_coord),
                            impl::get_vertex(below_neighbour, candidate_voxel_coord)
                        };

                        {
                            std::unique_lock<std::shared_mutex> lock;
                            auto& navmesh = chunk->navmesh.get(lock);

                            boost::add_edge(
                                this_voxel_vertex.here,
                                candidate_voxel_vertex.here,
                                navmesh->graph
                            );
                            boost::add_edge(
                                candidate_voxel_vertex.here,
                                this_voxel_vertex.here,
                                navmesh->graph
                            );
                        }

                        {
                            std::unique_lock<std::shared_mutex> lock;
                            auto& navmesh = below_neighbour->navmesh.get(lock);

                            boost::add_edge(
                                this_voxel_vertex.in_below_neighbour,
                                candidate_voxel_vertex.in_below_neighbour,
                                navmesh->graph
                            );
                            boost::add_edge(
                                candidate_voxel_vertex.in_below_neighbour,
                                this_voxel_vertex.in_below_neighbour,
                                navmesh->graph
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
            std::shared_lock<std::shared_mutex> neighbour_voxel_lock;
            auto& neighbour_voxels = neighbour->voxels.get(neighbour_voxel_lock);
            for (VoxelChunkPositionCoord x = 0; x < CHUNK_LENGTH; ++x) {
                for (VoxelChunkPositionCoord y = 1; y < CHUNK_LENGTH - 2; ++y) {
                    do_side_stitch_navigable_check(
                        neighbour, { x, y, 0 }, { x, y, CHUNK_LENGTH - 1 }, 2, -1
                    );
                }
            }

            // Special handling for
            //      y == 0, CHUNK_LENGTH - 2
            // where chunks above need to be loaded too.

            // y == 0, CHUNK_LENGTH - 2
            for (VoxelChunkPositionCoord x = 0; x < CHUNK_LENGTH; ++x) {
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
                std::shared_lock<std::shared_mutex> below_neighbour_voxel_lock;
                auto                                below_neighbour_voxels
                    = below_neighbour->voxels.get(below_neighbour_voxel_lock);

                for (VoxelChunkPositionCoord x = 0; x < CHUNK_LENGTH; ++x) {
                    VoxelIndex   this_voxel_index = hvox::voxel_index({ x, 0, 0 });
                    const Voxel* this_voxel       = &voxels[this_voxel_index];

                    VoxelIndex above_this_voxel_index = hvox::voxel_index({ x, 1, 0 });
                    const Voxel* above_this_voxel     = &voxels[above_this_voxel_index];

                    if (!is_solid(this_voxel) || is_solid(above_this_voxel)) continue;

                    ChunkNavmeshNode this_voxel_coord = {
                        {x, 0, 0},
                        chunk_pos
                    };

                    struct {
                        ChunkNavmeshVertexDescriptor here, in_below_neighbour;
                    } this_voxel_vertex
                        = { impl::get_vertex(chunk, this_voxel_coord),
                            impl::get_vertex(below_neighbour, this_voxel_coord) };

                    VoxelIndex twice_above_candidate_index
                        = hvox::voxel_index({ x, 1, CHUNK_LENGTH - 1 });
                    const Voxel* twice_above_candidate_voxel
                        = &neighbour_voxels[twice_above_candidate_index];

                    VoxelIndex above_candidate_index
                        = hvox::voxel_index({ x, 0, CHUNK_LENGTH - 1 });
                    const Voxel* above_candidate_voxel
                        = &neighbour_voxels[above_candidate_index];

                    VoxelIndex candidate_index
                        = hvox::voxel_index({ x, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1 });
                    const Voxel* candidate_voxel
                        = &below_neighbour_voxels[candidate_index];

                    if (is_solid(candidate_voxel) && !is_solid(above_candidate_voxel)
                        && !is_solid(twice_above_candidate_voxel))
                    {
                        ChunkNavmeshNode candidate_voxel_coord = {
                            {x, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1},
                            below_neighbour->position
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor here, in_below_neighbour;
                        } candidate_voxel_vertex = {
                            impl::get_vertex(chunk, candidate_voxel_coord),
                            impl::get_vertex(below_neighbour, candidate_voxel_coord)
                        };

                        {
                            std::unique_lock<std::shared_mutex> lock;
                            auto& navmesh = chunk->navmesh.get(lock);

                            boost::add_edge(
                                this_voxel_vertex.here,
                                candidate_voxel_vertex.here,
                                navmesh->graph
                            );
                            boost::add_edge(
                                candidate_voxel_vertex.here,
                                this_voxel_vertex.here,
                                navmesh->graph
                            );
                        }

                        {
                            std::unique_lock<std::shared_mutex> lock;
                            auto& navmesh = below_neighbour->navmesh.get(lock);

                            boost::add_edge(
                                this_voxel_vertex.in_below_neighbour,
                                candidate_voxel_vertex.in_below_neighbour,
                                navmesh->graph
                            );
                            boost::add_edge(
                                candidate_voxel_vertex.in_below_neighbour,
                                this_voxel_vertex.in_below_neighbour,
                                navmesh->graph
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
            std::shared_lock<std::shared_mutex> neighbour_voxel_lock;
            auto& neighbour_voxels = neighbour->voxels.get(neighbour_voxel_lock);
            for (VoxelChunkPositionCoord x = 1; x < CHUNK_LENGTH - 1; ++x) {
                for (VoxelChunkPositionCoord z = 1; z < CHUNK_LENGTH - 1; ++z) {
                    VoxelIndex this_voxel_index
                        = hvox::voxel_index({ x, CHUNK_LENGTH - 1, z });
                    const Voxel* this_voxel = &voxels[this_voxel_index];

                    VoxelIndex   neighbour_voxel_index = hvox::voxel_index({ x, 0, z });
                    const Voxel* neighbour_voxel
                        = &neighbour_voxels[neighbour_voxel_index];

                    VoxelIndex above_neighbour_voxel_index
                        = hvox::voxel_index({ x, 1, z });
                    const Voxel* above_neighbour_voxel
                        = &neighbour_voxels[above_neighbour_voxel_index];

                    if (!is_solid(this_voxel) || is_solid(neighbour_voxel)) continue;

                    // Ensure node exists for this voxel.
                    ChunkNavmeshNode this_voxel_coord = {
                        {x, CHUNK_LENGTH - 1, z},
                        chunk_pos
                    };

                    struct {
                        ChunkNavmeshVertexDescriptor here, in_neighbour;
                    } this_voxel_vertex
                        = { impl::get_vertex(chunk, this_voxel_coord),
                            impl::get_vertex(neighbour, this_voxel_coord) };

                    // Up
                    if (!is_solid(above_neighbour_voxel)) {
                        // Left
                        VoxelIndex left_of_neighbour_voxel_index
                            = hvox::voxel_index({ x - 1, 0, z });
                        const Voxel* left_of_neighbour_voxel
                            = &neighbour_voxels[left_of_neighbour_voxel_index];

                        VoxelIndex above_and_left_of_neighbour_voxel_index
                            = hvox::voxel_index({ x - 1, 1, z });
                        const Voxel* above_and_left_of_neighbour_voxel
                            = &neighbour_voxels
                                  [above_and_left_of_neighbour_voxel_index];

                        if (is_solid(left_of_neighbour_voxel)
                            && !is_solid(above_and_left_of_neighbour_voxel))
                        {
                            ChunkNavmeshNode left_of_neighbour_voxel_coord = {
                                {x - 1, 0, z},
                                neighbour->position
                            };

                            // Ensure node exists for this voxel.
                            struct {
                                ChunkNavmeshVertexDescriptor here, in_neighbour;
                            } left_of_neighbour_voxel_vertex = {
                                impl::get_vertex(chunk, left_of_neighbour_voxel_coord),
                                impl::get_vertex(
                                    neighbour, left_of_neighbour_voxel_coord
                                )
                            };

                            {
                                std::unique_lock<std::shared_mutex> lock;
                                auto& navmesh = chunk->navmesh.get(lock);

                                boost::add_edge(
                                    this_voxel_vertex.here,
                                    left_of_neighbour_voxel_vertex.here,
                                    navmesh->graph
                                );
                                boost::add_edge(
                                    left_of_neighbour_voxel_vertex.here,
                                    this_voxel_vertex.here,
                                    navmesh->graph
                                );
                            }

                            {
                                std::unique_lock<std::shared_mutex> lock;
                                auto& navmesh = neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    this_voxel_vertex.in_neighbour,
                                    left_of_neighbour_voxel_vertex.in_neighbour,
                                    navmesh->graph
                                );
                                boost::add_edge(
                                    left_of_neighbour_voxel_vertex.in_neighbour,
                                    this_voxel_vertex.in_neighbour,
                                    navmesh->graph
                                );
                            }
                        }

                        // Right
                        VoxelIndex right_of_neighbour_voxel_index
                            = hvox::voxel_index({ x + 1, 0, z });
                        const Voxel* right_of_neighbour_voxel
                            = &neighbour_voxels[right_of_neighbour_voxel_index];

                        VoxelIndex above_and_right_of_neighbour_voxel_index
                            = hvox::voxel_index({ x + 1, 1, z });
                        const Voxel* above_and_right_of_neighbour_voxel
                            = &neighbour_voxels
                                  [above_and_right_of_neighbour_voxel_index];

                        if (is_solid(right_of_neighbour_voxel)
                            && !is_solid(above_and_right_of_neighbour_voxel))
                        {
                            ChunkNavmeshNode right_of_neighbour_voxel_coord = {
                                {x + 1, 0, z},
                                neighbour->position
                            };

                            // Ensure node exists for this voxel.
                            struct {
                                ChunkNavmeshVertexDescriptor here, in_neighbour;
                            } right_of_neighbour_voxel_vertex = {
                                impl::get_vertex(chunk, right_of_neighbour_voxel_coord),
                                impl::get_vertex(
                                    neighbour, right_of_neighbour_voxel_coord
                                )
                            };

                            {
                                std::unique_lock<std::shared_mutex> lock;
                                auto& navmesh = chunk->navmesh.get(lock);

                                boost::add_edge(
                                    this_voxel_vertex.here,
                                    right_of_neighbour_voxel_vertex.here,
                                    navmesh->graph
                                );
                                boost::add_edge(
                                    right_of_neighbour_voxel_vertex.here,
                                    this_voxel_vertex.here,
                                    navmesh->graph
                                );
                            }

                            {
                                std::unique_lock<std::shared_mutex> lock;
                                auto& navmesh = neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    this_voxel_vertex.in_neighbour,
                                    right_of_neighbour_voxel_vertex.in_neighbour,
                                    navmesh->graph
                                );
                                boost::add_edge(
                                    right_of_neighbour_voxel_vertex.in_neighbour,
                                    this_voxel_vertex.in_neighbour,
                                    navmesh->graph
                                );
                            }
                        }

                        // Front
                        VoxelIndex front_of_neighbour_voxel_index
                            = hvox::voxel_index({ x, 0, z + 1 });
                        const Voxel* front_of_neighbour_voxel
                            = &neighbour_voxels[front_of_neighbour_voxel_index];

                        VoxelIndex above_and_front_of_neighbour_voxel_index
                            = hvox::voxel_index({ x, 1, z + 1 });
                        const Voxel* above_and_front_of_neighbour_voxel
                            = &neighbour_voxels
                                  [above_and_front_of_neighbour_voxel_index];

                        if (is_solid(front_of_neighbour_voxel)
                            && !is_solid(above_and_front_of_neighbour_voxel))
                        {
                            ChunkNavmeshNode front_of_neighbour_voxel_coord = {
                                {x, 0, z + 1},
                                neighbour->position
                            };

                            // Ensure node exists for this voxel.
                            struct {
                                ChunkNavmeshVertexDescriptor here, in_neighbour;
                            } front_of_neighbour_voxel_vertex = {
                                impl::get_vertex(chunk, front_of_neighbour_voxel_coord),
                                impl::get_vertex(
                                    neighbour, front_of_neighbour_voxel_coord
                                )
                            };

                            {
                                std::unique_lock<std::shared_mutex> lock;
                                auto& navmesh = chunk->navmesh.get(lock);

                                boost::add_edge(
                                    this_voxel_vertex.here,
                                    front_of_neighbour_voxel_vertex.here,
                                    navmesh->graph
                                );
                                boost::add_edge(
                                    front_of_neighbour_voxel_vertex.here,
                                    this_voxel_vertex.here,
                                    navmesh->graph
                                );
                            }

                            {
                                std::unique_lock<std::shared_mutex> lock;
                                auto& navmesh = neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    this_voxel_vertex.in_neighbour,
                                    front_of_neighbour_voxel_vertex.in_neighbour,
                                    navmesh->graph
                                );
                                boost::add_edge(
                                    front_of_neighbour_voxel_vertex.in_neighbour,
                                    this_voxel_vertex.in_neighbour,
                                    navmesh->graph
                                );
                            }
                        }

                        // Back
                        VoxelIndex back_of_neighbour_voxel_index
                            = hvox::voxel_index({ x, 0, z - 1 });
                        const Voxel* back_of_neighbour_voxel
                            = &neighbour_voxels[back_of_neighbour_voxel_index];

                        VoxelIndex above_and_back_of_neighbour_voxel_index
                            = hvox::voxel_index({ x, 1, z - 1 });
                        const Voxel* above_and_back_of_neighbour_voxel
                            = &neighbour_voxels
                                  [above_and_back_of_neighbour_voxel_index];

                        if (is_solid(back_of_neighbour_voxel)
                            && !is_solid(above_and_back_of_neighbour_voxel))
                        {
                            ChunkNavmeshNode back_of_neighbour_voxel_coord = {
                                {x, 0, z - 1},
                                neighbour->position
                            };

                            // Ensure node exists for this voxel.
                            struct {
                                ChunkNavmeshVertexDescriptor here, in_neighbour;
                            } back_of_neighbour_voxel_vertex = {
                                impl::get_vertex(chunk, back_of_neighbour_voxel_coord),
                                impl::get_vertex(
                                    neighbour, back_of_neighbour_voxel_coord
                                )
                            };

                            {
                                std::unique_lock<std::shared_mutex> lock;
                                auto& navmesh = chunk->navmesh.get(lock);

                                boost::add_edge(
                                    this_voxel_vertex.here,
                                    back_of_neighbour_voxel_vertex.here,
                                    navmesh->graph
                                );
                                boost::add_edge(
                                    back_of_neighbour_voxel_vertex.here,
                                    this_voxel_vertex.here,
                                    navmesh->graph
                                );
                            }

                            {
                                std::unique_lock<std::shared_mutex> lock;
                                auto& navmesh = neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    this_voxel_vertex.in_neighbour,
                                    back_of_neighbour_voxel_vertex.in_neighbour,
                                    navmesh->graph
                                );
                                boost::add_edge(
                                    back_of_neighbour_voxel_vertex.in_neighbour,
                                    this_voxel_vertex.in_neighbour,
                                    navmesh->graph
                                );
                            }
                        }
                    }

                    ///// Across and Down
                    // Left
                    VoxelIndex left_of_this_voxel_index
                        = hvox::voxel_index({ x - 1, CHUNK_LENGTH - 1, z });
                    const Voxel* left_of_this_voxel = &voxels[left_of_this_voxel_index];

                    VoxelIndex left_of_and_below_this_voxel_index
                        = hvox::voxel_index({ x - 1, CHUNK_LENGTH - 2, z });
                    const Voxel* left_of_and_below_this_voxel
                        = &voxels[left_of_and_below_this_voxel_index];

                    VoxelIndex left_of_neighbour_voxel_index
                        = hvox::voxel_index({ x - 1, 0, z });
                    const Voxel* left_of_neighbour_voxel
                        = &neighbour_voxels[left_of_neighbour_voxel_index];

                    // Across
                    if (is_solid(left_of_this_voxel)
                        && !is_solid(left_of_neighbour_voxel))
                    {
                        ChunkNavmeshNode left_of_this_voxel_coord = {
                            {x - 1, CHUNK_LENGTH - 1, z},
                            chunk_pos
                        };

                        // Ensure node exists for this voxel.
                        ChunkNavmeshVertexDescriptor left_of_this_voxel_vertex
                            = impl::get_vertex(chunk, left_of_this_voxel_coord);

                        {
                            std::unique_lock<std::shared_mutex> lock;
                            auto& navmesh = chunk->navmesh.get(lock);

                            boost::add_edge(
                                this_voxel_vertex.here,
                                left_of_this_voxel_vertex,
                                navmesh->graph
                            );
                            boost::add_edge(
                                left_of_this_voxel_vertex,
                                this_voxel_vertex.here,
                                navmesh->graph
                            );
                        }
                        // Down
                    } else if (is_solid(left_of_and_below_this_voxel) && !is_solid(left_of_this_voxel) && !is_solid(left_of_neighbour_voxel))
                    {
                        ChunkNavmeshNode left_of_and_below_this_voxel_coord = {
                            {x - 1, CHUNK_LENGTH - 2, z},
                            chunk_pos
                        };

                        // Ensure node exists for this voxel.
                        ChunkNavmeshVertexDescriptor left_of_and_below_this_voxel_vertex
                            = impl::get_vertex(
                                chunk, left_of_and_below_this_voxel_coord
                            );

                        {
                            std::unique_lock<std::shared_mutex> lock;
                            auto& navmesh = chunk->navmesh.get(lock);

                            boost::add_edge(
                                this_voxel_vertex.here,
                                left_of_and_below_this_voxel_vertex,
                                navmesh->graph
                            );
                            boost::add_edge(
                                left_of_and_below_this_voxel_vertex,
                                this_voxel_vertex.here,
                                navmesh->graph
                            );
                        }
                    }

                    // Right
                    VoxelIndex right_of_this_voxel_index
                        = hvox::voxel_index({ x + 1, CHUNK_LENGTH - 1, z });
                    const Voxel* right_of_this_voxel
                        = &voxels[right_of_this_voxel_index];

                    VoxelIndex right_of_and_below_this_voxel_index
                        = hvox::voxel_index({ x + 1, CHUNK_LENGTH - 2, z });
                    const Voxel* right_of_and_below_this_voxel
                        = &voxels[right_of_and_below_this_voxel_index];

                    VoxelIndex right_of_neighbour_voxel_index
                        = hvox::voxel_index({ x + 1, 0, z });
                    const Voxel* right_of_neighbour_voxel
                        = &neighbour_voxels[right_of_neighbour_voxel_index];

                    // Across
                    if (is_solid(right_of_this_voxel)
                        && !is_solid(right_of_neighbour_voxel))
                    {
                        ChunkNavmeshNode right_of_this_voxel_coord = {
                            {x + 1, CHUNK_LENGTH - 1, z},
                            chunk_pos
                        };

                        // Ensure node exists for this voxel.
                        ChunkNavmeshVertexDescriptor right_of_this_voxel_vertex
                            = impl::get_vertex(chunk, right_of_this_voxel_coord);

                        {
                            std::unique_lock<std::shared_mutex> lock;
                            auto& navmesh = chunk->navmesh.get(lock);

                            boost::add_edge(
                                this_voxel_vertex.here,
                                right_of_this_voxel_vertex,
                                navmesh->graph
                            );
                            boost::add_edge(
                                right_of_this_voxel_vertex,
                                this_voxel_vertex.here,
                                navmesh->graph
                            );
                        }
                        // Down
                    } else if (is_solid(right_of_and_below_this_voxel) && !is_solid(right_of_this_voxel) && !is_solid(right_of_neighbour_voxel))
                    {
                        ChunkNavmeshNode right_of_and_below_this_voxel_coord = {
                            {x + 1, CHUNK_LENGTH - 2, z},
                            chunk_pos
                        };

                        // Ensure node exists for this voxel.
                        ChunkNavmeshVertexDescriptor
                            right_of_and_below_this_voxel_vertex
                            = impl::get_vertex(
                                chunk, right_of_and_below_this_voxel_coord
                            );

                        {
                            std::unique_lock<std::shared_mutex> lock;
                            auto& navmesh = chunk->navmesh.get(lock);

                            boost::add_edge(
                                this_voxel_vertex.here,
                                right_of_and_below_this_voxel_vertex,
                                navmesh->graph
                            );
                            boost::add_edge(
                                right_of_and_below_this_voxel_vertex,
                                this_voxel_vertex.here,
                                navmesh->graph
                            );
                        }
                    }

                    // Front
                    VoxelIndex front_of_this_voxel_index
                        = hvox::voxel_index({ x, CHUNK_LENGTH - 1, z + 1 });
                    const Voxel* front_of_this_voxel
                        = &voxels[front_of_this_voxel_index];

                    VoxelIndex front_of_and_below_this_voxel_index
                        = hvox::voxel_index({ x, CHUNK_LENGTH - 2, z + 1 });
                    const Voxel* front_of_and_below_this_voxel
                        = &voxels[front_of_and_below_this_voxel_index];

                    VoxelIndex front_of_neighbour_voxel_index
                        = hvox::voxel_index({ x, 0, z + 1 });
                    const Voxel* front_of_neighbour_voxel
                        = &neighbour_voxels[front_of_neighbour_voxel_index];

                    // Across
                    if (is_solid(front_of_this_voxel)
                        && !is_solid(front_of_neighbour_voxel))
                    {
                        ChunkNavmeshNode front_of_this_voxel_coord = {
                            {x, CHUNK_LENGTH - 1, z + 1},
                            chunk_pos
                        };

                        // Ensure node exists for this voxel.
                        ChunkNavmeshVertexDescriptor front_of_this_voxel_vertex
                            = impl::get_vertex(chunk, front_of_this_voxel_coord);

                        {
                            std::unique_lock<std::shared_mutex> lock;
                            auto& navmesh = chunk->navmesh.get(lock);

                            boost::add_edge(
                                this_voxel_vertex.here,
                                front_of_this_voxel_vertex,
                                navmesh->graph
                            );
                            boost::add_edge(
                                front_of_this_voxel_vertex,
                                this_voxel_vertex.here,
                                navmesh->graph
                            );
                        }
                        // Down
                    } else if (is_solid(front_of_and_below_this_voxel) && !is_solid(front_of_this_voxel) && !is_solid(front_of_neighbour_voxel))
                    {
                        ChunkNavmeshNode front_of_and_below_this_voxel_coord = {
                            {x, CHUNK_LENGTH - 2, z + 1},
                            chunk_pos
                        };

                        // Ensure node exists for this voxel.
                        ChunkNavmeshVertexDescriptor
                            front_of_and_below_this_voxel_vertex
                            = impl::get_vertex(
                                chunk, front_of_and_below_this_voxel_coord
                            );

                        {
                            std::unique_lock<std::shared_mutex> lock;
                            auto& navmesh = chunk->navmesh.get(lock);

                            boost::add_edge(
                                this_voxel_vertex.here,
                                front_of_and_below_this_voxel_vertex,
                                navmesh->graph
                            );
                            boost::add_edge(
                                front_of_and_below_this_voxel_vertex,
                                this_voxel_vertex.here,
                                navmesh->graph
                            );
                        }
                    }

                    // Back
                    VoxelIndex back_of_this_voxel_index
                        = hvox::voxel_index({ x, CHUNK_LENGTH - 1, z - 1 });
                    const Voxel* back_of_this_voxel = &voxels[back_of_this_voxel_index];

                    VoxelIndex back_of_and_below_this_voxel_index
                        = hvox::voxel_index({ x, CHUNK_LENGTH - 2, z - 1 });
                    const Voxel* back_of_and_below_this_voxel
                        = &voxels[back_of_and_below_this_voxel_index];

                    VoxelIndex back_of_neighbour_voxel_index
                        = hvox::voxel_index({ x, 0, z - 1 });
                    const Voxel* back_of_neighbour_voxel
                        = &neighbour_voxels[back_of_neighbour_voxel_index];

                    // Across
                    if (is_solid(back_of_this_voxel)
                        && !is_solid(back_of_neighbour_voxel))
                    {
                        ChunkNavmeshNode back_of_this_voxel_coord = {
                            {x, CHUNK_LENGTH - 1, z - 1},
                            chunk_pos
                        };

                        // Ensure node exists for this voxel.
                        ChunkNavmeshVertexDescriptor back_of_this_voxel_vertex
                            = impl::get_vertex(chunk, back_of_this_voxel_coord);

                        {
                            std::unique_lock<std::shared_mutex> lock;
                            auto& navmesh = chunk->navmesh.get(lock);

                            boost::add_edge(
                                this_voxel_vertex.here,
                                back_of_this_voxel_vertex,
                                navmesh->graph
                            );
                            boost::add_edge(
                                back_of_this_voxel_vertex,
                                this_voxel_vertex.here,
                                navmesh->graph
                            );
                        }
                        // Down
                    } else if (is_solid(back_of_and_below_this_voxel) && !is_solid(back_of_this_voxel) && !is_solid(back_of_neighbour_voxel))
                    {
                        ChunkNavmeshNode back_of_and_below_this_voxel_coord = {
                            {x, CHUNK_LENGTH - 2, z - 1},
                            chunk_pos
                        };

                        // Ensure node exists for this voxel.
                        ChunkNavmeshVertexDescriptor back_of_and_below_this_voxel_vertex
                            = impl::get_vertex(
                                chunk, back_of_and_below_this_voxel_coord
                            );

                        {
                            std::unique_lock<std::shared_mutex> lock;
                            auto& navmesh = chunk->navmesh.get(lock);

                            boost::add_edge(
                                this_voxel_vertex.here,
                                back_of_and_below_this_voxel_vertex,
                                navmesh->graph
                            );
                            boost::add_edge(
                                back_of_and_below_this_voxel_vertex,
                                this_voxel_vertex.here,
                                navmesh->graph
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
                    std::shared_lock<std::shared_mutex> left_of_neighbour_voxel_lock;
                    auto                                left_of_neighbour_voxels
                        = left_of_neighbour->voxels.get(left_of_neighbour_voxel_lock);

                    for (VoxelChunkPositionCoord z = 0; z < CHUNK_LENGTH; ++z) {
                        VoxelIndex left_of_neighbour_voxel_index
                            = hvox::voxel_index({ CHUNK_LENGTH - 1, 0, z });
                        const Voxel* left_of_neighbour_voxel
                            = &left_of_neighbour_voxels[left_of_neighbour_voxel_index];

                        VoxelIndex above_left_of_neighbour_voxel_index
                            = hvox::voxel_index({ CHUNK_LENGTH - 1, 1, z });
                        const Voxel* above_left_of_neighbour_voxel
                            = &left_of_neighbour_voxels
                                  [above_left_of_neighbour_voxel_index];

                        if (!is_solid(left_of_neighbour_voxel)
                            || is_solid(above_left_of_neighbour_voxel))
                            continue;

                        ChunkNavmeshNode left_of_neighbour_voxel_coord = {
                            {CHUNK_LENGTH - 1, 0, z},
                            left_of_neighbour->position
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor here, in_left_of_neighbour;
                        } left_of_neighbour_voxel_vertex
                            = { impl::get_vertex(chunk, left_of_neighbour_voxel_coord),
                                impl::get_vertex(
                                    left_of_neighbour, left_of_neighbour_voxel_coord
                                ) };

                        VoxelIndex twice_above_candidate_index
                            = hvox::voxel_index({ 0, 1, z });
                        const Voxel* twice_above_candidate_voxel
                            = &neighbour_voxels[twice_above_candidate_index];

                        VoxelIndex above_candidate_index
                            = hvox::voxel_index({ 0, 0, z });
                        const Voxel* above_candidate_voxel
                            = &neighbour_voxels[above_candidate_index];

                        VoxelIndex candidate_index
                            = hvox::voxel_index({ 0, CHUNK_LENGTH - 1, z });
                        const Voxel* candidate_voxel = &voxels[candidate_index];

                        if (is_solid(candidate_voxel)
                            && !is_solid(above_candidate_voxel)
                            && !is_solid(twice_above_candidate_voxel))
                        {
                            ChunkNavmeshNode candidate_voxel_coord = {
                                {0, CHUNK_LENGTH - 1, z},
                                chunk_pos
                            };

                            struct {
                                ChunkNavmeshVertexDescriptor here, in_left_of_neighbour;
                            } candidate_voxel_vertex
                                = { impl::get_vertex(chunk, candidate_voxel_coord),
                                    impl::get_vertex(
                                        left_of_neighbour, candidate_voxel_coord
                                    ) };

                            {
                                std::unique_lock<std::shared_mutex> lock;
                                auto& navmesh = chunk->navmesh.get(lock);

                                boost::add_edge(
                                    left_of_neighbour_voxel_vertex.here,
                                    candidate_voxel_vertex.here,
                                    navmesh->graph
                                );
                                boost::add_edge(
                                    candidate_voxel_vertex.here,
                                    left_of_neighbour_voxel_vertex.here,
                                    navmesh->graph
                                );
                            }

                            {
                                std::unique_lock<std::shared_mutex> lock;
                                auto& navmesh = left_of_neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    left_of_neighbour_voxel_vertex.in_left_of_neighbour,
                                    candidate_voxel_vertex.in_left_of_neighbour,
                                    navmesh->graph
                                );
                                boost::add_edge(
                                    candidate_voxel_vertex.in_left_of_neighbour,
                                    left_of_neighbour_voxel_vertex.in_left_of_neighbour,
                                    navmesh->graph
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
                    std::shared_lock<std::shared_mutex> right_of_neighbour_voxel_lock;
                    auto                                right_of_neighbour_voxels
                        = right_of_neighbour->voxels.get(right_of_neighbour_voxel_lock);

                    for (VoxelChunkPositionCoord z = 0; z < CHUNK_LENGTH; ++z) {
                        VoxelIndex right_of_neighbour_voxel_index
                            = hvox::voxel_index({ 0, 0, z });
                        const Voxel* right_of_neighbour_voxel
                            = &right_of_neighbour_voxels
                                  [right_of_neighbour_voxel_index];

                        VoxelIndex above_right_of_neighbour_voxel_index
                            = hvox::voxel_index({ 0, 1, z });
                        const Voxel* above_right_of_neighbour_voxel
                            = &right_of_neighbour_voxels
                                  [above_right_of_neighbour_voxel_index];

                        if (!is_solid(right_of_neighbour_voxel)
                            || is_solid(above_right_of_neighbour_voxel))
                            continue;

                        ChunkNavmeshNode right_of_neighbour_voxel_coord = {
                            {0, 0, z},
                            right_of_neighbour->position
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor here, in_right_of_neighbour;
                        } right_of_neighbour_voxel_vertex
                            = { impl::get_vertex(chunk, right_of_neighbour_voxel_coord),
                                impl::get_vertex(
                                    right_of_neighbour, right_of_neighbour_voxel_coord
                                ) };

                        VoxelIndex twice_above_candidate_index
                            = hvox::voxel_index({ CHUNK_LENGTH - 1, 1, z });
                        const Voxel* twice_above_candidate_voxel
                            = &neighbour_voxels[twice_above_candidate_index];

                        VoxelIndex above_candidate_index
                            = hvox::voxel_index({ CHUNK_LENGTH - 1, 0, z });
                        const Voxel* above_candidate_voxel
                            = &neighbour_voxels[above_candidate_index];

                        VoxelIndex candidate_index = hvox::voxel_index(
                            { CHUNK_LENGTH - 1, CHUNK_LENGTH - 1, z }
                        );
                        const Voxel* candidate_voxel = &voxels[candidate_index];

                        if (is_solid(candidate_voxel)
                            && !is_solid(above_candidate_voxel)
                            && !is_solid(twice_above_candidate_voxel))
                        {
                            ChunkNavmeshNode candidate_voxel_coord = {
                                {CHUNK_LENGTH - 1, CHUNK_LENGTH - 1, z},
                                chunk_pos
                            };

                            struct {
                                ChunkNavmeshVertexDescriptor here,
                                    in_right_of_neighbour;
                            } candidate_voxel_vertex
                                = { impl::get_vertex(chunk, candidate_voxel_coord),
                                    impl::get_vertex(
                                        right_of_neighbour, candidate_voxel_coord
                                    ) };

                            {
                                std::unique_lock<std::shared_mutex> lock;
                                auto& navmesh = chunk->navmesh.get(lock);

                                boost::add_edge(
                                    right_of_neighbour_voxel_vertex.here,
                                    candidate_voxel_vertex.here,
                                    navmesh->graph
                                );
                                boost::add_edge(
                                    candidate_voxel_vertex.here,
                                    right_of_neighbour_voxel_vertex.here,
                                    navmesh->graph
                                );
                            }

                            {
                                std::unique_lock<std::shared_mutex> lock;
                                auto& navmesh = right_of_neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    right_of_neighbour_voxel_vertex
                                        .in_right_of_neighbour,
                                    candidate_voxel_vertex.in_right_of_neighbour,
                                    navmesh->graph
                                );
                                boost::add_edge(
                                    candidate_voxel_vertex.in_right_of_neighbour,
                                    right_of_neighbour_voxel_vertex
                                        .in_right_of_neighbour,
                                    navmesh->graph
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
                    std::shared_lock<std::shared_mutex> front_of_neighbour_voxel_lock;
                    auto                                front_of_neighbour_voxels
                        = front_of_neighbour->voxels.get(front_of_neighbour_voxel_lock);

                    for (VoxelChunkPositionCoord x = 0; x < CHUNK_LENGTH; ++x) {
                        VoxelIndex front_of_neighbour_voxel_index
                            = hvox::voxel_index({ x, 0, 0 });
                        const Voxel* front_of_neighbour_voxel
                            = &front_of_neighbour_voxels
                                  [front_of_neighbour_voxel_index];

                        VoxelIndex above_front_of_neighbour_voxel_index
                            = hvox::voxel_index({ x, 1, 0 });
                        const Voxel* above_front_of_neighbour_voxel
                            = &front_of_neighbour_voxels
                                  [above_front_of_neighbour_voxel_index];

                        if (!is_solid(front_of_neighbour_voxel)
                            || is_solid(above_front_of_neighbour_voxel))
                            continue;

                        ChunkNavmeshNode front_of_neighbour_voxel_coord = {
                            {x, 0, 0},
                            front_of_neighbour->position
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor here, in_front_of_neighbour;
                        } front_of_neighbour_voxel_vertex
                            = { impl::get_vertex(chunk, front_of_neighbour_voxel_coord),
                                impl::get_vertex(
                                    front_of_neighbour, front_of_neighbour_voxel_coord
                                ) };

                        VoxelIndex twice_above_candidate_index
                            = hvox::voxel_index({ x, 1, CHUNK_LENGTH - 1 });
                        const Voxel* twice_above_candidate_voxel
                            = &neighbour_voxels[twice_above_candidate_index];

                        VoxelIndex above_candidate_index
                            = hvox::voxel_index({ x, 0, CHUNK_LENGTH - 1 });
                        const Voxel* above_candidate_voxel
                            = &neighbour_voxels[above_candidate_index];

                        VoxelIndex candidate_index = hvox::voxel_index(
                            { x, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1 }
                        );
                        const Voxel* candidate_voxel = &voxels[candidate_index];

                        if (is_solid(candidate_voxel)
                            && !is_solid(above_candidate_voxel)
                            && !is_solid(twice_above_candidate_voxel))
                        {
                            ChunkNavmeshNode candidate_voxel_coord = {
                                {x, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1},
                                chunk_pos
                            };

                            struct {
                                ChunkNavmeshVertexDescriptor here,
                                    in_front_of_neighbour;
                            } candidate_voxel_vertex
                                = { impl::get_vertex(chunk, candidate_voxel_coord),
                                    impl::get_vertex(
                                        front_of_neighbour, candidate_voxel_coord
                                    ) };

                            {
                                std::unique_lock<std::shared_mutex> lock;
                                auto& navmesh = chunk->navmesh.get(lock);

                                boost::add_edge(
                                    front_of_neighbour_voxel_vertex.here,
                                    candidate_voxel_vertex.here,
                                    navmesh->graph
                                );
                                boost::add_edge(
                                    candidate_voxel_vertex.here,
                                    front_of_neighbour_voxel_vertex.here,
                                    navmesh->graph
                                );
                            }

                            {
                                std::unique_lock<std::shared_mutex> lock;
                                auto& navmesh = front_of_neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    front_of_neighbour_voxel_vertex
                                        .in_front_of_neighbour,
                                    candidate_voxel_vertex.in_front_of_neighbour,
                                    navmesh->graph
                                );
                                boost::add_edge(
                                    candidate_voxel_vertex.in_front_of_neighbour,
                                    front_of_neighbour_voxel_vertex
                                        .in_front_of_neighbour,
                                    navmesh->graph
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
                    std::shared_lock<std::shared_mutex> back_of_neighbour_voxel_lock;
                    auto                                back_of_neighbour_voxels
                        = back_of_neighbour->voxels.get(back_of_neighbour_voxel_lock);

                    for (VoxelChunkPositionCoord x = 0; x < CHUNK_LENGTH; ++x) {
                        VoxelIndex back_of_neighbour_voxel_index
                            = hvox::voxel_index({ x, 0, CHUNK_LENGTH - 1 });
                        const Voxel* back_of_neighbour_voxel
                            = &back_of_neighbour_voxels[back_of_neighbour_voxel_index];

                        VoxelIndex above_back_of_neighbour_voxel_index
                            = hvox::voxel_index({ x, 1, CHUNK_LENGTH - 1 });
                        const Voxel* above_back_of_neighbour_voxel
                            = &back_of_neighbour_voxels
                                  [above_back_of_neighbour_voxel_index];

                        if (!is_solid(back_of_neighbour_voxel)
                            || is_solid(above_back_of_neighbour_voxel))
                            continue;

                        ChunkNavmeshNode back_of_neighbour_voxel_coord = {
                            {x, 0, CHUNK_LENGTH - 1},
                            back_of_neighbour->position
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor here, in_back_of_neighbour;
                        } back_of_neighbour_voxel_vertex
                            = { impl::get_vertex(chunk, back_of_neighbour_voxel_coord),
                                impl::get_vertex(
                                    back_of_neighbour, back_of_neighbour_voxel_coord
                                ) };

                        VoxelIndex twice_above_candidate_index
                            = hvox::voxel_index({ x, 1, 0 });
                        const Voxel* twice_above_candidate_voxel
                            = &neighbour_voxels[twice_above_candidate_index];

                        VoxelIndex above_candidate_index
                            = hvox::voxel_index({ x, 0, 0 });
                        const Voxel* above_candidate_voxel
                            = &neighbour_voxels[above_candidate_index];

                        VoxelIndex candidate_index
                            = hvox::voxel_index({ x, CHUNK_LENGTH - 1, 0 });
                        const Voxel* candidate_voxel = &voxels[candidate_index];

                        if (is_solid(candidate_voxel)
                            && !is_solid(above_candidate_voxel)
                            && !is_solid(twice_above_candidate_voxel))
                        {
                            ChunkNavmeshNode candidate_voxel_coord = {
                                {x, CHUNK_LENGTH - 1, 0},
                                chunk_pos
                            };

                            struct {
                                ChunkNavmeshVertexDescriptor here, in_back_of_neighbour;
                            } candidate_voxel_vertex
                                = { impl::get_vertex(chunk, candidate_voxel_coord),
                                    impl::get_vertex(
                                        back_of_neighbour, candidate_voxel_coord
                                    ) };

                            {
                                std::unique_lock<std::shared_mutex> lock;
                                auto& navmesh = chunk->navmesh.get(lock);

                                boost::add_edge(
                                    back_of_neighbour_voxel_vertex.here,
                                    candidate_voxel_vertex.here,
                                    navmesh->graph
                                );
                                boost::add_edge(
                                    candidate_voxel_vertex.here,
                                    back_of_neighbour_voxel_vertex.here,
                                    navmesh->graph
                                );
                            }

                            {
                                std::unique_lock<std::shared_mutex> lock;
                                auto& navmesh = back_of_neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    back_of_neighbour_voxel_vertex.in_back_of_neighbour,
                                    candidate_voxel_vertex.in_back_of_neighbour,
                                    navmesh->graph
                                );
                                boost::add_edge(
                                    candidate_voxel_vertex.in_back_of_neighbour,
                                    back_of_neighbour_voxel_vertex.in_back_of_neighbour,
                                    navmesh->graph
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
                    std::shared_lock<std::shared_mutex> left_neighbour_voxel_lock,
                        above_left_neighbour_voxel_lock;
                    auto& left_neighbour_voxels
                        = left_neighbour->voxels.get(left_neighbour_voxel_lock);
                    auto& above_left_neighbour_voxels
                        = above_left_neighbour->voxels.get(
                            above_left_neighbour_voxel_lock
                        );

                    // Step up from y == CHUNK_LENGTH - 2
                    for (VoxelChunkPositionCoord z = 0; z < CHUNK_LENGTH; ++z) {
                        VoxelIndex this_voxel_index
                            = hvox::voxel_index({ 0, CHUNK_LENGTH - 2, z });
                        const Voxel* this_voxel = &voxels[this_voxel_index];

                        VoxelIndex above_this_voxel_index
                            = hvox::voxel_index({ 0, CHUNK_LENGTH - 1, z });
                        const Voxel* above_this_voxel = &voxels[above_this_voxel_index];

                        VoxelIndex twice_above_this_voxel_index
                            = hvox::voxel_index({ 0, 0, z });
                        const Voxel* twice_above_this_voxel
                            = &neighbour_voxels[twice_above_this_voxel_index];

                        if (!is_solid(this_voxel) || is_solid(above_this_voxel)
                            || is_solid(twice_above_this_voxel))
                            continue;

                        ChunkNavmeshNode this_voxel_coord = {
                            {0, CHUNK_LENGTH - 2, z},
                            chunk_pos
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor here, in_left_neighbour;
                        } this_voxel_vertex
                            = { impl::get_vertex(chunk, this_voxel_coord),
                                impl::get_vertex(left_neighbour, this_voxel_coord) };

                        VoxelIndex above_candidate_index
                            = hvox::voxel_index({ CHUNK_LENGTH - 1, 0, z });
                        const Voxel* above_candidate_voxel
                            = &above_left_neighbour_voxels[above_candidate_index];

                        VoxelIndex candidate_index = hvox::voxel_index(
                            { CHUNK_LENGTH - 1, CHUNK_LENGTH - 1, z }
                        );
                        const Voxel* candidate_voxel
                            = &left_neighbour_voxels[candidate_index];

                        if (!is_solid(candidate_voxel)
                            || is_solid(above_candidate_voxel))
                            continue;

                        ChunkNavmeshNode candidate_voxel_coord = {
                            {CHUNK_LENGTH - 1, CHUNK_LENGTH - 1, z},
                            left_neighbour->position
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor here, in_left_neighbour;
                        } candidate_voxel_vertex = {
                            impl::get_vertex(chunk, candidate_voxel_coord),
                            impl::get_vertex(left_neighbour, candidate_voxel_coord)
                        };

                        {
                            std::unique_lock<std::shared_mutex> lock;
                            auto& navmesh = chunk->navmesh.get(lock);

                            boost::add_edge(
                                this_voxel_vertex.here,
                                candidate_voxel_vertex.here,
                                navmesh->graph
                            );
                            boost::add_edge(
                                candidate_voxel_vertex.here,
                                this_voxel_vertex.here,
                                navmesh->graph
                            );
                        }

                        {
                            std::unique_lock<std::shared_mutex> lock;
                            auto& navmesh = left_neighbour->navmesh.get(lock);

                            boost::add_edge(
                                this_voxel_vertex.in_left_neighbour,
                                candidate_voxel_vertex.in_left_neighbour,
                                navmesh->graph
                            );
                            boost::add_edge(
                                candidate_voxel_vertex.in_left_neighbour,
                                this_voxel_vertex.in_left_neighbour,
                                navmesh->graph
                            );
                        }
                    }

                    // Step across and down from y == CHUNK_LENGTH - 1
                    for (VoxelChunkPositionCoord z = 0; z < CHUNK_LENGTH; ++z) {
                        VoxelIndex this_voxel_index
                            = hvox::voxel_index({ 0, CHUNK_LENGTH - 1, z });
                        const Voxel* this_voxel = &voxels[this_voxel_index];

                        VoxelIndex above_this_voxel_index
                            = hvox::voxel_index({ 0, 0, z });
                        const Voxel* above_this_voxel
                            = &neighbour_voxels[above_this_voxel_index];

                        // Necessary condition for step across and down.
                        if (!is_solid(this_voxel) || is_solid(above_this_voxel))
                            continue;

                        ChunkNavmeshNode this_voxel_coord = {
                            {0, CHUNK_LENGTH - 1, z},
                            chunk_pos
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor here, in_left_neighbour;
                        } this_voxel_vertex
                            = { impl::get_vertex(chunk, this_voxel_coord),
                                impl::get_vertex(left_neighbour, this_voxel_coord) };

                        VoxelIndex step_down_candidate_index = hvox::voxel_index(
                            { CHUNK_LENGTH - 1, CHUNK_LENGTH - 2, z }
                        );
                        const Voxel* step_down_candidate_voxel
                            = &left_neighbour_voxels[step_down_candidate_index];

                        VoxelIndex step_across_candidate_index = hvox::voxel_index(
                            { CHUNK_LENGTH - 1, CHUNK_LENGTH - 1, z }
                        );
                        const Voxel* step_across_candidate_voxel
                            = &left_neighbour_voxels[step_across_candidate_index];

                        VoxelIndex above_candidates_index
                            = hvox::voxel_index({ CHUNK_LENGTH - 1, 0, z });
                        const Voxel* above_candidates_voxel
                            = &above_left_neighbour_voxels[above_candidates_index];

                        // Step across
                        if (is_solid(step_across_candidate_voxel)
                            && !is_solid(above_candidates_voxel))
                        {
                            ChunkNavmeshNode candidate_voxel_coord = {
                                {CHUNK_LENGTH - 1, CHUNK_LENGTH - 1, z},
                                left_neighbour->position
                            };

                            struct {
                                ChunkNavmeshVertexDescriptor here, in_left_neighbour;
                            } candidate_voxel_vertex = {
                                impl::get_vertex(chunk, candidate_voxel_coord),
                                impl::get_vertex(left_neighbour, candidate_voxel_coord)
                            };

                            {
                                std::unique_lock<std::shared_mutex> lock;
                                auto& navmesh = chunk->navmesh.get(lock);

                                boost::add_edge(
                                    this_voxel_vertex.here,
                                    candidate_voxel_vertex.here,
                                    navmesh->graph
                                );
                                boost::add_edge(
                                    candidate_voxel_vertex.here,
                                    this_voxel_vertex.here,
                                    navmesh->graph
                                );
                            }

                            {
                                std::unique_lock<std::shared_mutex> lock;
                                auto& navmesh = left_neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    this_voxel_vertex.in_left_neighbour,
                                    candidate_voxel_vertex.in_left_neighbour,
                                    navmesh->graph
                                );
                                boost::add_edge(
                                    candidate_voxel_vertex.in_left_neighbour,
                                    this_voxel_vertex.in_left_neighbour,
                                    navmesh->graph
                                );
                            }
                        }
                        // Step down
                        else if (is_solid(step_down_candidate_voxel) && !is_solid(step_across_candidate_voxel) && !is_solid(above_candidates_voxel))
                        {
                            ChunkNavmeshNode candidate_voxel_coord = {
                                {CHUNK_LENGTH - 1, CHUNK_LENGTH - 2, z},
                                left_neighbour->position
                            };

                            struct {
                                ChunkNavmeshVertexDescriptor here, in_left_neighbour;
                            } candidate_voxel_vertex = {
                                impl::get_vertex(chunk, candidate_voxel_coord),
                                impl::get_vertex(left_neighbour, candidate_voxel_coord)
                            };

                            {
                                std::unique_lock<std::shared_mutex> lock;
                                auto& navmesh = chunk->navmesh.get(lock);

                                boost::add_edge(
                                    this_voxel_vertex.here,
                                    candidate_voxel_vertex.here,
                                    navmesh->graph
                                );
                                boost::add_edge(
                                    candidate_voxel_vertex.here,
                                    this_voxel_vertex.here,
                                    navmesh->graph
                                );
                            }

                            {
                                std::unique_lock<std::shared_mutex> lock;
                                auto& navmesh = left_neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    this_voxel_vertex.in_left_neighbour,
                                    candidate_voxel_vertex.in_left_neighbour,
                                    navmesh->graph
                                );
                                boost::add_edge(
                                    candidate_voxel_vertex.in_left_neighbour,
                                    this_voxel_vertex.in_left_neighbour,
                                    navmesh->graph
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
                    std::shared_lock<std::shared_mutex> right_neighbour_voxel_lock;
                    auto                                right_neighbour_voxels
                        = right_neighbour->voxels.get(right_neighbour_voxel_lock);
                    std::shared_lock<std::shared_mutex>
                         above_right_neighbour_voxel_lock;
                    auto above_right_neighbour_voxels
                        = above_right_neighbour->voxels.get(
                            above_right_neighbour_voxel_lock
                        );

                    // Step up from y == CHUNK_LENGTH - 2
                    for (VoxelChunkPositionCoord z = 0; z < CHUNK_LENGTH; ++z) {
                        VoxelIndex this_voxel_index = hvox::voxel_index(
                            { CHUNK_LENGTH - 1, CHUNK_LENGTH - 2, z }
                        );
                        const Voxel* this_voxel = &voxels[this_voxel_index];

                        VoxelIndex above_this_voxel_index = hvox::voxel_index(
                            { CHUNK_LENGTH - 1, CHUNK_LENGTH - 1, z }
                        );
                        const Voxel* above_this_voxel = &voxels[above_this_voxel_index];

                        VoxelIndex twice_above_this_voxel_index
                            = hvox::voxel_index({ CHUNK_LENGTH - 1, 0, z });
                        const Voxel* twice_above_this_voxel
                            = &neighbour_voxels[twice_above_this_voxel_index];

                        if (!is_solid(this_voxel) || is_solid(above_this_voxel)
                            || is_solid(twice_above_this_voxel))
                            continue;

                        ChunkNavmeshNode this_voxel_coord = {
                            {CHUNK_LENGTH - 1, CHUNK_LENGTH - 2, z},
                            chunk_pos
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor here, in_right_neighbour;
                        } this_voxel_vertex
                            = { impl::get_vertex(chunk, this_voxel_coord),
                                impl::get_vertex(right_neighbour, this_voxel_coord) };

                        VoxelIndex above_candidate_index
                            = hvox::voxel_index({ 0, 0, z });
                        const Voxel* above_candidate_voxel
                            = &above_right_neighbour_voxels[above_candidate_index];

                        VoxelIndex candidate_index
                            = hvox::voxel_index({ 0, CHUNK_LENGTH - 1, z });
                        const Voxel* candidate_voxel
                            = &right_neighbour_voxels[candidate_index];

                        if (!is_solid(candidate_voxel)
                            || is_solid(above_candidate_voxel))
                            continue;

                        ChunkNavmeshNode candidate_voxel_coord = {
                            {0, CHUNK_LENGTH - 1, z},
                            right_neighbour->position
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor here, in_right_neighbour;
                        } candidate_voxel_vertex = {
                            impl::get_vertex(chunk, candidate_voxel_coord),
                            impl::get_vertex(right_neighbour, candidate_voxel_coord)
                        };

                        {
                            std::unique_lock<std::shared_mutex> lock;
                            auto& navmesh = chunk->navmesh.get(lock);

                            boost::add_edge(
                                this_voxel_vertex.here,
                                candidate_voxel_vertex.here,
                                navmesh->graph
                            );
                            boost::add_edge(
                                candidate_voxel_vertex.here,
                                this_voxel_vertex.here,
                                navmesh->graph
                            );
                        }

                        {
                            std::unique_lock<std::shared_mutex> lock;
                            auto& navmesh = right_neighbour->navmesh.get(lock);

                            boost::add_edge(
                                this_voxel_vertex.in_right_neighbour,
                                candidate_voxel_vertex.in_right_neighbour,
                                navmesh->graph
                            );
                            boost::add_edge(
                                candidate_voxel_vertex.in_right_neighbour,
                                this_voxel_vertex.in_right_neighbour,
                                navmesh->graph
                            );
                        }
                    }

                    // Step across and down from y == CHUNK_LENGTH - 1
                    for (VoxelChunkPositionCoord z = 0; z < CHUNK_LENGTH; ++z) {
                        VoxelIndex this_voxel_index = hvox::voxel_index(
                            { CHUNK_LENGTH - 1, CHUNK_LENGTH - 1, z }
                        );
                        const Voxel* this_voxel = &voxels[this_voxel_index];

                        VoxelIndex above_this_voxel_index
                            = hvox::voxel_index({ CHUNK_LENGTH - 1, 0, z });
                        const Voxel* above_this_voxel
                            = &neighbour_voxels[above_this_voxel_index];

                        // Necessary condition for step across and down.
                        if (!is_solid(this_voxel) || is_solid(above_this_voxel))
                            continue;

                        ChunkNavmeshNode this_voxel_coord = {
                            {CHUNK_LENGTH - 1, CHUNK_LENGTH - 1, z},
                            chunk_pos
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor here, in_right_neighbour;
                        } this_voxel_vertex
                            = { impl::get_vertex(chunk, this_voxel_coord),
                                impl::get_vertex(right_neighbour, this_voxel_coord) };

                        VoxelIndex step_down_candidate_index
                            = hvox::voxel_index({ 0, CHUNK_LENGTH - 2, z });
                        const Voxel* step_down_candidate_voxel
                            = &right_neighbour_voxels[step_down_candidate_index];

                        VoxelIndex step_across_candidate_index
                            = hvox::voxel_index({ 0, CHUNK_LENGTH - 1, z });
                        const Voxel* step_across_candidate_voxel
                            = &right_neighbour_voxels[step_across_candidate_index];

                        VoxelIndex above_candidates_index
                            = hvox::voxel_index({ 0, 0, z });
                        const Voxel* above_candidates_voxel
                            = &above_right_neighbour_voxels[above_candidates_index];

                        // Step across
                        if (is_solid(step_across_candidate_voxel)
                            && !is_solid(above_candidates_voxel))
                        {
                            ChunkNavmeshNode candidate_voxel_coord = {
                                {0, CHUNK_LENGTH - 1, z},
                                right_neighbour->position
                            };

                            struct {
                                ChunkNavmeshVertexDescriptor here, in_right_neighbour;
                            } candidate_voxel_vertex = {
                                impl::get_vertex(chunk, candidate_voxel_coord),
                                impl::get_vertex(right_neighbour, candidate_voxel_coord)
                            };

                            {
                                std::unique_lock<std::shared_mutex> lock;
                                auto& navmesh = chunk->navmesh.get(lock);

                                boost::add_edge(
                                    this_voxel_vertex.here,
                                    candidate_voxel_vertex.here,
                                    navmesh->graph
                                );
                                boost::add_edge(
                                    candidate_voxel_vertex.here,
                                    this_voxel_vertex.here,
                                    navmesh->graph
                                );
                            }

                            {
                                std::unique_lock<std::shared_mutex> lock;
                                auto& navmesh = right_neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    this_voxel_vertex.in_right_neighbour,
                                    candidate_voxel_vertex.in_right_neighbour,
                                    navmesh->graph
                                );
                                boost::add_edge(
                                    candidate_voxel_vertex.in_right_neighbour,
                                    this_voxel_vertex.in_right_neighbour,
                                    navmesh->graph
                                );
                            }
                        }
                        // Step down
                        else if (is_solid(step_down_candidate_voxel) && !is_solid(step_across_candidate_voxel) && !is_solid(above_candidates_voxel))
                        {
                            ChunkNavmeshNode candidate_voxel_coord = {
                                {0, CHUNK_LENGTH - 2, z},
                                right_neighbour->position
                            };

                            struct {
                                ChunkNavmeshVertexDescriptor here, in_right_neighbour;
                            } candidate_voxel_vertex = {
                                impl::get_vertex(chunk, candidate_voxel_coord),
                                impl::get_vertex(right_neighbour, candidate_voxel_coord)
                            };

                            {
                                std::unique_lock<std::shared_mutex> lock;
                                auto& navmesh = chunk->navmesh.get(lock);

                                boost::add_edge(
                                    this_voxel_vertex.here,
                                    candidate_voxel_vertex.here,
                                    navmesh->graph
                                );
                                boost::add_edge(
                                    candidate_voxel_vertex.here,
                                    this_voxel_vertex.here,
                                    navmesh->graph
                                );
                            }

                            {
                                std::unique_lock<std::shared_mutex> lock;
                                auto& navmesh = right_neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    this_voxel_vertex.in_right_neighbour,
                                    candidate_voxel_vertex.in_right_neighbour,
                                    navmesh->graph
                                );
                                boost::add_edge(
                                    candidate_voxel_vertex.in_right_neighbour,
                                    this_voxel_vertex.in_right_neighbour,
                                    navmesh->graph
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
                    std::shared_lock<std::shared_mutex> front_neighbour_voxel_lock;
                    auto                                front_neighbour_voxels
                        = front_neighbour->voxels.get(front_neighbour_voxel_lock);
                    std::shared_lock<std::shared_mutex>
                         above_front_neighbour_voxel_lock;
                    auto above_front_neighbour_voxels
                        = above_front_neighbour->voxels.get(
                            above_front_neighbour_voxel_lock
                        );

                    // Step up from y == CHUNK_LENGTH - 2
                    for (VoxelChunkPositionCoord x = 0; x < CHUNK_LENGTH; ++x) {
                        VoxelIndex this_voxel_index = hvox::voxel_index(
                            { x, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1 }
                        );
                        const Voxel* this_voxel = &voxels[this_voxel_index];

                        VoxelIndex above_this_voxel_index = hvox::voxel_index(
                            { x, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1 }
                        );
                        const Voxel* above_this_voxel = &voxels[above_this_voxel_index];

                        VoxelIndex twice_above_this_voxel_index
                            = hvox::voxel_index({ x, 0, CHUNK_LENGTH - 1 });
                        const Voxel* twice_above_this_voxel
                            = &neighbour_voxels[twice_above_this_voxel_index];

                        if (!is_solid(this_voxel) || is_solid(above_this_voxel)
                            || is_solid(twice_above_this_voxel))
                            continue;

                        ChunkNavmeshNode this_voxel_coord = {
                            {x, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1},
                            chunk_pos
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor here, in_front_neighbour;
                        } this_voxel_vertex
                            = { impl::get_vertex(chunk, this_voxel_coord),
                                impl::get_vertex(front_neighbour, this_voxel_coord) };

                        VoxelIndex above_candidate_index
                            = hvox::voxel_index({ x, 0, 0 });
                        const Voxel* above_candidate_voxel
                            = &above_front_neighbour_voxels[above_candidate_index];

                        VoxelIndex candidate_index
                            = hvox::voxel_index({ x, CHUNK_LENGTH - 1, 0 });
                        const Voxel* candidate_voxel
                            = &front_neighbour_voxels[candidate_index];

                        if (!is_solid(candidate_voxel)
                            || is_solid(above_candidate_voxel))
                            continue;

                        ChunkNavmeshNode candidate_voxel_coord = {
                            {x, CHUNK_LENGTH - 1, 0},
                            front_neighbour->position
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor here, in_front_neighbour;
                        } candidate_voxel_vertex = {
                            impl::get_vertex(chunk, candidate_voxel_coord),
                            impl::get_vertex(front_neighbour, candidate_voxel_coord)
                        };

                        {
                            std::unique_lock<std::shared_mutex> lock;
                            auto& navmesh = chunk->navmesh.get(lock);

                            boost::add_edge(
                                this_voxel_vertex.here,
                                candidate_voxel_vertex.here,
                                navmesh->graph
                            );
                            boost::add_edge(
                                candidate_voxel_vertex.here,
                                this_voxel_vertex.here,
                                navmesh->graph
                            );
                        }

                        {
                            std::unique_lock<std::shared_mutex> lock;
                            auto& navmesh = front_neighbour->navmesh.get(lock);

                            boost::add_edge(
                                this_voxel_vertex.in_front_neighbour,
                                candidate_voxel_vertex.in_front_neighbour,
                                navmesh->graph
                            );
                            boost::add_edge(
                                candidate_voxel_vertex.in_front_neighbour,
                                this_voxel_vertex.in_front_neighbour,
                                navmesh->graph
                            );
                        }
                    }

                    // Step across and down from y == CHUNK_LENGTH - 1
                    for (VoxelChunkPositionCoord x = 0; x < CHUNK_LENGTH; ++x) {
                        VoxelIndex this_voxel_index = hvox::voxel_index(
                            { x, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1 }
                        );
                        const Voxel* this_voxel = &voxels[this_voxel_index];

                        VoxelIndex above_this_voxel_index
                            = hvox::voxel_index({ x, 0, CHUNK_LENGTH - 1 });
                        const Voxel* above_this_voxel
                            = &neighbour_voxels[above_this_voxel_index];

                        // Necessary condition for step across and down.
                        if (!is_solid(this_voxel) || is_solid(above_this_voxel))
                            continue;

                        ChunkNavmeshNode this_voxel_coord = {
                            {x, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1},
                            chunk_pos
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor here, in_front_neighbour;
                        } this_voxel_vertex
                            = { impl::get_vertex(chunk, this_voxel_coord),
                                impl::get_vertex(front_neighbour, this_voxel_coord) };

                        VoxelIndex step_down_candidate_index
                            = hvox::voxel_index({ x, CHUNK_LENGTH - 2, 0 });
                        const Voxel* step_down_candidate_voxel
                            = &front_neighbour_voxels[step_down_candidate_index];

                        VoxelIndex step_across_candidate_index
                            = hvox::voxel_index({ x, CHUNK_LENGTH - 1, 0 });
                        const Voxel* step_across_candidate_voxel
                            = &front_neighbour_voxels[step_across_candidate_index];

                        VoxelIndex above_candidates_index
                            = hvox::voxel_index({ x, 0, 0 });
                        const Voxel* above_candidates_voxel
                            = &above_front_neighbour_voxels[above_candidates_index];

                        // Step across
                        if (is_solid(step_across_candidate_voxel)
                            && !is_solid(above_candidates_voxel))
                        {
                            ChunkNavmeshNode candidate_voxel_coord = {
                                {x, CHUNK_LENGTH - 1, 0},
                                front_neighbour->position
                            };

                            struct {
                                ChunkNavmeshVertexDescriptor here, in_front_neighbour;
                            } candidate_voxel_vertex = {
                                impl::get_vertex(chunk, candidate_voxel_coord),
                                impl::get_vertex(front_neighbour, candidate_voxel_coord)
                            };

                            {
                                std::unique_lock<std::shared_mutex> lock;
                                auto& navmesh = chunk->navmesh.get(lock);

                                boost::add_edge(
                                    this_voxel_vertex.here,
                                    candidate_voxel_vertex.here,
                                    navmesh->graph
                                );
                                boost::add_edge(
                                    candidate_voxel_vertex.here,
                                    this_voxel_vertex.here,
                                    navmesh->graph
                                );
                            }

                            {
                                std::unique_lock<std::shared_mutex> lock;
                                auto& navmesh = front_neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    this_voxel_vertex.in_front_neighbour,
                                    candidate_voxel_vertex.in_front_neighbour,
                                    navmesh->graph
                                );
                                boost::add_edge(
                                    candidate_voxel_vertex.in_front_neighbour,
                                    this_voxel_vertex.in_front_neighbour,
                                    navmesh->graph
                                );
                            }
                        }
                        // Step down
                        else if (is_solid(step_down_candidate_voxel) && !is_solid(step_across_candidate_voxel) && !is_solid(above_candidates_voxel))
                        {
                            ChunkNavmeshNode candidate_voxel_coord = {
                                {x, CHUNK_LENGTH - 2, 0},
                                front_neighbour->position
                            };

                            struct {
                                ChunkNavmeshVertexDescriptor here, in_front_neighbour;
                            } candidate_voxel_vertex = {
                                impl::get_vertex(chunk, candidate_voxel_coord),
                                impl::get_vertex(front_neighbour, candidate_voxel_coord)
                            };

                            {
                                std::unique_lock<std::shared_mutex> lock;
                                auto& navmesh = chunk->navmesh.get(lock);

                                boost::add_edge(
                                    this_voxel_vertex.here,
                                    candidate_voxel_vertex.here,
                                    navmesh->graph
                                );
                                boost::add_edge(
                                    candidate_voxel_vertex.here,
                                    this_voxel_vertex.here,
                                    navmesh->graph
                                );
                            }

                            {
                                std::unique_lock<std::shared_mutex> lock;
                                auto& navmesh = front_neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    this_voxel_vertex.in_front_neighbour,
                                    candidate_voxel_vertex.in_front_neighbour,
                                    navmesh->graph
                                );
                                boost::add_edge(
                                    candidate_voxel_vertex.in_front_neighbour,
                                    this_voxel_vertex.in_front_neighbour,
                                    navmesh->graph
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
                    std::shared_lock<std::shared_mutex> back_neighbour_voxel_lock,
                        above_back_neighbour_voxel_lock;
                    auto& back_neighbour_voxels
                        = back_neighbour->voxels.get(back_neighbour_voxel_lock);
                    auto& above_back_neighbour_voxels
                        = above_back_neighbour->voxels.get(
                            above_back_neighbour_voxel_lock
                        );

                    // Step up from y == CHUNK_LENGTH - 2
                    for (VoxelChunkPositionCoord x = 0; x < CHUNK_LENGTH; ++x) {
                        VoxelIndex this_voxel_index
                            = hvox::voxel_index({ x, CHUNK_LENGTH - 2, 0 });
                        const Voxel* this_voxel = &voxels[this_voxel_index];

                        VoxelIndex above_this_voxel_index
                            = hvox::voxel_index({ x, CHUNK_LENGTH - 1, 0 });
                        const Voxel* above_this_voxel = &voxels[above_this_voxel_index];

                        VoxelIndex twice_above_this_voxel_index
                            = hvox::voxel_index({ CHUNK_LENGTH - 1, 0, 0 });
                        const Voxel* twice_above_this_voxel
                            = &neighbour_voxels[twice_above_this_voxel_index];

                        if (!is_solid(this_voxel) || is_solid(above_this_voxel)
                            || is_solid(twice_above_this_voxel))
                            continue;

                        ChunkNavmeshNode this_voxel_coord = {
                            {x, CHUNK_LENGTH - 2, 0},
                            chunk_pos
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor here, in_back_neighbour;
                        } this_voxel_vertex
                            = { impl::get_vertex(chunk, this_voxel_coord),
                                impl::get_vertex(back_neighbour, this_voxel_coord) };

                        VoxelIndex above_candidate_index
                            = hvox::voxel_index({ x, 0, CHUNK_LENGTH - 1 });
                        const Voxel* above_candidate_voxel
                            = &above_back_neighbour_voxels[above_candidate_index];

                        VoxelIndex candidate_index = hvox::voxel_index(
                            { x, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1 }
                        );
                        const Voxel* candidate_voxel
                            = &back_neighbour_voxels[candidate_index];

                        if (!is_solid(candidate_voxel)
                            || is_solid(above_candidate_voxel))
                            continue;

                        ChunkNavmeshNode candidate_voxel_coord = {
                            {x, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1},
                            back_neighbour->position
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor here, in_back_neighbour;
                        } candidate_voxel_vertex = {
                            impl::get_vertex(chunk, candidate_voxel_coord),
                            impl::get_vertex(back_neighbour, candidate_voxel_coord)
                        };

                        {
                            std::unique_lock<std::shared_mutex> lock;
                            auto& navmesh = chunk->navmesh.get(lock);

                            boost::add_edge(
                                this_voxel_vertex.here,
                                candidate_voxel_vertex.here,
                                navmesh->graph
                            );
                            boost::add_edge(
                                candidate_voxel_vertex.here,
                                this_voxel_vertex.here,
                                navmesh->graph
                            );
                        }

                        {
                            std::unique_lock<std::shared_mutex> lock;
                            auto& navmesh = back_neighbour->navmesh.get(lock);

                            boost::add_edge(
                                this_voxel_vertex.in_back_neighbour,
                                candidate_voxel_vertex.in_back_neighbour,
                                navmesh->graph
                            );
                            boost::add_edge(
                                candidate_voxel_vertex.in_back_neighbour,
                                this_voxel_vertex.in_back_neighbour,
                                navmesh->graph
                            );
                        }
                    }

                    // Step across and down from y == CHUNK_LENGTH - 1
                    for (VoxelChunkPositionCoord x = 0; x < CHUNK_LENGTH; ++x) {
                        VoxelIndex this_voxel_index
                            = hvox::voxel_index({ x, CHUNK_LENGTH - 1, 0 });
                        const Voxel* this_voxel = &voxels[this_voxel_index];

                        VoxelIndex above_this_voxel_index
                            = hvox::voxel_index({ x, 0, 0 });
                        const Voxel* above_this_voxel
                            = &neighbour_voxels[above_this_voxel_index];

                        // Necessary condition for step across and down.
                        if (!is_solid(this_voxel) || is_solid(above_this_voxel))
                            continue;

                        ChunkNavmeshNode this_voxel_coord = {
                            {x, CHUNK_LENGTH - 1, 0},
                            chunk_pos
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor here, in_back_neighbour;
                        } this_voxel_vertex
                            = { impl::get_vertex(chunk, this_voxel_coord),
                                impl::get_vertex(back_neighbour, this_voxel_coord) };

                        VoxelIndex step_down_candidate_index = hvox::voxel_index(
                            { x, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1 }
                        );
                        const Voxel* step_down_candidate_voxel
                            = &back_neighbour_voxels[step_down_candidate_index];

                        VoxelIndex step_across_candidate_index = hvox::voxel_index(
                            { x, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1 }
                        );
                        const Voxel* step_across_candidate_voxel
                            = &back_neighbour_voxels[step_across_candidate_index];

                        VoxelIndex above_candidates_index
                            = hvox::voxel_index({ x, 0, CHUNK_LENGTH - 1 });
                        const Voxel* above_candidates_voxel
                            = &above_back_neighbour_voxels[above_candidates_index];

                        // Step across
                        if (is_solid(step_across_candidate_voxel)
                            && !is_solid(above_candidates_voxel))
                        {
                            ChunkNavmeshNode candidate_voxel_coord = {
                                {x, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1},
                                back_neighbour->position
                            };

                            struct {
                                ChunkNavmeshVertexDescriptor here, in_back_neighbour;
                            } candidate_voxel_vertex = {
                                impl::get_vertex(chunk, candidate_voxel_coord),
                                impl::get_vertex(back_neighbour, candidate_voxel_coord)
                            };

                            {
                                std::unique_lock<std::shared_mutex> lock;
                                auto& navmesh = chunk->navmesh.get(lock);

                                boost::add_edge(
                                    this_voxel_vertex.here,
                                    candidate_voxel_vertex.here,
                                    navmesh->graph
                                );
                                boost::add_edge(
                                    candidate_voxel_vertex.here,
                                    this_voxel_vertex.here,
                                    navmesh->graph
                                );
                            }

                            {
                                std::unique_lock<std::shared_mutex> lock;
                                auto& navmesh = back_neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    this_voxel_vertex.in_back_neighbour,
                                    candidate_voxel_vertex.in_back_neighbour,
                                    navmesh->graph
                                );
                                boost::add_edge(
                                    candidate_voxel_vertex.in_back_neighbour,
                                    this_voxel_vertex.in_back_neighbour,
                                    navmesh->graph
                                );
                            }
                        }
                        // Step down
                        else if (is_solid(step_down_candidate_voxel) && !is_solid(step_across_candidate_voxel) && !is_solid(above_candidates_voxel))
                        {
                            ChunkNavmeshNode candidate_voxel_coord = {
                                {x, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1},
                                back_neighbour->position
                            };

                            struct {
                                ChunkNavmeshVertexDescriptor here, in_back_neighbour;
                            } candidate_voxel_vertex = {
                                impl::get_vertex(chunk, candidate_voxel_coord),
                                impl::get_vertex(back_neighbour, candidate_voxel_coord)
                            };

                            {
                                std::unique_lock<std::shared_mutex> lock;
                                auto& navmesh = chunk->navmesh.get(lock);

                                boost::add_edge(
                                    this_voxel_vertex.here,
                                    candidate_voxel_vertex.here,
                                    navmesh->graph
                                );
                                boost::add_edge(
                                    candidate_voxel_vertex.here,
                                    this_voxel_vertex.here,
                                    navmesh->graph
                                );
                            }

                            {
                                std::unique_lock<std::shared_mutex> lock;
                                auto& navmesh = back_neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    this_voxel_vertex.in_back_neighbour,
                                    candidate_voxel_vertex.in_back_neighbour,
                                    navmesh->graph
                                );
                                boost::add_edge(
                                    candidate_voxel_vertex.in_back_neighbour,
                                    this_voxel_vertex.in_back_neighbour,
                                    navmesh->graph
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
            std::shared_lock<std::shared_mutex> neighbour_voxel_lock;
            auto& neighbour_voxels = neighbour->voxels.get(neighbour_voxel_lock);
            for (VoxelChunkPositionCoord x = 1; x < CHUNK_LENGTH - 1; ++x) {
                for (VoxelChunkPositionCoord z = 1; z < CHUNK_LENGTH - 1; ++z) {
                    VoxelIndex   this_voxel_index = hvox::voxel_index({ x, 0, z });
                    const Voxel* this_voxel       = &voxels[this_voxel_index];

                    VoxelIndex   above_this_index = hvox::voxel_index({ x, 1, z });
                    const Voxel* above_this_voxel = &voxels[above_this_index];

                    VoxelIndex neighbour_voxel_index
                        = hvox::voxel_index({ x, CHUNK_LENGTH - 1, z });
                    const Voxel* neighbour_voxel
                        = &neighbour_voxels[neighbour_voxel_index];

                    if (!is_solid(neighbour_voxel) || is_solid(this_voxel)) continue;

                    // Ensure node exists for this voxel.
                    ChunkNavmeshNode neighbour_voxel_coord = {
                        {x, CHUNK_LENGTH - 1, z},
                        neighbour->position
                    };

                    struct {
                        ChunkNavmeshVertexDescriptor here, in_neighbour;
                    } neighbour_voxel_vertex
                        = { impl::get_vertex(chunk, neighbour_voxel_coord),
                            impl::get_vertex(neighbour, neighbour_voxel_coord) };

                    // Up
                    if (!is_solid(above_this_voxel)) {
                        // Left
                        VoxelIndex left_of_this_voxel_index
                            = hvox::voxel_index({ x - 1, 0, z });
                        const Voxel* left_of_this_voxel
                            = &voxels[left_of_this_voxel_index];

                        VoxelIndex above_and_left_of_this_voxel_index
                            = hvox::voxel_index({ x - 1, 1, z });
                        const Voxel* above_and_left_of_this_voxel
                            = &voxels[above_and_left_of_this_voxel_index];

                        if (is_solid(left_of_this_voxel)
                            && !is_solid(above_and_left_of_this_voxel))
                        {
                            ChunkNavmeshNode left_of_this_voxel_coord = {
                                {x - 1, 0, z},
                                chunk_pos
                            };

                            // Ensure node exists for this voxel.
                            struct {
                                ChunkNavmeshVertexDescriptor here, in_neighbour;
                            } left_of_this_voxel_vertex = {
                                impl::get_vertex(chunk, left_of_this_voxel_coord),
                                impl::get_vertex(neighbour, left_of_this_voxel_coord)
                            };

                            {
                                std::unique_lock<std::shared_mutex> lock;
                                auto& navmesh = chunk->navmesh.get(lock);

                                boost::add_edge(
                                    neighbour_voxel_vertex.here,
                                    left_of_this_voxel_vertex.here,
                                    navmesh->graph
                                );
                                boost::add_edge(
                                    left_of_this_voxel_vertex.here,
                                    neighbour_voxel_vertex.here,
                                    navmesh->graph
                                );
                            }

                            {
                                std::unique_lock<std::shared_mutex> lock;
                                auto& navmesh = neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    neighbour_voxel_vertex.in_neighbour,
                                    left_of_this_voxel_vertex.in_neighbour,
                                    navmesh->graph
                                );
                                boost::add_edge(
                                    left_of_this_voxel_vertex.in_neighbour,
                                    neighbour_voxel_vertex.in_neighbour,
                                    navmesh->graph
                                );
                            }
                        }

                        // Right
                        VoxelIndex right_of_this_voxel_index
                            = hvox::voxel_index({ x + 1, 0, z });
                        const Voxel* right_of_this_voxel
                            = &voxels[right_of_this_voxel_index];

                        VoxelIndex above_and_right_of_this_voxel_index
                            = hvox::voxel_index({ x + 1, 1, z });
                        const Voxel* above_and_right_of_this_voxel
                            = &voxels[above_and_right_of_this_voxel_index];

                        if (is_solid(right_of_this_voxel)
                            && !is_solid(above_and_right_of_this_voxel))
                        {
                            ChunkNavmeshNode right_of_this_voxel_coord = {
                                {x + 1, 0, z},
                                chunk_pos
                            };

                            // Ensure node exists for this voxel.
                            struct {
                                ChunkNavmeshVertexDescriptor here, in_neighbour;
                            } right_of_this_voxel_vertex = {
                                impl::get_vertex(chunk, right_of_this_voxel_coord),
                                impl::get_vertex(neighbour, right_of_this_voxel_coord)
                            };

                            {
                                std::unique_lock<std::shared_mutex> lock;
                                auto& navmesh = chunk->navmesh.get(lock);

                                boost::add_edge(
                                    neighbour_voxel_vertex.here,
                                    right_of_this_voxel_vertex.here,
                                    navmesh->graph
                                );
                                boost::add_edge(
                                    right_of_this_voxel_vertex.here,
                                    neighbour_voxel_vertex.here,
                                    navmesh->graph
                                );
                            }

                            {
                                std::unique_lock<std::shared_mutex> lock;
                                auto& navmesh = neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    neighbour_voxel_vertex.in_neighbour,
                                    right_of_this_voxel_vertex.in_neighbour,
                                    navmesh->graph
                                );
                                boost::add_edge(
                                    right_of_this_voxel_vertex.in_neighbour,
                                    neighbour_voxel_vertex.in_neighbour,
                                    navmesh->graph
                                );
                            }
                        }

                        // Front
                        VoxelIndex front_of_this_voxel_index
                            = hvox::voxel_index({ x, 0, z + 1 });
                        const Voxel* front_of_this_voxel
                            = &voxels[front_of_this_voxel_index];

                        VoxelIndex above_and_front_of_this_voxel_index
                            = hvox::voxel_index({ x, 1, z + 1 });
                        const Voxel* above_and_front_of_this_voxel
                            = &voxels[above_and_front_of_this_voxel_index];

                        if (is_solid(front_of_this_voxel)
                            && !is_solid(above_and_front_of_this_voxel))
                        {
                            ChunkNavmeshNode front_of_this_voxel_coord = {
                                {x, 0, z + 1},
                                chunk_pos
                            };

                            // Ensure node exists for this voxel.
                            struct {
                                ChunkNavmeshVertexDescriptor here, in_neighbour;
                            } front_of_this_voxel_vertex = {
                                impl::get_vertex(chunk, front_of_this_voxel_coord),
                                impl::get_vertex(neighbour, front_of_this_voxel_coord)
                            };

                            {
                                std::unique_lock<std::shared_mutex> lock;
                                auto& navmesh = chunk->navmesh.get(lock);

                                boost::add_edge(
                                    neighbour_voxel_vertex.here,
                                    front_of_this_voxel_vertex.here,
                                    navmesh->graph
                                );
                                boost::add_edge(
                                    front_of_this_voxel_vertex.here,
                                    neighbour_voxel_vertex.here,
                                    navmesh->graph
                                );
                            }

                            {
                                std::unique_lock<std::shared_mutex> lock;
                                auto& navmesh = neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    neighbour_voxel_vertex.in_neighbour,
                                    front_of_this_voxel_vertex.in_neighbour,
                                    navmesh->graph
                                );
                                boost::add_edge(
                                    front_of_this_voxel_vertex.in_neighbour,
                                    neighbour_voxel_vertex.in_neighbour,
                                    navmesh->graph
                                );
                            }
                        }

                        // Back
                        VoxelIndex back_of_this_voxel_index
                            = hvox::voxel_index({ x, 0, z - 1 });
                        const Voxel* back_of_this_voxel
                            = &voxels[back_of_this_voxel_index];

                        VoxelIndex above_and_back_of_this_voxel_index
                            = hvox::voxel_index({ x, 1, z - 1 });
                        const Voxel* above_and_back_of_this_voxel
                            = &voxels[above_and_back_of_this_voxel_index];

                        if (is_solid(back_of_this_voxel)
                            && !is_solid(above_and_back_of_this_voxel))
                        {
                            ChunkNavmeshNode back_of_this_voxel_coord = {
                                {x, 0, z - 1},
                                chunk_pos
                            };

                            // Ensure node exists for this voxel.
                            struct {
                                ChunkNavmeshVertexDescriptor here, in_neighbour;
                            } back_of_this_voxel_vertex = {
                                impl::get_vertex(chunk, back_of_this_voxel_coord),
                                impl::get_vertex(neighbour, back_of_this_voxel_coord)
                            };

                            {
                                std::unique_lock<std::shared_mutex> lock;
                                auto& navmesh = chunk->navmesh.get(lock);

                                boost::add_edge(
                                    neighbour_voxel_vertex.here,
                                    back_of_this_voxel_vertex.here,
                                    navmesh->graph
                                );
                                boost::add_edge(
                                    back_of_this_voxel_vertex.here,
                                    neighbour_voxel_vertex.here,
                                    navmesh->graph
                                );
                            }

                            {
                                std::unique_lock<std::shared_mutex> lock;
                                auto& navmesh = neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    neighbour_voxel_vertex.in_neighbour,
                                    back_of_this_voxel_vertex.in_neighbour,
                                    navmesh->graph
                                );
                                boost::add_edge(
                                    back_of_this_voxel_vertex.in_neighbour,
                                    neighbour_voxel_vertex.in_neighbour,
                                    navmesh->graph
                                );
                            }
                        }
                    }

                    ///// Across and Down

                    // Left
                    VoxelIndex left_of_neighbour_voxel_index
                        = hvox::voxel_index({ x - 1, CHUNK_LENGTH - 1, z });
                    const Voxel* left_of_neighbour_voxel
                        = &neighbour_voxels[left_of_neighbour_voxel_index];

                    VoxelIndex left_of_and_below_neighbour_voxel_index
                        = hvox::voxel_index({ x - 1, CHUNK_LENGTH - 2, z });
                    const Voxel* left_of_and_below_neighbour_voxel
                        = &neighbour_voxels[left_of_and_below_neighbour_voxel_index];

                    VoxelIndex left_of_this_voxel_index
                        = hvox::voxel_index({ x - 1, 0, z });
                    const Voxel* left_of_this_voxel = &voxels[left_of_this_voxel_index];

                    // Across
                    if (is_solid(left_of_neighbour_voxel)
                        && !is_solid(left_of_this_voxel))
                    {
                        ChunkNavmeshNode left_of_neighbour_voxel_coord = {
                            {x - 1, CHUNK_LENGTH - 1, z},
                            neighbour->position
                        };

                        // Ensure node exists for this voxel.
                        ChunkNavmeshVertexDescriptor left_of_neighbour_voxel_vertex
                            = impl::get_vertex(
                                neighbour, left_of_neighbour_voxel_coord
                            );

                        {
                            std::unique_lock<std::shared_mutex> lock;
                            auto& navmesh = neighbour->navmesh.get(lock);

                            boost::add_edge(
                                neighbour_voxel_vertex.in_neighbour,
                                left_of_neighbour_voxel_vertex,
                                navmesh->graph
                            );
                            boost::add_edge(
                                left_of_neighbour_voxel_vertex,
                                neighbour_voxel_vertex.in_neighbour,
                                navmesh->graph
                            );
                        }
                        // Down
                    } else if (is_solid(left_of_and_below_neighbour_voxel) && !is_solid(left_of_neighbour_voxel) && !is_solid(left_of_this_voxel))
                    {
                        ChunkNavmeshNode left_of_and_below_neighbour_voxel_coord = {
                            {x - 1, CHUNK_LENGTH - 2, z},
                            neighbour->position
                        };

                        // Ensure node exists for this voxel.
                        ChunkNavmeshVertexDescriptor
                            left_of_and_below_neighbour_voxel_vertex
                            = impl::get_vertex(
                                neighbour, left_of_and_below_neighbour_voxel_coord
                            );

                        {
                            std::unique_lock<std::shared_mutex> lock;
                            auto& navmesh = neighbour->navmesh.get(lock);

                            boost::add_edge(
                                neighbour_voxel_vertex.in_neighbour,
                                left_of_and_below_neighbour_voxel_vertex,
                                navmesh->graph
                            );
                            boost::add_edge(
                                left_of_and_below_neighbour_voxel_vertex,
                                neighbour_voxel_vertex.in_neighbour,
                                navmesh->graph
                            );
                        }
                    }

                    // Right
                    VoxelIndex right_of_neighbour_voxel_index
                        = hvox::voxel_index({ x + 1, CHUNK_LENGTH - 1, z });
                    const Voxel* right_of_neighbour_voxel
                        = &neighbour_voxels[right_of_neighbour_voxel_index];

                    VoxelIndex right_of_and_below_neighbour_voxel_index
                        = hvox::voxel_index({ x + 1, CHUNK_LENGTH - 2, z });
                    const Voxel* right_of_and_below_neighbour_voxel
                        = &neighbour_voxels[right_of_and_below_neighbour_voxel_index];

                    VoxelIndex right_of_this_voxel_index
                        = hvox::voxel_index({ x + 1, 0, z });
                    const Voxel* right_of_this_voxel
                        = &voxels[right_of_this_voxel_index];

                    // Across
                    if (is_solid(right_of_neighbour_voxel)
                        && !is_solid(right_of_this_voxel))
                    {
                        ChunkNavmeshNode right_of_neighbour_voxel_coord = {
                            {x + 1, CHUNK_LENGTH - 1, z},
                            neighbour->position
                        };

                        // Ensure node exists for this voxel.
                        ChunkNavmeshVertexDescriptor right_of_neighbour_voxel_vertex
                            = impl::get_vertex(
                                neighbour, right_of_neighbour_voxel_coord
                            );

                        {
                            std::unique_lock<std::shared_mutex> lock;
                            auto& navmesh = neighbour->navmesh.get(lock);

                            boost::add_edge(
                                neighbour_voxel_vertex.in_neighbour,
                                right_of_neighbour_voxel_vertex,
                                navmesh->graph
                            );
                            boost::add_edge(
                                right_of_neighbour_voxel_vertex,
                                neighbour_voxel_vertex.in_neighbour,
                                navmesh->graph
                            );
                        }
                        // Down
                    } else if (is_solid(right_of_and_below_neighbour_voxel) && !is_solid(right_of_neighbour_voxel) && !is_solid(right_of_this_voxel))
                    {
                        ChunkNavmeshNode right_of_and_below_neighbour_voxel_coord = {
                            {x + 1, CHUNK_LENGTH - 2, z},
                            neighbour->position
                        };

                        // Ensure node exists for this voxel.
                        ChunkNavmeshVertexDescriptor
                            right_of_and_below_neighbour_voxel_vertex
                            = impl::get_vertex(
                                neighbour, right_of_and_below_neighbour_voxel_coord
                            );

                        {
                            std::unique_lock<std::shared_mutex> lock;
                            auto& navmesh = neighbour->navmesh.get(lock);

                            boost::add_edge(
                                neighbour_voxel_vertex.in_neighbour,
                                right_of_and_below_neighbour_voxel_vertex,
                                navmesh->graph
                            );
                            boost::add_edge(
                                right_of_and_below_neighbour_voxel_vertex,
                                neighbour_voxel_vertex.in_neighbour,
                                navmesh->graph
                            );
                        }
                    }

                    // Front
                    VoxelIndex front_of_neighbour_voxel_index
                        = hvox::voxel_index({ x, CHUNK_LENGTH - 1, z + 1 });
                    const Voxel* front_of_neighbour_voxel
                        = &neighbour_voxels[front_of_neighbour_voxel_index];

                    VoxelIndex front_of_and_below_neighbour_voxel_index
                        = hvox::voxel_index({ x, CHUNK_LENGTH - 2, z + 1 });
                    const Voxel* front_of_and_below_neighbour_voxel
                        = &neighbour_voxels[front_of_and_below_neighbour_voxel_index];

                    VoxelIndex front_of_this_voxel_index
                        = hvox::voxel_index({ x, 0, z + 1 });
                    const Voxel* front_of_this_voxel
                        = &voxels[front_of_this_voxel_index];

                    // Across
                    if (is_solid(front_of_neighbour_voxel)
                        && !is_solid(front_of_this_voxel))
                    {
                        ChunkNavmeshNode front_of_neighbour_voxel_coord = {
                            {x, CHUNK_LENGTH - 1, z + 1},
                            neighbour->position
                        };

                        // Ensure node exists for this voxel.
                        ChunkNavmeshVertexDescriptor front_of_neighbour_voxel_vertex
                            = impl::get_vertex(
                                neighbour, front_of_neighbour_voxel_coord
                            );

                        {
                            std::unique_lock<std::shared_mutex> lock;
                            auto& navmesh = neighbour->navmesh.get(lock);

                            boost::add_edge(
                                neighbour_voxel_vertex.in_neighbour,
                                front_of_neighbour_voxel_vertex,
                                navmesh->graph
                            );
                            boost::add_edge(
                                front_of_neighbour_voxel_vertex,
                                neighbour_voxel_vertex.in_neighbour,
                                navmesh->graph
                            );
                        }
                        // Down
                    } else if (is_solid(front_of_and_below_neighbour_voxel) && !is_solid(front_of_neighbour_voxel) && !is_solid(front_of_this_voxel))
                    {
                        ChunkNavmeshNode front_of_and_below_neighbour_voxel_coord = {
                            {x, CHUNK_LENGTH - 2, z + 1},
                            neighbour->position
                        };

                        // Ensure node exists for this voxel.
                        ChunkNavmeshVertexDescriptor
                            front_of_and_below_neighbour_voxel_vertex
                            = impl::get_vertex(
                                neighbour, front_of_and_below_neighbour_voxel_coord
                            );

                        {
                            std::unique_lock<std::shared_mutex> lock;
                            auto& navmesh = neighbour->navmesh.get(lock);

                            boost::add_edge(
                                neighbour_voxel_vertex.in_neighbour,
                                front_of_and_below_neighbour_voxel_vertex,
                                navmesh->graph
                            );
                            boost::add_edge(
                                front_of_and_below_neighbour_voxel_vertex,
                                neighbour_voxel_vertex.in_neighbour,
                                navmesh->graph
                            );
                        }
                    }

                    // Back
                    VoxelIndex back_of_neighbour_voxel_index
                        = hvox::voxel_index({ x, CHUNK_LENGTH - 1, z - 1 });
                    const Voxel* back_of_neighbour_voxel
                        = &neighbour_voxels[back_of_neighbour_voxel_index];

                    VoxelIndex back_of_and_below_neighbour_voxel_index
                        = hvox::voxel_index({ x, CHUNK_LENGTH - 2, z - 1 });
                    const Voxel* back_of_and_below_neighbour_voxel
                        = &neighbour_voxels[back_of_and_below_neighbour_voxel_index];

                    VoxelIndex back_of_this_voxel_index
                        = hvox::voxel_index({ x, 0, z - 1 });
                    const Voxel* back_of_this_voxel = &voxels[back_of_this_voxel_index];

                    // Across
                    if (is_solid(back_of_neighbour_voxel)
                        && !is_solid(back_of_this_voxel))
                    {
                        ChunkNavmeshNode back_of_neighbour_voxel_coord = {
                            {x, CHUNK_LENGTH - 1, z - 1},
                            neighbour->position
                        };

                        // Ensure node exists for this voxel.
                        ChunkNavmeshVertexDescriptor back_of_neighbour_voxel_vertex
                            = impl::get_vertex(
                                neighbour, back_of_neighbour_voxel_coord
                            );

                        {
                            std::unique_lock<std::shared_mutex> lock;
                            auto& navmesh = neighbour->navmesh.get(lock);

                            boost::add_edge(
                                neighbour_voxel_vertex.in_neighbour,
                                back_of_neighbour_voxel_vertex,
                                navmesh->graph
                            );
                            boost::add_edge(
                                back_of_neighbour_voxel_vertex,
                                neighbour_voxel_vertex.in_neighbour,
                                navmesh->graph
                            );
                        }
                        // Down
                    } else if (is_solid(back_of_and_below_neighbour_voxel) && !is_solid(back_of_neighbour_voxel) && !is_solid(back_of_this_voxel))
                    {
                        ChunkNavmeshNode back_of_and_below_neighbour_voxel_coord = {
                            {x, CHUNK_LENGTH - 2, z - 1},
                            neighbour->position
                        };

                        // Ensure node exists for this voxel.
                        ChunkNavmeshVertexDescriptor
                            back_of_and_below_neighbour_voxel_vertex
                            = impl::get_vertex(
                                neighbour, back_of_and_below_neighbour_voxel_coord
                            );

                        {
                            std::unique_lock<std::shared_mutex> lock;
                            auto& navmesh = neighbour->navmesh.get(lock);

                            boost::add_edge(
                                neighbour_voxel_vertex.in_neighbour,
                                back_of_and_below_neighbour_voxel_vertex,
                                navmesh->graph
                            );
                            boost::add_edge(
                                back_of_and_below_neighbour_voxel_vertex,
                                neighbour_voxel_vertex.in_neighbour,
                                navmesh->graph
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
                    std::shared_lock<std::shared_mutex> left_neighbour_voxel_lock,
                        below_left_neighbour_voxel_lock;
                    auto& left_neighbour_voxels
                        = left_neighbour->voxels.get(left_neighbour_voxel_lock);
                    auto& below_left_neighbour_voxels
                        = below_left_neighbour->voxels.get(
                            below_left_neighbour_voxel_lock
                        );

                    // Step up from y == CHUNK_LENGTH - 2
                    for (VoxelChunkPositionCoord z = 0; z < CHUNK_LENGTH; ++z) {
                        VoxelIndex neighbour_voxel_index
                            = hvox::voxel_index({ 0, CHUNK_LENGTH - 2, z });
                        const Voxel* neighbour_voxel
                            = &neighbour_voxels[neighbour_voxel_index];

                        VoxelIndex above_neighbour_voxel_index
                            = hvox::voxel_index({ 0, CHUNK_LENGTH - 1, z });
                        const Voxel* above_neighbour_voxel
                            = &neighbour_voxels[above_neighbour_voxel_index];

                        VoxelIndex twice_above_neighbour_voxel_index
                            = hvox::voxel_index({ 0, 0, z });
                        const Voxel* twice_above_neighbour_voxel
                            = &voxels[twice_above_neighbour_voxel_index];

                        if (!is_solid(neighbour_voxel)
                            || is_solid(above_neighbour_voxel)
                            || is_solid(twice_above_neighbour_voxel))
                            continue;

                        ChunkNavmeshNode neighbour_voxel_coord = {
                            {0, CHUNK_LENGTH - 2, z},
                            neighbour->position
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor in_neighbour,
                                in_below_left_neighbour;
                        } neighbour_voxel_vertex
                            = { impl::get_vertex(neighbour, neighbour_voxel_coord),
                                impl::get_vertex(
                                    below_left_neighbour, neighbour_voxel_coord
                                ) };

                        VoxelIndex above_candidate_index
                            = hvox::voxel_index({ CHUNK_LENGTH - 1, 0, z });
                        const Voxel* above_candidate_voxel
                            = &left_neighbour_voxels[above_candidate_index];

                        VoxelIndex candidate_index = hvox::voxel_index(
                            { CHUNK_LENGTH - 1, CHUNK_LENGTH - 1, z }
                        );
                        const Voxel* candidate_voxel
                            = &below_left_neighbour_voxels[candidate_index];

                        if (!is_solid(candidate_voxel)
                            || is_solid(above_candidate_voxel))
                            continue;

                        ChunkNavmeshNode candidate_voxel_coord = {
                            {CHUNK_LENGTH - 1, CHUNK_LENGTH - 1, z},
                            below_left_neighbour->position
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor in_neighbour,
                                in_below_left_neighbour;
                        } candidate_voxel_vertex
                            = { impl::get_vertex(neighbour, candidate_voxel_coord),
                                impl::get_vertex(
                                    below_left_neighbour, candidate_voxel_coord
                                ) };

                        {
                            std::unique_lock<std::shared_mutex> lock;
                            auto& navmesh = neighbour->navmesh.get(lock);

                            boost::add_edge(
                                neighbour_voxel_vertex.in_neighbour,
                                candidate_voxel_vertex.in_neighbour,
                                navmesh->graph
                            );
                            boost::add_edge(
                                candidate_voxel_vertex.in_neighbour,
                                neighbour_voxel_vertex.in_neighbour,
                                navmesh->graph
                            );
                        }

                        {
                            std::unique_lock<std::shared_mutex> lock;
                            auto& navmesh = below_left_neighbour->navmesh.get(lock);

                            boost::add_edge(
                                neighbour_voxel_vertex.in_below_left_neighbour,
                                candidate_voxel_vertex.in_below_left_neighbour,
                                navmesh->graph
                            );
                            boost::add_edge(
                                candidate_voxel_vertex.in_below_left_neighbour,
                                neighbour_voxel_vertex.in_below_left_neighbour,
                                navmesh->graph
                            );
                        }
                    }

                    // Step across and down from y == CHUNK_LENGTH - 1
                    for (VoxelChunkPositionCoord z = 0; z < CHUNK_LENGTH; ++z) {
                        VoxelIndex neighbour_voxel_index
                            = hvox::voxel_index({ 0, CHUNK_LENGTH - 1, z });
                        const Voxel* neighbour_voxel
                            = &neighbour_voxels[neighbour_voxel_index];

                        VoxelIndex above_neighbour_voxel_index
                            = hvox::voxel_index({ 0, 0, z });
                        const Voxel* above_neighbour_voxel
                            = &voxels[above_neighbour_voxel_index];

                        // Necessary condition for step across and down.
                        if (!is_solid(neighbour_voxel)
                            || is_solid(above_neighbour_voxel))
                            continue;

                        ChunkNavmeshNode neighbour_voxel_coord = {
                            {0, CHUNK_LENGTH - 1, z},
                            neighbour->position
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor in_neighbour,
                                in_below_left_neighbour;
                        } neighbour_voxel_vertex
                            = { impl::get_vertex(neighbour, neighbour_voxel_coord),
                                impl::get_vertex(
                                    below_left_neighbour, neighbour_voxel_coord
                                ) };

                        VoxelIndex step_down_candidate_index = hvox::voxel_index(
                            { CHUNK_LENGTH - 1, CHUNK_LENGTH - 2, z }
                        );
                        const Voxel* step_down_candidate_voxel
                            = &below_left_neighbour_voxels[step_down_candidate_index];

                        VoxelIndex step_across_candidate_index = hvox::voxel_index(
                            { CHUNK_LENGTH - 1, CHUNK_LENGTH - 1, z }
                        );
                        const Voxel* step_across_candidate_voxel
                            = &below_left_neighbour_voxels[step_across_candidate_index];

                        VoxelIndex above_candidates_index
                            = hvox::voxel_index({ CHUNK_LENGTH - 1, 0, z });
                        const Voxel* above_candidates_voxel
                            = &left_neighbour_voxels[above_candidates_index];

                        // Step across
                        if (is_solid(step_across_candidate_voxel)
                            && !is_solid(above_candidates_voxel))
                        {
                            ChunkNavmeshNode candidate_voxel_coord = {
                                {CHUNK_LENGTH - 1, CHUNK_LENGTH - 1, z},
                                below_left_neighbour->position
                            };

                            struct {
                                ChunkNavmeshVertexDescriptor in_neighbour,
                                    in_below_left_neighbour;
                            } candidate_voxel_vertex
                                = { impl::get_vertex(neighbour, candidate_voxel_coord),
                                    impl::get_vertex(
                                        below_left_neighbour, candidate_voxel_coord
                                    ) };

                            {
                                std::unique_lock<std::shared_mutex> lock;
                                auto& navmesh = neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    neighbour_voxel_vertex.in_neighbour,
                                    candidate_voxel_vertex.in_neighbour,
                                    navmesh->graph
                                );
                                boost::add_edge(
                                    candidate_voxel_vertex.in_neighbour,
                                    neighbour_voxel_vertex.in_neighbour,
                                    navmesh->graph
                                );
                            }

                            {
                                std::unique_lock<std::shared_mutex> lock;
                                auto& navmesh = below_left_neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    neighbour_voxel_vertex.in_below_left_neighbour,
                                    candidate_voxel_vertex.in_below_left_neighbour,
                                    navmesh->graph
                                );
                                boost::add_edge(
                                    candidate_voxel_vertex.in_below_left_neighbour,
                                    neighbour_voxel_vertex.in_below_left_neighbour,
                                    navmesh->graph
                                );
                            }
                        }
                        // Step down
                        else if (is_solid(step_down_candidate_voxel) && !is_solid(step_across_candidate_voxel) && !is_solid(above_candidates_voxel))
                        {
                            ChunkNavmeshNode candidate_voxel_coord = {
                                {CHUNK_LENGTH - 1, CHUNK_LENGTH - 2, z},
                                below_left_neighbour->position
                            };

                            struct {
                                ChunkNavmeshVertexDescriptor in_neighbour,
                                    in_below_left_neighbour;
                            } candidate_voxel_vertex
                                = { impl::get_vertex(neighbour, candidate_voxel_coord),
                                    impl::get_vertex(
                                        below_left_neighbour, candidate_voxel_coord
                                    ) };

                            {
                                std::unique_lock<std::shared_mutex> lock;
                                auto& navmesh = chunk->navmesh.get(lock);

                                boost::add_edge(
                                    neighbour_voxel_vertex.in_neighbour,
                                    candidate_voxel_vertex.in_neighbour,
                                    navmesh->graph
                                );
                                boost::add_edge(
                                    candidate_voxel_vertex.in_neighbour,
                                    neighbour_voxel_vertex.in_neighbour,
                                    navmesh->graph
                                );
                            }

                            {
                                std::unique_lock<std::shared_mutex> lock;
                                auto& navmesh = left_neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    neighbour_voxel_vertex.in_below_left_neighbour,
                                    candidate_voxel_vertex.in_below_left_neighbour,
                                    navmesh->graph
                                );
                                boost::add_edge(
                                    candidate_voxel_vertex.in_below_left_neighbour,
                                    neighbour_voxel_vertex.in_below_left_neighbour,
                                    navmesh->graph
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
                    std::shared_lock<std::shared_mutex> right_neighbour_voxel_lock;
                    auto                                right_neighbour_voxels
                        = right_neighbour->voxels.get(right_neighbour_voxel_lock);
                    std::shared_lock<std::shared_mutex>
                         below_right_neighbour_voxel_lock;
                    auto below_right_neighbour_voxels
                        = below_right_neighbour->voxels.get(
                            below_right_neighbour_voxel_lock
                        );

                    // Step up from y == CHUNK_LENGTH - 2
                    for (VoxelChunkPositionCoord z = 0; z < CHUNK_LENGTH; ++z) {
                        VoxelIndex neighbour_voxel_index = hvox::voxel_index(
                            { CHUNK_LENGTH - 1, CHUNK_LENGTH - 2, z }
                        );
                        const Voxel* neighbour_voxel
                            = &neighbour_voxels[neighbour_voxel_index];

                        VoxelIndex above_neighbour_voxel_index = hvox::voxel_index(
                            { CHUNK_LENGTH - 1, CHUNK_LENGTH - 1, z }
                        );
                        const Voxel* above_neighbour_voxel
                            = &neighbour_voxels[above_neighbour_voxel_index];

                        VoxelIndex twice_above_neighbour_voxel_index
                            = hvox::voxel_index({ CHUNK_LENGTH - 1, 0, z });
                        const Voxel* twice_above_neighbour_voxel
                            = &voxels[twice_above_neighbour_voxel_index];

                        if (!is_solid(neighbour_voxel)
                            || is_solid(above_neighbour_voxel)
                            || is_solid(twice_above_neighbour_voxel))
                            continue;

                        ChunkNavmeshNode neighbour_voxel_coord = {
                            {CHUNK_LENGTH - 1, CHUNK_LENGTH - 2, z},
                            neighbour->position
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor in_neighbour,
                                in_below_right_neighbour;
                        } neighbour_voxel_vertex = {
                            impl::get_vertex(neighbour, neighbour_voxel_coord),
                            impl::get_vertex(right_neighbour, neighbour_voxel_coord)
                        };

                        VoxelIndex above_candidate_index
                            = hvox::voxel_index({ 0, 0, z });
                        const Voxel* above_candidate_voxel
                            = &right_neighbour_voxels[above_candidate_index];

                        VoxelIndex candidate_index
                            = hvox::voxel_index({ 0, CHUNK_LENGTH - 1, z });
                        const Voxel* candidate_voxel
                            = &below_right_neighbour_voxels[candidate_index];

                        if (!is_solid(candidate_voxel)
                            || is_solid(above_candidate_voxel))
                            continue;

                        ChunkNavmeshNode candidate_voxel_coord = {
                            {0, CHUNK_LENGTH - 1, z},
                            below_right_neighbour->position
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor in_neighbour,
                                in_below_right_neighbour;
                        } candidate_voxel_vertex
                            = { impl::get_vertex(neighbour, candidate_voxel_coord),
                                impl::get_vertex(
                                    below_right_neighbour, candidate_voxel_coord
                                ) };

                        {
                            std::unique_lock<std::shared_mutex> lock;
                            auto& navmesh = neighbour->navmesh.get(lock);

                            boost::add_edge(
                                neighbour_voxel_vertex.in_neighbour,
                                candidate_voxel_vertex.in_neighbour,
                                navmesh->graph
                            );
                            boost::add_edge(
                                candidate_voxel_vertex.in_neighbour,
                                neighbour_voxel_vertex.in_neighbour,
                                navmesh->graph
                            );
                        }

                        {
                            std::unique_lock<std::shared_mutex> lock;
                            auto& navmesh = below_right_neighbour->navmesh.get(lock);

                            boost::add_edge(
                                neighbour_voxel_vertex.in_below_right_neighbour,
                                candidate_voxel_vertex.in_below_right_neighbour,
                                navmesh->graph
                            );
                            boost::add_edge(
                                candidate_voxel_vertex.in_below_right_neighbour,
                                neighbour_voxel_vertex.in_below_right_neighbour,
                                navmesh->graph
                            );
                        }
                    }

                    // Step across and down from y == CHUNK_LENGTH - 1
                    for (VoxelChunkPositionCoord z = 0; z < CHUNK_LENGTH; ++z) {
                        VoxelIndex neighbour_voxel_index = hvox::voxel_index(
                            { CHUNK_LENGTH - 1, CHUNK_LENGTH - 1, z }
                        );
                        const Voxel* neighbour_voxel
                            = &neighbour_voxels[neighbour_voxel_index];

                        VoxelIndex above_neighbour_voxel_index
                            = hvox::voxel_index({ CHUNK_LENGTH - 1, 0, z });
                        const Voxel* above_neighbour_voxel
                            = &voxels[above_neighbour_voxel_index];

                        // Necessary condition for step across and down.
                        if (!is_solid(neighbour_voxel)
                            || is_solid(above_neighbour_voxel))
                            continue;

                        ChunkNavmeshNode neighbour_voxel_coord = {
                            {CHUNK_LENGTH - 1, CHUNK_LENGTH - 1, z},
                            neighbour->position
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor in_neighbour,
                                in_below_right_neighbour;
                        } neighbour_voxel_vertex
                            = { impl::get_vertex(neighbour, neighbour_voxel_coord),
                                impl::get_vertex(
                                    below_right_neighbour, neighbour_voxel_coord
                                ) };

                        VoxelIndex step_down_candidate_index
                            = hvox::voxel_index({ 0, CHUNK_LENGTH - 2, z });
                        const Voxel* step_down_candidate_voxel
                            = &below_right_neighbour_voxels[step_down_candidate_index];

                        VoxelIndex step_across_candidate_index
                            = hvox::voxel_index({ 0, CHUNK_LENGTH - 1, z });
                        const Voxel* step_across_candidate_voxel
                            = &below_right_neighbour_voxels
                                  [step_across_candidate_index];

                        VoxelIndex above_candidates_index
                            = hvox::voxel_index({ 0, 0, z });
                        const Voxel* above_candidates_voxel
                            = &right_neighbour_voxels[above_candidates_index];

                        // Step across
                        if (is_solid(step_across_candidate_voxel)
                            && !is_solid(above_candidates_voxel))
                        {
                            ChunkNavmeshNode candidate_voxel_coord = {
                                {0, CHUNK_LENGTH - 1, z},
                                below_right_neighbour->position
                            };

                            struct {
                                ChunkNavmeshVertexDescriptor in_neighbour,
                                    in_below_right_neighbour;
                            } candidate_voxel_vertex
                                = { impl::get_vertex(neighbour, candidate_voxel_coord),
                                    impl::get_vertex(
                                        below_right_neighbour, candidate_voxel_coord
                                    ) };

                            {
                                std::unique_lock<std::shared_mutex> lock;
                                auto& navmesh = neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    neighbour_voxel_vertex.in_neighbour,
                                    candidate_voxel_vertex.in_neighbour,
                                    navmesh->graph
                                );
                                boost::add_edge(
                                    candidate_voxel_vertex.in_neighbour,
                                    neighbour_voxel_vertex.in_neighbour,
                                    navmesh->graph
                                );
                            }

                            {
                                std::unique_lock<std::shared_mutex> lock;
                                auto&                               navmesh
                                    = below_right_neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    neighbour_voxel_vertex.in_below_right_neighbour,
                                    candidate_voxel_vertex.in_below_right_neighbour,
                                    navmesh->graph
                                );
                                boost::add_edge(
                                    candidate_voxel_vertex.in_below_right_neighbour,
                                    neighbour_voxel_vertex.in_below_right_neighbour,
                                    navmesh->graph
                                );
                            }
                        }
                        // Step down
                        else if (is_solid(step_down_candidate_voxel) && !is_solid(step_across_candidate_voxel) && !is_solid(above_candidates_voxel))
                        {
                            ChunkNavmeshNode candidate_voxel_coord = {
                                {0, CHUNK_LENGTH - 2, z},
                                below_right_neighbour->position
                            };

                            struct {
                                ChunkNavmeshVertexDescriptor in_neighbour,
                                    in_below_right_neighbour;
                            } candidate_voxel_vertex
                                = { impl::get_vertex(neighbour, candidate_voxel_coord),
                                    impl::get_vertex(
                                        below_right_neighbour, candidate_voxel_coord
                                    ) };

                            {
                                std::unique_lock<std::shared_mutex> lock;
                                auto& navmesh = neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    neighbour_voxel_vertex.in_neighbour,
                                    candidate_voxel_vertex.in_neighbour,
                                    navmesh->graph
                                );
                                boost::add_edge(
                                    candidate_voxel_vertex.in_neighbour,
                                    neighbour_voxel_vertex.in_neighbour,
                                    navmesh->graph
                                );
                            }

                            {
                                std::unique_lock<std::shared_mutex> lock;
                                auto&                               navmesh
                                    = below_right_neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    neighbour_voxel_vertex.in_below_right_neighbour,
                                    candidate_voxel_vertex.in_below_right_neighbour,
                                    navmesh->graph
                                );
                                boost::add_edge(
                                    candidate_voxel_vertex.in_below_right_neighbour,
                                    neighbour_voxel_vertex.in_below_right_neighbour,
                                    navmesh->graph
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
                    std::shared_lock<std::shared_mutex> front_neighbour_voxel_lock;
                    auto                                front_neighbour_voxels
                        = front_neighbour->voxels.get(front_neighbour_voxel_lock);
                    std::shared_lock<std::shared_mutex>
                         below_front_neighbour_voxel_lock;
                    auto below_front_neighbour_voxels
                        = below_front_neighbour->voxels.get(
                            below_front_neighbour_voxel_lock
                        );

                    // Step up from y == CHUNK_LENGTH - 2
                    for (VoxelChunkPositionCoord x = 0; x < CHUNK_LENGTH; ++x) {
                        VoxelIndex neighbour_voxel_index = hvox::voxel_index(
                            { x, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1 }
                        );
                        const Voxel* neighbour_voxel
                            = &neighbour_voxels[neighbour_voxel_index];

                        VoxelIndex above_neighbour_voxel_index = hvox::voxel_index(
                            { x, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1 }
                        );
                        const Voxel* above_neighbour_voxel
                            = &neighbour_voxels[above_neighbour_voxel_index];

                        VoxelIndex twice_above_neighbour_voxel_index
                            = hvox::voxel_index({ x, 0, CHUNK_LENGTH - 1 });
                        const Voxel* twice_above_neighbour_voxel
                            = &voxels[twice_above_neighbour_voxel_index];

                        if (!is_solid(neighbour_voxel)
                            || is_solid(above_neighbour_voxel)
                            || is_solid(twice_above_neighbour_voxel))
                            continue;

                        ChunkNavmeshNode neighbour_voxel_coord = {
                            {x, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1},
                            neighbour->position
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor in_neighbour,
                                in_below_front_neighbour;
                        } neighbour_voxel_vertex
                            = { impl::get_vertex(neighbour, neighbour_voxel_coord),
                                impl::get_vertex(
                                    below_front_neighbour, neighbour_voxel_coord
                                ) };

                        VoxelIndex above_candidate_index
                            = hvox::voxel_index({ x, 0, 0 });
                        const Voxel* above_candidate_voxel
                            = &front_neighbour_voxels[above_candidate_index];

                        VoxelIndex candidate_index
                            = hvox::voxel_index({ x, CHUNK_LENGTH - 1, 0 });
                        const Voxel* candidate_voxel
                            = &below_front_neighbour_voxels[candidate_index];

                        if (!is_solid(candidate_voxel)
                            || is_solid(above_candidate_voxel))
                            continue;

                        ChunkNavmeshNode candidate_voxel_coord = {
                            {x, CHUNK_LENGTH - 1, 0},
                            below_front_neighbour->position
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor in_neighbour,
                                in_below_front_neighbour;
                        } candidate_voxel_vertex
                            = { impl::get_vertex(neighbour, candidate_voxel_coord),
                                impl::get_vertex(
                                    below_front_neighbour, candidate_voxel_coord
                                ) };

                        {
                            std::unique_lock<std::shared_mutex> lock;
                            auto& navmesh = neighbour->navmesh.get(lock);

                            boost::add_edge(
                                neighbour_voxel_vertex.in_neighbour,
                                candidate_voxel_vertex.in_neighbour,
                                navmesh->graph
                            );
                            boost::add_edge(
                                candidate_voxel_vertex.in_neighbour,
                                neighbour_voxel_vertex.in_neighbour,
                                navmesh->graph
                            );
                        }

                        {
                            std::unique_lock<std::shared_mutex> lock;
                            auto& navmesh = below_front_neighbour->navmesh.get(lock);

                            boost::add_edge(
                                neighbour_voxel_vertex.in_below_front_neighbour,
                                candidate_voxel_vertex.in_below_front_neighbour,
                                navmesh->graph
                            );
                            boost::add_edge(
                                candidate_voxel_vertex.in_below_front_neighbour,
                                neighbour_voxel_vertex.in_below_front_neighbour,
                                navmesh->graph
                            );
                        }
                    }

                    // Step across and down from y == CHUNK_LENGTH - 1
                    for (VoxelChunkPositionCoord x = 0; x < CHUNK_LENGTH; ++x) {
                        VoxelIndex neighbour_voxel_index = hvox::voxel_index(
                            { x, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1 }
                        );
                        const Voxel* neighbour_voxel
                            = &neighbour_voxels[neighbour_voxel_index];

                        VoxelIndex above_neighbour_voxel_index
                            = hvox::voxel_index({ x, 0, CHUNK_LENGTH - 1 });
                        const Voxel* above_neighbour_voxel
                            = &voxels[above_neighbour_voxel_index];

                        // Necessary condition for step across and down.
                        if (!is_solid(neighbour_voxel)
                            || is_solid(above_neighbour_voxel))
                            continue;

                        ChunkNavmeshNode neighbour_voxel_coord = {
                            {x, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1},
                            neighbour->position
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor in_neighbour,
                                in_below_front_neighbour;
                        } neighbour_voxel_vertex
                            = { impl::get_vertex(neighbour, neighbour_voxel_coord),
                                impl::get_vertex(
                                    below_front_neighbour, neighbour_voxel_coord
                                ) };

                        VoxelIndex step_down_candidate_index
                            = hvox::voxel_index({ x, CHUNK_LENGTH - 2, 0 });
                        const Voxel* step_down_candidate_voxel
                            = &below_front_neighbour_voxels[step_down_candidate_index];

                        VoxelIndex step_across_candidate_index
                            = hvox::voxel_index({ x, CHUNK_LENGTH - 1, 0 });
                        const Voxel* step_across_candidate_voxel
                            = &below_front_neighbour_voxels
                                  [step_across_candidate_index];

                        VoxelIndex above_candidates_index
                            = hvox::voxel_index({ x, 0, 0 });
                        const Voxel* above_candidates_voxel
                            = &front_neighbour_voxels[above_candidates_index];

                        // Step across
                        if (is_solid(step_across_candidate_voxel)
                            && !is_solid(above_candidates_voxel))
                        {
                            ChunkNavmeshNode candidate_voxel_coord = {
                                {x, CHUNK_LENGTH - 1, 0},
                                below_front_neighbour->position
                            };

                            struct {
                                ChunkNavmeshVertexDescriptor in_neighbour,
                                    in_below_front_neighbour;
                            } candidate_voxel_vertex
                                = { impl::get_vertex(neighbour, candidate_voxel_coord),
                                    impl::get_vertex(
                                        below_front_neighbour, candidate_voxel_coord
                                    ) };

                            {
                                std::unique_lock<std::shared_mutex> lock;
                                auto& navmesh = neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    neighbour_voxel_vertex.in_neighbour,
                                    candidate_voxel_vertex.in_neighbour,
                                    navmesh->graph
                                );
                                boost::add_edge(
                                    candidate_voxel_vertex.in_neighbour,
                                    neighbour_voxel_vertex.in_neighbour,
                                    navmesh->graph
                                );
                            }

                            {
                                std::unique_lock<std::shared_mutex> lock;
                                auto&                               navmesh
                                    = below_front_neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    neighbour_voxel_vertex.in_below_front_neighbour,
                                    candidate_voxel_vertex.in_below_front_neighbour,
                                    navmesh->graph
                                );
                                boost::add_edge(
                                    candidate_voxel_vertex.in_below_front_neighbour,
                                    neighbour_voxel_vertex.in_below_front_neighbour,
                                    navmesh->graph
                                );
                            }
                        }
                        // Step down
                        else if (is_solid(step_down_candidate_voxel) && !is_solid(step_across_candidate_voxel) && !is_solid(above_candidates_voxel))
                        {
                            ChunkNavmeshNode candidate_voxel_coord = {
                                {x, CHUNK_LENGTH - 2, 0},
                                below_front_neighbour->position
                            };

                            struct {
                                ChunkNavmeshVertexDescriptor in_neighbour,
                                    in_below_front_neighbour;
                            } candidate_voxel_vertex
                                = { impl::get_vertex(neighbour, candidate_voxel_coord),
                                    impl::get_vertex(
                                        below_front_neighbour, candidate_voxel_coord
                                    ) };

                            {
                                std::unique_lock<std::shared_mutex> lock;
                                auto& navmesh = neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    neighbour_voxel_vertex.in_neighbour,
                                    candidate_voxel_vertex.in_neighbour,
                                    navmesh->graph
                                );
                                boost::add_edge(
                                    candidate_voxel_vertex.in_neighbour,
                                    neighbour_voxel_vertex.in_neighbour,
                                    navmesh->graph
                                );
                            }

                            {
                                std::unique_lock<std::shared_mutex> lock;
                                auto&                               navmesh
                                    = below_front_neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    neighbour_voxel_vertex.in_below_front_neighbour,
                                    candidate_voxel_vertex.in_below_front_neighbour,
                                    navmesh->graph
                                );
                                boost::add_edge(
                                    candidate_voxel_vertex.in_below_front_neighbour,
                                    neighbour_voxel_vertex.in_below_front_neighbour,
                                    navmesh->graph
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
                    std::shared_lock<std::shared_mutex> back_neighbour_voxel_lock,
                        below_back_neighbour_voxel_lock;
                    auto& below_back_neighbour_voxels
                        = below_back_neighbour->voxels.get(
                            below_back_neighbour_voxel_lock
                        );
                    auto& back_neighbour_voxels
                        = back_neighbour->voxels.get(back_neighbour_voxel_lock);

                    // Step up from y == CHUNK_LENGTH - 2
                    for (VoxelChunkPositionCoord x = 0; x < CHUNK_LENGTH; ++x) {
                        VoxelIndex neighbour_voxel_index
                            = hvox::voxel_index({ x, CHUNK_LENGTH - 2, 0 });
                        const Voxel* neighbour_voxel
                            = &neighbour_voxels[neighbour_voxel_index];

                        VoxelIndex above_neighbour_voxel_index
                            = hvox::voxel_index({ x, CHUNK_LENGTH - 1, 0 });
                        const Voxel* above_neighbour_voxel
                            = &neighbour_voxels[above_neighbour_voxel_index];

                        VoxelIndex twice_above_neighbour_voxel_index
                            = hvox::voxel_index({ CHUNK_LENGTH - 1, 0, 0 });
                        const Voxel* twice_above_neighbour_voxel
                            = &voxels[twice_above_neighbour_voxel_index];

                        if (!is_solid(neighbour_voxel)
                            || is_solid(above_neighbour_voxel)
                            || is_solid(twice_above_neighbour_voxel))
                            continue;

                        ChunkNavmeshNode neighbour_voxel_coord = {
                            {x, CHUNK_LENGTH - 2, 0},
                            neighbour->position
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor in_neighbour,
                                in_below_back_neighbour;
                        } neighbour_voxel_vertex
                            = { impl::get_vertex(neighbour, neighbour_voxel_coord),
                                impl::get_vertex(
                                    below_back_neighbour, neighbour_voxel_coord
                                ) };

                        VoxelIndex above_candidate_index
                            = hvox::voxel_index({ x, 0, CHUNK_LENGTH - 1 });
                        const Voxel* above_candidate_voxel
                            = &back_neighbour_voxels[above_candidate_index];

                        VoxelIndex candidate_index = hvox::voxel_index(
                            { x, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1 }
                        );
                        const Voxel* candidate_voxel
                            = &below_back_neighbour_voxels[candidate_index];

                        if (!is_solid(candidate_voxel)
                            || is_solid(above_candidate_voxel))
                            continue;

                        ChunkNavmeshNode candidate_voxel_coord = {
                            {x, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1},
                            below_back_neighbour->position
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor in_neighbour,
                                in_below_back_neighbour;
                        } candidate_voxel_vertex
                            = { impl::get_vertex(neighbour, candidate_voxel_coord),
                                impl::get_vertex(
                                    below_back_neighbour, candidate_voxel_coord
                                ) };

                        {
                            std::unique_lock<std::shared_mutex> lock;
                            auto& navmesh = neighbour->navmesh.get(lock);

                            boost::add_edge(
                                neighbour_voxel_vertex.in_neighbour,
                                candidate_voxel_vertex.in_neighbour,
                                navmesh->graph
                            );
                            boost::add_edge(
                                candidate_voxel_vertex.in_neighbour,
                                neighbour_voxel_vertex.in_neighbour,
                                navmesh->graph
                            );
                        }

                        {
                            std::unique_lock<std::shared_mutex> lock;
                            auto& navmesh = below_back_neighbour->navmesh.get(lock);

                            boost::add_edge(
                                neighbour_voxel_vertex.in_below_back_neighbour,
                                candidate_voxel_vertex.in_below_back_neighbour,
                                navmesh->graph
                            );
                            boost::add_edge(
                                candidate_voxel_vertex.in_below_back_neighbour,
                                neighbour_voxel_vertex.in_below_back_neighbour,
                                navmesh->graph
                            );
                        }
                    }

                    // Step across and down from y == CHUNK_LENGTH - 1
                    for (VoxelChunkPositionCoord x = 0; x < CHUNK_LENGTH; ++x) {
                        VoxelIndex neighbour_voxel_index
                            = hvox::voxel_index({ x, CHUNK_LENGTH - 1, 0 });
                        const Voxel* neighbour_voxel
                            = &neighbour_voxels[neighbour_voxel_index];

                        VoxelIndex above_neighbour_voxel_index
                            = hvox::voxel_index({ x, 0, 0 });
                        const Voxel* above_neighbour_voxel
                            = &voxels[above_neighbour_voxel_index];

                        // Necessary condition for step across and down.
                        if (!is_solid(neighbour_voxel)
                            || is_solid(above_neighbour_voxel))
                            continue;

                        ChunkNavmeshNode neighbour_voxel_coord = {
                            {x, CHUNK_LENGTH - 1, 0},
                            neighbour->position
                        };

                        struct {
                            ChunkNavmeshVertexDescriptor in_neighbour,
                                in_below_back_neighbour;
                        } neighbour_voxel_vertex
                            = { impl::get_vertex(neighbour, neighbour_voxel_coord),
                                impl::get_vertex(
                                    below_back_neighbour, neighbour_voxel_coord
                                ) };

                        VoxelIndex step_down_candidate_index = hvox::voxel_index(
                            { x, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1 }
                        );
                        const Voxel* step_down_candidate_voxel
                            = &below_back_neighbour_voxels[step_down_candidate_index];

                        VoxelIndex step_across_candidate_index = hvox::voxel_index(
                            { x, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1 }
                        );
                        const Voxel* step_across_candidate_voxel
                            = &below_back_neighbour_voxels[step_across_candidate_index];

                        VoxelIndex above_candidates_index
                            = hvox::voxel_index({ x, 0, CHUNK_LENGTH - 1 });
                        const Voxel* above_candidates_voxel
                            = &back_neighbour_voxels[above_candidates_index];

                        // Step across
                        if (is_solid(step_across_candidate_voxel)
                            && !is_solid(above_candidates_voxel))
                        {
                            ChunkNavmeshNode candidate_voxel_coord = {
                                {x, CHUNK_LENGTH - 1, CHUNK_LENGTH - 1},
                                below_back_neighbour->position
                            };

                            struct {
                                ChunkNavmeshVertexDescriptor in_neighbour,
                                    in_below_back_neighbour;
                            } candidate_voxel_vertex
                                = { impl::get_vertex(neighbour, candidate_voxel_coord),
                                    impl::get_vertex(
                                        below_back_neighbour, candidate_voxel_coord
                                    ) };

                            {
                                std::unique_lock<std::shared_mutex> lock;
                                auto& navmesh = neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    neighbour_voxel_vertex.in_neighbour,
                                    candidate_voxel_vertex.in_neighbour,
                                    navmesh->graph
                                );
                                boost::add_edge(
                                    candidate_voxel_vertex.in_neighbour,
                                    neighbour_voxel_vertex.in_neighbour,
                                    navmesh->graph
                                );
                            }

                            {
                                std::unique_lock<std::shared_mutex> lock;
                                auto& navmesh = below_back_neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    neighbour_voxel_vertex.in_below_back_neighbour,
                                    candidate_voxel_vertex.in_below_back_neighbour,
                                    navmesh->graph
                                );
                                boost::add_edge(
                                    candidate_voxel_vertex.in_below_back_neighbour,
                                    neighbour_voxel_vertex.in_below_back_neighbour,
                                    navmesh->graph
                                );
                            }
                        }
                        // Step down
                        else if (is_solid(step_down_candidate_voxel) && !is_solid(step_across_candidate_voxel) && !is_solid(above_candidates_voxel))
                        {
                            ChunkNavmeshNode candidate_voxel_coord = {
                                {x, CHUNK_LENGTH - 2, CHUNK_LENGTH - 1},
                                below_back_neighbour->position
                            };

                            struct {
                                ChunkNavmeshVertexDescriptor in_neighbour,
                                    in_below_back_neighbour;
                            } candidate_voxel_vertex
                                = { impl::get_vertex(neighbour, candidate_voxel_coord),
                                    impl::get_vertex(
                                        below_back_neighbour, candidate_voxel_coord
                                    ) };

                            {
                                std::unique_lock<std::shared_mutex> lock;
                                auto& navmesh = neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    neighbour_voxel_vertex.in_neighbour,
                                    candidate_voxel_vertex.in_neighbour,
                                    navmesh->graph
                                );
                                boost::add_edge(
                                    candidate_voxel_vertex.in_neighbour,
                                    neighbour_voxel_vertex.in_neighbour,
                                    navmesh->graph
                                );
                            }

                            {
                                std::unique_lock<std::shared_mutex> lock;
                                auto& navmesh = below_back_neighbour->navmesh.get(lock);

                                boost::add_edge(
                                    neighbour_voxel_vertex.in_below_back_neighbour,
                                    candidate_voxel_vertex.in_below_back_neighbour,
                                    navmesh->graph
                                );
                                boost::add_edge(
                                    candidate_voxel_vertex.in_below_back_neighbour,
                                    neighbour_voxel_vertex.in_below_back_neighbour,
                                    navmesh->graph
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
