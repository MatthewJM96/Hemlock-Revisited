#ifndef __hemlock_physics_common_components_hpp
#define __hemlock_physics_common_components_hpp

namespace hemlock {
    namespace physics {
        class ChunkGridCollider {
        public:
            ChunkGridCollider();
            ~ChunkGridCollider() { /* Empty. */ }

            void init(hmem::Handle<ChunkGrid> chunk_grid);
            void dispose();

            
        protected:
            hmem::Handle<ChunkGrid> m_chunk_grid;
        };
    }
}
namespace hphys = hemlock::physics;

#endif // __hemlock_physics_common_components_hpp
