#ifndef __hemlock_voxel_chunk_builder_h
#define __hemlock_voxel_chunk_builder_h

#include "voxel/block_manager.h"
#include "voxel/coordinate_system.h"

namespace hemlock {
    namespace voxel {
        // TODO(Matthew): we want some guarantees on some kind of mesh component too
        //                all others being optional.
        class ChunkBuilder {
            using ComponentBuilder
                = hemlock::Delegate<void(entt::registry&, entt::entity)>;
            using ComponentBuilders = std::vector<ComponentBuilder>;
        public:
            void init(hmem::Handle<ChunkBlockPager> block_pager);

            template <typename Component, typename... Args>
            void register_component(Args... args) {
                m_builders.emplace_back(ComponentBuilder{
                    [args...](entt::registry& registry, entt::entity entity) {
                        registry.emplace<Component>(entity, args...);
                    } });
            }

            // void register_setup_events();

            entt::entity
            build(entt::registry& registry, ChunkGridPosition chunk_position);
        protected:
            hmem::Handle<ChunkBlockPager> m_block_pager;

            ComponentBuilders m_builders;
        };
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#endif  // __hemlock_voxel_chunk_builder_h
