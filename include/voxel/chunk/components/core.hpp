#ifndef __hemlock_voxel_chunk_components_core_hpp
#define __hemlock_voxel_chunk_components_core_hpp

#include "voxel/block.hpp"
#include "voxel/chunk/constants.hpp"
#include "voxel/chunk/events/block_change.hpp"
#include "voxel/chunk/events/bulk_block_change.hpp"
#include "voxel/chunk/state.hpp"
#include "voxel/coordinate_system.h"

namespace hemlock {
    namespace voxel {
        using ChunkBlockPager = hmem::Pager<Block, CHUNK_VOLUME, 3>;

        // TODO(Matthew): pointer stability probably can't be relied on, should this
        //                just be the entities directly?
        /**
         * @brief Holds references to the neighbours of this chunk.
         */
        union Neighbours {
            Neighbours() : all{} { /* Empty. */
            }

            Neighbours(const Neighbours& rhs) {
                for (size_t i = 0; i < 8; ++i) all[i] = rhs.all[i];
            }

            ~Neighbours() { /* Empty. */
            }

            Neighbours& operator=(const Neighbours& rhs) {
                for (size_t i = 0; i < 8; ++i) all[i] = rhs.all[i];
                return *this;
            }

            struct {
                entt::entity left, right, top, bottom, front, back;
            } one;

            entt::entity all[8];
        };

        /**
         * A chunk MUST be constructed with this component as a basic requirement. Other
         * components are all optionals.
         */
        struct ChunkCoreComponent {
            ChunkCoreComponent(hmem::Handle<ChunkBlockPager> block_pager) :
                position({}), neighbours({}) {
                blocks        = block_pager->get_page();
                m_block_pager = block_pager;

                // TODO(Matthew): how are we going to do this? sender as pointer to
                //                entity stored in grid? what about how to know which
                //                registry that entity is stored in?
                //                  note that we probably don't want to guarantee
                //                  pointer stability of 'this' as per EnTT docs, and
                //                  anyway using 'this' would mean not giving listener
                //                  access to any other components of the chunk.
                on_block_change.set_sender(Sender());
                on_bulk_block_change.set_sender(Sender());
                on_load.set_sender(Sender());
                on_unload.set_sender(Sender());
            }

            ~ChunkCoreComponent() {
                if (blocks) m_block_pager->free_page(blocks);
                blocks = nullptr;

                neighbours = {};
            }

            /**
             * @brief Returns the ID of this chunk.
             *
             * @return ChunkID the ID of this chunk.
             */
            ChunkID id() const { return position.id; }

            ChunkGridPosition position;
            Neighbours        neighbours;

            std::shared_mutex blocks_mutex;
            Block*            blocks;

            std::atomic<ChunkState> generation;

            // NOTE(Matthew): Chunk events, at least on_mesh_change, can be
            //                called from multiple threads. Events are NOT
            //                thread-safe. Our one guarantee is not really
            //                a full guarantee but should hold true: only
            //                one thread that could make a state change
            //                should be processing a task regarding this
            //                chunk at any point in time. If this fails
            //                to hold up, then we could easily get race
            //                conditions inside the events.

            CancellableEvent<BlockChangeEvent>     on_block_change;
            CancellableEvent<BulkBlockChangeEvent> on_bulk_block_change;

            Event<> on_load;
            Event<> on_unload;
        protected:
            hmem::Handle<ChunkBlockPager> m_block_pager;
        };
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

namespace std {
    template <>
    struct hash<hvox::ChunkCoreComponent> {
        std::size_t operator()(const hvox::ChunkCoreComponent& chunk) const {
            std::hash<hvox::ColumnID> _hash;
            return _hash(chunk.id());
        }
    };
}  // namespace std

#endif  // __hemlock_voxel_chunk_components_core_hpp
